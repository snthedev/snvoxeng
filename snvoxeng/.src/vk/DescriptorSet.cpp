#include <snvoxeng/snvoxeng/vk/DescriptorSet.hpp>
#include <snvoxeng/snvoxeng/vk/DescriptorSetsContainer.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

DescriptorSet::DescriptorSet(const DescriptorSetsContainer& container, size_t idx) noexcept
	: m_pContainer(&container), m_containerIdx(idx)
{
	snassert(m_containerIdx < m_pContainer->count(), "Out of DescriptorSetsContainer::count()", "Provide valid idx");
}

void DescriptorSet::write(VkWriteDescriptorSet write) const
{
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = vkHandle();
	m_pContainer->getDescriptorPool().getDevice().updateDescriptorSets(1u, &write, 0, nullptr);
}
void DescriptorSet::updateStorageImage(uint32_t binding, VkImageView imageView, VkImageLayout layout) const
{
    VkDescriptorImageInfo imageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = imageView,
        .imageLayout = layout,
    };
    write({
        .pNext = nullptr,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1u,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imageInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr,
        });
}

VkDescriptorSet DescriptorSet::vkHandle() const noexcept { return m_pContainer->vkHandle(m_containerIdx); }
const DescriptorSetsContainer& DescriptorSet::getContainer() const noexcept { return *m_pContainer; }
size_t DescriptorSet::getContainerIdx() const noexcept { return m_containerIdx; }
