#include <snvoxeng/snvoxeng/vk/PhysicalDevice.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

PhysicalDevice::PhysicalDevice(const PhysicalDeviceRegistry& registry, size_t idx) noexcept
    : m_pRegistry(&registry), m_registryIdx(idx)
{
    snassert(m_registryIdx < m_pRegistry->count(), "Out of PhysicalDeviceRegistry::count()", "Provide valid idx");
}

VkSurfaceCapabilitiesKHR PhysicalDevice::getSurfaceCapabilities(VkSurfaceKHR surface) const
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(getHandle(), surface, &surfaceCapabilities);
    return surfaceCapabilities;
}

uint32_t PhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    const auto& memProperties = getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    return nmatch;
}

VkPhysicalDevice PhysicalDevice::getHandle() const noexcept { return m_pRegistry->vkHandle(m_registryIdx); }
const PhysicalDeviceRegistry& PhysicalDevice::getRegistry() const noexcept { return *m_pRegistry; }
size_t PhysicalDevice::getRegistryIdx() const noexcept { return m_registryIdx; }


const VkPhysicalDeviceProperties& PhysicalDevice::getProperties() const noexcept
{
    return m_pRegistry->getProperties(m_registryIdx);
}
const VkPhysicalDeviceFeatures& PhysicalDevice::getFeatures() const noexcept
{
    return m_pRegistry->getFeatures(m_registryIdx);
}
const VkPhysicalDeviceMemoryProperties& PhysicalDevice::getMemoryProperties() const noexcept
{
    return m_pRegistry->getMemoryProperties(m_registryIdx);
}
std::span<const VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties() const noexcept
{
    return m_pRegistry->getQueueFamilyProperties(m_registryIdx);
}
std::span<const VkExtensionProperties> PhysicalDevice::getExtensionProperties() const noexcept
{
    return m_pRegistry->getExtensionProperties(m_registryIdx);
}
const VkExtensionProperties* PhysicalDevice::findExtensionProperties(const char* extensionName) const noexcept
{
    return m_pRegistry->findExtensionProperties(m_registryIdx, extensionName);
}

uint32_t PhysicalDevice::findQueueFamily(const QueueRequest& request) const noexcept
{
    const auto& queueFamilies = getQueueFamilyProperties();
    uint32_t backupMatch = nmatch;

    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); ++i)
    {
        if (queueFamilies[i].queueCount == 0) continue;

        const VkQueueFlags currentFamilyFlags = queueFamilies[i].queueFlags;

        if ((currentFamilyFlags & request.flags) != request.flags) continue;

        if (request.surface != VK_NULL_HANDLE)
        {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(getHandle(), i, request.surface, &presentSupport);
            if (presentSupport != VK_TRUE) continue;
        }

        if (request.preferDedicated)
        {
            if (// Transfer-only request (DMA)
                ((request.flags & VK_QUEUE_TRANSFER_BIT) == request.flags) &&
                // And Graphics, Compute are NOT present
                ((currentFamilyFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == 0))
                return i;
            if (// Compute-only request
                ((request.flags & VK_QUEUE_COMPUTE_BIT) == request.flags) &&
                // And Graphics is NOT present
                ((currentFamilyFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                return i;
        }

        if (backupMatch == nmatch)
            backupMatch = i;
    }

    return backupMatch;
}