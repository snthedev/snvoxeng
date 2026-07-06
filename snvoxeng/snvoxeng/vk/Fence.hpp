#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <vector>

#define _INCLUDE
#include <snvoxeng\.def\vk\Fence.h>

namespace sn::voxeng::vk
{
	// Use Fence::Builder for build
	class SNVOXENG_API Fence
	{
		struct Data;
		Data* m_pData;

	public:
		Fence(Data*& pData);
		~Fence() noexcept;

		VkResult wait(uint64_t timeout = UINT64_MAX) const;
		VkResult reset() const;

		VkFence getHandle() const noexcept;
		operator VkFence() const noexcept;

		Fence(const Fence&) = delete;
		Fence& operator=(const Fence&) = delete;
		Fence(Fence&& other) noexcept;
		Fence& operator=(Fence&& other) noexcept;

#define _RVAR(storetype, argtype, name) argtype get##name() const noexcept;
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
		const std::vector<type>& get##name() const noexcept;\
		std::vector<type>::size_type get##name##Size() const noexcept;\
		const std::vector<type>::value_type* get##name##Data() const noexcept;\
		const std::vector<type>::value_type& get##name(size_t idx) const noexcept;
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool is##name() const noexcept;
#include <snvoxeng\.def\vk\Fence.h>

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
#include <snvoxeng\.def\vk\Fence.h>

			// Builder is invalid after .sbuild()
			// Creates Fence on stack
			Fence sbuild();
			// Builder is invalid after .build()
			// Creates Fence on heap
			Fence* build();
		};
	};
}
