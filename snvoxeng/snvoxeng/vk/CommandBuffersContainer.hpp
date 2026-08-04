#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/CommandBuffersContainer.h>

namespace sn::voxeng::vk
{
	class CommandBuffer;

	// Use CommandBuffersContainer::Builder for build
	class SNVOXENG_API CommandBuffersContainer
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		CommandBuffersContainer(data_t*& pData);
		CommandBuffersContainer(data_t*& pData, std::span<const VkCommandBuffer> view);

	public:
		~CommandBuffersContainer() noexcept;

		CommandBuffer get(size_t idx) const;
		CommandBuffer first() const;
		CommandBuffer last() const;
		size_t count() const noexcept;

		CommandBuffersContainer(const CommandBuffersContainer&) = delete;
		CommandBuffersContainer& operator=(const CommandBuffersContainer&) = delete;
		CommandBuffersContainer(CommandBuffersContainer&&) = delete;
		CommandBuffersContainer& operator=(CommandBuffersContainer&&) = delete;

		std::span<const VkCommandBuffer> vkHandle() const noexcept;
		VkCommandBuffer vkHandle(size_t idx) const noexcept;
		operator std::span<const VkCommandBuffer>() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/CommandBuffersContainer.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class CommandBuffersContainer ^

	class SNVOXENG_API CommandBuffersContainer::Builder
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
#include <snvoxeng/.def/vk/CommandBuffersContainer.h>

		// Builder is invalid after .sbuild()
		CommandBuffersContainer build();
		// Builder is invalid after .sbuild(std::span<const VkCommandBuffer>)
		CommandBuffersContainer build(std::span<const VkCommandBuffer> view);
	}; // ^ class CommandBuffersContainer::Builder ^
} // ^ namespace sn::voxeng::vk ^
