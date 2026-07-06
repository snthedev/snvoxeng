#include <snvoxeng\snvoxeng\vk\Device.hpp>

#include <snassert/snassert.hpp>
#include <string>
#include <stdexcept>

using namespace sn::voxeng::vk;

// === Device ===

//  > Data
struct Device::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\Device.h>
	
	VkDevice vkDevice;
	std::vector<NamedQueue> namedQueues;
};

//  > Init
Device::Device(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	snassert(!m_pData->QueueRequests.empty(), "No queues requested", "Add at least one queue via Builder::addQueueRequests()");

	struct FamilyGroup
	{
		uint32_t familyIndex;
		std::vector<float> priorities;
		std::vector<size_t> requestIndices;
	};
	std::vector<FamilyGroup> uniqueFamilies;

	for (size_t i = 0; i < m_pData->QueueRequests.size(); ++i)
	{
		const auto& req = m_pData->QueueRequests[i];
		auto it = std::find_if(uniqueFamilies.begin(), uniqueFamilies.end(),
			[req](const FamilyGroup& group) { return group.familyIndex == req.familyIndex; });
		if (it != uniqueFamilies.end())
		{
			it->priorities.push_back(req.priority);
			it->requestIndices.push_back(i);
			continue;
		}
		uniqueFamilies.push_back({
			.familyIndex = req.familyIndex,
			.priorities = { req.priority },
			.requestIndices = { i }
			});
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(uniqueFamilies.size());

	for (const auto& group : uniqueFamilies)
	{
		uint32_t maxAvailable = m_pData->PhysicalDevice->getQueueFamilyProperties()[group.familyIndex].queueCount;
		snassert(group.priorities.size() <= maxAvailable,
			"Requested more queues than family contains",
			("Family: " + std::to_string(group.familyIndex) + ", Available: " + std::to_string(maxAvailable)).c_str()
		);
		queueCreateInfos.push_back({
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = group.familyIndex,
			.queueCount = static_cast<uint32_t>(group.priorities.size()),
			.pQueuePriorities = group.priorities.data(),
			});
	}

	m_pData->PhysicalDevice13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

	m_pData->PhysicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	VkDeviceCreateInfo deviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		// TODO: withNext() builder's method
		.pNext = &m_pData->PhysicalDevice13Features,
		.flags{},
		.queueCreateInfoCount{ static_cast<uint32_t>(queueCreateInfos.size()) },
		.pQueueCreateInfos{ queueCreateInfos.data() },
		.enabledExtensionCount{ static_cast<uint32_t>(m_pData->Extensions.size()) },
		.ppEnabledExtensionNames{ m_pData->Extensions.data() },
		.pEnabledFeatures{ &m_pData->PhysicalDeviceFeatures },
	};

	if (m_pData->PhysicalDevice->createDevice(&deviceCreateInfo, nullptr, &m_pData->vkDevice) != VK_SUCCESS)
		throw std::runtime_error("Failed to create VkDevice.");

	m_pData->namedQueues.resize(m_pData->QueueRequests.size());
	for (const auto& group : uniqueFamilies)
	{
		for (size_t i = 0; i < group.priorities.size(); ++i)
		{
			size_t reqIdx = group.requestIndices[i];
			const auto& req = m_pData->QueueRequests[reqIdx];

			VkQueue queueHandle = VK_NULL_HANDLE;
			getDeviceQueue(group.familyIndex, static_cast<uint32_t>(i), &queueHandle);

			m_pData->namedQueues[reqIdx] = {
				.name = req.name,
				.handle = queueHandle,
				.family = group.familyIndex,
				.index = static_cast<uint32_t>(i),
			};
		}
	}
}
Device::~Device() noexcept
{
	if (m_pData)
	{
		vkDestroyDevice(getHandle(), nullptr);
		delete m_pData;
	}
}

void Device::getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const
{
	return vkGetDeviceQueue(getHandle(), queueFamilyIndex, queueIndex, pQueue);
}

