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

        struct SNVOXENG_API shader_t
        {
            const uint32_t* const pCode;
            const size_t sizeInBytes;

            ~shader_t();
        };

        ShaderCompiler() = delete;

        static settings_t getSettings();
        static void setSettings(const settings_t& settings);

        static shader_t loadFromFile(const char* filepath, bool forceCompile = false);
    };
}
