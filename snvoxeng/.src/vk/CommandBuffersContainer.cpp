#include <snvoxeng\snvoxeng\vk\CommandBuffersContainer.hpp>
#include <snvoxeng\snvoxeng\vk\CommandBuffer.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

// === CommandBuffersContainer ===

//  > Data
struct CommandBuffersContainer::Data
{
#define _RVAR(storetype, argtype, name) storetype name;
#define _OVAR(storetype, argtype, name, value) storetype name{ value };
#define _RARR(type, name) std::vector<type> name;
#define _OARR(type, name, ...) std::vector<type> name{ __VA_ARGS__ };
#define _FLG(name) bool name{ false };
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>

	std::vector<VkCommandBuffer> vkCommandBuffers;
};

//  > Init
CommandBuffersContainer::CommandBuffersContainer(Data*& pData)
	: m_pData(pData)
{
	pData = nullptr;

	VkCommandBufferAllocateInfo allocateInfo{
		.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
		.pNext{ nullptr },
		.commandPool{ m_pData->CommandPool->getHandle() },
		.level{ m_pData->Level },
		.commandBufferCount{ m_pData->CommandBufferCount },
	};
	m_pData->vkCommandBuffers.resize(allocateInfo.commandBufferCount);
	snassert(m_pData->CommandPool->getDevice()->allocateCommandBuffers(&allocateInfo, m_pData->vkCommandBuffers.data()) == VK_SUCCESS,
		"Failed to allocate command buffers", "Check builder settings");
}
CommandBuffersContainer::~CommandBuffersContainer() noexcept
{
	if (m_pData)
	{
		m_pData->CommandPool->getDevice()->freeCommandBuffers(m_pData->CommandPool->getHandle(), m_pData->CommandBufferCount, m_pData->vkCommandBuffers.data());
		delete m_pData;
	}
}

const std::vector<VkCommandBuffer> CommandBuffersContainer::getHandle() const noexcept
{
	return m_pData->vkCommandBuffers;
}
VkCommandBuffer CommandBuffersContainer::getHandle(size_t idx) const noexcept
{
	return m_pData->vkCommandBuffers[idx];
}

CommandBuffer CommandBuffersContainer::get(size_t idx) const { return CommandBuffer(*this, idx); }
CommandBuffer CommandBuffersContainer::first() const { return CommandBuffer(*this, 0); }
CommandBuffer CommandBuffersContainer::last() const { return CommandBuffer(*this, count() - 1u); }
size_t CommandBuffersContainer::count() const noexcept { return m_pData->vkCommandBuffers.size(); }
//  > Move
CommandBuffersContainer::CommandBuffersContainer(CommandBuffersContainer&& other) noexcept
	: m_pData(other.m_pData)
{
	other.m_pData = nullptr;
}
CommandBuffersContainer& CommandBuffersContainer::operator=(CommandBuffersContainer&& other) noexcept
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
#define _RVAR(storetype, argtype, name) argtype CommandBuffersContainer::get##name() const noexcept { return m_pData->name; }
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
	const std::vector<type>& CommandBuffersContainer::get##name() const noexcept { return m_pData->name; }\
	std::vector<type>::size_type CommandBuffersContainer::get##name##Size() const noexcept { return m_pData->name.size(); }\
	const std::vector<type>::value_type* CommandBuffersContainer::get##name##Data() const noexcept { return m_pData->name.data(); }\
	const std::vector<type>::value_type& CommandBuffersContainer::get##name(size_t idx) const noexcept { return m_pData->name[idx]; }
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool CommandBuffersContainer::is##name() const noexcept { return m_pData->name; }
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>



// === Builder ===
#include <snassert/snassert.hpp>

typedef CommandBuffersContainer::Builder Builder;

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
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>

	void validate() const;
};
#endif

//  > Init
Builder::Builder()
	: m_pData(new CommandBuffersContainer::Data{})
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
CommandBuffersContainer Builder::sbuild()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return CommandBuffersContainer{ m_pData };
}
CommandBuffersContainer* Builder::build()
{
#ifdef _DEBUG
	m_pTemp->validate();
#endif
	return new CommandBuffersContainer{ m_pData };
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
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>
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
	
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>
