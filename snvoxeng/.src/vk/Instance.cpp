#include <snvoxeng/snvoxeng/vk/Instance.hpp>
#include <snvoxeng/snvoxeng/utils/vk-getSType.hpp>

#include <vulkan/vulkan.h>
#include <snassert/snassert.hpp>

#include <stdexcept>
#include <vector>

using namespace sn::voxeng::vk;

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

namespace default_values
{
#define SNBCG_DEFAULT_VALUES
#include <snvoxeng/.def/vk/Instance.h>
}

// === Instance : private ===
struct Instance::data_t
{
	VkInstanceCreateInfo vkCreateInfo{ .sType{ ::sn::voxeng::utils::vk::getSType<VkInstanceCreateInfo>() } };
	VkApplicationInfo vkApplicationInfo{ .sType{ ::sn::voxeng::utils::vk::getSType<VkApplicationInfo>() } };

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#include <snvoxeng/.def/vk/Instance.h>

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
#include <snvoxeng/.def/vk/Instance.h>
	}

	VkInstance vkHandle{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT{ VK_NULL_HANDLE };
};

void Instance::onCreate(data_t& data)
{
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (data.isDebugMessengerEnabled)
	{
		debugCreateInfo = VkDebugUtilsMessengerCreateInfoEXT{
			.sType{ ::sn::voxeng::utils::vk::getSType<VkDebugUtilsMessengerCreateInfoEXT>() },
			.pNext = data.vkCreateInfo.pNext,
			.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debugCallback,
			.pUserData = data.debugStream,
		};
		data.vkCreateInfo.pNext = &debugCreateInfo;
	}
	{
		auto result = vkCreateInstance(&data.vkCreateInfo, data.vkPAllocator, &data.vkHandle);
		snassert(result == VK_SUCCESS,
			"Failed to create VkInstance", "Check Builder settings");
	}

	if (data.isDebugMessengerEnabled)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(data.vkHandle, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) func(data.vkHandle, &debugCreateInfo, data.vkPAllocator, &data.vkDebugUtilsMessengerEXT);
	}

	if (data.debugStream)
		*data.debugStream
			<< "[trace]: Instance 0x" << std::hex << this << std::dec << " created" << std::endl;
}
void Instance::onDestroy(data_t& data) noexcept
{
	if (data.vkDebugUtilsMessengerEXT != VK_NULL_HANDLE)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(data.vkHandle, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) func(data.vkHandle, data.vkDebugUtilsMessengerEXT, data.vkPAllocator);
	}

	vkDestroyInstance(data.vkHandle, data.vkPAllocator);

	if (data.debugStream)
		*data.debugStream
			<< "[trace]: Instance 0x" << std::hex << this << std::dec << " destroyed" << std::endl;
}

Instance::Instance(data_t*& pData)
	: m_pData(pData)
	, m_isView(false)
{
	pData = nullptr;
	onCreate(*m_pData);
}
Instance::Instance(data_t*& pData, VkInstance view)
	: m_pData(pData)
	, m_isView(true)
{
	pData = nullptr;
	m_pData->vkHandle = view;
}

// === Instance : public ===
Instance::~Instance() noexcept
{
	if (m_pData) [[likely]]
	{
		if (!m_isView) [[likely]] onDestroy(*m_pData);
		delete m_pData;
	}
}

VkResult Instance::enumeratePhysicalDevices(uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const
{
	return vkEnumeratePhysicalDevices(vkHandle(), pPhysicalDeviceCount, pPhysicalDevices);
}

Instance::Instance(Instance&& other) noexcept
	: m_pData(other.m_pData)
	, m_isView(other.m_isView)
{
	other.m_pData = nullptr;
}
Instance& Instance::operator=(Instance&& other) noexcept
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

VkInstance Instance::vkHandle() const noexcept { return m_pData->vkHandle; }
Instance::operator VkInstance() const noexcept { return m_pData->vkHandle; }

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) Instance::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) Instance::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) Instance::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) Instance::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#include <snvoxeng/.def/vk/Instance.h>



typedef Instance::Builder Builder;

// === Builder : private ===
void Builder::finalize(data_t& data)
{
	data.vkCreateInfo.pApplicationInfo = &data.vkApplicationInfo;
	data.vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(data.validationLayers.size());
	data.vkCreateInfo.ppEnabledLayerNames = data.validationLayers.data();
	data.vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(data.extensions.size());
	data.vkCreateInfo.ppEnabledExtensionNames = data.extensions.data();
}

#ifdef DETAIL_SNBCG_DEBUG
struct Builder::temp_t
{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#include <snvoxeng/.def/vk/Instance.h>

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
#include <snvoxeng/.def/vk/Instance.h>
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
#include <snvoxeng/.def/vk/Instance.h>

Instance Builder::sbuild()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return Instance{ m_pData };
}
Instance* Builder::build()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return new Instance{ m_pData };
}

Instance Builder::sbuild(VkInstance view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return Instance{ m_pData, view };
}
Instance* Builder::build(VkInstance view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif // ^ DETAIL_SNBCG_DEBUG ^
	finalize(*m_pData);
	return new Instance{ m_pData, view };
}
