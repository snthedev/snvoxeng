#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>

namespace sn::voxeng::vk
{
	class DescriptorSet;

	// Use DescriptorSetsContainer::Builder for build
	class SNVOXENG_API DescriptorSetsContainer
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		DescriptorSetsContainer(data_t*& pData);
		DescriptorSetsContainer(data_t*& pData, std::vector<VkDescriptorSet> view);

	public:
		~DescriptorSetsContainer() noexcept;

		DescriptorSet get(size_t idx) const;
		DescriptorSet first() const;
		DescriptorSet last() const;
		size_t count() const noexcept;

		DescriptorSetsContainer(const DescriptorSetsContainer&) = delete;
		DescriptorSetsContainer& operator=(const DescriptorSetsContainer&) = delete;
		DescriptorSetsContainer(DescriptorSetsContainer&& other) noexcept;
		DescriptorSetsContainer& operator=(DescriptorSetsContainer&& other) noexcept;

		std::span<const VkDescriptorSet> vkHandle() const noexcept;
		VkDescriptorSet vkHandle(size_t idx) const noexcept;
		operator std::span<const VkDescriptorSet>() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class DescriptorSetsContainer ^

	class SNVOXENG_API DescriptorSetsContainer::Builder
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
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>

		// Builds DescriptorSetsContainer on stack;
		// Builder is invalid after .sbuild()
		DescriptorSetsContainer sbuild();
		// Builds DescriptorSetsContainer on heap;
		// Builder is invalid after .build()
		DescriptorSetsContainer* build();

		// Builds DescriptorSetsContainer (view) on stack;
		// Builder is invalid after .sbuild(std::vector<VkDescriptorSet>)
		DescriptorSetsContainer sbuild(std::vector<VkDescriptorSet> view);
		// Builds DescriptorSetsContainer (view) on heap;
		// Builder is invalid after .build(std::vector<VkDescriptorSet>)
		DescriptorSetsContainer* build(std::vector<VkDescriptorSet> view);
	}; // ^ class DescriptorSetsContainer::Builder ^
} // ^ namespace sn::voxeng::vk ^
