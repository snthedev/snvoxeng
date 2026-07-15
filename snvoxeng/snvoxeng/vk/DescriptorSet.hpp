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

        void write(VkWriteDescriptorSet write) const;
        void updateStorageImage(uint32_t binding, VkImageView imageView, VkImageLayout layout) const;

        VkDescriptorSet vkHandle() const noexcept;
        const DescriptorSetsContainer& getContainer() const noexcept;
        size_t getContainerIdx() const noexcept;
    };
}