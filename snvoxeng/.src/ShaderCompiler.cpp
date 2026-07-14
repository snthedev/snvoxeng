#include <snvoxeng/snvoxeng/ShaderCompiler.hpp>
#include <snvoxeng/snvoxeng/ShaderStages.hpp>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>
#include <snassert/snassert.hpp>

#include <filesystem>
#include <mutex>
#include <fstream>

using namespace sn::voxeng;

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

static ShaderCompiler::shader_t readSpvFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    snassert(file.is_open(), "Failed to open SPV file", path.string().c_str());

    size_t fileSize = static_cast<size_t>(file.tellg());
    snassert(fileSize % 4u == 0, "SPV file size is not a multiple of 4", path.string().c_str());

    file.seekg(0);

    uint32_t* buffer = new uint32_t[fileSize / 4u];
    file.read(reinterpret_cast<char*>(buffer), fileSize);
    file.close();

    return {
        .pCode = buffer,
        .sizeInBytes = fileSize
    };
}

// ShaderCompiler::shader_t

ShaderCompiler::shader_t::~shader_t() { if (pCode) delete[] pCode; }

// ShaderCompiler::_Impl

class ShaderCompiler::_Impl
{
public:
    std::mutex m_settings_mutex;
    settings_t m_settings{
        .apiVersion = VK_API_VERSION_1_0,
        .optLevel = ShaderCompiler::settings_t::eOptLevel::ePerformance
    };

    _Impl() = default;
    ~_Impl() noexcept = default;

    shader_t loadFromFile(const char* filepath, bool forceCompile)
    {
        std::filesystem::path sourcePath(filepath);
        if (sourcePath.extension() == ".spv")
            return readSpvFile(sourcePath);

        std::filesystem::path cachePath = sourcePath.string() + ".spv";

        bool cacheValid = false;
        if (std::filesystem::exists(sourcePath) && std::filesystem::exists(cachePath))
        {
            auto sourceTime = std::filesystem::last_write_time(sourcePath);
            auto cacheTime = std::filesystem::last_write_time(cachePath);
            if (cacheTime >= sourceTime)
                cacheValid = true;
        }

        if (cacheValid && !forceCompile)
            return readSpvFile(cachePath);

        snassert(std::filesystem::exists(sourcePath), "Shader source file does not exist", filepath);

        auto shader_stage_info = sn::voxeng::ShaderStages::fromFilename(filepath);

        std::string sourceCode;
        {
            std::ifstream file(sourcePath, std::ios::in | std::ios::binary);
            snassert(file.is_open(), "Failed to open shader source file", filepath);

            sourceCode = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
        }

        shaderc::CompileOptions compilerOptions;
        {
            std::lock_guard<std::mutex> lock(m_settings_mutex);
            compilerOptions.SetOptimizationLevel(getShadercOptLevel(m_settings.optLevel));
            compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, m_settings.apiVersion);
        }

        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(
            sourceCode,
            shader_stage_info.shadercType,
            sourcePath.filename().string().c_str(),
            compilerOptions
        );

        snassert(result.GetCompilationStatus() == shaderc_compilation_status_success,
            "Shader compilation failed!", result.GetErrorMessage().c_str());

        size_t sizeInBytes = std::distance(result.cbegin(), result.cend()) * sizeof(uint32_t);
        uint32_t* buffer = new uint32_t[sizeInBytes / 4u];
        std::memcpy(buffer, result.cbegin(), sizeInBytes);

        std::ofstream cacheFile(cachePath, std::ios::out | std::ios::binary);
        if (cacheFile.is_open())
        {
            cacheFile.write(reinterpret_cast<const char*>(buffer), sizeInBytes);
            cacheFile.close();
        }

        return {
            .pCode = buffer,
            .sizeInBytes = sizeInBytes
        };
    }
};

// ShaderCompiler

ShaderCompiler::_Impl& ShaderCompiler::get()
{
    static _Impl impl;
    return impl;
}

ShaderCompiler::settings_t ShaderCompiler::getSettings()
{
    std::lock_guard<std::mutex> lock(get().m_settings_mutex);
    return get().m_settings;
}
void ShaderCompiler::setSettings(const settings_t& settings)
{
    std::lock_guard<std::mutex> lock(get().m_settings_mutex);
    get().m_settings = settings;
}

ShaderCompiler::shader_t ShaderCompiler::loadFromFile(const char* filepath, bool forceCompile)
{
	return get().loadFromFile(filepath, forceCompile);
}
