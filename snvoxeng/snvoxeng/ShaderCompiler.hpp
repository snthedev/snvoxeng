#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <cstdint>

namespace sn::voxeng
{
    class SNVOXENG_API ShaderCompiler final
    {
        class _Impl;
        static _Impl& get();

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

        ShaderCompiler() = delete;

        static settings_t getSettings();
        static void setSettings(const settings_t& settings);

        static shader_t loadFromFile(const char* filepath, bool forceCompile = false);
    };
}
