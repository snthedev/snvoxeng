#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/Semaphore.h>

namespace sn::voxeng::vk
{
	// Use Semaphore::Builder for build
	class SNVOXENG_API Semaphore
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		Semaphore(data_t*& pData);
		Semaphore(data_t*& pData, VkSemaphore view);

	public:
		~Semaphore() noexcept;

		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;
		Semaphore(Semaphore&& other) noexcept;
		Semaphore& operator=(Semaphore&& other) noexcept;

		VkSemaphore vkHandle() const noexcept;
		operator VkSemaphore() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/Semaphore.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class Semaphore ^

	class SNVOXENG_API Semaphore::Builder
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
#include <snvoxeng/.def/vk/Semaphore.h>

		// Builds Semaphore on stack;
		// Builder is invalid after .sbuild()
		Semaphore sbuild();
		// Builds Semaphore on heap;
		// Builder is invalid after .build()
		Semaphore* build();

		// Builds Semaphore (view) on stack;
		// Builder is invalid after .sbuild(VkSemaphore)
		Semaphore sbuild(VkSemaphore view);
		// Builds Semaphore (view) on heap;
		// Builder is invalid after .build(VkSemaphore)
		Semaphore* build(VkSemaphore view);
	}; // ^ class Semaphore::Builder ^
} // ^ namespace sn::voxeng::vk ^
