#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <cstdint>
#include <bit>

namespace sn::voxeng::vk
{
	class SNVOXENG_API PhysicalDeviceSelector final : public _impl::PhysicalDeviceSet
	{
		const PhysicalDeviceRegistry* const m_pRegistry;

		typedef uint8_t mask_t;
		std::vector<mask_t> m_suitableMask;
		static constexpr size_t s_bitsPerMask = sizeof(mask_t) * 8u;

		bool mask(size_t bit_idx) const noexcept;
		void flip(size_t bit_idx) noexcept;

	public:
		explicit PhysicalDeviceSelector(const PhysicalDeviceRegistry& registry);
		~PhysicalDeviceSelector() noexcept;

		PhysicalDeviceSelector& pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data = nullptr);
		PhysicalDeviceSelector pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data = nullptr) const;

		PhysicalDevice get(size_t idx) const override;
		PhysicalDevice first() const override;
		PhysicalDevice last() const override;
		size_t count() const noexcept override;
	};
}
