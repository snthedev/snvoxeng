#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/Result.hpp>

#include <cstdint>

namespace sn::voxeng
{
    // Recoverable shader loading/compilation failures (see the project
    // error-handling policy: these belong to Result, not to snassert).
    enum class CompileError : uint32_t
    {
        sourceNotFound = 1,
        sourceReadFailed = 2,
        compilationFailed = 3,
    };

    // Runtime GLSL -> SPIR-V compiler with on-disk caching.
    //
    // Instantiable by design (no hidden global state): every instance owns
    // its settings, and cache entries are keyed by [settings + source content],
    // so changing optimization level or target API version automatically
    // invalidates previously cached binaries.
    class SNVOXENG_API ShaderCompiler final
    {
        struct data_t;
        data_t* m_pData;

    public:
        struct settings_t
        {
            uint32_t apiVersion;
            enum class eOptLevel
            {
                eNone = 0,
                eSize,
                ePerformance,
                MAX_ENUM
            } optLevel;
        };

        class SNVOXENG_API shader_t
        {
            const uint32_t* m_pCode;
            size_t m_sizeInBytes;

        public:
            shader_t(const uint32_t* pCode, size_t sizeInBytes);
            ~shader_t() noexcept;

            shader_t(const shader_t&) = delete;
            shader_t& operator=(const shader_t&) = delete;
            shader_t(shader_t&& other) noexcept;
            shader_t& operator=(shader_t&& other) noexcept;

            const uint32_t* getCode() const noexcept;

            // in bytes
            size_t getSize() const noexcept;
        };

        // Default settings: VK_API_VERSION_1_0, eOptLevel::ePerformance.
        ShaderCompiler();
        explicit ShaderCompiler(const settings_t& settings);
        ~ShaderCompiler() noexcept;

        ShaderCompiler(const ShaderCompiler&) = delete;
        ShaderCompiler& operator=(const ShaderCompiler&) = delete;

        settings_t getSettings() const;
        void setSettings(const settings_t& settings);

        // Compiles GLSL into SPIR-V (or serves a matching cache entry).
        // A file with the '.spv' extension is treated as a precompiled
        // binary and loaded as-is. Cache misses are written back next to
        // the source ('<name>.spv'); cache write failures are silently
        // ignored by design (the compilation result is returned anyway).
        Result<shader_t> loadFromFile(const char* filepath, bool forceCompile = false) const;
    };
}
