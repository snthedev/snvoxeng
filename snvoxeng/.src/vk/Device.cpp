#include <snvoxeng/snvoxeng/vk/Device.hpp>
#include <snvoxeng/snvoxeng/utils/vk-getSType.hpp>

#include <snvoxeng/snvoxeng/vk/Queue.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <vulkan/vulkan.h>
#include <snassert/snassert.hpp>

#include <string>
#include <vector>

using namespace sn::voxeng::vk;

struct QueueFamilyRequest_t
{
	uint32_t queueFamilyIndex;
	std::vector<float> queuePriorities;
	std::vector<size_t*> pOutQueueIndices;

	QueueFamilyRequest_t() = default;
	QueueFamilyRequest_t(const Device::QueueRequest_t& request)
		: queueFamilyIndex(request.queueFamilyIndex)
	{
		add(request);
	}

	void add(const Device::QueueRequest_t& request)
	{
		queuePriorities.emplace_back(request.queuePriority);
		pOutQueueIndices.emplace_back(request.pOutQueueIndex);
	}

	~QueueFamilyRequest_t() noexcept = default;
};

namespace default_values
{
#define SNBCG_DEFAULT_VALUES
#include <snvoxeng/.def/vk/Device.h>
}

// === Device : private ===
struct Device::data_t
{
	VkDeviceCreateInfo vkCreateInfo{ .sType{ ::sn::voxeng::utils::vk::getSType<VkDeviceCreateInfo>() } };

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#include <snvoxeng/.def/vk/Device.h>

	data_t()
	{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		subdata name = {};
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		subdata name = default_values::name;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		subdata name = {};
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		subdata name = default_values::name;
#include <snvoxeng/.def/vk/Device.h>
	}

	VkDevice vkHandle{ VK_NULL_HANDLE };
	std::vector<VkQueue> vkQueues;

	std::vector<QueueFamilyRequest_t> queueFamilyRequests;
};

void Device::onCreate(data_t& data)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(data.queueFamilyRequests.size());
	for (const auto& queueFamilyRequest : data.queueFamilyRequests)
		queueCreateInfos.push_back({
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queueFamilyRequest.queueFamilyIndex,
			.queueCount = static_cast<uint32_t>(queueFamilyRequest.queuePriorities.size()),
			.pQueuePriorities = queueFamilyRequest.queuePriorities.data(),
			});

	data.vkCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	data.vkCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	{
		auto result = vkCreateDevice(data.pPhysicalDevice->getHandle(), &data.vkCreateInfo, data.vkPAllocator, &data.vkHandle);
		snassert(result == VK_SUCCESS,
			"Failed to create VkDevice", "Check Builder settings");
	}

	size_t queueIndex = 0;
	for (const auto& queueFamilyRequest : data.queueFamilyRequests)
	{
		for (size_t i = 0; i < queueFamilyRequest.queuePriorities.size(); ++i, ++queueIndex)
		{
			*queueFamilyRequest.pOutQueueIndices[i] = queueIndex;
			vkGetDeviceQueue(data.vkHandle, queueFamilyRequest.queueFamilyIndex, static_cast<uint32_t>(i), &data.vkQueues[queueIndex]);
		}
	}

	if (m_pData->pPhysicalDevice->getRegistry().getInstance().getDebugStream())
		*m_pData->pPhysicalDevice->getRegistry().getInstance().getDebugStream()
		<< "[trace]: Device 0x" << std::hex << this << std::dec << " created" << std::endl;
}
void Device::onDestroy(data_t& data) noexcept
{
	vkDestroyDevice(data.vkHandle, data.vkPAllocator);

	if (m_pData->pPhysicalDevice->getRegistry().getInstance().getDebugStream())
		*m_pData->pPhysicalDevice->getRegistry().getInstance().getDebugStream()
		<< "[trace]: Device 0x" << std::hex << this << std::dec << " destroyed" << std::endl;
}

Device::Device(data_t*& pData)
	: m_pData(pData)
	, m_isView(false)
{
	pData = nullptr;
	onCreate(*m_pData);
}
Device::Device(data_t*& pData, VkDevice view)
	: m_pData(pData)
	, m_isView(true)
{
	pData = nullptr;
	m_pData->vkHandle = view;
}

// === Device : public ===
Device::~Device() noexcept
{
	if (m_pData) [[likely]]
	{
		if (!m_isView) [[likely]] onDestroy(*m_pData);
		delete m_pData;
	}
}

