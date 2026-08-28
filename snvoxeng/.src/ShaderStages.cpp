#include <snvoxeng/snvoxeng/ShaderStages.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng;

#define CHECK_SHADER_STAGE_RANGE(ARGNAME)                                           \
    snassert(                                                                       \
        static_cast<size_t>(ARGNAME) < static_cast<size_t>(eShaderStage::MAX_ENUM), \
        "Out of eShaderStage::MAX_ENUM",                                            \
        "Provide a valid eShaderStage"                                              \
    );


// === ShaderStageInfosContainer ===


eShaderStageFlag ShaderStageInfosContainer::getShaderStageFlag(eShaderStage etype) noexcept
{
    CHECK_SHADER_STAGE_RANGE(etype);
    constexpr static eShaderStageFlag s_table[static_cast<size_t>(eShaderStage::MAX_ENUM)]{
#define _X(_0, _1, _2, _3, _4) eShaderStageFlag::eNone,
#define _XKHR(_0, _1, _2, _3, _4) eShaderStageFlag::eKHR,
#define _XEXT(_0, _1, _2, _3, _4) eShaderStageFlag::eEXT,
#include <snvoxeng/.def/ShaderStages.h>
    };
    return s_table[static_cast<size_t>(etype)];
}
VkShaderStageFlagBits ShaderStageInfosContainer::getShaderStageVkType(eShaderStage etype) noexcept
{
    CHECK_SHADER_STAGE_RANGE(etype);
    constexpr static VkShaderStageFlagBits s_table[static_cast<size_t>(eShaderStage::MAX_ENUM)]{
#define _X(_0, _1, _2, VK_TYPE, _4) VkShaderStageFlagBits::VK_SHADER_STAGE_ ## VK_TYPE ## _BIT,
#define _XKHR(_0, _1, _2, VK_TYPE, _4) VkShaderStageFlagBits::VK_SHADER_STAGE_ ## VK_TYPE ## _BIT_KHR,
#define _XEXT(_0, _1, _2, VK_TYPE, _4) VkShaderStageFlagBits::VK_SHADER_STAGE_ ## VK_TYPE ## _BIT_EXT,
#include <snvoxeng/.def/ShaderStages.h>
    };
    return s_table[static_cast<size_t>(etype)];
}
shaderc_shader_kind ShaderStageInfosContainer::getShaderStageShadercType(eShaderStage etype) noexcept
{
    CHECK_SHADER_STAGE_RANGE(etype);
    constexpr static shaderc_shader_kind s_table[static_cast<size_t>(eShaderStage::MAX_ENUM)]{
#define _X(_0, _1, _2, _3, shaderc_type) shaderc_shader_kind::shaderc_ ## shaderc_type ## _shader,
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    };
    return s_table[static_cast<size_t>(etype)];
}
std::string_view ShaderStageInfosContainer::getShaderStage_name(eShaderStage etype) noexcept
{
    CHECK_SHADER_STAGE_RANGE(etype);
    constexpr static std::string_view s_table[static_cast<size_t>(eShaderStage::MAX_ENUM)]{
#define _X(name, _1, _2, _3, _4) #name,
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    };
    return s_table[static_cast<size_t>(etype)];
}
std::string_view ShaderStageInfosContainer::getShaderStageName(eShaderStage etype) noexcept
{
    CHECK_SHADER_STAGE_RANGE(etype);
    constexpr static std::string_view s_table[static_cast<size_t>(eShaderStage::MAX_ENUM)]{
#define _X(_0, Name, _2, _3, _4) #Name,
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    };
    return s_table[static_cast<size_t>(etype)];
}
std::string_view ShaderStageInfosContainer::getShaderStageExt(eShaderStage etype) noexcept
{
    CHECK_SHADER_STAGE_RANGE(etype);
    constexpr static std::string_view s_table[static_cast<size_t>(eShaderStage::MAX_ENUM)]{
#define _X(_0, _1, ext, _3, _4) #ext,
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    };
    return s_table[static_cast<size_t>(etype)];
}

