#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/Device.h>

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
		struct NamedQueue final
		{
			const char* name = nullptr;
			VkQueue handle = VK_NULL_HANDLE;
			uint32_t family = ~(uint32_t)(0);
			uint32_t index = 0;
		};

	private:
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		Device(data_t*& pData);
		Device(data_t*& pData, VkDevice view);

	public:
		~Device() noexcept;

		const NamedQueue* getQueueInfo(const char* name) const noexcept;
		std::span<const NamedQueue> getQueueInfos() const noexcept;

		void getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const;

		VkResult createSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const;
		void destroySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const;
		VkResult getSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const;

		VkResult createImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const;
		void destroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const;

		VkResult createImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const;
		void destroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const;

		VkResult createFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const;
		void destroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const;

		VkResult waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const;
		VkResult resetFences(uint32_t fenceCount, const VkFence* pFences) const;

		VkResult createSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const;
		void destroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const;

		VkResult createCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const;
		void destroyCommandPool(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const;

		VkResult allocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const;
		void freeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const;

		void getImageMemoryRequirements(VkImage image, VkMemoryRequirements* pMemoryRequirements) const;

		VkResult allocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const;
		void freeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const;

		VkResult bindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const;

		VkResult createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const;
		void destroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const;

		VkResult createPipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const;
		void destroyPipelineLayout(VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

		VkDevice vkHandle() const noexcept;
		operator VkDevice() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/Device.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class Device ^

	class SNVOXENG_API Device::Builder
	{
		data_t* m_pData;
		void finalize(data_t& data);

#ifdef DETAIL_SNBCG_DEBUG
		struct temp_t;
		temp_t* m_pTemp;
#endif // ^ DETAIL_SNBCG_DEBUG ^

	public:
		Builder();
		~Builder() noexcept;

		// Copies this instance of the Builder.
		Builder clone() const;

		Builder(const Builder&) = delete;
		Builder& operator=(const Builder&) = delete;
		Builder(Builder&& other) noexcept;
		Builder& operator=(Builder&& other) noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		Builder& with##Name(arg_t name);
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		Builder& with##Name(arg_t name);
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		Builder& with##Name(args_t name);\
		Builder& add##Name(args_t name);\
		Builder& add##Name(arg_t name);
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		Builder& with##Name(args_t name);\
		Builder& add##Name(args_t name);\
		Builder& add##Name(arg_t name);
#include <snvoxeng/.def/vk/Device.h>

		// Builds Device on stack;
		// Builder is invalid after .sbuild()
		Device sbuild();
		// Builds Device on heap;
		// Builder is invalid after .build()
		Device* build();

		// Builds Device (view) on stack;
		// Builder is invalid after .sbuild(VkDevice)
		Device sbuild(VkDevice view);
		// Builds Device (view) on heap;
		// Builder is invalid after .build(VkDevice)
		Device* build(VkDevice view);
	}; // ^ class Device::Builder ^
} // ^ namespace sn::voxeng::vk ^
