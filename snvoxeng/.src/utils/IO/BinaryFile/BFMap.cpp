#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFMap.hpp>

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFHandle.hpp>

using namespace sn::voxeng::io;

BFMapBase::BFMapBase(const BFHandle& handle, size_t offset, size_t size)
	: m_pHandle(&handle)
	, m_offset(offset)
	, m_size(size)
{
}

BFMapBase::BFMapBase(BFMapBase&& other) noexcept
	: m_pHandle(std::exchange(other.m_pHandle, nullptr))
	, m_offset(std::exchange(other.m_offset, 0))
	, m_size(std::exchange(other.m_size, 0))
{
}
BFMapBase& BFMapBase::operator=(BFMapBase&& other) noexcept
{
	if (this != &other)
	{
		m_pHandle = std::exchange(other.m_pHandle, nullptr);
		m_offset = std::exchange(other.m_offset, 0);
		m_size = std::exchange(other.m_size, 0);
	}
	return *this;
}

BFMapBase::~BFMapBase() noexcept { if (m_pHandle) m_pHandle->onMapRelease(); }

const BFHandle& BFMapBase::getHandle() const noexcept { return *m_pHandle; }

size_t BFMapBase::offset() const noexcept { return m_offset; }
size_t BFMapBase::size() const noexcept { return m_size; }

eBFMapAccessFlags BFMapBase::accessFlags() const noexcept { return eBFMapAccessFlags::NONE; }

BFViewRO BFMapBase::viewRO() const { throw std::logic_error("RO View is not allowed."); }
BFViewRW BFMapBase::viewRW() { throw std::logic_error("RW View is not allowed."); }
