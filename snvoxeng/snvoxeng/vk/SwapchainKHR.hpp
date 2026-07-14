#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <ThirdParty/snbcg/bcg.hpp>

#define SNBCG_HEADER_INCLUDE
#include <snvoxeng/.def/vk/SwapchainKHR.h>

namespace sn::voxeng::vk
{
	class Image;
	class ImageView;

	// Use SwapchainKHR::Builder for build
	class SNVOXENG_API SwapchainKHR
	{
		struct data_t;
		data_t* m_pData;

		void onCreate(data_t& data);
		void onDestroy(data_t& data) noexcept;

		bool m_isView;

		SwapchainKHR(data_t*& pData);
		SwapchainKHR(data_t*& pData, VkSwapchainKHR view);

	public:
		~SwapchainKHR() noexcept;

		uint32_t getMinImageCount() const noexcept;
		VkExtent2D getImageExtent() const noexcept;
		VkSurfaceTransformFlagBitsKHR getPreTransform() const noexcept;

		std::span<const Image> getImages() const noexcept;
		std::span<const ImageView> getImageViews() const noexcept;

		SwapchainKHR(const SwapchainKHR&) = delete;
		SwapchainKHR& operator=(const SwapchainKHR&) = delete;
		SwapchainKHR(SwapchainKHR&& other) noexcept;
		SwapchainKHR& operator=(SwapchainKHR&& other) noexcept;

		VkSwapchainKHR vkHandle() const noexcept;
		operator VkSwapchainKHR() const noexcept;

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		DETAIL_##return_policy##_t(store_t) get##Name() const noexcept;
#include <snvoxeng/.def/vk/SwapchainKHR.h>
	
		class Builder;
		friend class Builder;
	}; // ^ class SwapchainKHR ^

	class SNVOXENG_API SwapchainKHR::Builder
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
#include <snvoxeng/.def/vk/SwapchainKHR.h>

		// Builds SwapchainKHR on stack;
		// Builder is invalid after .sbuild()
		SwapchainKHR sbuild();
		// Builds SwapchainKHR on heap;
		// Builder is invalid after .build()
		SwapchainKHR* build();

		// Builds SwapchainKHR (view) on stack;
		// Builder is invalid after .sbuild(VkSwapchainKHR)
		SwapchainKHR sbuild(VkSwapchainKHR view);
		// Builds SwapchainKHR (view) on heap;
		// Builder is invalid after .build(VkSwapchainKHR)
		SwapchainKHR* build(VkSwapchainKHR view);
	}; // ^ class SwapchainKHR::Builder ^
} // ^ namespace sn::voxeng::vk ^