VkQueue Device::getQueue(const char* name) const noexcept
{
	const auto* info = findQueueInfo(name);
	snassert(info != nullptr, "Requested queue not found", "Check queue name registration in Device::Builder");
	return info->handle;
}

const Device::NamedQueue* Device::findQueueInfo(const char* name) const noexcept
{
	auto it = std::find_if(m_pData->namedQueues.begin(), m_pData->namedQueues.end(),
		[name](const decltype(m_pData->namedQueues)::value_type& val) {
			return std::strcmp(val.name, name) == 0;
		});
	if (it == m_pData->namedQueues.end()) return nullptr;
	return &(*it);
}

VkResult Device::createSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const
{
	return vkCreateSwapchainKHR(getHandle(), pCreateInfo, pAllocator, pSwapchain);
}
void Device::destoySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroySwapchainKHR(getHandle(), swapchain, pAllocator);
}

VkResult Device::getSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const
{
	return vkGetSwapchainImagesKHR(getHandle(), swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult Device::createImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const
{
	return vkCreateImage(getHandle(), pCreateInfo, pAllocator, pImage);
}
void Device::destroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyImage(getHandle(), image, pAllocator);
}

VkResult Device::createImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const
{
	return vkCreateImageView(getHandle(), pCreateInfo, pAllocator, pView);
}
void Device::destroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyImageView(getHandle(), imageView, pAllocator);
}

VkResult Device::createFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const
{
	return vkCreateFence(getHandle(), pCreateInfo, pAllocator, pFence);
}
void Device::destroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyFence(getHandle(), fence, pAllocator);
}

VkResult Device::waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const
{
	return vkWaitForFences(getHandle(), fenceCount, pFences, waitAll, timeout);
}
VkResult Device::resetFences(uint32_t fenceCount, const VkFence* pFences) const
{
	return vkResetFences(getHandle(), fenceCount, pFences);
}

VkResult Device::createSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const
{
	return vkCreateSemaphore(getHandle(), pCreateInfo, pAllocator, pSemaphore);
}
void Device::destroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroySemaphore(getHandle(), semaphore, pAllocator);
}

VkResult Device::createCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const
{
	return vkCreateCommandPool(getHandle(), pCreateInfo, pAllocator, pCommandPool);
}
void Device::destroyCommandPool(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const
{
	vkDestroyCommandPool(getHandle(), commandPool, pAllocator);
}

VkResult Device::allocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const
{
	return vkAllocateCommandBuffers(getHandle(), pAllocateInfo, pCommandBuffers);
}
void Device::freeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const
{
	vkFreeCommandBuffers(getHandle(), commandPool, commandBufferCount, pCommandBuffers);
}

VkDevice Device::getHandle() const noexcept
{
	return m_pData->vkDevice;
}

Device::operator VkDevice() const noexcept
{
	return getHandle();
}

//  > Move
Device::Device(Device&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
Device& Device::operator=(Device&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData) delete m_pData;
		m_pData = other.m_pData;
		other.m_pData = nullptr;
	}
	return *this;
}

//  > Methods
#define _RVAR(storetype, argtype, name) argtype Device::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& Device::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type Device::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* Device::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& Device::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool Device::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\Device.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef Device::Builder Builder;

//  > Data
#ifdef _DEBUG
struct Builder::Temp
{
	enum class eDefFlag
	{
		eNone,
		eAddCalled,
		eWithCalled,
	};
#define _RVAR(storetype, argtype, name) eDefFlag f##name{ eDefFlag::eNone };
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name) _RVAR(std::vector<type>, const std::vector<type>&, name)
#define _OARR(type, name, ...) _RVAR(std::vector<type>, const std::vector<type>&, name)
#define _FLG(name) _RVAR(bool, bool, name)
#include <snvoxeng\.def\vk\Device.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new Device::Data{})
#ifdef _DEBUG
	, m_pTemp(new Temp{})
#endif
{
}
Builder::~Builder() noexcept
{
#ifdef _DEBUG
	delete m_pTemp;
#endif
	if (m_pData)
	{
		delete m_pData;
		snassert(
			false,
			"Builder initialized, but not used",
			"Try to call .build()\n"
			"or do not use original Builder"
		);
	}
}

