#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <snvoxeng/snvoxeng/vk/PhysicalDeviceSelector.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

VkPhysicalDevice PhysicalDeviceRegistry::vkHandle(size_t idx) const noexcept { return m_physicalDevices[idx]; }

PhysicalDeviceRegistry::PhysicalDeviceRegistry(const Instance& instance)
	: m_pInstance(&instance)
	, m_physicalDevices(m_pInstance->enumeratePhysicalDevices())
	, m_cached_properties(m_physicalDevices.size(), std::nullopt)
	, m_cached_features(m_physicalDevices.size(), std::nullopt)
	, m_cached_memoryProperties(m_physicalDevices.size(), std::nullopt)
	, m_cached_queueFamilyProperties(m_physicalDevices.size(), std::nullopt)
	, m_cached_extensionProperties(m_physicalDevices.size(), std::nullopt)
{
	if (m_pInstance->getDebugStream()) (*m_pInstance->getDebugStream()) 
		<< "[PhysicalDeviceRegistry()]: device count "
		<< m_physicalDevices.size()
		<< "\n";
}
PhysicalDeviceRegistry::~PhysicalDeviceRegistry() noexcept = default;

const VkPhysicalDeviceProperties& PhysicalDeviceRegistry::getProperties(size_t idx) const noexcept
{
    snassert(idx < m_physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_cached_properties[idx].has_value())
    {
        m_cached_properties[idx].emplace();
        vkGetPhysicalDeviceProperties(m_physicalDevices[idx], &m_cached_properties[idx].value());
    }
    return m_cached_properties[idx].value();
}

const VkPhysicalDeviceFeatures& PhysicalDeviceRegistry::getFeatures(size_t idx) const noexcept
{
    snassert(idx < m_physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_cached_features[idx].has_value())
    {
        m_cached_features[idx].emplace();
        vkGetPhysicalDeviceFeatures(m_physicalDevices[idx], &m_cached_features[idx].value());
    }
    return m_cached_features[idx].value();
}

const VkPhysicalDeviceMemoryProperties& PhysicalDeviceRegistry::getMemoryProperties(size_t idx) const noexcept
{
    snassert(idx < m_physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_cached_memoryProperties[idx].has_value())
    {
        m_cached_memoryProperties[idx].emplace();
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevices[idx], &m_cached_memoryProperties[idx].value());
    }
    return m_cached_memoryProperties[idx].value();
}

const std::vector<VkQueueFamilyProperties>& PhysicalDeviceRegistry::getQueueFamilyProperties(size_t idx) const noexcept
{
    snassert(idx < m_physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_cached_queueFamilyProperties[idx].has_value())
    {
        uint32_t queueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevices[idx], &queueFamilyPropertyCount, nullptr);

        m_cached_queueFamilyProperties[idx].emplace(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevices[idx], &queueFamilyPropertyCount, m_cached_queueFamilyProperties[idx].value().data());
    }
    return m_cached_queueFamilyProperties[idx].value();
}

const std::vector<VkExtensionProperties>& PhysicalDeviceRegistry::getExtensionProperties(size_t idx) const noexcept
{
    snassert(idx < m_physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_cached_extensionProperties[idx].has_value())
    {
        uint32_t propertyCount = 0;
        vkEnumerateDeviceExtensionProperties(m_physicalDevices[idx], nullptr, &propertyCount, nullptr);

        m_cached_extensionProperties[idx].emplace(propertyCount);
        vkEnumerateDeviceExtensionProperties(m_physicalDevices[idx], nullptr, &propertyCount, m_cached_extensionProperties[idx].value().data());
    }
    return m_cached_extensionProperties[idx].value();
}

const VkExtensionProperties* PhysicalDeviceRegistry::findExtensionProperties(size_t idx, const char* extensionName) const noexcept
{
    const auto& extensions = getExtensionProperties(idx);
    for (size_t i = 0; i < extensions.size(); ++i)
        if (std::strcmp(extensions[i].extensionName, extensionName) == 0)
            return &extensions[i];
    return nullptr;
}

const Instance& PhysicalDeviceRegistry::getInstance() const noexcept { return *m_pInstance; }

PhysicalDeviceSelector PhysicalDeviceRegistry::pick(fSelector_t* fSelector, void* user_data) const
{
	return PhysicalDeviceSelector(*this).pick(fSelector, user_data);
}

PhysicalDeviceRegistry::operator PhysicalDeviceSelector() const
{
	return PhysicalDeviceSelector(*this);
}

PhysicalDevice PhysicalDeviceRegistry::get(size_t idx) const { return PhysicalDevice(*this, idx); }
PhysicalDevice PhysicalDeviceRegistry::first() const { return PhysicalDevice(*this, 0); }
PhysicalDevice PhysicalDeviceRegistry::last() const { return PhysicalDevice(*this, m_physicalDevices.size() - 1u); }
size_t PhysicalDeviceRegistry::count() const noexcept { return m_physicalDevices.size(); }

SNVOXENG_API_POD bool fPhysicalDeviseSelectors::fDeviceType(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data)
{
    auto* data = reinterpret_cast<fDeviceType_user_data_t*>(user_data);
    snassert(data != nullptr, "user_data is null", "Provide fDeviceType_user_data_t");
    return registry.getProperties(idx).deviceType == data->deviceType;
}
SNVOXENG_API_POD bool fPhysicalDeviseSelectors::fExtensions(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data)
{
    auto* data = reinterpret_cast<fExtensions_user_data_t*>(user_data);
    snassert(data != nullptr, "user_data is null", "Provide fExtensions_user_data_t");
    for (size_t i = 0; i < data->extensionCount; ++i)
    {
        const auto* extension = registry.findExtensionProperties(idx, data->ppExtensionNames[i]);
        if (!extension) return false;
    }
    return true;
}
SNVOXENG_API_POD bool fPhysicalDeviseSelectors::fQueueSupport(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data)
{
    auto* data = reinterpret_cast<fQueueSupport_user_data_t*>(user_data);
    snassert(data != nullptr, "user_data is null", "Provide fQueueSupport_user_data_t");

    const auto& queueFamilies = registry.getQueueFamilyProperties(idx);

    bool requiredFlagsAnd_satisfied{ false };

    VkQueueFlags accumulatedFlags = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0)
        {
            accumulatedFlags |= queueFamily.queueFlags;
            requiredFlagsAnd_satisfied |= ((queueFamily.queueFlags & data->requiredFlagsAnd) == data->requiredFlagsAnd);
        }
    }

    return requiredFlagsAnd_satisfied && ((accumulatedFlags & data->requiredFlagsOr) == data->requiredFlagsOr);
}
SNVOXENG_API_POD bool fPhysicalDeviseSelectors::fSurfaceSupport(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data)
{
    auto* data = reinterpret_cast<fSurfaceSupport_user_data_t*>(user_data);
    snassert(data != nullptr, "user_data is null", "Provide fSurfaceSupport_user_data_t");

    if (data->surface == VK_NULL_HANDLE) return true;

    PhysicalDevice device = registry.get(idx);
    const auto& queueFamilies = registry.getQueueFamilyProperties(idx);

    for (uint32_t queueFamilyIdx = 0; queueFamilyIdx < queueFamilies.size(); ++queueFamilyIdx)
    {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device.getHandle(), queueFamilyIdx, data->surface, &presentSupport);
        if (presentSupport == VK_TRUE) return true;
    }
    return false;
}
