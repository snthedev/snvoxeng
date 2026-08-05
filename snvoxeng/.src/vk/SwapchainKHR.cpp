#include <snvoxeng/snvoxeng/vk/SwapchainKHR.hpp>
#include <snvoxeng/snvoxeng/utils/vk-getSType.hpp>

#include <snvoxeng/snvoxeng/vk/Image.hpp>
#include <snvoxeng/snvoxeng/vk/ImageView.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <vulkan/vulkan.h>
#include <snassert/snassert.hpp>

#include <vector>
#include <snvoxeng/snvoxeng/utils/dumb_vector.hpp>

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
		subdata name = default_values::Name;
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		subdata name = {};
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		subdata name = default_values::Name;
#include <snvoxeng/.def/vk/SwapchainKHR.h>
	}

	VkSwapchainKHR vkHandle{ VK_NULL_HANDLE };
};

struct SwapchainKHR::SwapchainResources
{
	dumb_vector<Image> images;
	dumb_vector<ImageView> imageViews;

	SwapchainResources(const vk::Device& device, VkSwapchainKHR vkHandle, const VkSwapchainCreateInfoKHR& vkCreateInfo, std::span<const uint32_t> queueFamilyIndices)
	{
		uint32_t imageCount = 0;
		if (device.getSwapchainImagesKHR(vkHandle, &imageCount, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Failed to get swapchain images");

		std::vector<VkImage> vkImages(imageCount);
		if (device.getSwapchainImagesKHR(vkHandle, &imageCount, vkImages.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to get swapchain images");

		create(
			imageCount,
			Image::Builder()
				.withDevice(device)
				.withImageType(VK_IMAGE_TYPE_2D)
				.withFormat(vkCreateInfo.imageFormat)
				.withExtent(VkExtent3D{
					.width = vkCreateInfo.imageExtent.width,
					.height = vkCreateInfo.imageExtent.height,
					.depth = 1u,
					})
					.withMipLevels(1u)
				.withArrayLayers(vkCreateInfo.imageArrayLayers)
				.withSamples({})
				.withTiling({})
				.withUsage(vkCreateInfo.imageUsage)
				.withSharingMode(vkCreateInfo.imageSharingMode)
				.withInitialLayout({})
				.withQueueFamilyIndices(queueFamilyIndices),
			ImageView::Builder()
				.withViewType(VK_IMAGE_VIEW_TYPE_2D)
				.withSubresourceRange({
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
					}),
			vkImages
		);
	}
	SwapchainResources(
		uint32_t imageCount,
		Image::Builder& imageBuilderBase,
		ImageView::Builder& imageViewBuilderBase,
		std::span<const VkImage> vkImages
	)
	{
		create(imageCount, imageBuilderBase, imageViewBuilderBase, vkImages);
	}
	~SwapchainResources() noexcept = default;

	SwapchainResources(const SwapchainResources&) = delete;
	SwapchainResources& operator=(const SwapchainResources&) = delete;
	SwapchainResources(SwapchainResources&& other) noexcept
		: images(std::move(other.images))
		, imageViews(std::move(other.imageViews))
	{
	}
	SwapchainResources& operator=(SwapchainResources&& other) noexcept
	{
		images = std::move(other.images);
		imageViews = std::move(other.imageViews);
		return *this;
	}

private:
	void create(
		uint32_t imageCount,
		Image::Builder& imageBuilderBase,
		ImageView::Builder& imageViewBuilderBase,
		std::span<const VkImage> vkImages
	)
	{
		images.reserve(imageCount);
		for (uint32_t i = 0; i < imageCount; ++i)
			images.emplace_builder(imageBuilderBase.clone(), vkImages[i]);
		imageViews.reserve(imageCount);
		for (uint32_t i = 0; i < imageCount; ++i)
			imageViews.emplace_builder(imageViewBuilderBase.clone().withImage(images[i]));
	}
};

void SwapchainKHR::onCreate(data_t& data)
{
	if (data.pDevice->createSwapchainKHR(&data.vkCreateInfo, data.vkPAllocator, &data.vkHandle) != VK_SUCCESS) [[unlikely]]
		throw std::runtime_error("Failed to create VkSwapchainKHR");

	if (data.pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream())
		*data.pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream()
		<< "[trace]: SwapchainKHR 0x" << std::hex << data.vkHandle << std::dec << " created" << std::endl;
}
void SwapchainKHR::onDestroy(data_t& data) noexcept
{
	data.pDevice->destroySwapchainKHR(data.vkHandle, data.vkPAllocator);

	if (data.pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream())
		*data.pDevice->getPhysicalDevice().getRegistry().getInstance().getDebugStream()
		<< "[trace]: SwapchainKHR 0x" << std::hex << data.vkHandle << std::dec << " destroyed" << std::endl;
}

SwapchainKHR::SwapchainKHR(data_t*& pData)
	: m_pData(pData)
	, m_isView(false)
	, m_pResources(nullptr)
{
	onCreate(*m_pData);
	try { m_pResources = new SwapchainResources(*m_pData->pDevice, m_pData->vkHandle, m_pData->vkCreateInfo, m_pData->queueFamilyIndices); }
	catch (...) { onDestroy(*m_pData); throw; }
	pData = nullptr;
}
SwapchainKHR::SwapchainKHR(data_t*& pData, VkSwapchainKHR view)
	: m_pData(pData)
	, m_isView(true)
	, m_pResources(nullptr)
{
	m_pData->vkHandle = view;
	try { m_pResources = new SwapchainResources(*m_pData->pDevice, m_pData->vkHandle, m_pData->vkCreateInfo, m_pData->queueFamilyIndices); }
	catch (...) { throw; }
	pData = nullptr;
}

// === SwapchainKHR : public ===
SwapchainKHR::~SwapchainKHR() noexcept
{
	if (m_pData) [[likely]]
	{
		delete m_pResources;
		m_pResources = nullptr;

		if (!m_isView) [[likely]] onDestroy(*m_pData);
		delete m_pData;
		m_pData = nullptr;
	}
}

bool SwapchainKHR::recreate()
{
	if (m_isView) [[unlikely]] throw std::logic_error("View for SwapchainKHR can not be recreated.");

	auto capabilities = m_pData->pDevice->getPhysicalDevice().getSurfaceCapabilities(m_pData->pSurfaceKHR->vkHandle());
	if (capabilities.currentExtent.width == 0
		|| capabilities.currentExtent.height == 0
		) return false;

	/*		  _
			  \`*-.
			   )  _`-.
			  .  : `. .
			  : _   '  \
			  ; *` _.   `*-._
			  `-.-'          `-.
				;       `       `.
				:.       .        \
				. \  .   :   .-'   .
				'  `+.;  ;  '      :
				:  '  |    ;       ;-.
	      		; '   : :`-:     _.`* ;
	[code]	 .*' /  .*' ; .*`- +'  `*'
			 `*-*   `*-*  `*-*/

	// prepare temp data for new swapchain
	data_t* pData = new data_t{ *m_pData };
	// with old swapchain
	pData->vkCreateInfo.oldSwapchain = m_pData->vkHandle;

	// update capabilities
	uint32_t imageCount = capabilities.minImageCount + 1u;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		imageCount = capabilities.maxImageCount;
	pData->vkCreateInfo.minImageCount = imageCount;
	pData->vkCreateInfo.imageExtent = capabilities.currentExtent;
	pData->vkCreateInfo.preTransform = capabilities.currentTransform;

	try
	{
		SwapchainKHR newSwapchain(pData);

		// take ownership
		std::swap(m_pData, newSwapchain.m_pData);
		std::swap(m_pResources, newSwapchain.m_pResources);
		
		// newSwapchain.~SwapchainKHR() will automatically clean up the old data
	}
	catch (...)
	{
		// clean up temp data and rethrow
		delete pData;
		throw;
	}

	return true;
}

VkResult SwapchainKHR::acquireNextImageKHR(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const
{
	return m_pData->pDevice->acquireNextImageKHR(m_pData->vkHandle, timeout, semaphore, fence, pImageIndex);
}

uint32_t SwapchainKHR::getMinImageCount() const noexcept { return m_pData->vkCreateInfo.minImageCount; }
VkExtent2D SwapchainKHR::getImageExtent() const noexcept { return m_pData->vkCreateInfo.imageExtent; }
VkSurfaceTransformFlagBitsKHR SwapchainKHR::getPreTransform() const noexcept { return m_pData->vkCreateInfo.preTransform; }

std::span<const Image> SwapchainKHR::getImages() const noexcept { return m_pResources->images; }
std::span<const ImageView> SwapchainKHR::getImageViews() const noexcept { return m_pResources->imageViews; }

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
	uint32_t imageCount = capabilities.minImageCount + 1u;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		imageCount = capabilities.maxImageCount;
	data.vkCreateInfo.minImageCount = imageCount;
	data.vkCreateInfo.imageExtent = capabilities.currentExtent;
	data.vkCreateInfo.preTransform = capabilities.currentTransform;
}

#ifdef DETAIL_SNBCG_DEBUG
struct Builder::temp_t
{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t Name{ 0 };
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t Name{ 0 };
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t Name{ 0 };
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t Name{ 0 };
#include <snvoxeng/.def/vk/SwapchainKHR.h>

	void validate() const
	{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		snassert((Name & 0b01) != 0,\
			#Name " is required, but not defined",\
			"Call Builder::with" #Name "(...)"\
		);\
		snassert((Name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once"\
		);
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
		snassert((Name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once"\
		);
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		snassert((Name & 0b01) != 0,\
			#Name " is required, but not defined",\
			"Call Builder::with" #Name "(...)\n"\
			"  or Builder::add" #Name "(...)"\
		);\
		snassert((Name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once\n"\
			"  and do not call Builder::with" #Name "(...) after calling\n"\
			"  Builder::add" #Name "(...)"\
		);
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
		snassert((Name & 0b10) == 0,\
			#Name " is defined twice",\
			"Call Builder::with" #Name "(...) once\n"\
			"  and do not call Builder::with" #Name "(...) after calling\n"\
			"  Builder::add" #Name "(...)"\
		);
#include <snvoxeng/.def/vk/SwapchainKHR.h>
	}
};
#define SNBCG_VALIDATE_ON_WITH(name, Name) m_pTemp->Name = ((m_pTemp->Name << 1u) & 0b11) | 0b01;
#define SNBCG_VALIDATE_ON_ADD(name, Name) m_pTemp->Name = m_pTemp->Name | 0b01;
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

SwapchainKHR Builder::build()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return SwapchainKHR{ m_pData };
}
SwapchainKHR Builder::build(VkSwapchainKHR view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return SwapchainKHR{ m_pData, view };
}
