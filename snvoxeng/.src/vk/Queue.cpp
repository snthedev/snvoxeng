#include <snvoxeng/snvoxeng/vk/Queue.hpp>
#include <snvoxeng/snvoxeng/vk/Device.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

Queue::Queue(const Device& container, size_t idx) noexcept
	: m_pContainer(&container), m_containerIdx(idx)
{
	snassert(m_containerIdx < m_pContainer->countQueue(), "Out of Device::countQueue()", "Provide valid idx");
}

VkResult Queue::submit(const VkSubmitInfo& submit, VkFence fence) const
{
	return vkQueueSubmit(vkHandle(), 1u, &submit, fence);
}
VkResult Queue::submit(std::span<const VkSubmitInfo> submits, VkFence fence) const
{
	return vkQueueSubmit(vkHandle(), static_cast<uint32_t>(submits.size()), submits.data(), fence);
}
VkResult Queue::waitIdle() const
{
	return vkQueueWaitIdle(vkHandle());
}

VkResult Queue::presentKHR(const VkPresentInfoKHR& presentInfo) const
{
	return vkQueuePresentKHR(vkHandle(), &presentInfo);
}

VkQueue Queue::vkHandle() const noexcept { return m_pContainer->vkQueueHandle(m_containerIdx); }
const Device& Queue::getContainer() const noexcept { return *m_pContainer; }
size_t Queue::getContainerIdx() const noexcept { return m_containerIdx; }
