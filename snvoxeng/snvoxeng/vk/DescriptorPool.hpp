#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/DescriptorPool.h>

namespace sn::voxeng::vk
{
	// Use DescriptorPool::Builder for build
	class SNVOXENG_API DescriptorPool
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		DescriptorPool(data_t*& pData);
		DescriptorPool(data_t*& pData, VkDescriptorPool view);

	public:
		~DescriptorPool() noexcept;

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept;
		DescriptorPool& operator=(DescriptorPool&& other) noexcept;

		VkDescriptorPool vkHandle() const noexcept;
		operator VkDescriptorPool() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/DescriptorPool.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class DescriptorPool ^

	class SNVOXENG_API DescriptorPool::Builder
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
#include <snvoxeng/.def/vk/DescriptorPool.h>

		// Builds DescriptorPool on stack;
		// Builder is invalid after .sbuild()
		DescriptorPool sbuild();
		// Builds DescriptorPool on heap;
		// Builder is invalid after .build()
		DescriptorPool* build();

		// Builds DescriptorPool (view) on stack;
		// Builder is invalid after .sbuild(VkDescriptorPool)
		DescriptorPool sbuild(VkDescriptorPool view);
		// Builds DescriptorPool (view) on heap;
		// Builder is invalid after .build(VkDescriptorPool)
		DescriptorPool* build(VkDescriptorPool view);
	}; // ^ class DescriptorPool::Builder ^
} // ^ namespace sn::voxeng::vk ^
