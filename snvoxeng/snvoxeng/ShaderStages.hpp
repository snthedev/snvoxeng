#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include <string_view>
#include <stdexcept>
#include <filesystem>

namespace sn::voxeng
{
    enum class eShaderStageFlag : uint8_t
    {
        eNone,
        eKHR,
        eEXT,
        MAX_ENUM
    };
    enum class eShaderStage : uint8_t
    {
#define _X(_0, Name, _2, _3, _4) e##Name,
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
        MAX_ENUM
    };

    class SNVOXENG_API ShaderStageInfosContainer
    {
    public:
        ShaderStageInfosContainer() = delete;

        static eShaderStageFlag getShaderStageFlag(eShaderStage etype) noexcept;
        static VkShaderStageFlagBits getShaderStageVkType(eShaderStage etype) noexcept;
        static shaderc_shader_kind getShaderStageShadercType(eShaderStage etype) noexcept;
        static std::string_view getShaderStage_name(eShaderStage etype) noexcept;
        static std::string_view getShaderStageName(eShaderStage etype) noexcept;
        static std::string_view getShaderStageExt(eShaderStage etype) noexcept;

        static eShaderStage getShaderStageByVkType(VkShaderStageFlagBits vktype);
        static eShaderStage getShaderStageByShadercType(shaderc_shader_kind shaderctype);
        static eShaderStage getShaderStageByFileExtension(std::string_view fileext);
        static eShaderStage getShaderStageByFileExtension(std::wstring_view fileext);
        static eShaderStage getShaderStageByFilePath(const std::filesystem::path& filepath);
    };

    class SNVOXENG_API ShaderStageInfo
    {
        eShaderStage m_etype;

    public:
        explicit ShaderStageInfo(eShaderStage etype);
        explicit ShaderStageInfo(VkShaderStageFlagBits vktype);
        explicit ShaderStageInfo(shaderc_shader_kind shaderctype);
        explicit ShaderStageInfo(const std::filesystem::path& path);

        eShaderStage get() const noexcept;

        eShaderStageFlag getFlag() const noexcept;
        VkShaderStageFlagBits getVkType() const noexcept;
        shaderc_shader_kind getShadercType() const noexcept;
        std::string_view get_name() const noexcept;
        std::string_view getName() const noexcept;
        std::string_view getExt() const noexcept;
    };
}