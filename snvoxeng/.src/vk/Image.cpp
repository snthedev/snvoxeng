#include <snvoxeng\snvoxeng\vk\Image.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

// === Image ===

//  > Data
struct Image::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\Image.h>

	VkImage vkImage;
};

//  > Init
Image::Image(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	if (m_pData->ViewHandle != VK_NULL_HANDLE)
	{
		m_pData->vkImage = m_pData->ViewHandle;
		return;
	}

	VkImageCreateInfo createInfo{
		.sType{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO },
		.pNext{ nullptr },
		.flags{},
		.imageType{ m_pData->ImageType },
		.format{ m_pData->Format },
		.extent{ m_pData->Extent },
		.mipLevels{ m_pData->MipLevels },
		.arrayLayers{ m_pData->ArrayLevels },
		.samples{ m_pData->Samples },
		.tiling{ m_pData->Tiling },
		.usage{ m_pData->Usage },
		.sharingMode{ m_pData->SharingMode },
		.queueFamilyIndexCount{ static_cast<uint32_t>(m_pData->QueueFamilyIndices.size()) },
		.pQueueFamilyIndices{ m_pData->QueueFamilyIndices.data() },
		.initialLayout{ m_pData->InitialLayout },
	};
	snassert(m_pData->Device->createImage(&createInfo, nullptr, &m_pData->vkImage) == VK_SUCCESS,
		"Failed to create image", "Check builder settings");
}
Image::~Image() noexcept
{
	if (m_pData)
	{
		if (m_pData->ViewHandle == VK_NULL_HANDLE)
			m_pData->Device->destroyImage(getHandle(), nullptr);
		delete m_pData;
	}
}

VkImage Image::getHandle() const noexcept
{
	return m_pData->vkImage;
}

Image::operator VkImage() const noexcept
{
	return getHandle();
}

//  > Move
Image::Image(Image&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
Image& Image::operator=(Image&& other) noexcept
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
#define _RVAR(storetype, argtype, name) argtype Image::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& Image::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type Image::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* Image::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& Image::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool Image::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\Image.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef Image::Builder Builder;

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
#include <snvoxeng\.def\vk\Image.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new Image::Data{})
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
Image Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return Image{ m_pData };
}
Image* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new Image{ m_pData };
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
#include <snvoxeng\.def\vk\Image.h>
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
	
#include <snvoxeng\.def\vk\Image.h>