eShaderStage ShaderStageInfosContainer::getShaderStageByVkType(VkShaderStageFlagBits vktype)
{
    switch (vktype)
    {
#define _X(_0, Name, _2, VK_TYPE, _4) case VkShaderStageFlagBits::VK_SHADER_STAGE_ ## VK_TYPE ## _BIT: return eShaderStage::e##Name;
#define _XKHR(_0, Name, _2, VK_TYPE, _4) case VkShaderStageFlagBits::VK_SHADER_STAGE_ ## VK_TYPE ## _BIT_KHR: return eShaderStage::e##Name;
#define _XEXT(_0, Name, _2, VK_TYPE, _4) case VkShaderStageFlagBits::VK_SHADER_STAGE_ ## VK_TYPE ## _BIT_EXT: return eShaderStage::e##Name;
#include <snvoxeng/.def/ShaderStages.h>
    default: throw std::invalid_argument("Unsupported Vulkan Shader Type.");
    }
}
eShaderStage ShaderStageInfosContainer::getShaderStageByShadercType(shaderc_shader_kind shaderctype)
{
    switch (shaderctype)
    {
#define _X(_0, Name, _2, _3, shaderc_type) case shaderc_shader_kind::shaderc_ ## shaderc_type ## _shader: return eShaderStage::e##Name;
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    default: throw std::invalid_argument("Unsupported Shaderc Shader Type.");
    }
}
eShaderStage ShaderStageInfosContainer::getShaderStageByFileExtension(std::string_view fileext)
{
    if (fileext.size() > 0)
    {
        size_t dot_pos = fileext.find_last_of('.');
        if (dot_pos != std::string_view::npos)
            fileext = fileext.substr(dot_pos + 1u);

#define _X(_0, Name, ext, _3, _4) if (fileext == #ext) return eShaderStage::e##Name;
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    }
    throw std::invalid_argument("Unsupported file extension.");
}
eShaderStage ShaderStageInfosContainer::getShaderStageByFileExtension(std::wstring_view fileext)
{
    if (fileext.size() > 0)
    {
        size_t dot_pos = fileext.find_last_of(L'.');
        if (dot_pos != std::wstring_view::npos)
            fileext = fileext.substr(dot_pos + 1u);

#define _X(_0, Name, ext, _3, _4) if (fileext == L""#ext) return eShaderStage::e##Name;
#define _XKHR _X
#define _XEXT _X
#include <snvoxeng/.def/ShaderStages.h>
    }
    throw std::invalid_argument("Unsupported file extension.");
}
eShaderStage ShaderStageInfosContainer::getShaderStageByFilePath(const std::filesystem::path& filepath)
{
    if (!filepath.has_extension()) throw std::invalid_argument("Path does not have extension.");
    return getShaderStageByFileExtension(filepath.extension().native());
}


// === ShaderStageInfo ===


ShaderStageInfo::ShaderStageInfo(eShaderStage etype)
    : m_etype(etype)
{
    CHECK_SHADER_STAGE_RANGE(m_etype)
}

ShaderStageInfo::ShaderStageInfo(VkShaderStageFlagBits vktype)
    : m_etype(ShaderStageInfosContainer::getShaderStageByVkType(vktype))
{
}
ShaderStageInfo::ShaderStageInfo(shaderc_shader_kind shaderctype)
    : m_etype(ShaderStageInfosContainer::getShaderStageByShadercType(shaderctype))
{
}

ShaderStageInfo::ShaderStageInfo(const std::filesystem::path& path)
    : m_etype(ShaderStageInfosContainer::getShaderStageByFilePath(path))
{
}

eShaderStage ShaderStageInfo::get() const noexcept { return m_etype; }

eShaderStageFlag ShaderStageInfo::getFlag() const noexcept { return ShaderStageInfosContainer::getShaderStageFlag(m_etype); }
VkShaderStageFlagBits ShaderStageInfo::getVkType() const noexcept { return ShaderStageInfosContainer::getShaderStageVkType(m_etype); }
shaderc_shader_kind ShaderStageInfo::getShadercType() const noexcept { return ShaderStageInfosContainer::getShaderStageShadercType(m_etype); }
std::string_view ShaderStageInfo::get_name() const noexcept { return ShaderStageInfosContainer::getShaderStage_name(m_etype); }
std::string_view ShaderStageInfo::getName() const noexcept { return ShaderStageInfosContainer::getShaderStageName(m_etype); }
std::string_view ShaderStageInfo::getExt() const noexcept { return ShaderStageInfosContainer::getShaderStageExt(m_etype); }

#undef CHECK_SHADER_STAGE_RANGE
