#include <snvoxeng/snvoxeng/vk/DescriptorSetsContainer.hpp>
#include <snvoxeng/snvoxeng/utils/vk-getSType.hpp>

#include <snvoxeng/snvoxeng/vk/DescriptorSet.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <vulkan/vulkan.h>
#include <snassert/snassert.hpp>

#include <vector>

using namespace sn::voxeng::vk;

namespace default_values
{
#define SNBCG_DEFAULT_VALUES
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>
}

// === DescriptorSetsContainer : private ===
struct DescriptorSetsContainer::data_t
{
	VkDescriptorSetAllocateInfo vkAllocateInfo{ .sType{ ::sn::voxeng::utils::vk::getSType<VkDescriptorSetAllocateInfo>() } };

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
	DETAIL_SNBCG_MACRO_ISEMPTY(subdata, store_t name;, )
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>

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
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>
	}

	std::vector<VkDescriptorSet> vkHandle{ VK_NULL_HANDLE };
};

void DescriptorSetsContainer::onCreate(data_t& data)
{
	auto result = data.pDescriptorPool->getDevice().allocateDescriptorSets(&data.vkAllocateInfo, data.vkHandle.data());
	snassert(result == VK_SUCCESS,
		"Failed to allocate VkDescriptorSet-s", "Check Builder settings");

	if (m_pData->pDescriptorPool->getDevice().getPhysicalDevice().getRegistry().getInstance().getDebugStream())
		*m_pData->pDescriptorPool->getDevice().getPhysicalDevice().getRegistry().getInstance().getDebugStream()
		<< "[trace]: DescriptorSets 0x" << std::hex << this << std::dec << " created" << std::endl;
}
void DescriptorSetsContainer::onDestroy(data_t& data) noexcept
{
	auto result = data.pDescriptorPool->getDevice().freeDescriptorSets(data.pDescriptorPool->vkHandle(), data.vkHandle.size(), data.vkHandle.data());
	snassert(result == VK_SUCCESS,
		"Failed to free VkDescriptorSet-s", "Check Builder settings");

	if (m_pData->pDescriptorPool->getDevice().getPhysicalDevice().getRegistry().getInstance().getDebugStream())
		*m_pData->pDescriptorPool->getDevice().getPhysicalDevice().getRegistry().getInstance().getDebugStream()
		<< "[trace]: DescriptorSets 0x" << std::hex << this << std::dec << " destroyed" << std::endl;
}

DescriptorSetsContainer::DescriptorSetsContainer(data_t*& pData)
	: m_pData(pData)
	, m_isView(false)
{
	pData = nullptr;
	onCreate(*m_pData);
}
DescriptorSetsContainer::DescriptorSetsContainer(data_t*& pData, std::vector<VkDescriptorSet> view)
	: m_pData(pData)
	, m_isView(true)
{
	pData = nullptr;
}

// === DescriptorSetsContainer : public ===
DescriptorSetsContainer::~DescriptorSetsContainer() noexcept
{
	if (m_pData) [[likely]]
	{
		if (!m_isView) [[likely]] onDestroy(*m_pData);
		delete m_pData;
	}
}

DescriptorSet DescriptorSetsContainer::get(size_t idx) const { return DescriptorSet(*this, idx); }
DescriptorSet DescriptorSetsContainer::first() const { return DescriptorSet(*this, 0); }
DescriptorSet DescriptorSetsContainer::last() const { return DescriptorSet(*this, count() - 1u); }
size_t DescriptorSetsContainer::count() const noexcept { return m_pData->vkHandle.size(); }

DescriptorSetsContainer::DescriptorSetsContainer(DescriptorSetsContainer&& other) noexcept
	: m_pData(other.m_pData)
	, m_isView(other.m_isView)
{
	other.m_pData = nullptr;
}
DescriptorSetsContainer& DescriptorSetsContainer::operator=(DescriptorSetsContainer&& other) noexcept
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

std::span<const VkDescriptorSet> DescriptorSetsContainer::vkHandle() const noexcept { return m_pData->vkHandle; }
VkDescriptorSet DescriptorSetsContainer::vkHandle(size_t idx) const noexcept { return m_pData->vkHandle[idx]; }
DescriptorSetsContainer::operator std::span<const VkDescriptorSet>() const noexcept { return m_pData->vkHandle; }

#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) DescriptorSetsContainer::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy)\
DETAIL_##return_policy##_t(store_t) DescriptorSetsContainer::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) DescriptorSetsContainer::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action)\
DETAIL_##return_policy##_t(store_t) DescriptorSetsContainer::get##Name() const noexcept { std::add_lvalue_reference_t<std::add_const_t<store_t>> val = m_pData->subdata name; return return_policy; }
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>



typedef DescriptorSetsContainer::Builder Builder;

// === Builder : private ===
void Builder::finalize(data_t& data)
{
	data.vkAllocateInfo.descriptorPool = data.pDescriptorPool->vkHandle();
	data.vkAllocateInfo.descriptorSetCount = static_cast<uint32_t>(data.setLayouts.size());
	data.vkAllocateInfo.pSetLayouts = data.setLayouts.data();
}

#ifdef DETAIL_SNBCG_DEBUG
struct Builder::temp_t
{
#define SNBCG_REQUIRED(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_OPTIONAL(store_t, arg_t, subdata, name, Name, return_policy, store_policy) uint8_t name{ 0 };
#define SNBCG_REQUIRED_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#define SNBCG_OPTIONAL_ADDITIVE(store_t, arg_t, args_t, subdata, name, Name, return_policy, store_policy, store_action) uint8_t name{ 0 };
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>

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
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>
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
#include <snvoxeng/.def/vk/DescriptorSetsContainer.h>

DescriptorSetsContainer Builder::sbuild()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif
	finalize(*m_pData);
	return DescriptorSetsContainer{ m_pData };
}
DescriptorSetsContainer* Builder::build()
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif
	finalize(*m_pData);
	return new DescriptorSetsContainer{ m_pData };
}

DescriptorSetsContainer Builder::sbuild(std::vector<VkDescriptorSet> view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif
	finalize(*m_pData);
	return DescriptorSetsContainer{ m_pData, view };
}
DescriptorSetsContainer* Builder::build(std::vector<VkDescriptorSet> view)
{
#ifdef DETAIL_SNBCG_DEBUG
	m_pTemp->validate();
#endif
	finalize(*m_pData);
	return new DescriptorSetsContainer{ m_pData, view };
}