Queue Device::getQueue(size_t idx) const { return Queue(*this, idx); }
Queue Device::firstQueue() const { return Queue(*this, 0); }
Queue Device::lastQueue() const { return Queue(*this, countQueue() - 1u); }
size_t Device::countQueue() const noexcept { return m_pData->vkQueues.size(); }

void Device::getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const
{
	return vkGetDeviceQueue(m_pData->vkHandle, queueFamilyIndex, queueIndex, pQueue);
}

VkResult Device::createSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const
{
	return vkCreateSwapchainKHR(m_pData->vkHandle, pCreateInfo, pAllocator, pSwapchain);
}
void Device::destroySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroySwapchainKHR(m_pData->vkHandle, swapchain, pAllocator);
}

VkResult Device::getSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const
{
	return vkGetSwapchainImagesKHR(m_pData->vkHandle, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult Device::createImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const
{
	return vkCreateImage(m_pData->vkHandle, pCreateInfo, pAllocator, pImage);
}
void Device::destroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyImage(m_pData->vkHandle, image, pAllocator);
}

VkResult Device::createImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const
{
	return vkCreateImageView(m_pData->vkHandle, pCreateInfo, pAllocator, pView);
}
void Device::destroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyImageView(m_pData->vkHandle, imageView, pAllocator);
}

VkResult Device::createFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const
{
	return vkCreateFence(m_pData->vkHandle, pCreateInfo, pAllocator, pFence);
}
void Device::destroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyFence(m_pData->vkHandle, fence, pAllocator);
}

VkResult Device::waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const
{
	return vkWaitForFences(m_pData->vkHandle, fenceCount, pFences, waitAll, timeout);
}
VkResult Device::resetFences(uint32_t fenceCount, const VkFence* pFences) const
{
	return vkResetFences(m_pData->vkHandle, fenceCount, pFences);
}

VkResult Device::createSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const
{
	return vkCreateSemaphore(m_pData->vkHandle, pCreateInfo, pAllocator, pSemaphore);
}
void Device::destroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroySemaphore(m_pData->vkHandle, semaphore, pAllocator);
}

VkResult Device::createCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const
{
	return vkCreateCommandPool(m_pData->vkHandle, pCreateInfo, pAllocator, pCommandPool);
}
void Device::destroyCommandPool(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyCommandPool(m_pData->vkHandle, commandPool, pAllocator);
}

VkResult Device::allocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const
{
	return vkAllocateCommandBuffers(m_pData->vkHandle, pAllocateInfo, pCommandBuffers);
}
void Device::freeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const
{
	vkFreeCommandBuffers(m_pData->vkHandle, commandPool, commandBufferCount, pCommandBuffers);
}

void Device::getImageMemoryRequirements(VkImage image, VkMemoryRequirements* pMemoryRequirements) const
{
	vkGetImageMemoryRequirements(m_pData->vkHandle, image, pMemoryRequirements);
}
void Device::getBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) const
{
	vkGetBufferMemoryRequirements(m_pData->vkHandle, buffer, pMemoryRequirements);
}

VkResult Device::allocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const
{
	return vkAllocateMemory(m_pData->vkHandle, pAllocateInfo, pAllocator, pMemory);
}
void Device::freeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const
{
	vkFreeMemory(m_pData->vkHandle, memory, pAllocator);
}

VkResult Device::mapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) const
{
	return vkMapMemory(m_pData->vkHandle, memory, offset, size, flags, ppData);
}
void Device::unmapMemory(VkDeviceMemory memory) const
{
	vkUnmapMemory(m_pData->vkHandle, memory);
}

VkResult Device::bindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const
{
	return vkBindImageMemory(m_pData->vkHandle, image, memory, memoryOffset);
}
VkResult Device::bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const
{
	return vkBindBufferMemory(m_pData->vkHandle, buffer, memory, memoryOffset);
}

VkResult Device::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const
{
	return vkCreateDescriptorSetLayout(m_pData->vkHandle, pCreateInfo, pAllocator, pSetLayout);
}
void Device::destroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyDescriptorSetLayout(m_pData->vkHandle, descriptorSetLayout, pAllocator);
}

VkResult Device::createPipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const
{
	return vkCreatePipelineLayout(m_pData->vkHandle, pCreateInfo, pAllocator, pPipelineLayout);
}

void Device::destroyPipelineLayout(VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyPipelineLayout(m_pData->vkHandle, pipelineLayout, pAllocator);
}

