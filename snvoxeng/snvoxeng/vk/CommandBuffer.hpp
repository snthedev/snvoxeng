#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <span>

namespace sn::voxeng::vk
{
    class CommandBuffersContainer;
    class SNVOXENG_API CommandBuffer final
    {
        const CommandBuffersContainer* m_pContainer = nullptr;
        size_t m_containerIdx = 0;

    public:
        CommandBuffer(const CommandBuffersContainer& container, size_t idx) noexcept;

        void begin(VkCommandBufferBeginInfo info = {}) const;
        void end() const;

        void cmdBindPipeline(
            VkPipelineBindPoint pipelineBindPoint,
            VkPipeline pipeline
        ) const noexcept;

        void cmdPipelineBarrier(
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkDependencyFlags dependencyFlags,
            std::span<const VkMemoryBarrier> memoryBarriers,
            std::span<const VkBufferMemoryBarrier> bufferMemoryBarriers,
            std::span<const VkImageMemoryBarrier> imageMemoryBarriers
        ) const noexcept;

        void cmdBindDescriptorSets(
            VkPipelineBindPoint pipelineBindPoint,
            VkPipelineLayout layout,
            uint32_t firstSet,
            std::span<const VkDescriptorSet> descriptorSets,
            std::span<const uint32_t> dynamicOffsets
        ) const noexcept;

        void cmdDispatch(
            uint32_t groupCountX,
            uint32_t groupCountY,
            uint32_t groupCountZ
        ) const noexcept;

        void cmdCopyImageToBuffer(
            VkImage srcImage,
            VkImageLayout srcImageLayout,
            VkBuffer dstBuffer,
            std::span<const VkBufferImageCopy> regions
        ) const noexcept;

        VkCommandBuffer vkHandle() const noexcept;

        const CommandBuffersContainer& getContainer() const noexcept;
        size_t getContainerIdx() const noexcept;
    };
}