#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <optional>
#include <vector>

namespace sn::voxeng::vk
{
    class CommandBuffersContainer;
    class SNVOXENG_API CommandBuffer final
    {
        const CommandBuffersContainer* m_pContainer = nullptr;
        size_t m_containerIdx = 0;

    public:
        CommandBuffer(const CommandBuffersContainer& container, size_t idx) noexcept;

        VkCommandBuffer vkHandle() const noexcept;

        const CommandBuffersContainer& getContainer() const noexcept;
        size_t getContainerIdx() const noexcept;
    };
}