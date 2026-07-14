#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/DescriptorSetLayout.h>

namespace sn::voxeng::vk
{
	// Use DescriptorSetLayout::Builder for build
	class SNVOXENG_API DescriptorSetLayout
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		DescriptorSetLayout(data_t*& pData);
		DescriptorSetLayout(data_t*& pData, VkDescriptorSetLayout view);

	public:
		~DescriptorSetLayout() noexcept;

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
		DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

		VkDescriptorSetLayout vkHandle() const noexcept;
		operator VkDescriptorSetLayout() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/DescriptorSetLayout.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class DescriptorSetLayout ^

	class SNVOXENG_API DescriptorSetLayout::Builder
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
#include <snvoxeng/.def/vk/DescriptorSetLayout.h>

		// Builds DescriptorSetLayout on stack;
		// Builder is invalid after .sbuild()
		DescriptorSetLayout sbuild();
		// Builds DescriptorSetLayout on heap;
		// Builder is invalid after .build()
		DescriptorSetLayout* build();

		// Builds DescriptorSetLayout (view) on stack;
		// Builder is invalid after .sbuild(VkDescriptorSetLayout)
		DescriptorSetLayout sbuild(VkDescriptorSetLayout view);
		// Builds DescriptorSetLayout (view) on heap;
		// Builder is invalid after .build(VkDescriptorSetLayout)
		DescriptorSetLayout* build(VkDescriptorSetLayout view);
	}; // ^ class DescriptorSetLayout::Builder ^
} // ^ namespace sn::voxeng::vk ^
