/*
 * ====== Instance Builder Configuration ======
 *
 * Available macros:
 *   > Required value
 *   _RVAR(storetype, argtype, name)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& Instance::getName()
 *   
 *   > Optional value with default
 *   _OVAR(storetype, argtype, name, val)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& Instance::getName()
 *
 *   > Required array (std::vector<type>)
 *   _RARR(type, name)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> Instance::getName()
 *     -> Instance::getName(idx)
 *     -> Instance::getNameSize()
 *     -> Instance::getNameData()
 *
 *   > Optional array with default values
 *   _OARR(type, name, ...)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> Instance::getName()
 *     -> Instance::getName(idx)
 *     -> Instance::getNameSize()
 *     -> Instance::getNameData()
 *
 *   > Boolean flag (default: false)
 *   _FLG(name)
 *     -> setName(...)
 *     -> Instance::isName(...)
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
#include <ostream>
#endif

#ifdef _RVAR
/* ====== Required variables ====== */
_RVAR(uint32_t, uint32_t, ApiVersion)
_RVAR(const char*, const char*, ApplicationName)
#endif

#ifdef _OVAR
/* ====== Optional variables ====== */
_OVAR(uint32_t, uint32_t, ApplicationVersion, VK_MAKE_API_VERSION(0, 0, 1, 0))
_OVAR(const char*, const char*, EngineName, "snvoxeng")
_OVAR(uint32_t, uint32_t, EngineVersion, VK_MAKE_API_VERSION(0, 0, 1, 0))

_OVAR(std::ostream*, std::ostream*, DebugStream, nullptr)
#endif

#ifdef _RARR
/* ======   Required arrays  ====== */
#endif

#ifdef _OARR
/* ======   Optional arrays  ====== */
_OARR(const char*, ValidationLayers)
_OARR(const char*, InstanceExtensions)
#endif

#ifdef _FLG
/* ======        Flags       ====== */
_FLG(EnableDebugMessenger)
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