VkResult Device::createShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const
{
	return vkCreateShaderModule(m_pData->vkHandle, pCreateInfo, pAllocator, pShaderModule);
}
void Device::destroyShaderModule(VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyShaderModule(m_pData->vkHandle, shaderModule, pAllocator);
}

VkResult Device::createComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const
{
	return vkCreateComputePipelines(m_pData->vkHandle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
void Device::destroyPipeline(VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyPipeline(m_pData->vkHandle, pipeline, pAllocator);
}

VkResult Device::createDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const
{
	return vkCreateDescriptorPool(m_pData->vkHandle, pCreateInfo, pAllocator, pDescriptorPool);
}
void Device::destroyDescriptorPool(VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyDescriptorPool(m_pData->vkHandle, descriptorPool, pAllocator);
}

VkResult Device::allocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) const
{
	return vkAllocateDescriptorSets(m_pData->vkHandle, pAllocateInfo, pDescriptorSets);
}
VkResult Device::freeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const
{
	return vkFreeDescriptorSets(m_pData->vkHandle, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void Device::updateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) const
{
	vkUpdateDescriptorSets(m_pData->vkHandle, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VkResult Device::createBuffer(const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const
{
	return vkCreateBuffer(m_pData->vkHandle, pCreateInfo, pAllocator, pBuffer);
}
void Device::destroyBuffer(VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyBuffer(m_pData->vkHandle, buffer, pAllocator);
}

Device::Device(Device&& other) noexcept
	: m_pData(other.m_pData)
	, m_isView(other.m_isView)
{
	other.m_pData = nullptr;
}
Device& Device::operator=(Device&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData)
		{
			if (!m_isView) [[likely]] onDestroy(*m_pData);
			delete m_pData;
		}
		m_pData = other.m_pData;
		m_isView = other.m_isView;
		other.m_pData = nullptr;
	}
	return *this;
}

std::span<const VkQueue> Device::vkQueueHandle() const noexcept { return m_pData->vkQueues; }
VkQueue Device::vkQueueHandle(size_t idx) const noexcept { return m_pData->vkQueues[idx]; }
Device::operator std::span<const VkQueue>() const noexcept { return m_pData->vkQueues; }

VkDevice Device::vkHandle() const noexcept { return m_pData->vkHandle; }
Device::operator VkDevice() const noexcept { return m_pData->vkHandle; }

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) Device::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) Device::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) Device::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) Device::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#include <snvoxeng/.def/vk/Device.h>



typedef Device::Builder Builder;

// === Builder : private ===
void Builder::finalize(data_t& data)
{
	for (const auto& queue_request : data.queueRequests)
	{
		snassert(queue_request.pOutQueueIndex != nullptr,
			"pOutQueueIndex can not be nullptr.",
			"Provide valid pOutQueueIndex"
		);
	}

	data.queueFamilyRequests.reserve(data.queueRequests.size());
	for (const auto& queue_request : data.queueRequests)
	{
		auto it = std::find_if(data.queueFamilyRequests.begin(), data.queueFamilyRequests.end(),
			[&queue_request](const QueueFamilyRequest_t& val) { return val.queueFamilyIndex == queue_request.queueFamilyIndex; });
		if (it != data.queueFamilyRequests.end()) it->add(queue_request);
		else data.queueFamilyRequests.emplace_back(queue_request);
	}

	data.vkQueues.resize(data.queueRequests.size());
	snassert(!data.vkQueues.empty(),
		"No queues requested",
		"Add at least one queue request via Builder::addQueueRequests()"
	);

	for (const auto& queueFamilyRequests : data.queueFamilyRequests)
	{
		uint32_t maxAvailable = data.pPhysicalDevice->getQueueFamilyProperties()[queueFamilyRequests.queueFamilyIndex].queueCount;
		snassert(queueFamilyRequests.queuePriorities.size() <= maxAvailable,
			"Requested more queues than family contains",
			(
				"Family idx: " + std::to_string(queueFamilyRequests.queueFamilyIndex)
				+ ", Available: " + std::to_string(maxAvailable)
			).c_str()
		);
	}

	data.physicalDeviceFeatures.samplerAnisotropy = VK_TRUE; // required

	// TODO: make standart for pNext chain building
	data.physicalDevice13Features.sType = ::sn::voxeng::utils::vk::getSType<VkPhysicalDeviceVulkan13Features>();
	data.physicalDevice13Features.pNext = const_cast<void*>(data.vkCreateInfo.pNext);
	data.vkCreateInfo.pNext = &data.physicalDevice13Features;

	data.vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(data.extensions.size());
	data.vkCreateInfo.ppEnabledExtensionNames = data.extensions.data();
	data.vkCreateInfo.pEnabledFeatures = &data.physicalDeviceFeatures;
}

