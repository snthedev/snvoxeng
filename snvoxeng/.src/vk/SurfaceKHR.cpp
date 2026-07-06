#include <snvoxeng\snvoxeng\vk\SurfaceKHR.hpp>

#ifdef _WIN32
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#endif

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

// === SurfaceKHR ===

//  > Data
struct SurfaceKHR::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\SurfaceKHR.h>

	VkSurfaceKHR vkSurfaceKHR;
};

//  > Init
SurfaceKHR::SurfaceKHR(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	VkResult result = VK_SUCCESS;
	switch (m_pData->WindowDescription.platform)
	{
	case WindowDescription_t::platform_type::headless:
		m_pData->vkSurfaceKHR = VK_NULL_HANDLE;
		return;
#ifdef VK_USE_PLATFORM_WIN32_KHR
	case WindowDescription_t::platform_type::win32:
	{
		VkWin32SurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = static_cast<HINSTANCE>(m_pData->WindowDescription.handle_param1),
			.hwnd = static_cast<HWND>(m_pData->WindowDescription.handle_param2),
		};

		result = vkCreateWin32SurfaceKHR(*m_pData->Instance, &createInfo, nullptr, &m_pData->vkSurfaceKHR);
		break;
	}
#endif
	default:
		throw std::invalid_argument("Unsupported or unimplemented platform type.");
	}

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create window surface.");
}
SurfaceKHR::~SurfaceKHR() noexcept
{
	if (m_pData)
	{
		vkDestroySurfaceKHR(*m_pData->Instance, m_pData->vkSurfaceKHR, nullptr);
		delete m_pData;
	}
}

VkSurfaceKHR SurfaceKHR::getHandle() const noexcept
{
	return VkSurfaceKHR();
}

SurfaceKHR::operator VkSurfaceKHR() const noexcept
{
	return getHandle();
}

//  > Move
SurfaceKHR::SurfaceKHR(SurfaceKHR&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
SurfaceKHR& SurfaceKHR::operator=(SurfaceKHR&& other) noexcept
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
#define _RVAR(storetype, argtype, name) argtype SurfaceKHR::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& SurfaceKHR::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type SurfaceKHR::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* SurfaceKHR::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& SurfaceKHR::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool SurfaceKHR::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\SurfaceKHR.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef SurfaceKHR::Builder Builder;

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
#include <snvoxeng\.def\vk\SurfaceKHR.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new SurfaceKHR::Data{})
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
SurfaceKHR Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return SurfaceKHR{ m_pData };
}
SurfaceKHR* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new SurfaceKHR{ m_pData };
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
#include <snvoxeng\.def\vk\SurfaceKHR.h>
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
	
#include <snvoxeng\.def\vk\SurfaceKHR.h>
