#include <snvoxeng/snvoxeng/ShaderCompiler.hpp>
#include <snvoxeng/snvoxeng/ShaderStages.hpp>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>
#include <snassert/snassert.hpp>

#include <filesystem>
#include <mutex>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

using namespace sn::voxeng;

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
	};
#pragma pack(pop)
	static_assert(sizeof(CacheHeader) == 40u);

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

static bool tryReadTextFile(const std::filesystem::path& path, std::string& out)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    out.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return true;
}

static Result<ShaderCompiler::shader_t> readSpvFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        return Error{ uint32_t(CompileError::sourceReadFailed),
            "Failed to open SPV file: " + path.string() };

    const auto fileSize = static_cast<size_t>(file.tellg());
    if (fileSize < 4u || fileSize % 4u != 0)
        return Error{ uint32_t(CompileError::sourceReadFailed),
            "SPV file size is invalid: " + path.string() };

    file.seekg(0);

    uint32_t* buffer = new uint32_t[fileSize / 4u];
    file.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(fileSize));
    if (static_cast<size_t>(file.gcount()) != fileSize)
        return Error{ uint32_t(CompileError::sourceReadFailed),
            "Failed to read the whole SPV file: " + path.string() };

    return ShaderCompiler::shader_t{ buffer, fileSize };
}

// Returns false when the cache is missing, corrupt, or stale (any identity
// field mismatch against the expected compilation context).
static bool tryReadCache(
    const std::filesystem::path& path,
    const CacheHeader& expected,
    std::vector<uint32_t>& out)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return false;

    CacheHeader header{};
    if (!file.read(reinterpret_cast<char*>(&header), sizeof(header)))
        return false;

    // Compare identity fields only: spvSize describes the payload, it is
    // validated below rather than matched against 'expected'.
    if (header.magic != expected.magic ||
        header.version != expected.version ||
        header.apiVersion != expected.apiVersion ||
        header.optLevel != expected.optLevel ||
        header.sourceSize != expected.sourceSize ||
        header.sourceHash != expected.sourceHash)
        return false;

    if (header.spvSize == 0 || header.spvSize % 4u != 0)
        return false;

    out.resize(static_cast<size_t>(header.spvSize / 4u));
    if (!file.read(
        reinterpret_cast<char*>(out.data()),
        static_cast<std::streamsize>(header.spvSize)))
        return false;

    return true;
}

// Best-effort by design: a missing or unwritable cache never breaks the
// actual compilation result.
static void writeCache(
    const std::filesystem::path& path,
    const CacheHeader& header,
    const uint32_t* code,
    size_t sizeInBytes)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        return;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(code), static_cast<std::streamsize>(sizeInBytes));
}

// ShaderCompiler::shader_t

ShaderCompiler::shader_t::shader_t(const uint32_t* pCode, size_t sizeInBytes)
    : m_pCode(pCode)
    , m_sizeInBytes(sizeInBytes)
{
}
ShaderCompiler::shader_t::~shader_t() { if (m_pCode) delete[] m_pCode; }

ShaderCompiler::shader_t::shader_t(shader_t&& other) noexcept
    : m_pCode(other.m_pCode)
    , m_sizeInBytes(other.m_sizeInBytes)
{
    other.m_pCode = nullptr;
}
ShaderCompiler::shader_t& ShaderCompiler::shader_t::operator=(shader_t&& other) noexcept
{
    if (this != &other) [[likely]]
    {
        m_pCode = other.m_pCode;
        m_sizeInBytes = other.m_sizeInBytes;
        other.m_pCode = nullptr;
    }
    return *this;
}

const uint32_t* ShaderCompiler::shader_t::getCode() const noexcept { return m_pCode; }
size_t ShaderCompiler::shader_t::getSize() const noexcept { return m_sizeInBytes; }

// ShaderCompiler::data_t

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

    Result<shader_t> loadFromFile(const char* filepath, bool forceCompile) const
    {
        const std::filesystem::path sourcePath(filepath);
        if (sourcePath.extension() == ".spv")
            return readSpvFile(sourcePath);

        // Snapshot the settings so compilation never races with setSettings().
        settings_t settings;
        {
            std::lock_guard<std::mutex> lock(m_settings_mutex);
            settings = m_settings;
        }

        if (!std::filesystem::exists(sourcePath))
            return Error{ uint32_t(CompileError::sourceNotFound),
                "Shader source file does not exist: " + sourcePath.string() };

        std::string sourceCode;
        if (!tryReadTextFile(sourcePath, sourceCode))
            return Error{ uint32_t(CompileError::sourceReadFailed),
                "Failed to read shader source file: " + sourcePath.string() };

        CacheHeader expectedHeader{};
        expectedHeader.magic = kCacheMagic;
        expectedHeader.version = kCacheVersion;
        expectedHeader.apiVersion = settings.apiVersion;
        expectedHeader.optLevel = static_cast<uint32_t>(settings.optLevel);
        expectedHeader.sourceSize = static_cast<uint64_t>(sourceCode.size());
        expectedHeader.sourceHash = fnv1a64(sourceCode.data(), sourceCode.size());

        const std::filesystem::path cachePath = sourcePath.string() + ".spv";

        std::vector<uint32_t> cachedSpirv;
        if (!forceCompile && tryReadCache(cachePath, expectedHeader, cachedSpirv))
        {
            uint32_t* buffer = new uint32_t[cachedSpirv.size()];
            std::memcpy(buffer, cachedSpirv.data(), cachedSpirv.size() * sizeof(uint32_t));
            return shader_t{ buffer, cachedSpirv.size() * sizeof(uint32_t) };
        }

        auto shader_stage_info = sn::voxeng::ShaderStages::fromFilename(filepath);

        shaderc::CompileOptions compilerOptions;
        compilerOptions.SetOptimizationLevel(getShadercOptLevel(settings.optLevel));
        compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, settings.apiVersion);

        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(
            sourceCode,
            shader_stage_info.shadercType,
            sourcePath.filename().string().c_str(),
            compilerOptions
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
            return Error{ uint32_t(CompileError::compilationFailed),
                result.GetErrorMessage() };

        const size_t sizeInBytes = std::distance(result.cbegin(), result.cend()) * sizeof(uint32_t);
        if (sizeInBytes == 0)
            return Error{ uint32_t(CompileError::compilationFailed),
                "Shader compilation produced no SPIR-V words: " + sourcePath.string() };

        uint32_t* buffer = new uint32_t[sizeInBytes / 4u];
        std::memcpy(buffer, result.cbegin(), sizeInBytes);

        expectedHeader.spvSize = static_cast<uint64_t>(sizeInBytes);
        writeCache(cachePath, expectedHeader, buffer, sizeInBytes);

        return shader_t{ buffer, sizeInBytes };
    }
};

// ShaderCompiler

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

Result<ShaderCompiler::shader_t> ShaderCompiler::loadFromFile(const char* filepath, bool forceCompile) const
{
    return m_pData->loadFromFile(filepath, forceCompile);
}
