#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <filesystem>
#include <mutex>

namespace sn::voxeng
{
    class SNVOXENG_API ShaderCompiler final
    {
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

    private:
        static std::mutex s_settings_mutex;
        static settings_t s_settings;

        class _Impl final
        {
        public:
            _Impl();
            ~_Impl() noexcept;

            shader_t loadFromFile(const char* filepath, bool forceCompile);
        };

        static _Impl& get();

    public:
        ShaderCompiler() = delete;

        static settings_t getSettings();
        static void setSettings(const settings_t& settings);

        static shader_t loadFromFile(const char* filepath, bool forceCompile = false);
    };
}
