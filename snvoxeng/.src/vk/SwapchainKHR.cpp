#include <snvoxeng/snvoxeng/vk/SwapchainKHR.hpp>
#include <snvoxeng/snvoxeng/utils/vk-getSType.hpp>

#include <snvoxeng/snvoxeng/vk/Image.hpp>
#include <snvoxeng/snvoxeng/vk/ImageView.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <vulkan/vulkan.h>
#include <snassert/snassert.hpp>

#include <vector>

using namespace sn::voxeng::vk;

namespace default_values
{
#define SNBCG_DEFAULT_VALUES
#include <snvoxeng/.def/vk/SwapchainKHR.h>
}

// === SwapchainKHR : private ===
struct SwapchainKHR::data_t
{
	VkSwapchainCreateInfoKHR vkCreateInfo{ .sType{ ::sn::voxeng::utils::vk::getSType<VkSwapchainCreateInfoKHR>() } };

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#include <snvoxeng/.def/vk/SwapchainKHR.h>

	data_t()
	{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		subdata name = {};
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		subdata name = default_values::name;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		subdata name = {};
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		subdata name = default_values::name;
#include <snvoxeng/.def/vk/SwapchainKHR.h>
	}

	VkSwapchainKHR vkHandle{ VK_NULL_HANDLE };

	uint32_t imageCount;
	Image* pImages;
	ImageView* pImageViews;
};

void SwapchainKHR::onCreate(data_t& data)
{
	{
		auto result = data.pDevice->createSwapchainKHR(&data.vkCreateInfo, data.vkPAllocator, &data.vkHandle);
		snassert(result == VK_SUCCESS,
			"Failed to create VkSwapchainKHR", "Check Builder settings");
	}

	{
		auto result = data.pDevice->getSwapchainImagesKHR(data.vkHandle, &data.imageCount, nullptr);
		snassert(result == VK_SUCCESS,
			"Failed to get swapchain images", "Check builder settings");
	}

	std::vector<VkImage> vkImages(data.imageCount);
	{
		auto result = data.pDevice->getSwapchainImagesKHR(data.vkHandle, &data.imageCount, vkImages.data());
		snassert(result == VK_SUCCESS,
			"Failed to get swapchain images", "Check builder settings");
	}

	data.pImages = static_cast<Image*>(::operator new(data.imageCount * sizeof(Image)));
	data.pImageViews = static_cast<ImageView*>(::operator new(data.imageCount * sizeof(ImageView)));
	for (size_t i = 0; i < data.imageCount; ++i)
	{
		new (&data.pImages[i]) Image(Image::Builder()
			.withDevice(*data.pDevice)
			.withImageType(VK_IMAGE_TYPE_2D)
			.withFormat(data.vkCreateInfo.imageFormat)
			.withExtent(VkExtent3D{
				.width = data.vkCreateInfo.imageExtent.width,
				.height = data.vkCreateInfo.imageExtent.height,
				.depth = 1u,
				})
			.withMipLevels(1u)
			.withArrayLayers(data.vkCreateInfo.imageArrayLayers)
			.withSamples({})
			.withTiling({})
			.withUsage(data.vkCreateInfo.imageUsage)
			.withSharingMode(data.vkCreateInfo.imageSharingMode)
			.withInitialLayout({})
			.withQueueFamilyIndices(data.queueFamilyIndices)
			.sbuild(vkImages[i]));
		new (&data.pImageViews[i]) ImageView(ImageView::Builder()
			.withImage(data.pImages[i])
			.withViewType(VK_IMAGE_VIEW_TYPE_2D)
			.withSubresourceRange({
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
				})
			.sbuild());
	}

	if (m_pData->pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream())
		*m_pData->pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream()
		<< "[trace]: SwapchainKHR 0x" << std::hex << this << std::dec << " created" << std::endl;
}
void SwapchainKHR::onDestroy(data_t& data) noexcept
{
	for (size_t i = 0; i < data.imageCount; ++i)
	{
		data.pImageViews[i].~ImageView();
		data.pImages[i].~Image();
	}
	::operator delete(data.pImageViews);
	::operator delete(data.pImages);

	data.pDevice->destroySwapchainKHR(data.vkHandle, data.vkPAllocator);

	if (m_pData->pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream())
		*m_pData->pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream()
		<< "[trace]: SwapchainKHR 0x" << std::hex << this << std::dec << " destroyed" << std::endl;
}