#ifdef DETAIL_SNBCG_DEBUG
struct Builder::temp_t
{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#include <snvoxeng/.def/vk/Device.h>

	void validate() const
	{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		snassert((name & 0b01) != 0,\
			#Name " is required, but not defined",\
			"Call Builder::with" #Name "(...)"\
		);\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once"\
		);
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once"\
		);
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		snassert((name & 0b01) != 0,\
			#Name " is required, but not defined",\
			"Call Builder::with" #Name "(...)\n"\
			"  or Builder::add" #Name "(...)"\
		);\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once\n"\
			"  and do not call Builder::with" #Name "(...) after calling\n"\
			"  Builder::add" #Name "(...)"\
		);
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once\n"\
			"  and do not call Builder::with" #Name "(...) after calling\n"\
			"  Builder::add" #Name "(...)"\
		);
#include <snvoxeng/.def/vk/Device.h>
	}
};
#define SNBCG_VALIDATE_ON_WITH(name, Name) m_pTemp->name = ((m_pTemp->name << 1u) & 0b11) | 0b01;
#define SNBCG_VALIDATE_ON_ADD(name, Name) m_pTemp->name = m_pTemp->name | 0b01;
#else // ^ DETAIL_SNBCG_DEBUG ^
#define SNBCG_VALIDATE_ON_WITH(name, Name)
#define SNBCG_VALIDATE_ON_ADD(name, Name)
#endif // ^ ~DETAIL_SNBCG_DEBUG ^

// === Builder : public ===
Builder::Builder()
	: m_pData(new data_t{})
#ifdef DETAIL_SNBCG_DEBUG
	, m_pTemp(new temp_t{})
#endif // ^ DETAIL_SNBCG_DEBUG ^
{
}
Builder::~Builder() noexcept
{
	if (m_pData) [[unlikely]] delete m_pData;
#ifdef DETAIL_SNBCG_DEBUG
	if (m_pTemp) [[likely]] delete m_pTemp;
#endif // ^ DETAIL_SNBCG_DEBUG ^
}

Builder Builder::clone() const
{
	auto builder = Builder();
	(*builder.m_pData) = (*m_pData);
#ifdef DETAIL_SNBCG_DEBUG
	(*builder.m_pTemp) = (*m_pTemp);
#endif // ^ DETAIL_SNBCG_DEBUG ^
	return builder;
}

Builder::Builder(Builder&& other) noexcept
	: m_pData(other.m_pData)
#ifdef DETAIL_SNBCG_DEBUG
	, m_pTemp(other.m_pTemp)
#endif // ^ DETAIL_SNBCG_DEBUG ^
{
	other.m_pData = nullptr;
#ifdef DETAIL_SNBCG_DEBUG
	other.m_pTemp = nullptr;
#endif // ^ DETAIL_SNBCG_DEBUG ^
}
Builder& Builder::operator=(Builder&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData) delete m_pData;
		m_pData = other.m_pData;
		other.m_pData = nullptr;
#ifdef DETAIL_SNBCG_DEBUG
		if (m_pTemp) delete m_pTemp;
		m_pTemp = other.m_pTemp;
		other.m_pTemp = nullptr;
#endif // ^ DETAIL_SNBCG_DEBUG ^
	}
	return *this;
}

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
Builder& Builder::with##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
Builder& Builder::with##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
Builder& Builder::with##Name(args_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<args_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}\
Builder& Builder::add##Name(args_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<args_t> args = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_MULTI;\
	return *this;\
}\
Builder& Builder::add##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_SINGLE;\
	return *this;\
}
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
Builder& Builder::with##Name(args_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<args_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}\
Builder& Builder::add##Name(args_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<args_t> args = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_MULTI;\
	return *this;\
}\
Builder& Builder::add##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_SINGLE;\
	return *this;\
}
#include <snvoxeng/.def/vk/Device.h>

Device Builder::sbuild()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return Device{ m_pData };
}
Device* Builder::build()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return new Device{ m_pData };
}

Device Builder::sbuild(VkDevice view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return Device{ m_pData, view };
}
Device* Builder::build(VkDevice view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return new Device{ m_pData, view };
}
