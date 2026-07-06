#include <snvoxeng\snvoxeng\vk\SwapchainKHR.hpp>

#include <snvoxeng/snvoxeng/vk/Image.hpp>
#include <snvoxeng/snvoxeng/vk/ImageView.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

// === SwapchainKHR ===

//  > Data
struct SwapchainKHR::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\SwapchainKHR.h>

	VkSwapchainKHR vkSwapchainKHR;
	std::vector<Image> images;
	std::vector<ImageView> imageViews;

	uint32_t minImageCount;
	VkExtent2D imageExtent;
	VkSurfaceTransformFlagBitsKHR preTransform;
};

//  > Init
SwapchainKHR::SwapchainKHR(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	auto capabilities = m_pData->Device->getPhysicalDevice()->getSurfaceCapabilities(m_pData->SurfaceKHR->getHandle());
	m_pData->minImageCount = capabilities.minImageCount + 1u;
	m_pData->imageExtent = capabilities.currentExtent;
	m_pData->preTransform = capabilities.currentTransform;

	VkSwapchainCreateInfoKHR createInfo{
		.sType{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR },
		.pNext{ nullptr },
		.flags{},
		.surface{ m_pData->SurfaceKHR->getHandle() },
		//.minImageCount{ m_pData->MinImageCount },
		.minImageCount{ m_pData->minImageCount },
		.imageFormat{ m_pData->ImageFormat },
		.imageColorSpace{ m_pData->ImageColorSpace },
		//.imageExtent{ m_pData->ImageExtent },
		.imageExtent{ m_pData->imageExtent },
		.imageArrayLayers{ m_pData->ImageArrayLayers },
		.imageUsage{ m_pData->ImageUsage },
		.imageSharingMode{ m_pData->ImageSharingMode },
		.queueFamilyIndexCount{ static_cast<uint32_t>(m_pData->QueueFamilyIndices.size()) },
		.pQueueFamilyIndices{ m_pData->QueueFamilyIndices.data() },
		//.preTransform{ m_pData->PreTransform },
		.preTransform{ m_pData->preTransform },
		.compositeAlpha{ m_pData->CompositeAlpha },
		.presentMode{ m_pData->PresentMode },
		.clipped{ m_pData->Clipped ? VK_TRUE : VK_FALSE },
		.oldSwapchain{ m_pData->OldSwapchain },
	};

	snassert(m_pData->Device->createSwapchainKHR(&createInfo, nullptr, &m_pData->vkSwapchainKHR) == VK_SUCCESS,
		"Failed to create swapchain", "Check builder settings");

	{
		uint32_t imageCount;
		snassert(m_pData->Device->getSwapchainImagesKHR(getHandle(), &imageCount, nullptr) == VK_SUCCESS,
			"Failed to get swapchain images", "Check builder settings");

		std::vector<VkImage> vkImages(imageCount);
		snassert(m_pData->Device->getSwapchainImagesKHR(getHandle(), &imageCount, vkImages.data()) == VK_SUCCESS,
			"Failed to get swapchain images", "Check builder settings");

		m_pData->images.reserve(vkImages.size());
		m_pData->imageViews.reserve(vkImages.size());
		for (VkImage vkImage : vkImages)
		{
			m_pData->images.emplace_back(std::move(Image::Builder()
				.withViewHandle(vkImage)
				.withDevice(getDevice())
				.withImageType(VK_IMAGE_TYPE_2D)
				.withFormat(getImageFormat())
				.withExtent(VkExtent3D{
					.width = m_pData->imageExtent.width,
					.height = m_pData->imageExtent.height,
					.depth = 1u,
					})
				.withMipLevels(1u)
				.withArrayLevels(getImageArrayLayers())
				.withSamples({})
				.withTiling({})
				.withUsage(getImageUsage())
				.withSharingMode(getImageSharingMode())
				.withInitialLayout({})
				.withQueueFamilyIndices(m_pData->QueueFamilyIndices)
				.sbuild()));
			m_pData->imageViews.emplace_back(std::move(ImageView::Builder()
				.withImage(&m_pData->images.back())
				.withViewType(VK_IMAGE_VIEW_TYPE_2D)
				.withSubresourceRange({
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
					})
				.sbuild()));
		}
	}
}
SwapchainKHR::~SwapchainKHR() noexcept
{
	if (m_pData)
	{
		m_pData->Device->destoySwapchainKHR(getHandle(), nullptr);
		delete m_pData;
	}
}

