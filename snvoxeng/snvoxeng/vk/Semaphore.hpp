#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <vector>

#define _INCLUDE
#include <snvoxeng\.def\vk\Semaphore.h>

namespace sn::voxeng::vk
{
	// Use Semaphore::Builder for build
	class SNVOXENG_API Semaphore
	{
		struct Data;
		Data* m_pData;

	public:
		Semaphore(Data*& pData);
		~Semaphore() noexcept;

		VkSemaphore getHandle() const noexcept;
		operator VkSemaphore() const noexcept;

		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;
		Semaphore(Semaphore&& other) noexcept;
		Semaphore& operator=(Semaphore&& other) noexcept;

#define _RVAR(storetype, argtype, name) argtype get##name() const noexcept;
#define _OVAR(storetype, argtype, name, value) _RVAR(storetype, argtype, name)
#define _RARR(type, name)\
		const std::vector<type>& get##name() const noexcept;\
		std::vector<type>::size_type get##name##Size() const noexcept;\
		const std::vector<type>::value_type* get##name##Data() const noexcept;\
		const std::vector<type>::value_type& get##name(size_t idx) const noexcept;
#define _OARR(type, name, ...) _RARR(type, name)
#define _FLG(name) bool is##name() const noexcept;
#include <snvoxeng\.def\vk\Semaphore.h>

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
#include <snvoxeng\.def\vk\Semaphore.h>

			// Builder is invalid after .sbuild()
			// Creates Semaphore on stack
			Semaphore sbuild();
			// Builder is invalid after .build()
			// Creates Semaphore on heap
			Semaphore* build();
		};
	};
}
