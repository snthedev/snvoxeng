#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/DeviceMemory.h>

namespace sn::voxeng::vk
{
	class Image;

	// Use DeviceMemory::Builder for build
	class SNVOXENG_API DeviceMemory
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		DeviceMemory(data_t*& pData);
		DeviceMemory(data_t*& pData, VkDeviceMemory view);

	public:
		~DeviceMemory() noexcept;

		void bindImage(const Image& image, VkDeviceSize memoryOffset) const;

		DeviceMemory(const DeviceMemory&) = delete;
		DeviceMemory& operator=(const DeviceMemory&) = delete;
		DeviceMemory(DeviceMemory&& other) noexcept;
		DeviceMemory& operator=(DeviceMemory&& other) noexcept;

		VkDeviceMemory vkHandle() const noexcept;
		operator VkDeviceMemory() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/DeviceMemory.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class DeviceMemory ^

	class SNVOXENG_API DeviceMemory::Builder
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
#include <snvoxeng/.def/vk/DeviceMemory.h>

		// Builds DeviceMemory on stack;
		// Builder is invalid after .sbuild()
		DeviceMemory sbuild();
		// Builds DeviceMemory on heap;
		// Builder is invalid after .build()
		DeviceMemory* build();

		// Builds DeviceMemory (view) on stack;
		// Builder is invalid after .sbuild(VkDeviceMemory)
		DeviceMemory sbuild(VkDeviceMemory view);
		// Builds DeviceMemory (view) on heap;
		// Builder is invalid after .build(VkDeviceMemory)
		DeviceMemory* build(VkDeviceMemory view);
	}; // ^ class DeviceMemory::Builder ^
} // ^ namespace sn::voxeng::vk ^
