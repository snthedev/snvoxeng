#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/Fence.h>

namespace sn::voxeng::vk
{
	// Use Fence::Builder for build
	class SNVOXENG_API Fence
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		Fence(data_t*& pData);
		Fence(data_t*& pData, VkFence view);

	public:
		~Fence() noexcept;

		VkResult wait(uint64_t timeout = UINT64_MAX) const;
		VkResult reset() const;

		Fence(const Fence&) = delete;
		Fence& operator=(const Fence&) = delete;
		Fence(Fence&& other) noexcept;
		Fence& operator=(Fence&& other) noexcept;

		VkFence vkHandle() const noexcept;
		operator VkFence() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/Fence.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class Fence ^

	class SNVOXENG_API Fence::Builder
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
#include <snvoxeng/.def/vk/Fence.h>

		// Builds Fence on stack;
		// Builder is invalid after .sbuild()
		Fence sbuild();
		// Builds Fence on heap;
		// Builder is invalid after .build()
		Fence* build();

		// Builds Fence (view) on stack;
		// Builder is invalid after .sbuild(VkFence)
		Fence sbuild(VkFence view);
		// Builds Fence (view) on heap;
		// Builder is invalid after .build(VkFence)
		Fence* build(VkFence view);
	}; // ^ class Fence::Builder ^
} // ^ namespace sn::voxeng::vk ^
