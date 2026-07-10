#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/SurfaceKHR.h>

namespace sn::voxeng::vk
{
	// Use SurfaceKHR::Builder for build
	class SNVOXENG_API SurfaceKHR
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		SurfaceKHR(data_t*& pData);
		SurfaceKHR(data_t*& pData, VkSurfaceKHR view);

	public:
		~SurfaceKHR() noexcept;

		SurfaceKHR(const SurfaceKHR&) = delete;
		SurfaceKHR& operator=(const SurfaceKHR&) = delete;
		SurfaceKHR(SurfaceKHR&& other) noexcept;
		SurfaceKHR& operator=(SurfaceKHR&& other) noexcept;

		VkSurfaceKHR vkHandle() const noexcept;
		operator VkSurfaceKHR() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/SurfaceKHR.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class SurfaceKHR ^

	class SNVOXENG_API SurfaceKHR::Builder
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
#include <snvoxeng/.def/vk/SurfaceKHR.h>

		// Builds SurfaceKHR on stack;
		// Builder is invalid after .sbuild()
		SurfaceKHR sbuild();
		// Builds SurfaceKHR on heap;
		// Builder is invalid after .build()
		SurfaceKHR* build();

		// Builds SurfaceKHR (view) on stack;
		// Builder is invalid after .sbuild(VkSurfaceKHR)
		SurfaceKHR sbuild(VkSurfaceKHR view);
		// Builds SurfaceKHR (view) on heap;
		// Builder is invalid after .build(VkSurfaceKHR)
		SurfaceKHR* build(VkSurfaceKHR view);
	}; // ^ class SurfaceKHR::Builder ^
} // ^ namespace sn::voxeng::vk ^
