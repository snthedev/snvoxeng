#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/vk/PhysicalDeviceRegistry.hpp>

#include <cstdint>
#include <bit>

namespace sn::voxeng::vk
{
	class SNVOXENG_API PhysicalDeviceSelector final : public _impl::PhysicalDeviceSet
	{
		const PhysicalDeviceRegistry* m_pRegistry;

		typedef uint8_t mask_t;
		static constexpr size_t s_bitsPerMask = sizeof(mask_t) * 8u;

		// TODO: move this to struct SBO<T, N>
		static inline constexpr size_t s_mask_buf_size = 32u;
		mask_t m_suitableMaskBuf[s_mask_buf_size];

		mask_t* m_pSuitableMask; // = &m_suitableMask or = new mask_t[N] if s_mask_buf_size not enought
		size_t m_suitableMaskSize;
		void initSuitableMaskStorage(mask_t value = ~(mask_t)(0));
		// ^ TODO ^

		bool mask(size_t bit_idx) const noexcept;
		void flip(size_t bit_idx) noexcept;

	public:
		explicit PhysicalDeviceSelector(const PhysicalDeviceRegistry& registry);
		~PhysicalDeviceSelector() noexcept;

		PhysicalDeviceSelector(const PhysicalDeviceSelector& other);
		PhysicalDeviceSelector& operator=(const PhysicalDeviceSelector& other);
		PhysicalDeviceSelector(PhysicalDeviceSelector&& other) noexcept;
		PhysicalDeviceSelector& operator=(PhysicalDeviceSelector&& other) noexcept;

		PhysicalDeviceSelector& pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data = nullptr);
		PhysicalDeviceSelector pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data = nullptr) const;

		PhysicalDevice get(size_t idx) const override;
		PhysicalDevice first() const override;
		PhysicalDevice last() const override;
		size_t count() const noexcept override;
	};
}
