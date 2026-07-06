/*
 * ====== Image Builder Configuration ======
 *
 * Available macros:
 *   > Required value
 *   _RVAR(storetype, argtype, name)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& Image::getName()
 *   
 *   > Optional value with default
 *   _OVAR(storetype, argtype, name, val)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& Image::getName()
 *
 *   > Required array (std::vector<type>)
 *   _RARR(type, name)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> Image::getName()
 *     -> Image::getName(idx)
 *     -> Image::getNameSize()
 *     -> Image::getNameData()
 *
 *   > Optional array with default values
 *   _OARR(type, name, ...)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> Image::getName()
 *     -> Image::getName(idx)
 *     -> Image::getNameSize()
 *     -> Image::getNameData()
 *
 *   > Boolean flag (default: false)
 *   _FLG(name)
 *     -> setName(...)
 *     -> Image::isName(...)
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
#endif

#ifdef _RVAR
/* ====== Required variables ====== */
_RVAR(sn::voxeng::vk::Device*, sn::voxeng::vk::Device*, Device)

_RVAR(VkImageType, VkImageType, ImageType)
_RVAR(VkFormat, VkFormat, Format)
_RVAR(VkExtent3D, VkExtent3D, Extent)
_RVAR(uint32_t, uint32_t, MipLevels)
_RVAR(uint32_t, uint32_t, ArrayLevels)
_RVAR(VkSampleCountFlagBits, VkSampleCountFlagBits, Samples)
_RVAR(VkImageTiling, VkImageTiling, Tiling)
_RVAR(VkImageUsageFlags, VkImageUsageFlags, Usage)
_RVAR(VkSharingMode, VkSharingMode, SharingMode)
_RVAR(VkImageLayout, VkImageLayout, InitialLayout)
#endif

#ifdef _OVAR
/* ====== Optional variables ====== */
_OVAR(VkImage, VkImage, ViewHandle, VK_NULL_HANDLE)
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
