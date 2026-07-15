#include <snvoxeng/snvoxeng/vk/Queue.hpp>
#include <snvoxeng/snvoxeng/vk/Device.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

Queue::Queue(const Device& container, size_t idx) noexcept
	: m_pContainer(&container), m_containerIdx(idx)
{
	snassert(m_containerIdx < m_pContainer->countQueue(), "Out of Device::countQueue()", "Provide valid idx");
}

void Queue::submit(const VkSubmitInfo& submit, VkFence fence) const
{
	auto result = queueSubmit(1u, &submit, fence);
	snassert(result == VK_SUCCESS,
		"Failed to submit", "");
}
void Queue::submit(std::span<const VkSubmitInfo> submits, VkFence fence) const
{
	auto result = queueSubmit(static_cast<uint32_t>(submits.size()), submits.data(), fence);
	snassert(result == VK_SUCCESS,
		"Failed to submit", "");
}
void Queue::waitIdle() const
{
	auto result = queueWaitIdle();
	snassert(result == VK_SUCCESS,
		"Failed to wait idle", "");
}

VkResult Queue::queueSubmit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const
{
	return vkQueueSubmit(vkHandle(), submitCount, pSubmits, fence);
}
VkResult Queue::queueWaitIdle() const
{
	return vkQueueWaitIdle(vkHandle());
}

VkQueue Queue::vkHandle() const noexcept { return m_pContainer->vkQueueHandle(m_containerIdx); }
const Device& Queue::getContainer() const noexcept { return *m_pContainer; }
size_t Queue::getContainerIdx() const noexcept { return m_containerIdx; }
