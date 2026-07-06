#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <vector>

#define _INCLUDE
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>

namespace sn::voxeng::vk
{
	class CommandBuffer;
	// Use CommandBuffersContainer::Builder for build
	class SNVOXENG_API CommandBuffersContainer
	{
		struct Data;
		Data* m_pData;

	public:
		CommandBuffersContainer(Data*& pData);
		~CommandBuffersContainer() noexcept;

		const std::vector<VkCommandBuffer> getHandle() const noexcept;
		VkCommandBuffer getHandle(size_t idx) const noexcept;

		CommandBuffer get(size_t idx) const;
		CommandBuffer first() const;
		CommandBuffer last() const;
		size_t count() const noexcept;

		CommandBuffersContainer(const CommandBuffersContainer&) = delete;
		CommandBuffersContainer& operator=(const CommandBuffersContainer&) = delete;
		CommandBuffersContainer(CommandBuffersContainer&& other) noexcept;
		CommandBuffersContainer& operator=(CommandBuffersContainer&& other) noexcept;

#define _RVAR(storetype, argtype, name) argtype get##name() const noexcept;
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
		const std::vector<type>& get##name() const noexcept;\
		std::vector<type>::size_type get##name##Size() const noexcept;\
		const std::vector<type>::value_type* get##name##Data() const noexcept;\
		const std::vector<type>::value_type& get##name(size_t idx) const noexcept;
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool is##name() const noexcept;
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>

		class SNVOXENG_API Builder
		{
			Data* m_pData;

#ifdef _DEBUG
			struct Temp;
			Temp* m_pTemp;
#endif

		public:
			Builder();
			~Builder() noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&& other) noexcept;
			Builder& operator=(Builder&& other) noexcept;

#define _RVAR(storetype, argtype, name) Builder& with##name(argtype name);
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
			Builder& with##name(const std::vector<type>& name);\
			Builder& add##name(const std::vector<type>& name);
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) Builder& set##name();
#include <snvoxeng\.def\vk\CommandBuffersContainer.h>

			// Builder is invalid after .sbuild()
			// Creates CommandBuffersContainer on stack
			CommandBuffersContainer sbuild();
			// Builder is invalid after .build()
			// Creates CommandBuffersContainer on heap
			CommandBuffersContainer* build();
		};
	};
}