SwapchainKHR::SwapchainKHR(data_t*& pData)
	: m_pData(pData)
	, m_isView(false)
{
	pData = nullptr;
	onCreate(*m_pData);
}
SwapchainKHR::SwapchainKHR(data_t*& pData, VkSwapchainKHR view)
	: m_pData(pData)
	, m_isView(true)
{
	pData = nullptr;
	m_pData->vkHandle = view;
}

// === SwapchainKHR : public ===
SwapchainKHR::~SwapchainKHR() noexcept
{
	if (m_pData) [[likely]]
	{
		if (!m_isView) [[likely]] onDestroy(*m_pData);
		delete m_pData;
	}
}

uint32_t SwapchainKHR::getMinImageCount() const noexcept { return m_pData->vkCreateInfo.minImageCount; }
VkExtent2D SwapchainKHR::getImageExtent() const noexcept { return m_pData->vkCreateInfo.imageExtent; }
VkSurfaceTransformFlagBitsKHR SwapchainKHR::getPreTransform() const noexcept { return m_pData->vkCreateInfo.preTransform; }

std::span<const Image> SwapchainKHR::getImages() const noexcept { return { m_pData->pImages, m_pData->imageCount }; }
std::span<const ImageView> SwapchainKHR::getImageViews() const noexcept { return { m_pData->pImageViews, m_pData->imageCount }; }

SwapchainKHR::SwapchainKHR(SwapchainKHR&& other) noexcept
	: m_pData(other.m_pData)
	, m_isView(other.m_isView)
{
	other.m_pData = nullptr;
}
SwapchainKHR& SwapchainKHR::operator=(SwapchainKHR&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData)
		{
			if (!m_isView) [[likely]] onDestroy(*m_pData);
			delete m_pData;
		}
		m_pData = other.m_pData;
		m_isView = other.m_isView;
		other.m_pData = nullptr;
	}
	return *this;
}

VkSwapchainKHR SwapchainKHR::vkHandle() const noexcept { return m_pData->vkHandle; }
SwapchainKHR::operator VkSwapchainKHR() const noexcept { return m_pData->vkHandle; }

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) SwapchainKHR::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) SwapchainKHR::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) SwapchainKHR::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) SwapchainKHR::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#include <snvoxeng/.def/vk/SwapchainKHR.h>



typedef SwapchainKHR::Builder Builder;

// === Builder : private ===
void Builder::finalize(data_t& data)
{
	data.vkCreateInfo.surface = data.pSurfaceKHR->vkHandle();

	auto capabilities = data.pDevice->getPhysicalDevice().getSurfaceCapabilities(data.pSurfaceKHR->vkHandle());
	data.vkCreateInfo.minImageCount = capabilities.minImageCount + 1u;
	data.vkCreateInfo.imageExtent = capabilities.currentExtent;
	data.vkCreateInfo.preTransform = capabilities.currentTransform;
}

#ifdef DETAIL_SNBCG_DEBUG
struct Builder::temp_t
{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#include <snvoxeng/.def/vk/SwapchainKHR.h>

