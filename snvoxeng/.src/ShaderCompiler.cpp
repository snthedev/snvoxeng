#include <snvoxeng/snvoxeng/ShaderCompiler.hpp>
#include <snvoxeng/snvoxeng/ShaderStages.hpp>

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFHandle.hpp>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>
#include <snassert/snassert.hpp>

#include <filesystem>
#include <mutex>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <span>

using namespace sn::voxeng;

// === utils ===

// Cache Header
namespace
{
	constexpr uint32_t kCacheMagic = 0x53504331;   // 'SPC1'
	constexpr uint32_t kCacheVersion = 1;

#pragma pack(push, 1)
	// On-disk cache entry header. A cache file is valid only when ALL fields
	// match the current compilation context, which makes changing settings
	// (api version, optimization level) or editing the shader source
	// automatically invalidate previously cached binaries - no mtime games.
	struct CacheHeader
	{
		uint32_t magic;
		uint32_t version;
		uint32_t apiVersion;
		uint32_t optLevel;
		uint64_t sourceSize;
		uint64_t sourceHash; // FNV-1a 64 over the source bytes
		uint64_t spvSize;

        void read(io::BFViewRO map)
        {
            magic = map.read_front<decltype(magic)>();
            version = map.read_front<decltype(version)>();
            apiVersion = map.read_front<decltype(apiVersion)>();
            optLevel = map.read_front<decltype(optLevel)>();
            sourceSize = map.read_front<decltype(sourceSize)>();
            sourceHash = map.read_front<decltype(sourceHash)>();
            spvSize = map.read_front<decltype(spvSize)>();
        }

        void write(io::BFViewRW map) const
        {
            map.write_front(magic);
            map.write_front(version);
            map.write_front(apiVersion);
            map.write_front(optLevel);
            map.write_front(sourceSize);
            map.write_front(sourceHash);
            map.write_front(spvSize);
        }

        bool isSourceSame(const CacheHeader& other) const noexcept
        {
            return
                magic == other.magic
                && version == other.version
                && apiVersion == other.apiVersion
                && optLevel == other.optLevel
                && sourceSize == other.sourceSize
                && sourceHash == other.sourceHash
                ;
        }

        bool operator==(const CacheHeader& other) noexcept
        {
            return
                magic == other.magic
                && version == other.version
                && apiVersion == other.apiVersion
                && optLevel == other.optLevel
                && sourceSize == other.sourceSize
                && sourceHash == other.sourceHash
                && spvSize == other.spvSize
                ;
        }
        bool operator!=(const CacheHeader& other) noexcept { return !(*this == other); }
	};
#pragma pack(pop)
	static_assert(sizeof(CacheHeader) == 40u);
    static_assert(sizeof(CacheHeader) % alignof(uint32_t) == 0,
        "CacheHeader size must be a multiple of uint32_t alignment");

	constexpr uint64_t kFnv1aOffsetBasis = 0xcbf29ce484222325ull;
	constexpr uint64_t kFnv1aPrime = 0x100000001b3ull;

	uint64_t fnv1a64(const char* data, size_t size)
	{
		uint64_t hash = kFnv1aOffsetBasis;
		for (size_t i = 0; i < size; ++i)
		{
			hash ^= static_cast<uint8_t>(data[i]);
			hash *= kFnv1aPrime;
		}
		return hash;
	}
}

// Shaderc
static shaderc_optimization_level getShadercOptLevel(ShaderCompiler::settings_t::eOptLevel optLevel)
{
    constexpr static shaderc_optimization_level optLevelTable[]{
        shaderc_optimization_level::shaderc_optimization_level_zero,
        shaderc_optimization_level::shaderc_optimization_level_size,
        shaderc_optimization_level::shaderc_optimization_level_performance,
    };
    static_assert(std::size(optLevelTable) == static_cast<size_t>(ShaderCompiler::settings_t::eOptLevel::MAX_ENUM),
        "optLevelTable size mismatch");

    snassert(optLevel < ShaderCompiler::settings_t::eOptLevel::MAX_ENUM,
        "Invalid optimization level", "Provide valid optimization level");
    return optLevelTable[static_cast<size_t>(optLevel)];
}

