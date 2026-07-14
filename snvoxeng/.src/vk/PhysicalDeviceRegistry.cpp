#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceSelector.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDevice.hpp>

#include <snassert/snassert.hpp>

#include <vector>
#include <optional>

using namespace sn::voxeng::vk;

struct PhysicalDeviceRegistry::data_t
{
    const Instance* pInstance;
    std::vector<VkPhysicalDevice> physicalDevices;

    mutable std::vector<std::optional<VkPhysicalDeviceProperties>> cached_properties;
    mutable std::vector<std::optional<VkPhysicalDeviceFeatures>> cached_features;
    mutable std::vector<std::optional<VkPhysicalDeviceMemoryProperties>> cached_memoryProperties;
    mutable std::vector<std::optional<std::vector<VkQueueFamilyProperties>>> cached_queueFamilyProperties;
    mutable std::vector<std::optional<std::vector<VkExtensionProperties>>> cached_extensionProperties;

    data_t(const Instance& instance)
        : pInstance(&instance)
    {
        uint32_t physicalDevicesCount{ 0u };
        {
            auto result = pInstance->enumeratePhysicalDevices(&physicalDevicesCount, nullptr);
            snassert(result == VK_SUCCESS,
                "Failed to enumeratePhysicalDevices", "");
        }
        physicalDevices.resize(physicalDevicesCount);
        {
            auto result = pInstance->enumeratePhysicalDevices(&physicalDevicesCount, physicalDevices.data());
            snassert(result == VK_SUCCESS,
                "Failed to enumeratePhysicalDevices", "");
        }

        cached_properties.resize(physicalDevices.size(), std::nullopt);
        cached_features.resize(physicalDevices.size(), std::nullopt);
        cached_memoryProperties.resize(physicalDevices.size(), std::nullopt);
        cached_queueFamilyProperties.resize(physicalDevices.size(), std::nullopt);
        cached_extensionProperties.resize(physicalDevices.size(), std::nullopt);
    }
    ~data_t() noexcept = default;
};

VkPhysicalDevice PhysicalDeviceRegistry::vkHandle(size_t idx) const noexcept { return m_pData->physicalDevices[idx]; }

PhysicalDeviceRegistry::PhysicalDeviceRegistry(const Instance& instance)
    : m_pData(new data_t{ instance })
{
    if (m_pData->pInstance->getDebugStream())
        *m_pData->pInstance->getDebugStream()
        << "[trace]: PhysicalDeviceRegistry 0x" << std::hex << this << std::dec << " created" << std::endl;
}
PhysicalDeviceRegistry::~PhysicalDeviceRegistry() noexcept
{
    if (m_pData->pInstance->getDebugStream())
        *m_pData->pInstance->getDebugStream()
        << "[trace]: PhysicalDeviceRegistry 0x" << std::hex << this << std::dec << " destroyed" << std::endl;

    delete m_pData;
}

const VkPhysicalDeviceProperties& PhysicalDeviceRegistry::getProperties(size_t idx) const noexcept
{
    snassert(idx < m_pData->physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_pData->cached_properties[idx].has_value())
    {
        m_pData->cached_properties[idx].emplace();
        vkGetPhysicalDeviceProperties(m_pData->physicalDevices[idx], &m_pData->cached_properties[idx].value());
    }
    return m_pData->cached_properties[idx].value();
}

const VkPhysicalDeviceFeatures& PhysicalDeviceRegistry::getFeatures(size_t idx) const noexcept
{
    snassert(idx < m_pData->physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_pData->cached_features[idx].has_value())
    {
        m_pData->cached_features[idx].emplace();
        vkGetPhysicalDeviceFeatures(m_pData->physicalDevices[idx], &m_pData->cached_features[idx].value());
    }
    return m_pData->cached_features[idx].value();
}

const VkPhysicalDeviceMemoryProperties& PhysicalDeviceRegistry::getMemoryProperties(size_t idx) const noexcept
{
    snassert(idx < m_pData->physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_pData->cached_memoryProperties[idx].has_value())
    {
        m_pData->cached_memoryProperties[idx].emplace();
        vkGetPhysicalDeviceMemoryProperties(m_pData->physicalDevices[idx], &m_pData->cached_memoryProperties[idx].value());
    }
    return m_pData->cached_memoryProperties[idx].value();
}

std::span<const VkQueueFamilyProperties> PhysicalDeviceRegistry::getQueueFamilyProperties(size_t idx) const noexcept
{
    snassert(idx < m_pData->physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_pData->cached_queueFamilyProperties[idx].has_value())
    {
        uint32_t queueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_pData->physicalDevices[idx], &queueFamilyPropertyCount, nullptr);

        m_pData->cached_queueFamilyProperties[idx].emplace(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_pData->physicalDevices[idx], &queueFamilyPropertyCount, m_pData->cached_queueFamilyProperties[idx].value().data());
    }
    return m_pData->cached_queueFamilyProperties[idx].value();
}

std::span<const VkExtensionProperties> PhysicalDeviceRegistry::getExtensionProperties(size_t idx) const noexcept
{
    snassert(idx < m_pData->physicalDevices.size(), "Index out of bounds", "Valid range is [0; device count)");

    if (!m_pData->cached_extensionProperties[idx].has_value())
    {
        uint32_t propertyCount = 0;
        vkEnumerateDeviceExtensionProperties(m_pData->physicalDevices[idx], nullptr, &propertyCount, nullptr);

        m_pData->cached_extensionProperties[idx].emplace(propertyCount);
        vkEnumerateDeviceExtensionProperties(m_pData->physicalDevices[idx], nullptr, &propertyCount, m_pData->cached_extensionProperties[idx].value().data());
    }
    return m_pData->cached_extensionProperties[idx].value();
}

const VkExtensionProperties* PhysicalDeviceRegistry::findExtensionProperties(size_t idx, const char* extensionName) const noexcept
{
    const auto& extensions = getExtensionProperties(idx);
    for (size_t i = 0; i < extensions.size(); ++i)
        if (std::strcmp(extensions[i].extensionName, extensionName) == 0)
            return &extensions[i];
    return nullptr;
}

const Instance& PhysicalDeviceRegistry::getInstance() const noexcept { return *m_pData->pInstance; }

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
PhysicalDevice PhysicalDeviceRegistry::last() const { return PhysicalDevice(*this, m_pData->physicalDevices.size() - 1u); }
size_t PhysicalDeviceRegistry::count() const noexcept { return m_pData->physicalDevices.size(); }

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
