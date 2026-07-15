#include <snvoxeng/snvoxeng/vk/DescriptorSet.hpp>
#include <snvoxeng/snvoxeng/vk/DescriptorSetsContainer.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

DescriptorSet::DescriptorSet(const DescriptorSetsContainer& container, size_t idx) noexcept
	: m_pContainer(&container), m_containerIdx(idx)
{
	snassert(m_containerIdx < m_pContainer->count(), "Out of DescriptorSetsContainer::count()", "Provide valid idx");
}

VkDescriptorSet DescriptorSet::vkHandle() const noexcept { return m_pContainer->vkHandle(m_containerIdx); }
const DescriptorSetsContainer& DescriptorSet::getContainer() const noexcept { return *m_pContainer; }
size_t DescriptorSet::getContainerIdx() const noexcept { return m_containerIdx; }
