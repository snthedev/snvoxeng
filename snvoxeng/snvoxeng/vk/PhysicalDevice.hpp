#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <span>
#include <optional>

namespace sn::voxeng::vk
{
    class PhysicalDeviceRegistry;
    class SNVOXENG_API PhysicalDevice final
    {
        const PhysicalDeviceRegistry* m_pRegistry = nullptr;
        size_t m_registryIdx = 0;

    public:
        PhysicalDevice(const PhysicalDeviceRegistry& registry, size_t idx) noexcept;

        static inline constexpr uint32_t nmatch = ~(uint32_t)(0);

        VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;

        // returns PhysicalDevice::nmatch if memory type is not found
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        VkPhysicalDevice getHandle() const noexcept;
        const PhysicalDeviceRegistry& getRegistry() const noexcept;
        size_t getRegistryIdx() const noexcept;

        const VkPhysicalDeviceProperties& getProperties() const noexcept;
        const VkPhysicalDeviceFeatures& getFeatures() const noexcept;
        const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const noexcept;
        std::span<const VkQueueFamilyProperties> getQueueFamilyProperties() const noexcept;
        std::span<const VkExtensionProperties> getExtensionProperties() const noexcept;
        const VkExtensionProperties* findExtensionProperties(const char* extensionName) const noexcept;

        struct QueueRequest
        {
            VkQueueFlags flags;
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            bool preferDedicated = true;
        };

        // returns PhysicalDevice::nmatch if queue family is not found
        uint32_t findQueueFamily(const QueueRequest& request) const noexcept;
    };
}