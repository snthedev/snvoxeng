#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/CommandPool.h>

namespace sn::voxeng::vk
{
	class CommandBuffersContainer;

	// Use CommandPool::Builder for build
	class SNVOXENG_API CommandPool
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		CommandPool(data_t*& pData);
		CommandPool(data_t*& pData, VkCommandPool view);

	public:
		~CommandPool() noexcept;

		CommandBuffersContainer allocateCommandBuffers(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool(CommandPool&& other) noexcept;
		CommandPool& operator=(CommandPool&& other) noexcept;

		VkCommandPool vkHandle() const noexcept;
		operator VkCommandPool() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/CommandPool.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class CommandPool ^

	class SNVOXENG_API CommandPool::Builder
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
#include <snvoxeng/.def/vk/CommandPool.h>

		// Builds CommandPool on stack;
		// Builder is invalid after .sbuild()
		CommandPool sbuild();
		// Builds CommandPool on heap;
		// Builder is invalid after .build()
		CommandPool* build();

		// Builds CommandPool (view) on stack;
		// Builder is invalid after .sbuild(VkCommandPool)
		CommandPool sbuild(VkCommandPool view);
		// Builds CommandPool (view) on heap;
		// Builder is invalid after .build(VkCommandPool)
		CommandPool* build(VkCommandPool view);
	}; // ^ class CommandPool::Builder ^
} // ^ namespace sn::voxeng::vk ^
