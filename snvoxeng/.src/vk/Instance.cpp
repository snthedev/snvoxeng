#include <snvoxeng\snvoxeng\vk\Instance.hpp>

static VKAPI_ATTR VkBool32 VKAPI_PTR debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (pUserData) 
	{
		*(reinterpret_cast<std::ostream*>(pUserData)) << "[Vulkan Validation]: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}

using namespace sn::voxeng::vk;

// === Instance ===

//  > Data
struct Instance::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\Instance.h>

	VkInstance vkInstance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT{ VK_NULL_HANDLE };
};

//  > Init
Instance::Instance(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	{
		VkApplicationInfo appInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = m_pData->ApplicationName,
			.applicationVersion = m_pData->ApplicationVersion,
			.pEngineName = m_pData->EngineName,
			.engineVersion = m_pData->EngineVersion,
			.apiVersion = m_pData->ApiVersion,
		};

		VkInstanceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(m_pData->ValidationLayers.size()),
			.ppEnabledLayerNames = m_pData->ValidationLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(m_pData->InstanceExtensions.size()),
			.ppEnabledExtensionNames = m_pData->InstanceExtensions.data(),
		};

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (m_pData->EnableDebugMessenger)
		{
			debugCreateInfo = decltype(debugCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.pNext = createInfo.pNext,

					.messageSeverity =
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					.messageType =
					VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
					.pfnUserCallback = debugCallback,
					.pUserData = m_pData->DebugStream,
			};
			createInfo.pNext = &debugCreateInfo;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_pData->vkInstance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan instance.");

		if (m_pData->EnableDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_pData->vkInstance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) func(m_pData->vkInstance, &debugCreateInfo, nullptr, &m_pData->vkDebugUtilsMessengerEXT);
		}
	}

	if (m_pData->DebugStream)
	{
		(*reinterpret_cast<std::ostream*>(m_pData->DebugStream))
			<< "[snvoxeng] Vulkan Instance created via RAII\n";
	}
}
Instance::~Instance() noexcept
{
	if (m_pData)
	{
		if (m_pData->vkDebugUtilsMessengerEXT != VK_NULL_HANDLE)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_pData->vkInstance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) func(m_pData->vkInstance, m_pData->vkDebugUtilsMessengerEXT, nullptr);
		}

		vkDestroyInstance(m_pData->vkInstance, nullptr);

		if (m_pData->DebugStream)
		{
			(*reinterpret_cast<std::ostream*>(m_pData->DebugStream))
				<< "[snvoxeng] Vulkan Instance destroyed via RAII\n";
		}

		delete m_pData;
	}
}

VkInstance Instance::getHandle() const noexcept
{
	return m_pData->vkInstance;
}

Instance::operator VkInstance() const noexcept
{
	return getHandle();
}

std::vector<VkPhysicalDevice> Instance::enumeratePhysicalDevices() const
{
	uint32_t physicalDeviceCount;
	if (vkEnumeratePhysicalDevices(m_pData->vkInstance, &physicalDeviceCount, nullptr) != VK_SUCCESS) [[unlikely]]
		throw std::runtime_error("Failed to enumerate physical devices.");

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	if (vkEnumeratePhysicalDevices(m_pData->vkInstance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) [[unlikely]]
		throw std::runtime_error("Failed to enumerate physical devices.");

	return physicalDevices;
}

//  > Move
Instance::Instance(Instance&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
Instance& Instance::operator=(Instance&& other) noexcept
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
#define _RVAR(storetype, argtype, name) argtype Instance::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& Instance::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type Instance::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* Instance::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& Instance::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool Instance::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\Instance.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef Instance::Builder Builder;

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
#include <snvoxeng\.def\vk\Instance.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new Instance::Data{})
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
Instance Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return Instance{ m_pData };
}
Instance* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new Instance{ m_pData };
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
#include <snvoxeng\.def\vk\Instance.h>
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
	
#include <snvoxeng\.def\vk\Instance.h>