uint32_t SwapchainKHR::getMinImageCount() const noexcept { return m_pData->minImageCount; }
VkExtent2D SwapchainKHR::getImageExtent() const noexcept { return m_pData->imageExtent; }
VkSurfaceTransformFlagBitsKHR SwapchainKHR::getPreTransform() const noexcept { return m_pData->preTransform; }

const std::vector<Image>& SwapchainKHR::getImages() const noexcept { return m_pData->images; }
std::vector<Image>::size_type SwapchainKHR::getImagesCount() const noexcept { return m_pData->images.size(); }
const std::vector<Image>::value_type* SwapchainKHR::getImagesData() const noexcept { return m_pData->images.data(); }
const std::vector<Image>::value_type& SwapchainKHR::getImages(size_t idx) const noexcept { return m_pData->images[idx]; }

const std::vector<ImageView>& SwapchainKHR::getImageViews() const noexcept { return m_pData->imageViews; }
std::vector<ImageView>::size_type SwapchainKHR::getImageViewsCount() const noexcept { return m_pData->imageViews.size(); }
const std::vector<ImageView>::value_type* SwapchainKHR::getImageViewsData() const noexcept { return m_pData->imageViews.data(); }
const std::vector<ImageView>::value_type& SwapchainKHR::getImageViews(size_t idx) const noexcept { return m_pData->imageViews[idx]; }

VkSwapchainKHR SwapchainKHR::getHandle() const noexcept
{
	return m_pData->vkSwapchainKHR;
}

SwapchainKHR::operator VkSwapchainKHR() const noexcept
{
	return getHandle();
}

//  > Move
SwapchainKHR::SwapchainKHR(SwapchainKHR&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
SwapchainKHR& SwapchainKHR::operator=(SwapchainKHR&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData) delete m_pData;
		m_pData = other.m_pData;
		other.m_pData = nullptr;
	}
	return *this;
}

//  > Methods
#define _RVAR(storetype, argtype, name) argtype SwapchainKHR::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& SwapchainKHR::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type SwapchainKHR::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* SwapchainKHR::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& SwapchainKHR::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool SwapchainKHR::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\SwapchainKHR.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef SwapchainKHR::Builder Builder;

//  > Data
#ifdef _DEBUG
struct Builder::Temp
{
	enum class eDefFlag
	{
		eNone,
		eAddCalled,
		eWithCalled,
	};
#define _RVAR(storetype, argtype, name) eDefFlag f##name{ eDefFlag::eNone };
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name) _RVAR(std::vector<type>, const std::vector<type>&, name)
#define _OARR(type, name, ...) _RVAR(std::vector<type>, const std::vector<type>&, name)
#define _FLG(name) _RVAR(bool, bool, name)
#include <snvoxeng\.def\vk\SwapchainKHR.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new SwapchainKHR::Data{})
#ifdef _DEBUG
	, m_pTemp(new Temp{})
#endif
{
}
Builder::~Builder() noexcept
{
#ifdef _DEBUG
	delete m_pTemp;
#endif
	if (m_pData)
	{
		delete m_pData;
		snassert(
			false,
			"Builder initialized, but not used",
			"Try to call .build()\n"
			"or do not use original Builder"
		);
	}
}

//  > Move
Builder::Builder(Builder&& other) noexcept
	: m_pData(other.m_pData)
