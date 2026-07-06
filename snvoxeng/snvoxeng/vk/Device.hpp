#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <vector>

#define _INCLUDE
#include <snvoxeng\.def\vk\Device.h>

namespace sn::voxeng::vk
{
	// Use Device::Builder for build
	class SNVOXENG_API Device
	{
	public:
		struct QueueRequest final
		{
			const char* name = nullptr;
			uint32_t familyIndex = ~(uint32_t)(0);
			float priority = 1.0f;
		};
		
		struct NamedQueue
		{
			const char* name = nullptr;
			VkQueue handle = VK_NULL_HANDLE;
			uint32_t family = ~(uint32_t)(0);
			uint32_t index = 0;
		};

	private:
		struct Data;
		Data* m_pData;

	public:
		Device(Data*& pData);
		~Device() noexcept;

		void getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const;

		VkQueue getQueue(const char* name) const noexcept;
		const NamedQueue* findQueueInfo(const char* name) const noexcept;

		VkResult createSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const;
		void destoySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const;
		VkResult getSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const;

		VkResult createImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const;
		void destroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const;

		VkResult createImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const;
		void destroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const;

		VkResult createFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const;
		void destroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const;

		VkResult waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
		VkResult resetFences(uint32_t fenceCount, const VkFence* pFences);

		VkResult createSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const;
		void destroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const;

		VkDevice getHandle() const noexcept;
		operator VkDevice() const noexcept;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

#define _RVAR(storetype, argtype, name) argtype get##name() const noexcept;
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
		const std::vector<type>& get##name() const noexcept;\
		std::vector<type>::size_type get##name##Size() const noexcept;\
		const std::vector<type>::value_type* get##name##Data() const noexcept;\
		const std::vector<type>::value_type& get##name(size_t idx) const noexcept;
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool is##name() const noexcept;
#include <snvoxeng\.def\vk\Device.h>

		class SNVOXENG_API Builder
		{
			Data* m_pData;

#ifdef _DEBUG
			struct Temp;
			Temp* m_pTemp;
#endif

		public:
			Builder();
			~Builder() noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&& other) noexcept;
			Builder& operator=(Builder&& other) noexcept;

#define _RVAR(storetype, argtype, name) Builder& with##name(argtype name);
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
			Builder& with##name(const std::vector<type>& name);\
			Builder& add##name(const std::vector<type>& name);
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) Builder& set##name();
#include <snvoxeng\.def\vk\Device.h>

			// Builder is invalid after .sbuild()
			// Creates Device on stack
			Device sbuild();
			// Builder is invalid after .build()
			// Creates Device on heap
			Device* build();
		};
	};
}
