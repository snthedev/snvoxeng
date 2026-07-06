#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <vector>

#define _INCLUDE
#include <snvoxeng\.def\vk\SwapchainKHR.h>

namespace sn::voxeng::vk
{
	class Image;
	class ImageView;

	// Use SwapchainKHR::Builder for build
	class SNVOXENG_API SwapchainKHR
	{
		struct Data;
		Data* m_pData;

	public:
		SwapchainKHR(Data*& pData);
		~SwapchainKHR() noexcept;

		uint32_t getMinImageCount() const noexcept;
		VkExtent2D getImageExtent() const noexcept;
		VkSurfaceTransformFlagBitsKHR getPreTransform() const noexcept;

		const std::vector<Image>& getImages() const noexcept;
		std::vector<Image>::size_type getImagesCount() const noexcept;
		const std::vector<Image>::value_type* getImagesData() const noexcept;
		const std::vector<Image>::value_type& getImages(size_t idx) const noexcept;

		const std::vector<ImageView>& getImageViews() const noexcept;
		std::vector<ImageView>::size_type getImageViewsCount() const noexcept;
		const std::vector<ImageView>::value_type* getImageViewsData() const noexcept;
		const std::vector<ImageView>::value_type& getImageViews(size_t idx) const noexcept;

		VkSwapchainKHR getHandle() const noexcept;
		operator VkSwapchainKHR() const noexcept;

		SwapchainKHR(const SwapchainKHR&) = delete;
		SwapchainKHR& operator=(const SwapchainKHR&) = delete;
		SwapchainKHR(SwapchainKHR&& other) noexcept;
		SwapchainKHR& operator=(SwapchainKHR&& other) noexcept;

#define _RVAR(storetype, argtype, name) argtype get##name() const noexcept;
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
		const std::vector<type>& get##name() const noexcept;\
		std::vector<type>::size_type get##name##Size() const noexcept;\
		const std::vector<type>::value_type* get##name##Data() const noexcept;\
		const std::vector<type>::value_type& get##name(size_t idx) const noexcept;
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool is##name() const noexcept;
#include <snvoxeng\.def\vk\SwapchainKHR.h>

		class SNVOXENG_API Builder
		{
			Data* m_pData;

#ifdef _DEBUG
			struct Temp;
			Temp* m_pTemp;
#endif

		public:
			Builder();
			~Builder() noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&& other) noexcept;
			Builder& operator=(Builder&& other) noexcept;

#define _RVAR(storetype, argtype, name) Builder& with##name(argtype name);
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
			Builder& with##name(const std::vector<type>& name);\
			Builder& add##name(const std::vector<type>& name);
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) Builder& set##name();
#include <snvoxeng\.def\vk\SwapchainKHR.h>

			// Builder is invalid after .sbuild()
			// Creates SwapchainKHR on stack
			SwapchainKHR sbuild();
			// Builder is invalid after .build()
			// Creates SwapchainKHR on heap
			SwapchainKHR* build();
		};
	};
}