#ifdef _DEBUG
	, m_pTemp(other.m_pTemp)
#endif
{
	other.m_pData = nullptr;
#ifdef _DEBUG
	other.m_pTemp = nullptr;
#endif
}
Builder& Builder::operator=(Builder&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pData) delete m_pData;
		m_pData = other.m_pData;
		other.m_pData = nullptr;
#ifdef _DEBUG
		m_pTemp = other.m_pTemp;
		other.m_pTemp = nullptr;
#endif
	}
	return *this;
}

//  > Build
SwapchainKHR Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return SwapchainKHR{ m_pData };
}
SwapchainKHR* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new SwapchainKHR{ m_pData };
}

#ifdef _DEBUG
void Builder::Temp::validate() const
{
#define _RVAR(storetype, argtype, name)\
	snassert(\
		f##name != eDefFlag::eNone,\
		#name " is required, but not defined",\
		"Try to call .with" #name "(...)"\
	);
#define _RARR(type, name)\
	snassert(\
		f##name != eDefFlag::eNone,\
		#name " is required, but not defined",\
		"Try to call .with" #name "(...)\n"\
		"or          .add" #name "(...)"\
	);
#include <snvoxeng\.def\vk\SwapchainKHR.h>
}

#define _SET_DEF_FLAG_ADD(name) { m_pTemp->f##name = Temp::eDefFlag::eAddCalled; }

#define _SET_DEF_FLAG_WITH(name){\
	snassert(m_pTemp->f##name != Builder::Temp::eDefFlag::eWithCalled,\
		"Builder::with" #name " is called twice",\
		"Do not call .with-method twice"\
	);\
	snassert(m_pTemp->f##name != Builder::Temp::eDefFlag::eAddCalled,\
		"Builder::with" #name " is called after Builder::add" #name,\
		"Do not call .with-method after calling .add-method\n"\
	);\
	m_pTemp->f##name = Temp::eDefFlag::eWithCalled;}

#define _SET_DEF_FLAG_SET(name){\
	snassert(m_pTemp->f##name != Builder::Temp::eDefFlag::eWithCalled,\
		"Builder::set" #name " is called twice",\
		"Do not call .set-method twice"\
	);\
	m_pTemp->f##name = Temp::eDefFlag::eWithCalled;}

#else
#define _SET_DEF_FLAG_ADD(name) ((void)(0));
#define _SET_DEF_FLAG_WITH(name) ((void)(0));
#define _SET_DEF_FLAG_SET(name) ((void)(0));
#endif

#define _RVAR(storetype, argtype, name)\
	Builder& Builder::with##name(argtype name)\
	{\
		_SET_DEF_FLAG_WITH(name)\
		m_pData->name = name;\
		return *this;\
	}
#define _OVAR(storetype, argtype, name, value)\
	Builder& Builder::with##name(argtype name)\
	{\
		_SET_DEF_FLAG_WITH(name)\
		m_pData->name = name;\
		return *this;\
	}
#define _RARR(type, name)\
	_RVAR(std::vector<type>, const std::vector<type>&, name)\
	Builder& Builder::add##name(const std::vector<type>& name)\
	{\
		_SET_DEF_FLAG_ADD(name)\
		m_pData->name.insert(m_pData->name.end(), name.begin(), name.end());\
		return *this;\
	}
#define _OARR(type, name, ...)\
	_OVAR(std::vector<type>, const std::vector<type>&, name,)\
	Builder& Builder::add##name(const std::vector<type>& name)\
	{\
		_SET_DEF_FLAG_ADD(name)\
		m_pData->name.insert(m_pData->name.end(), name.begin(), name.end());\
		return *this;\
	}
#define _FLG(name) Builder& Builder::set##name()\
	{\
		_SET_DEF_FLAG_SET(name)\
		m_pData->name = true;\
		return *this;\
	}
	
#include <snvoxeng\.def\vk\SwapchainKHR.h>
