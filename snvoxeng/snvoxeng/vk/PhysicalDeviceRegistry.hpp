#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>
#include <snvoxeng/snvoxeng/vk/Instance.hpp>
#include <snvoxeng/snvoxeng/vk/PhysicalDeviceSet.hpp>

#include <snassert/snassert.hpp>

#include <optional>

namespace sn::voxeng::vk
{
	class PhysicalDeviceSelector;
	class SNVOXENG_API PhysicalDeviceRegistry final : public _impl::PhysicalDeviceSet
	{
	public:
		typedef bool(fSelector_t)(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data);

	private:
		const Instance* const m_pInstance;
		const std::vector<VkPhysicalDevice> m_physicalDevices;
		
		mutable std::vector<std::optional<VkPhysicalDeviceProperties>> m_cached_properties;
		mutable std::vector<std::optional<VkPhysicalDeviceFeatures>> m_cached_features;
		mutable std::vector<std::optional<VkPhysicalDeviceMemoryProperties>> m_cached_memoryProperties;
		mutable std::vector<std::optional<std::vector<VkQueueFamilyProperties>>> m_cached_queueFamilyProperties;
		mutable std::vector<std::optional<std::vector<VkExtensionProperties>>> m_cached_extensionProperties;

		friend class PhysicalDevice;
		VkPhysicalDevice getHandle(size_t idx) const noexcept;

	public:
		explicit PhysicalDeviceRegistry(const Instance& instance);
		~PhysicalDeviceRegistry() noexcept;

		const VkPhysicalDeviceProperties& getProperties(size_t idx) const noexcept;
		const VkPhysicalDeviceFeatures& getFeatures(size_t idx) const noexcept;
		const VkPhysicalDeviceMemoryProperties& getMemoryProperties(size_t idx) const noexcept;
		const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties(size_t idx) const noexcept;
		const std::vector<VkExtensionProperties>& getExtensionProperties(size_t idx) const noexcept;
		const VkExtensionProperties* findExtensionProperties(size_t idx, const char* extensionName) const noexcept;

		const Instance& getInstance() const noexcept;

		PhysicalDeviceSelector pick(fSelector_t* fSelector, void* user_data = nullptr) const;
		operator PhysicalDeviceSelector() const;

		PhysicalDevice get(size_t idx) const override;
		PhysicalDevice first() const override;
		PhysicalDevice last() const override;
		size_t count() const noexcept override;
	};
}

namespace sn::voxeng::vk::fPhysicalDeviseSelectors
{
	struct fDeviceType_user_data_t {
		VkPhysicalDeviceType deviceType;
	};
	SNVOXENG_API_POD bool fDeviceType(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data);

	struct fExtensions_user_data_t {
		const char* const* ppExtensionNames;
		size_t extensionCount;
	};
	SNVOXENG_API_POD bool fExtensions(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data);

	struct fQueueSupport_user_data_t {
		// Individual flags must be supported on at least one of the queues.
		VkQueueFlags requiredFlagsOr;
		// All these flags must be supported on at least one of the queues simultaneously.
		VkQueueFlags requiredFlagsAnd;
	};
	SNVOXENG_API_POD bool fQueueSupport(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data);

	struct fSurfaceSupport_user_data_t {
		VkSurfaceKHR surface;
	};
	SNVOXENG_API_POD bool fSurfaceSupport(size_t idx, const PhysicalDeviceRegistry& registry, void* user_data);
}