	void validate() const
	{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		snassert((name & 0b01) != 0,\
			#Name " is required, but not defined",\
			"Call Builder::with" #Name "(...)"\
		);\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once"\
		);
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once"\
		);
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		snassert((name & 0b01) != 0,\
			#Name " is required, but not defined",\
			"Call Builder::with" #Name "(...)\n"\
			"  or Builder::add" #Name "(...)"\
		);\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once\n"\
			"  and do not call Builder::with" #Name "(...) after calling\n"\
			"  Builder::add" #Name "(...)"\
		);
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		snassert((name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once\n"\
			"  and do not call Builder::with" #Name "(...) after calling\n"\
			"  Builder::add" #Name "(...)"\
		);
#include <snvoxeng/.def/vk/SwapchainKHR.h>
	}
};
#define SNBCG_VALIDATE_ON_WITH(name, Name) m_pTemp->name = ((m_pTemp->name << 1u) & 0b11) | 0b01;
#define SNBCG_VALIDATE_ON_ADD(name, Name) m_pTemp->name = m_pTemp->name | 0b01;
#else // ^ DETAIL_SNBCG_DEBUG ^
#define SNBCG_VALIDATE_ON_WITH(name, Name)
#define SNBCG_VALIDATE_ON_ADD(name, Name)
#endif // ^ ~DETAIL_SNBCG_DEBUG ^

// === Builder : public ===
Builder::Builder()
	: m_pData(new data_t{})
#ifdef DETAIL_SNBCG_DEBUG
	, m_pTemp(new temp_t{})
#endif // ^ DETAIL_SNBCG_DEBUG ^
{
}
Builder::~Builder() noexcept
{
	if (m_pData) [[unlikely]] delete m_pData;
#ifdef DETAIL_SNBCG_DEBUG
	if (m_pTemp) [[likely]] delete m_pTemp;
#endif // ^ DETAIL_SNBCG_DEBUG ^
}

Builder Builder::clone() const
{
	auto builder = Builder();
	(*builder.m_pData) = (*m_pData);
#ifdef DETAIL_SNBCG_DEBUG
	(*builder.m_pTemp) = (*m_pTemp);
#endif // ^ DETAIL_SNBCG_DEBUG ^
	return builder;
}

Builder::Builder(Builder&& other) noexcept
	: m_pData(other.m_pData)
#ifdef DETAIL_SNBCG_DEBUG
	, m_pTemp(other.m_pTemp)
#endif // ^ DETAIL_SNBCG_DEBUG ^
{
	other.m_pData = nullptr;
#ifdef DETAIL_SNBCG_DEBUG
	other.m_pTemp = nullptr;
#endif // ^ DETAIL_SNBCG_DEBUG ^
}
Builder& Builder::operator=(Builder&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData) delete m_pData;
		m_pData = other.m_pData;
		other.m_pData = nullptr;
#ifdef DETAIL_SNBCG_DEBUG
		if (m_pTemp) delete m_pTemp;
		m_pTemp = other.m_pTemp;
		other.m_pTemp = nullptr;
#endif // ^ DETAIL_SNBCG_DEBUG ^
	}
	return *this;
}

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
Builder& Builder::with##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
Builder& Builder::with##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
Builder& Builder::with##Name(args_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<args_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}\
Builder& Builder::add##Name(args_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<args_t> args = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_MULTI;\
	return *this;\
}\
Builder& Builder::add##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_SINGLE;\
	return *this;\
}
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
Builder& Builder::with##Name(args_t name) {\
	SNBCG_VALIDATE_ON_WITH(name, Name)\
	std::add_lvalue_reference_t<args_t> arg = name;\
	m_pData->subdata name = store_policy(store_t);\
	return *this;\
}\
Builder& Builder::add##Name(args_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<args_t> args = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_MULTI;\
	return *this;\
}\
Builder& Builder::add##Name(arg_t name) {\
	SNBCG_VALIDATE_ON_ADD(name, Name)\
	std::add_lvalue_reference_t<arg_t> arg = name;\
	std::add_lvalue_reference_t<store_t> val = m_pData->subdata name;\
	DETAIL_##store_action##_SINGLE;\
	return *this;\
}
#include <snvoxeng/.def/vk/SwapchainKHR.h>

SwapchainKHR Builder::sbuild()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return SwapchainKHR{ m_pData };
}
SwapchainKHR* Builder::build()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return new SwapchainKHR{ m_pData };
}

SwapchainKHR Builder::sbuild(VkSwapchainKHR view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return SwapchainKHR{ m_pData, view };
}
SwapchainKHR* Builder::build(VkSwapchainKHR view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return new SwapchainKHR{ m_pData, view };
}
