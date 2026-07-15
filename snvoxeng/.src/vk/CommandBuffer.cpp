#include <snvoxeng/snvoxeng/vk/CommandBuffer.hpp>
#include <snvoxeng/snvoxeng/vk/CommandBuffersContainer.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

CommandBuffer::CommandBuffer(const CommandBuffersContainer& container, size_t idx) noexcept
	: m_pContainer(&container), m_containerIdx(idx)
{
	snassert(m_containerIdx < m_pContainer->count(), "Out of CommandBuffersContainer::count()", "Provide valid idx");
}

void CommandBuffer::begin(VkCommandBufferBeginInfo info) const
{
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    {
        auto result = vkBeginCommandBuffer(vkHandle(), &info);
        snassert(result == VK_SUCCESS,
            "Failed to begin command buffer recording", "");
    }
}
void CommandBuffer::end() const
{
    {
        auto result = vkEndCommandBuffer(vkHandle());
        snassert(result == VK_SUCCESS,
            "Failed to record command buffer", "");
    }
}

void CommandBuffer::cmdBindPipeline(
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline
) const noexcept
{
    vkCmdBindPipeline(vkHandle(), pipelineBindPoint, pipeline);
}

void CommandBuffer::cmdPipelineBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    std::span<const VkMemoryBarrier> memoryBarriers,
    std::span<const VkBufferMemoryBarrier> bufferMemoryBarriers,
    std::span<const VkImageMemoryBarrier> imageMemoryBarriers
) const noexcept
{
    vkCmdPipelineBarrier(
        vkHandle(),
        srcStageMask,
        dstStageMask,
        dependencyFlags,
        static_cast<uint32_t>(memoryBarriers.size()), memoryBarriers.data(),
        static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(),
        static_cast<uint32_t>(imageMemoryBarriers.size()), imageMemoryBarriers.data()
    );
}

void CommandBuffer::cmdBindDescriptorSets(
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    std::span<const VkDescriptorSet> descriptorSets,
    std::span<const uint32_t> dynamicOffsets
) const noexcept
{
    vkCmdBindDescriptorSets(
        vkHandle(),
        pipelineBindPoint,
        layout,
        firstSet,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        static_cast<uint32_t>(dynamicOffsets.size()),
        dynamicOffsets.data()
    );
}

void CommandBuffer::cmdDispatch(
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ
) const noexcept
{
    vkCmdDispatch(
        vkHandle(),
        groupCountX,
        groupCountY,
        groupCountZ
    );
}

void CommandBuffer::cmdCopyImageToBuffer(
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkBuffer dstBuffer,
    std::span<const VkBufferImageCopy> regions
) const noexcept
{
    vkCmdCopyImageToBuffer(
        vkHandle(),
        srcImage,
        srcImageLayout,
        dstBuffer,
        static_cast<uint32_t>(regions.size()),
        regions.data()
    );
}

VkCommandBuffer CommandBuffer::vkHandle() const noexcept { return m_pContainer->vkHandle(m_containerIdx); }
const CommandBuffersContainer& CommandBuffer::getContainer() const noexcept { return *m_pContainer; }
size_t CommandBuffer::getContainerIdx() const noexcept { return m_containerIdx; }
