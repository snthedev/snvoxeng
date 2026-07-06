#include <snvoxeng/snvoxeng/vk/CommandBuffer.hpp>
#include <snvoxeng/snvoxeng/vk/CommandBuffersContainer.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

CommandBuffer::CommandBuffer(const CommandBuffersContainer& container, size_t idx) noexcept
	: m_pContainer(&container), m_containerIdx(idx)
{
	snassert(m_containerIdx < m_pContainer->count(), "Out of CommandBuffersContainer::count()", "Provide valid idx");
}

VkCommandBuffer CommandBuffer::getHandle() const noexcept { return m_pContainer->getHandle(m_containerIdx); }
const CommandBuffersContainer& CommandBuffer::getContainer() const noexcept { return *m_pContainer; }
size_t CommandBuffer::getContainerIdx() const noexcept { return m_containerIdx; }
