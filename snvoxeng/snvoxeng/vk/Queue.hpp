#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <span>

namespace sn::voxeng::vk
{
    class Device;
    class SNVOXENG_API Queue final
    {
        const Device* m_pContainer = nullptr;
        size_t m_containerIdx = 0;

    public:
        Queue(const Device& container, size_t idx) noexcept;

        VkResult submit(const VkSubmitInfo& submit, VkFence fence) const;
        VkResult submit(std::span<const VkSubmitInfo> submits, VkFence fence) const;
        VkResult waitIdle() const;

        VkResult presentKHR(const VkPresentInfoKHR& presentInfo) const;

        VkQueue vkHandle() const noexcept;
        const Device& getContainer() const noexcept;
        size_t getContainerIdx() const noexcept;
    };
}