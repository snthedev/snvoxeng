/*
 * ====== SwapchainKHR Builder Configuration ======
 *
 * Available macros:
 *   > Required value
 *   _RVAR(storetype, argtype, name)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& SwapchainKHR::getName()
 *   
 *   > Optional value with default
 *   _OVAR(storetype, argtype, name, val)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& SwapchainKHR::getName()
 *
 *   > Required array (std::vector<type>)
 *   _RARR(type, name)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> SwapchainKHR::getName()
 *     -> SwapchainKHR::getName(idx)
 *     -> SwapchainKHR::getNameSize()
 *     -> SwapchainKHR::getNameData()
 *
 *   > Optional array with default values
 *   _OARR(type, name, ...)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> SwapchainKHR::getName()
 *     -> SwapchainKHR::getName(idx)
 *     -> SwapchainKHR::getNameSize()
 *     -> SwapchainKHR::getNameData()
 *
 *   > Boolean flag (default: false)
 *   _FLG(name)
 *     -> setName(...)
 *     -> SwapchainKHR::isName(...)
 *
 * Macros must be in a block corresponding to the macro name:
 *   #ifdef _RVAR
 *   _RVAR(...)
 *   #endif
 * 
 * Inclusions must be in the _INCLUDE block:
 *   #ifdef _INCLUDE
 *   #include ...
 *   #endif
 */

#ifdef _INCLUDE
/* ======      Includes      ====== */
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>
#include <snvoxeng/snvoxeng/vk/Device.hpp>
#include <snvoxeng/snvoxeng/vk/SurfaceKHR.hpp>
#endif

#ifdef _RVAR
/* ====== Required variables ====== */
_RVAR(sn::voxeng::vk::Device*, sn::voxeng::vk::Device*, Device)

_RVAR(sn::voxeng::vk::SurfaceKHR*, sn::voxeng::vk::SurfaceKHR*, SurfaceKHR)
//_RVAR(VkExtent2D, VkExtent2D, ImageExtent)
// _RVAR(uint32_t, uint32_t, GraphicsFamilyIndex)
// _RVAR(uint32_t, uint32_t, PresentFamilyIndex)
#endif

#ifdef _OVAR
/* ====== Optional variables ====== */
//_OVAR(uint32_t, uint32_t, MinImageCount, 1u)
_OVAR(VkFormat, VkFormat, ImageFormat, VK_FORMAT_B8G8R8A8_SRGB)
_OVAR(VkColorSpaceKHR, VkColorSpaceKHR, ImageColorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
_OVAR(uint32_t, uint32_t, ImageArrayLayers, 1u)
_OVAR(VkImageUsageFlags, VkImageUsageFlags, ImageUsage, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
_OVAR(VkSharingMode, VkSharingMode, ImageSharingMode, VK_SHARING_MODE_EXCLUSIVE)
//_OVAR(VkSurfaceTransformFlagBitsKHR, VkSurfaceTransformFlagsKHR, PreTransform, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
_OVAR(VkCompositeAlphaFlagBitsKHR, VkCompositeAlphaFlagBitsKHR, CompositeAlpha, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
_OVAR(VkPresentModeKHR, VkPresentModeKHR, PresentMode, VK_PRESENT_MODE_FIFO_KHR)
_OVAR(VkSwapchainKHR, VkSwapchainKHR, OldSwapchain, VK_NULL_HANDLE)
#endif

#ifdef _RARR
/* ======   Required arrays  ====== */
_RARR(uint32_t, QueueFamilyIndices)
#endif

#ifdef _OARR
/* ======   Optional arrays  ====== */
#endif

#ifdef _FLG
/* ======        Flags       ====== */
_FLG(Clipped)
#endif



#ifdef _INCLUDE
#undef _INCLUDE
#endif
#ifdef _RVAR
#undef _RVAR
#endif
#ifdef _OVAR
#undef _OVAR
#endif
#ifdef _RARR
#undef _RARR
#endif
#ifdef _OARR
#undef _OARR
#endif
#ifdef _FLG
#undef _FLG
#endif
