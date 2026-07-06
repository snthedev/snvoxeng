/*
 * ====== CommandBuffersContainer Builder Configuration ======
 *
 * Available macros:
 *   > Required value
 *   _RVAR(storetype, argtype, name)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& CommandBuffersContainer::getName()
 *   
 *   > Optional value with default
 *   _OVAR(storetype, argtype, name, val)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& CommandBuffersContainer::getName()
 *
 *   > Required array (std::vector<type>)
 *   _RARR(type, name)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> CommandBuffersContainer::getName()
 *     -> CommandBuffersContainer::getName(idx)
 *     -> CommandBuffersContainer::getNameSize()
 *     -> CommandBuffersContainer::getNameData()
 *
 *   > Optional array with default values
 *   _OARR(type, name, ...)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> CommandBuffersContainer::getName()
 *     -> CommandBuffersContainer::getName(idx)
 *     -> CommandBuffersContainer::getNameSize()
 *     -> CommandBuffersContainer::getNameData()
 *
 *   > Boolean flag (default: false)
 *   _FLG(name)
 *     -> setName(...)
 *     -> CommandBuffersContainer::isName(...)
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
#include <snvoxeng/snvoxeng/vk/CommandPool.hpp>
#endif

#ifdef _RVAR
/* ====== Required variables ====== */
_RVAR(const sn::voxeng::vk::CommandPool*, const sn::voxeng::vk::CommandPool*, CommandPool)

_RVAR(VkCommandBufferLevel, VkCommandBufferLevel, Level)
_RVAR(uint32_t, uint32_t, CommandBufferCount)
#endif

#ifdef _OVAR
/* ====== Optional variables ====== */
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
