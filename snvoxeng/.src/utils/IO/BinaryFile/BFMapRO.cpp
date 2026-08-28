#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFMap.hpp>

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFHandle.hpp>

#include <thirdparty/mio/mio.hpp>

using namespace sn::voxeng::io;

class BFMapRO::impl
{
	mio::mmap_source m_map;

public:
	impl(BFMapBase& base)
	{
		std::error_code error;
		m_map.map(base.getHandle().path().native(), base.offset(), base.size(), error);
		if (error) throw std::system_error(error, "Failed to map file " + base.getHandle().path().string() + ".");
	}
	~impl() noexcept = default;

	BFViewRO viewRO() const noexcept { return { reinterpret_cast<const uint8_t*>(m_map.data()), m_map.size() }; };
};

BFMapRO::BFMapRO(BFMapBase& base)
	: BFMapBase(std::move(base))
	, m_pImpl(new impl{ *this })
{
}
BFMapRO::BFMapRO(BFMapBase&& base)
	: BFMapBase(std::move(base))
	, m_pImpl(new impl{ *this })
{
}

BFMapRO::BFMapRO(BFMapRO&& other) noexcept
	: BFMapBase(std::move((BFMapBase&&)(other)))
	, m_pImpl(std::exchange(other.m_pImpl, nullptr))
{
}
BFMapRO& BFMapRO::operator=(BFMapRO&& other) noexcept
{
	if (this != &other)
	{
		*this = std::move((BFMapBase&&)(other));
		m_pImpl = std::exchange(other.m_pImpl, nullptr);
	}
	return *this;
}

BFMapRO::~BFMapRO() noexcept { delete m_pImpl; }

eBFMapAccessFlags BFMapRO::accessFlags() const noexcept { return eBFMapAccessFlags::eRead; }
BFViewRO BFMapRO::viewRO() const { return m_pImpl->viewRO(); }