// Entire File Read
static std::string readTextFile(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        throw std::runtime_error("File does not exist.");

    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file.");

    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
static ShaderCompiler::shader_t readSpvFile(const std::filesystem::path& path)
{
    io::BFHandle file(path, io::BFHandle::eNotFoundBehaviour::eThrow);
    auto map = file.map_ro();
    auto view = map.viewRO();

    if (view.size() % sizeof(uint32_t) != 0)
        throw std::runtime_error("Corrupted .spv file.");

    return ShaderCompiler::shader_t{ reinterpret_cast<const uint32_t*>(view.data()), view.size() / sizeof(uint32_t) };
}

// Cache Read
static const uint32_t* tryReadCache(
    const io::BFMapRO& map,
    const CacheHeader& expectedHeader,
    size_t& outWordsCount)
{
    auto view = map.viewRO();
    if (view.size() < sizeof(CacheHeader)) return nullptr;

    CacheHeader actualHeader;
    actualHeader.read(view.subview(0, sizeof(CacheHeader)));

    if (!actualHeader.isSourceSame(expectedHeader)) return nullptr;
    if (actualHeader.spvSize == 0 || (actualHeader.spvSize % sizeof(uint32_t)) != 0) return nullptr;

    const size_t payloadBytes = actualHeader.spvSize;
    if (view.size() < sizeof(CacheHeader) + payloadBytes) return nullptr;

    outWordsCount = payloadBytes / sizeof(uint32_t);
    return reinterpret_cast<const uint32_t*>(view.data() + sizeof(CacheHeader));
}

// Cache Write
static void writeCache(
    io::BFHandle& file,
    const CacheHeader& header,
    const uint32_t* code,
    size_t sizeInBytes
)
{
    file.resize(sizeof(CacheHeader) + sizeInBytes);
    auto map = file.map_rw();

    auto view = map.viewRW();
    {
        auto header_view = view.subview(0, sizeof(CacheHeader));
        header.write(header_view);
    }
    {
        auto data_view = view.subview(sizeof(CacheHeader), sizeInBytes);
        std::memcpy(data_view.data(), code, sizeInBytes);
    }

    map.sync();
}

// === ShaderCompiler::shader_t ===

ShaderCompiler::shader_t::shader_t(const uint32_t* pCode, size_t size)
    : m_pCode(new uint32_t[size])
    , m_sizeInBytes(size * sizeof(uint32_t))
{
    std::memcpy(m_pCode, pCode, m_sizeInBytes);
}
ShaderCompiler::shader_t::~shader_t() { if (m_pCode) delete[] m_pCode; }

ShaderCompiler::shader_t::shader_t(shader_t&& other) noexcept
    : m_pCode(std::exchange(other.m_pCode, nullptr))
    , m_sizeInBytes(std::exchange(other.m_sizeInBytes, 0u))
{
}
ShaderCompiler::shader_t& ShaderCompiler::shader_t::operator=(shader_t&& other) noexcept
{
    if (this != &other) [[likely]]
    {
        if (m_pCode) delete[] m_pCode;

        m_pCode = std::exchange(other.m_pCode, nullptr);
        m_sizeInBytes = std::exchange(other.m_sizeInBytes, 0u);
    }
    return *this;
}

const uint32_t* ShaderCompiler::shader_t::getCode() const noexcept { return m_pCode; }
size_t ShaderCompiler::shader_t::getSize() const noexcept { return m_sizeInBytes; }

// === ShaderCompiler::data_t ===

struct ShaderCompiler::data_t
{
    mutable std::mutex m_settings_mutex;
    settings_t m_settings;

    explicit data_t(const settings_t& settings)
        : m_settings(settings)
    {
    }

    static settings_t defaultSettings()
    {
        return settings_t{
            .apiVersion = VK_API_VERSION_1_0,
            .optLevel = settings_t::eOptLevel::ePerformance
        };
    }

    shader_t loadFromFile(const std::filesystem::path& filepath, bool forceCompile) const
    {
        if (filepath.extension() == ".spv")
            return readSpvFile(filepath);

        // Snapshot the settings so compilation never races with setSettings().
        settings_t settings;
        {
            std::lock_guard<std::mutex> lock(m_settings_mutex);
            settings = m_settings;
        }

        std::string sourceCode = readTextFile(filepath);

        CacheHeader header{
            .magic = kCacheMagic,
            .version = kCacheVersion,
            .apiVersion = settings.apiVersion,
            .optLevel = static_cast<uint32_t>(settings.optLevel),
            .sourceSize = static_cast<uint64_t>(sourceCode.size()),
            .sourceHash = fnv1a64(sourceCode.data(), sourceCode.size()),
        };
        
        // Cache read

        const std::filesystem::path cachePath = std::filesystem::path(filepath) += ".spvs";
        if (!forceCompile)
        {
            try
            {
                io::BFHandle cache_ifile(cachePath, io::BFHandle::eNotFoundBehaviour::eThrow);

                auto map = cache_ifile.map_ro();
                const uint32_t* pData /* mapped RO file-data */ = tryReadCache(map, header, header.spvSize);
                if (header.spvSize != 0) return shader_t{ pData /* copy entire data */, header.spvSize};
            }
            catch (...) { /* cache read error is not a critical failure */ }
        }

        // Compiling

        sn::voxeng::ShaderStageInfo shader_stage_info(filepath);

        shaderc::CompileOptions compilerOptions;
        compilerOptions.SetOptimizationLevel(getShadercOptLevel(settings.optLevel));
        compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, settings.apiVersion);

        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(
            sourceCode,
            shader_stage_info.getShadercType(),
            filepath.filename().string().c_str(),
            compilerOptions
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            snassert(false, "Failed to compile shader " + filepath.string() + ".", result.GetErrorMessage().c_str());
            throw std::runtime_error("Failed to compile shader.");
        }

        const size_t size = std::distance(result.cbegin(), result.cend());
        const size_t sizeInBytes = size * sizeof(uint32_t);
        if (size == 0)
        {
            snassert(false, "Shader compilation produced no SPIR-V words.", filepath.string());
            throw std::runtime_error("Shader compilation produced no SPIR-V words.");
        }
        
        // Cache write

        header.spvSize = static_cast<uint64_t>(sizeInBytes);
        try
        {
            io::BFHandle cache_ofile(cachePath, io::BFHandle::eNotFoundBehaviour::eCreatePathAndFile);
            writeCache(cache_ofile, header, result.cbegin(), sizeInBytes);
        }
        catch (...) { /* cache write error is not a critical failure */ }

        return shader_t{ result.cbegin(), size };
    }
};

// === ShaderCompiler ===

ShaderCompiler::ShaderCompiler()
    : m_pData(new data_t{ data_t::defaultSettings() }) {}
ShaderCompiler::ShaderCompiler(const settings_t& settings)
    : m_pData(new data_t{ settings }) {}
ShaderCompiler::~ShaderCompiler() noexcept { delete m_pData; }

ShaderCompiler::settings_t ShaderCompiler::getSettings() const
{
    std::lock_guard<std::mutex> lock(m_pData->m_settings_mutex);
    return m_pData->m_settings;
}
void ShaderCompiler::setSettings(const settings_t& settings)
{
    std::lock_guard<std::mutex> lock(m_pData->m_settings_mutex);
    m_pData->m_settings = settings;
}

ShaderCompiler::shader_t ShaderCompiler::loadFromFile(const std::filesystem::path& filepath, bool forceCompile) const
{
    return m_pData->loadFromFile(filepath, forceCompile);
}
