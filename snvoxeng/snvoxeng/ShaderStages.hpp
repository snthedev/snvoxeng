#pragma once

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include <string_view>
#include <stdexcept>

namespace sn::voxeng
{
    struct ShaderStages;
    struct ShaderStageInfo
    {
        friend struct ShaderStages;

        VkShaderStageFlagBits vulkanType{};
        shaderc_shader_kind shadercType{};
        const char* extension{ nullptr };
        const char* name{ nullptr };

    private:
        constexpr ShaderStageInfo() = default;
        constexpr ShaderStageInfo(VkShaderStageFlagBits vulkanType, shaderc_shader_kind shadercType,
            const char* extension, const char* name)
            : vulkanType(vulkanType)
            , shadercType(shadercType)
            , extension(extension)
            , name(name)
        {
        }
    };

    struct ShaderStages
    {
    private:
        static inline constexpr ShaderStageInfo s_all[] = {
            { VK_SHADER_STAGE_VERTEX_BIT,shaderc_vertex_shader, "vert", "vertex" },
            { VK_SHADER_STAGE_GEOMETRY_BIT,shaderc_geometry_shader, "geom", "geometry" },
            { VK_SHADER_STAGE_FRAGMENT_BIT,shaderc_fragment_shader, "frag", "fragment" },
            { VK_SHADER_STAGE_COMPUTE_BIT,shaderc_compute_shader, "comp", "compute" },
            { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,shaderc_tess_control_shader, "tesc", "tess_control" },
            { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,shaderc_tess_evaluation_shader, "tese", "tess_evaluation" },
            { VK_SHADER_STAGE_RAYGEN_BIT_KHR,shaderc_raygen_shader, "rgen", "raygen" },
            { VK_SHADER_STAGE_ANY_HIT_BIT_KHR,shaderc_anyhit_shader, "rahit", "anyhit" },
            { VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,shaderc_closesthit_shader, "rchit", "closesthit" },
            { VK_SHADER_STAGE_MISS_BIT_KHR,shaderc_miss_shader, "rmiss", "miss" },
            { VK_SHADER_STAGE_INTERSECTION_BIT_KHR,shaderc_intersection_shader, "rint", "intersection" },
            { VK_SHADER_STAGE_CALLABLE_BIT_KHR,shaderc_callable_shader, "rcall", "callable" },
            { VK_SHADER_STAGE_TASK_BIT_EXT,shaderc_task_shader, "task", "task" },
            { VK_SHADER_STAGE_MESH_BIT_EXT,shaderc_mesh_shader, "mesh", "mesh" },
        };

    public:
        ShaderStages() = delete;

        static inline constexpr ShaderStageInfo vertex = s_all[0];
        static inline constexpr ShaderStageInfo geometry = s_all[1];
        static inline constexpr ShaderStageInfo fragment = s_all[2];
        static inline constexpr ShaderStageInfo compute = s_all[3];
        static inline constexpr ShaderStageInfo tessControl = s_all[4];
        static inline constexpr ShaderStageInfo tessEvaluation = s_all[5];
        static inline constexpr ShaderStageInfo raygen = s_all[6];
        static inline constexpr ShaderStageInfo anyhit = s_all[7];
        static inline constexpr ShaderStageInfo closesthit = s_all[8];
        static inline constexpr ShaderStageInfo miss = s_all[9];
        static inline constexpr ShaderStageInfo intersection = s_all[10];
        static inline constexpr ShaderStageInfo callable = s_all[11];
        static inline constexpr ShaderStageInfo task = s_all[12];
        static inline constexpr ShaderStageInfo mesh = s_all[13];

        static constexpr bool isSupported(VkShaderStageFlags vulkanType)
        {
            for (auto& stage : s_all)
                if (stage.vulkanType == vulkanType)
                    return true;
            return false;
        }
        static constexpr bool isSupported(shaderc_shader_kind shadercType)
        {
            for (auto& stage : s_all)
                if (stage.shadercType == shadercType)
                    return true;
            return false;
        }

        static constexpr const ShaderStageInfo fromExtension(std::string_view extension)
        {
            if (extension.size() > 0)
            {
                size_t dot_pos = extension.find_last_of('.');
                if (dot_pos != std::string_view::npos)
                    extension = extension.substr(dot_pos + 1u);

                for (auto stage : s_all)
                    if (stage.extension == extension)
                        return stage;
            }
            throw std::invalid_argument("Unsupported extension");
        }
        static constexpr const ShaderStageInfo fromFilename(std::string_view path) { return fromExtension(path); }
        static constexpr const ShaderStageInfo fromVulkanType(VkShaderStageFlagBits vulkanType)
        {
            for (auto& stage : s_all)
                if (stage.vulkanType == vulkanType)
                    return stage;
            throw std::invalid_argument("Unsupported vulkan type");
        }
        static constexpr const ShaderStageInfo fromShadercType(shaderc_shader_kind shadercType)
        {
            for (auto& stage : s_all)
                if (stage.shadercType == shadercType)
                    return stage;
            throw std::invalid_argument("Unsupported shaderc type");
        }
    };
}