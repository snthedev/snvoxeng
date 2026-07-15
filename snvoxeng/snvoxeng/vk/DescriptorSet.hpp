#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <span>

namespace sn::voxeng::vk
{
    class DescriptorSetsContainer;
    class SNVOXENG_API DescriptorSet final
    {
        const DescriptorSetsContainer* m_pContainer = nullptr;
        size_t m_containerIdx = 0;

    public:
        DescriptorSet(const DescriptorSetsContainer& container, size_t idx) noexcept;

        VkDescriptorSet vkHandle() const noexcept;

        const DescriptorSetsContainer& getContainer() const noexcept;
        size_t getContainerIdx() const noexcept;
    };
}