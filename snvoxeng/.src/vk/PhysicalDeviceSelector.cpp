#include <snvoxeng/snvoxeng/vk/PhysicalDeviceSelector.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

bool PhysicalDeviceSelector::mask(size_t bit_idx) const noexcept
{
	snassert(
		bit_idx < m_suitableMask.size() * s_bitsPerMask,
		"Out of m_suitableMask",
		"Allowed range is [0; m_suitableMask.size() * s_bitsPerMask)"
	);

	constexpr mask_t mask_first = (mask_t)(1u) << (s_bitsPerMask - 1u);

	mask_t mask = mask_first >> (bit_idx % s_bitsPerMask);
	return (m_suitableMask[bit_idx / s_bitsPerMask] & mask) != 0;
}
void PhysicalDeviceSelector::flip(size_t bit_idx) noexcept
{
	snassert(
		bit_idx < m_pRegistry->count(),
		"Out of m_suitableMask",
		"Allowed range is [0; m_pRegistry->count())"
	);

	constexpr mask_t mask_first = (mask_t)(1u) << (s_bitsPerMask - 1u);

	mask_t mask = mask_first >> (bit_idx % s_bitsPerMask);
	m_suitableMask[bit_idx / s_bitsPerMask] ^= mask;
}

PhysicalDeviceSelector::PhysicalDeviceSelector(const PhysicalDeviceRegistry& registry)
	: m_pRegistry(&registry)
{
	size_t mod = m_pRegistry->count() % s_bitsPerMask;
	if (mod == 0u) [[unlikely]]
	{
		m_suitableMask.resize(m_pRegistry->count() / s_bitsPerMask, ~(mask_t)(0));
		return;
	}

	m_suitableMask.resize(m_pRegistry->count() / s_bitsPerMask + 1u, ~(mask_t)(0));
	m_suitableMask.back() ^= (mask_t)(~(mask_t)(0)) >> mod;
}
PhysicalDeviceSelector::~PhysicalDeviceSelector() noexcept = default;

PhysicalDeviceSelector& PhysicalDeviceSelector::pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data)
{
	snassert(fSelector != nullptr, "fSelector can not be nullptr", "Provide a valid selector");

	for (size_t mask_idx = 0; mask_idx < m_suitableMask.size(); ++mask_idx)
	{
		size_t suitable_cnt = std::popcount(m_suitableMask[mask_idx]);
		if (suitable_cnt == 0) continue;

		size_t first_bit = mask_idx * s_bitsPerMask;
		for (size_t bit_idx = 0u; bit_idx < s_bitsPerMask; ++bit_idx)
		{
			if (!mask(first_bit + bit_idx)) continue;
			if (!fSelector(first_bit + bit_idx, *m_pRegistry, user_data)) flip(first_bit + bit_idx);
			if (--suitable_cnt == 0) break;
		}
	}
	return *this;
}

PhysicalDeviceSelector PhysicalDeviceSelector::pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data) const
{
	return PhysicalDeviceSelector(*this).pick(fSelector, user_data);
}

PhysicalDevice PhysicalDeviceSelector::get(size_t idx) const
{
	size_t suitable_idx = 0u;
	for (size_t mask_idx = 0; mask_idx < m_suitableMask.size(); ++mask_idx)
	{
		suitable_idx += std::popcount(m_suitableMask[mask_idx]);
		if (suitable_idx <= idx) continue;

		size_t first_bit = mask_idx * s_bitsPerMask;
		for (size_t bit_idx = s_bitsPerMask - 1u; bit_idx < s_bitsPerMask; --bit_idx)
		{
			if (!mask(first_bit + bit_idx)) continue;
			if (--suitable_idx == idx) return m_pRegistry->get(first_bit + bit_idx);
		}
	}
	throw std::invalid_argument("Out of PhysicalDeviceSelector::count()");
}
PhysicalDevice PhysicalDeviceSelector::first() const
{
	for (size_t mask_idx = 0; mask_idx < m_suitableMask.size(); ++mask_idx)
	{
		if (std::popcount(m_suitableMask[mask_idx]) == 0) continue;

		size_t first_bit = mask_idx * s_bitsPerMask;
		for (size_t bit_idx = 0u; bit_idx < s_bitsPerMask; ++bit_idx)
		{
			if (!mask(first_bit + bit_idx)) continue;
			return m_pRegistry->get(first_bit + bit_idx);
		}
	}
	throw std::invalid_argument("Out of PhysicalDeviceSelector::count()");
}
PhysicalDevice PhysicalDeviceSelector::last() const
{
	for (size_t mask_idx = m_suitableMask.size() - 1u; mask_idx < m_suitableMask.size(); --mask_idx)
	{
		if (std::popcount(m_suitableMask[mask_idx]) == 0) continue;

		size_t first_bit = mask_idx * s_bitsPerMask;
		for (size_t bit_idx = s_bitsPerMask - 1u; bit_idx < s_bitsPerMask; --bit_idx)
		{
			if (!mask(first_bit + bit_idx)) continue;
			return m_pRegistry->get(first_bit + bit_idx);
		}
	}
	throw std::invalid_argument("Out of PhysicalDeviceSelector::count()");
}
size_t PhysicalDeviceSelector::count() const noexcept
{
	size_t suitable_count = 0u;
	for (size_t mask_idx = 0u; mask_idx < m_suitableMask.size(); ++mask_idx)
		suitable_count += std::popcount(m_suitableMask[mask_idx]);
	return suitable_count;
}
