#include <snvoxeng/snvoxeng/vk/PhysicalDeviceSelector.hpp>

#include <snassert/snassert.hpp>

using namespace sn::voxeng::vk;

void PhysicalDeviceSelector::initSuitableMaskStorage(mask_t value)
{
	if (m_pSuitableMask != m_suitableMaskBuf) delete[] m_pSuitableMask;

	// alloc new storage
	if (m_suitableMaskSize > s_mask_buf_size) [[unlikely]]
	{
		m_pSuitableMask = new mask_t[m_suitableMaskSize]{ value };
		return;
	}

	// use local buffer
	m_pSuitableMask = m_suitableMaskBuf;
	for (size_t i = 0; i < m_suitableMaskSize; ++i) m_pSuitableMask[i] = value;
}

bool PhysicalDeviceSelector::mask(size_t bit_idx) const noexcept
{
	snassert(
		bit_idx < m_suitableMaskSize * s_bitsPerMask,
		"Out of m_suitableMask",
		"Allowed range is [0; m_suitableMaskSize * s_bitsPerMask)"
	);

	constexpr mask_t mask_first = (mask_t)(1u) << (s_bitsPerMask - 1u);

	mask_t mask = mask_first >> (bit_idx % s_bitsPerMask);
	return (m_pSuitableMask[bit_idx / s_bitsPerMask] & mask) != 0;
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
	m_pSuitableMask[bit_idx / s_bitsPerMask] ^= mask;
}

PhysicalDeviceSelector::PhysicalDeviceSelector(const PhysicalDeviceRegistry& registry)
	: m_pRegistry(&registry)
	, m_pSuitableMask(m_suitableMaskBuf)
{
	size_t mod = m_pRegistry->count() % s_bitsPerMask;
	if (mod == 0u) [[unlikely]]
	{
		m_suitableMaskSize = m_pRegistry->count() / s_bitsPerMask;
		initSuitableMaskStorage();
		return;
	}

	m_suitableMaskSize = m_pRegistry->count() / s_bitsPerMask + 1u;
	initSuitableMaskStorage();

	m_pSuitableMask[m_suitableMaskSize - 1u] ^= (mask_t)(~(mask_t)(0)) >> mod;
}
PhysicalDeviceSelector::~PhysicalDeviceSelector() noexcept
{
	if (m_pSuitableMask != m_suitableMaskBuf) delete[] m_pSuitableMask;
}

PhysicalDeviceSelector::PhysicalDeviceSelector(const PhysicalDeviceSelector& other)
	: m_pRegistry(other.m_pRegistry)
	, m_suitableMaskSize(other.m_suitableMaskSize)
{
	if (m_suitableMaskSize > s_mask_buf_size) [[unlikely]]
		m_pSuitableMask = new mask_t[m_suitableMaskSize];
	else m_pSuitableMask = m_suitableMaskBuf;

	for (size_t i = 0; i < m_suitableMaskSize; ++i)
		m_pSuitableMask[i] = other.m_pSuitableMask[i];
}
PhysicalDeviceSelector& PhysicalDeviceSelector::operator=(const PhysicalDeviceSelector& other)
{
	if (this != &other) [[likely]]
	{
		if (m_pSuitableMask != m_suitableMaskBuf) delete[] m_pSuitableMask;

		m_pRegistry = other.m_pRegistry;
		m_suitableMaskSize = other.m_suitableMaskSize;

		if (m_suitableMaskSize > s_mask_buf_size) [[unlikely]]
			m_pSuitableMask = new mask_t[m_suitableMaskSize];
		else m_pSuitableMask = m_suitableMaskBuf;

		for (size_t i = 0; i < m_suitableMaskSize; ++i)
			m_pSuitableMask[i] = other.m_pSuitableMask[i];
	}
	return *this;
}

PhysicalDeviceSelector::PhysicalDeviceSelector(PhysicalDeviceSelector&& other) noexcept
	: m_pRegistry(other.m_pRegistry)
	, m_suitableMaskSize(other.m_suitableMaskSize)
{
	if (m_suitableMaskSize > s_mask_buf_size) [[unlikely]]
	{
		m_pSuitableMask = other.m_pSuitableMask;

		other.m_pSuitableMask = other.m_suitableMaskBuf;
		other.m_suitableMaskSize = 0;
	}
	else
	{
		m_pSuitableMask = m_suitableMaskBuf;
		for (size_t i = 0; i < m_suitableMaskSize; ++i)
			m_pSuitableMask[i] = other.m_pSuitableMask[i];
	}
}
PhysicalDeviceSelector& PhysicalDeviceSelector::operator=(PhysicalDeviceSelector&& other) noexcept
{
	if (this != &other) [[likely]]
	{
		if (m_pSuitableMask != m_suitableMaskBuf) delete[] m_pSuitableMask;

		m_pRegistry = other.m_pRegistry;
		m_suitableMaskSize = other.m_suitableMaskSize;

		if (m_suitableMaskSize > s_mask_buf_size) [[unlikely]]
		{
			m_pSuitableMask = other.m_pSuitableMask;

			other.m_pSuitableMask = other.m_suitableMaskBuf;
			other.m_suitableMaskSize = 0;
		}
		else
		{
			m_pSuitableMask = m_suitableMaskBuf;
			for (size_t i = 0; i < m_suitableMaskSize; ++i)
				m_pSuitableMask[i] = other.m_pSuitableMask[i];
		}
	}
	return *this;
}

PhysicalDeviceSelector& PhysicalDeviceSelector::pick(PhysicalDeviceRegistry::fSelector_t* fSelector, void* user_data)
{
	snassert(fSelector != nullptr, "fSelector can not be nullptr", "Provide a valid selector");

	for (size_t mask_idx = 0; mask_idx < m_suitableMaskSize; ++mask_idx)
	{
		size_t suitable_cnt = std::popcount(m_pSuitableMask[mask_idx]);
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
	for (size_t mask_idx = 0; mask_idx < m_suitableMaskSize; ++mask_idx)
	{
		suitable_idx += std::popcount(m_pSuitableMask[mask_idx]);
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
	for (size_t mask_idx = 0; mask_idx < m_suitableMaskSize; ++mask_idx)
	{
		if (std::popcount(m_pSuitableMask[mask_idx]) == 0) continue;

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
	for (size_t mask_idx = m_suitableMaskSize - 1u; mask_idx < m_suitableMaskSize; --mask_idx)
	{
		if (std::popcount(m_pSuitableMask[mask_idx]) == 0) continue;

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
	for (size_t mask_idx = 0u; mask_idx < m_suitableMaskSize; ++mask_idx)
		suitable_count += std::popcount(m_pSuitableMask[mask_idx]);
	return suitable_count;
}
