/*
 * ====== Device Builder Configuration ======
 *
 * Available macros:
 *   > Required value
 *   _RVAR(storetype, argtype, name)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& Device::getName()
 *   
 *   > Optional value with default
 *   _OVAR(storetype, argtype, name, val)
 *     -> Builder::withName(argtype ...)
 *     -> const storetype& Device::getName()
 *
 *   > Required array (std::vector<type>)
 *   _RARR(type, name)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> Device::getName()
 *     -> Device::getName(idx)
 *     -> Device::getNameSize()
 *     -> Device::getNameData()
 *
 *   > Optional array with default values
 *   _OARR(type, name, ...)
 *     -> Builder::withName(...)
 *     -> Builder::addName(...)
 *     -> Device::getName()
 *     -> Device::getName(idx)
 *     -> Device::getNameSize()
 *     -> Device::getNameData()
 *
 *   > Boolean flag (default: false)
 *   _FLG(name)
 *     -> setName(...)
 *     -> Device::isName(...)
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
#include <snvoxeng/snvoxeng/vk/PhysicalDevice.hpp>
#endif

#ifdef _RVAR
/* ====== Required variables ====== */
_RVAR(const PhysicalDevice*, const PhysicalDevice*, PhysicalDevice)
#endif

#ifdef _OVAR
/* ====== Optional variables ====== */
_OVAR(VkPhysicalDeviceFeatures, const VkPhysicalDeviceFeatures&, PhysicalDeviceFeatures)
_OVAR(VkPhysicalDeviceVulkan13Features, const VkPhysicalDeviceVulkan13Features&, PhysicalDevice13Features)
// TODO: withNext() builder's method
#endif

#ifdef _RARR
/* ======   Required arrays  ====== */
_RARR(sn::voxeng::vk::Device::QueueRequest, QueueRequests, )
#endif

#ifdef _OARR
/* ======   Optional arrays  ====== */
_OARR(const char*, Extensions, )
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
