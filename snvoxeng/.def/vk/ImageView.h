/*
 * ====== ImageView Builder Configuration ======
 *
 * Available macros:
 *   > Required value
 *   _RVAR(storetype, argtype, name)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& ImageView::getName()
 *   
 *   > Optional value with default
 *   _OVAR(storetype, argtype, name, val)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& ImageView::getName()
 *
 *   > Required array (std::vector<type>)
 *   _RARR(type, name)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> ImageView::getName()
 *     -> ImageView::getName(idx)
 *     -> ImageView::getNameSize()
 *     -> ImageView::getNameData()
 *
 *   > Optional array with default values
 *   _OARR(type, name, ...)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> ImageView::getName()
 *     -> ImageView::getName(idx)
 *     -> ImageView::getNameSize()
 *     -> ImageView::getNameData()
 *
 *   > Boolean flag (default: false)
 *   _FLG(name)
 *     -> setName(...)
 *     -> ImageView::isName(...)
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

/*
typedef struct VkImageViewCreateInfo {
    VkStructureType            sType;
    const void*                pNext;
    VkImageViewCreateFlags     flags;
    VkImage                    image;
    VkImageViewType            viewType;
    VkFormat                   format;
    VkComponentMapping         components;
    VkImageSubresourceRange    subresourceRange;
} VkImageViewCreateInfo;
*/

#ifdef _INCLUDE
/* ======      Includes      ====== */
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>
#include <snvoxeng/snvoxeng/vk/Image.hpp>
#include <snvoxeng/snvoxeng/vk/SwapchainKHR.hpp>
#endif

#ifdef _RVAR
/* ====== Required variables ====== */
_RVAR(sn::voxeng::vk::Image*, sn::voxeng::vk::Image*, Image)
_RVAR(VkImageViewType, VkImageViewType, ViewType)
// _RVAR(VkFormat, VkFormat, Format)
_RVAR(VkImageSubresourceRange, const VkImageSubresourceRange&, SubresourceRange)
#endif

#ifdef _OVAR
/* ====== Optional variables ====== */
#define DEFAULT_VALUE_Component VkComponentMapping{ .r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY, }

_OVAR(VkComponentMapping, const VkComponentMapping&, Components, DEFAULT_VALUE_Component)

#undef DEFAULT_VALUE_Component
#endif

#ifdef _RARR
/* ======   Required arrays  ====== */
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
