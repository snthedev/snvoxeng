#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/vk/VkMinimal.hpp>

#include <snvoxeng/snvoxeng/vk/PhysicalDevice.hpp>

namespace sn::voxeng::vk::_impl
{
	class SNVOXENG_API PhysicalDeviceSet
	{
	public:
		PhysicalDeviceSet();
		virtual ~PhysicalDeviceSet() noexcept;

		// Returns the suitable device by index.
		// UB expected if count() == 0
		virtual PhysicalDevice get(size_t idx) const = 0;
		// Returns the first suitable device.
		// UB expected if count() == 0
		virtual PhysicalDevice first() const = 0;
		// Returns the last suitable device.
		// UB expected if count() == 0
		virtual PhysicalDevice last() const = 0;
		// Returns the number of suitable devices.
		virtual size_t count() const noexcept = 0;

		// Returns true if there is at least one suitable device.
		explicit operator bool() const noexcept;
	};
}
