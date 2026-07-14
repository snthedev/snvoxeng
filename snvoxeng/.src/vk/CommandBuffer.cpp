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

VkCommandBuffer CommandBuffer::vkHandle() const noexcept { return m_pContainer->vkHandle(m_containerIdx); }
const CommandBuffersContainer& CommandBuffer::getContainer() const noexcept { return *m_pContainer; }
size_t CommandBuffer::getContainerIdx() const noexcept { return m_containerIdx; }
