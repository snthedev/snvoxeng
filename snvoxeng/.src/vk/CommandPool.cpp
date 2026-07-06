#include <snvoxeng\snvoxeng\vk\CommandPool.hpp>
#include <snvoxeng\snvoxeng\vk\CommandBuffersContainer.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

// === CommandPool ===

//  > Data
struct CommandPool::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\CommandPool.h>

	VkCommandPool vkCommandPool;
};

//  > Init
CommandPool::CommandPool(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	VkCommandPoolCreateInfo createInfo{
		.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
		.pNext{ nullptr },
		.flags{ m_pData->Flags },
		.queueFamilyIndex{ m_pData->QueueFamilyIndex },
	};
	snassert(m_pData->Device->createCommandPool(&createInfo, nullptr, &m_pData->vkCommandPool) == VK_SUCCESS,
		"Failed to create command pool", "Check builder settings");
}
CommandPool::~CommandPool() noexcept
{
	if (m_pData)
	{
		m_pData->Device->destroyCommandPool(getHandle(), nullptr);
		delete m_pData;
	}
}

CommandBuffersContainer CommandPool::allocateCommandBuffers(uint32_t count, VkCommandBufferLevel level) const
{
	return CommandBuffersContainer::Builder()
		.withCommandPool(this)
		.withCommandBufferCount(count)
		.withLevel(level)
		.sbuild();
}

VkCommandPool CommandPool::getHandle() const noexcept
{
	return m_pData->vkCommandPool;
}

CommandPool::operator VkCommandPool() const noexcept
{
	return getHandle();
}

//  > Move
CommandPool::CommandPool(CommandPool&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
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
#define _RVAR(storetype, argtype, name) argtype CommandPool::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& CommandPool::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type CommandPool::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* CommandPool::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& CommandPool::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool CommandPool::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\CommandPool.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef CommandPool::Builder Builder;

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
#include <snvoxeng\.def\vk\CommandPool.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new CommandPool::Data{})
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
CommandPool Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return CommandPool{ m_pData };
}
CommandPool* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new CommandPool{ m_pData };
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
#include <snvoxeng\.def\vk\CommandPool.h>
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
	
#include <snvoxeng\.def\vk\CommandPool.h>
