#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFMap.hpp>

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFHandle.hpp>

#include <thirdparty/mio/mio.hpp>

using namespace sn::voxeng::io;

class BFMapRW::impl
{
	mio::mmap_sink m_map;

public:
	impl(BFMapBase& base)
	{
		std::error_code error;
		m_map.map(base.getHandle().path().native(), base.offset(), base.size(), error);
		if (error) throw std::system_error(error, "Failed to map file.");
	}
	~impl() noexcept = default;

	BFViewRO viewRO() const noexcept { return { reinterpret_cast<const uint8_t*>(m_map.data()), m_map.size() }; };
	BFViewRW viewRW() noexcept { return { reinterpret_cast<uint8_t*>(m_map.data()), m_map.size() }; };

	void sync()
	{
		std::error_code error;
		m_map.sync(error);
		if (error) throw std::system_error(error, "Failed to sync file.");
	}
};

BFMapRW::BFMapRW(BFMapBase& base)
	: BFMapBase(std::move(base))
	, m_pImpl(new impl{ *this })
{
}
BFMapRW::BFMapRW(BFMapBase&& base)
	: BFMapBase(std::move(base))
	, m_pImpl(new impl{ *this })
{
}

BFMapRW::BFMapRW(BFMapRW&& other) noexcept
	: BFMapBase(std::move((BFMapBase&&)(other)))
	, m_pImpl(std::exchange(other.m_pImpl, nullptr))
{
}
BFMapRW& BFMapRW::operator=(BFMapRW&& other) noexcept
{
	if (this != &other)
	{
		*this = std::move((BFMapBase&&)(other));
		m_pImpl = std::exchange(other.m_pImpl, nullptr);
	}
	return *this;
}

BFMapRW::~BFMapRW() noexcept { delete m_pImpl; }

BFHandle& BFMapRW::getHandle() noexcept { return const_cast<BFHandle&>(static_cast<const BFMapBase&>(*this).getHandle()); }

eBFMapAccessFlags BFMapRW::accessFlags() const noexcept { return eBFMapAccessFlags::eRead | eBFMapAccessFlags::eWrite; }
BFViewRO BFMapRW::viewRO() const { return m_pImpl->viewRO(); }
BFViewRW BFMapRW::viewRW() { return m_pImpl->viewRW(); }

void BFMapRW::sync() { m_pImpl->sync(); }