//  > Move
Builder::Builder(Builder&& other) noexcept
	: m_pData(other.m_pData)
#ifdef _DEBUG
	, m_pTemp(other.m_pTemp)
#endif
{
	other.m_pData = nullptr;
#ifdef _DEBUG
	other.m_pTemp = nullptr;
#endif
}
Builder& Builder::operator=(Builder&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData) delete m_pData;
		m_pData = other.m_pData;
		other.m_pData = nullptr;
#ifdef _DEBUG
		m_pTemp = other.m_pTemp;
		other.m_pTemp = nullptr;
#endif
	}
	return *this;
}

//  > Build
Device Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return Device{ m_pData };
}
Device* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new Device{ m_pData };
}

#ifdef _DEBUG
void Builder::Temp::validate() const
{
#define _RVAR(storetype, argtype, name)\
	snassert(\
		f##name != eDefFlag::eNone,\
		#name " is required, but not defined",\
		"Try to call .with" #name "(...)"\
	);
#define _RARR(type, name)\
	snassert(\
		f##name != eDefFlag::eNone,\
		#name " is required, but not defined",\
		"Try to call .with" #name "(...)\n"\
		"or          .add" #name "(...)"\
	);
#include <snvoxeng\.def\vk\Device.h>
}

#define _SET_DEF_FLAG_ADD(name) { m_pTemp->f##name = Temp::eDefFlag::eAddCalled; }

#define _SET_DEF_FLAG_WITH(name){\
	snassert(m_pTemp->f##name != Builder::Temp::eDefFlag::eWithCalled,\
		"Builder::with" #name " is called twice",\
		"Do not call .with-method twice"\
	);\
	snassert(m_pTemp->f##name != Builder::Temp::eDefFlag::eAddCalled,\
		"Builder::with" #name " is called after Builder::add" #name,\
		"Do not call .with-method after calling .add-method\n"\
	);\
	m_pTemp->f##name = Temp::eDefFlag::eWithCalled;}

#define _SET_DEF_FLAG_SET(name){\
	snassert(m_pTemp->f##name != Builder::Temp::eDefFlag::eWithCalled,\
		"Builder::set" #name " is called twice",\
		"Do not call .set-method twice"\
	);\
	m_pTemp->f##name = Temp::eDefFlag::eWithCalled;}

#else
#define _SET_DEF_FLAG_ADD(name) ((void)(0));
#define _SET_DEF_FLAG_WITH(name) ((void)(0));
#define _SET_DEF_FLAG_SET(name) ((void)(0));
#endif

#define _RVAR(storetype, argtype, name)\
	Builder& Builder::with##name(argtype name)\
	{\
		_SET_DEF_FLAG_WITH(name)\
		m_pData->name = name;\
		return *this;\
	}
#define _OVAR(storetype, argtype, name, value)\
	Builder& Builder::with##name(argtype name)\
	{\
		_SET_DEF_FLAG_WITH(name)\
		m_pData->name = name;\
		return *this;\
	}
#define _RARR(type, name)\
	_RVAR(std::vector<type>, const std::vector<type>&, name)\
	Builder& Builder::add##name(const std::vector<type>& name)\
	{\
		_SET_DEF_FLAG_ADD(name)\
		m_pData->name.insert(m_pData->name.end(), name.begin(), name.end());\
		return *this;\
	}
#define _OARR(type, name, ...)\
	_OVAR(std::vector<type>, const std::vector<type>&, name,)\
	Builder& Builder::add##name(const std::vector<type>& name)\
	{\
		_SET_DEF_FLAG_ADD(name)\
		m_pData->name.insert(m_pData->name.end(), name.begin(), name.end());\
		return *this;\
	}
#define _FLG(name) Builder& Builder::set##name()\
	{\
		_SET_DEF_FLAG_SET(name)\
		m_pData->name = true;\
		return *this;\
	}
	
#include <snvoxeng\.def\vk\Device.h>
