#define _CRT_SECURE_NO_WARNINGS

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFHandle.hpp>

#include <mutex>
#include <algorithm>

// === Utils ===

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)

#include <io.h>

#define NATIVE_CLOSE_FILE _close
#define NATIVE_OPEN_FILE(path, flags, mode) _wopen(path, flags, mode)

#else

#include <unistd.h>

#define NATIVE_CLOSE_FILE close
#define NATIVE_OPEN_FILE(path, flags, mode) open(path, flags, mode)

#endif

static bool createEmptyFile(const std::filesystem::path& path)
{
    const int fd = NATIVE_OPEN_FILE(
        path.c_str(),
        O_WRONLY | O_CREAT | O_EXCL,
        S_IREAD | S_IWRITE
    );

    if (fd == -1)
    {
        const int error = errno;

        if (error == EEXIST) return false;

        throw std::system_error(
            error,
            std::generic_category(),
            "Failed to create file: " + path.string()
        );
    }

    if (NATIVE_CLOSE_FILE(fd) == -1)
    {
        const int error = errno;
        throw std::system_error(
            error,
            std::generic_category(),
            "Failed to close file: " + path.string()
        );
    }

    return true;
}

// === BFHandle ===

using namespace sn::voxeng::io;

namespace details
{
    BFHandle::eNotFoundBehaviourResult findFile_eThrow(const std::filesystem::path& path)
    {
        const auto status = std::filesystem::status(path);

        switch (status.type())
        {
        [[likely]] case std::filesystem::file_type::regular:
            return BFHandle::eNotFoundBehaviourResult::eFileFound;
        case std::filesystem::file_type::not_found:
            throw std::invalid_argument("File is not found.");
        default:
            throw std::invalid_argument("File type mismatch.");
        }
    }
    BFHandle::eNotFoundBehaviourResult findFile_eCreateFile(const std::filesystem::path& path)
    {
        const auto status = std::filesystem::status(path);

        switch (status.type())
        {
        [[likely]] case std::filesystem::file_type::regular:
            return BFHandle::eNotFoundBehaviourResult::eFileFound;
        case std::filesystem::file_type::not_found:
            break;
        default:
            throw std::invalid_argument("File type mismatch.");
        }

        if (!createEmptyFile(path))
            throw std::runtime_error("Failed to create file: " + path.string());

        return BFHandle::eNotFoundBehaviourResult::eFileCreated;
    }
    BFHandle::eNotFoundBehaviourResult findFile_eCreatePathAndFile(const std::filesystem::path& path)
    {
        const auto status = std::filesystem::status(path);

        switch (status.type())
        {
        [[likely]] case std::filesystem::file_type::regular:
            return BFHandle::eNotFoundBehaviourResult::eFileFound;
        case std::filesystem::file_type::not_found:
            break;
        default:
            throw std::invalid_argument("File type mismatch.");
        }

        bool dirs_created{ false };
        const auto parent = path.parent_path();
        if (!parent.empty()) dirs_created = std::filesystem::create_directories(parent);

        if (!createEmptyFile(path))
            throw std::runtime_error("Failed to create file: " + path.string());

        return dirs_created
            ? BFHandle::eNotFoundBehaviourResult::ePathAndFileCreated
            : BFHandle::eNotFoundBehaviourResult::eFileCreated;
    }

    typedef BFHandle::eNotFoundBehaviourResult(*pfn_findFile)(const std::filesystem::path& path);
    constexpr static pfn_findFile pfn_findFiles[static_cast<uint8_t>(BFHandle::eNotFoundBehaviour::MAX_ENUM)]
    {
        findFile_eThrow,
        findFile_eCreateFile,
        findFile_eCreatePathAndFile
    };
}

class BFHandle::impl
{
    eNotFoundBehaviourResult m_notFoundBehaviourResult;

    mutable size_t m_mapsCount{ 0 };
    mutable std::mutex m_mapsCount_mtx{};
    std::filesystem::path m_path;

    static eNotFoundBehaviourResult findFile(const std::filesystem::path& path, eNotFoundBehaviour not_found_behaviour)
    {
        assert(static_cast<uint8_t>(not_found_behaviour) <static_cast<uint8_t>(eNotFoundBehaviour::MAX_ENUM));
        return details::pfn_findFiles[static_cast<uint8_t>(not_found_behaviour)](path);
    }

    mutable uintmax_t m_cachedSize{ 0 };
    mutable bool m_isSizeDirty{ true };
    mutable std::mutex m_cachedSize_mtx{};

    static constexpr size_t s_page_size = 4096u;
    static constexpr size_t s_max_bit_ceil_waste = 4u * 1024u * 1024u;
    static constexpr size_t s_growth_padding = 2048u;

    size_t calculateGrowth(size_t required_size) const
    {
        // Prefer power-of-two growth while the amount of unused
        // space remains reasonably small.
        if (required_size > (std::numeric_limits<size_t>::max() >> 1))
            throw std::length_error("File size overflow.");
        const auto bit_ceil_size =
            std::bit_ceil(required_size);

        const auto min_size = s_page_size;

        const auto opt_size =
            std::max(min_size, bit_ceil_size);

        if (opt_size >= required_size
            && opt_size - required_size < s_max_bit_ceil_waste)
        {
            return opt_size;
        }

        // Large allocations are rounded to the physical page size
        // instead of potentially jumping to a huge power of two.
        //
        // Physical:
        //     header + logical_size
        //
        // Round it up to a page and add one extra page of growth.
        if (required_size > std::numeric_limits<size_t>::max() - s_growth_padding)
            throw std::length_error("Map size overflow.");
        const auto physical_required = required_size + s_growth_padding;

        const auto physical_size =
            ((physical_required / s_page_size) + 1u) * s_page_size;

        if (physical_size < physical_required)
            throw std::length_error("File size overflow.");

        return physical_size;
    }

public:
    void onMapRelease() const noexcept
    {
        std::lock_guard<std::mutex> mapslock(m_mapsCount_mtx);
        --m_mapsCount;
    }

    impl(const std::filesystem::path& path, eNotFoundBehaviour not_found_behaviour)
        : m_notFoundBehaviourResult(findFile(path, not_found_behaviour))
        , m_path(path)
    {
    }
    ~impl() noexcept = default;

    BFHandle::eNotFoundBehaviourResult getNotFoundBehaviourResult() const noexcept { return m_notFoundBehaviourResult; }

    void resize(size_t new_size)
    {
        std::lock_guard<std::mutex> mapslock(m_mapsCount_mtx);
        if (m_mapsCount != 0)
            throw std::runtime_error("Unable to resize file, while at least one map exist.");

        std::lock_guard<std::mutex> sizelock(m_cachedSize_mtx);
        m_isSizeDirty = true;

        // jail + лимит размера + сама операция - внутри обёртки
        std::filesystem::resize_file(m_path, static_cast<std::uint64_t>(new_size));

        m_cachedSize = new_size;
        m_isSizeDirty = false;
    }
    size_t size() const
    {
        std::lock_guard<std::mutex> sizelock(m_cachedSize_mtx);
        if (!m_isSizeDirty) return m_cachedSize;

        m_cachedSize = std::filesystem::file_size(m_path);
        m_isSizeDirty = false;
        return m_cachedSize;
    }

    const std::filesystem::path& path() const noexcept { return m_path; }

    bool checkBounds(size_t offset, size_t& size) const
    {
        if (size == BFMapBase::s_entire_file) size = this->size();
        else if (offset > this->size() || size > this->size() - offset)
            return false;
        return true;
    }

    BFMapBase map_base(const BFHandle& self, size_t offset, size_t size) const
    {
        std::lock_guard<std::mutex> mapslock(m_mapsCount_mtx);
        ++m_mapsCount;
        try { return { self, offset, size }; }
        catch (...)
        {
            --m_mapsCount;
            throw;
        }
    }

    BFMapRO map_ro(const BFHandle& self, size_t offset, size_t size) const
    {
        if (offset > std::numeric_limits<size_t>::max() - size)
            throw std::length_error("Map size overflow.");

        if (!checkBounds(offset, size))
            throw std::out_of_range("Map is out of file bounds.");

        return BFMapRO{ map_base(self, offset, size) };
    }
    BFMapRW map_rw(BFHandle& self, size_t offset, size_t size)
    {
        if (offset > std::numeric_limits<size_t>::max() - size)
            throw std::length_error("Map size overflow.");

        if (!checkBounds(offset, size))
            resize(calculateGrowth(offset + size));

        return BFMapRW{ map_base(self, offset, size) };
    }
};

void BFHandle::onMapRelease() const noexcept { m_pImpl->onMapRelease(); }



void BFHandle::impldefault_resize(size_t new_size) { m_pImpl->resize(new_size); }
size_t BFHandle::impldefault_size() const { return m_pImpl->size(); }

BFMapRO BFHandle::impldefault_map_ro(size_t offset, size_t size) const { return m_pImpl->map_ro(*this, offset, size); }
BFMapRW BFHandle::impldefault_map_rw(size_t offset, size_t size) { return m_pImpl->map_rw(*this, offset, size); }



BFHandle::BFHandle(const std::filesystem::path& path, eNotFoundBehaviour not_found_behaviour) : m_pImpl(new impl{ path, not_found_behaviour }) {}
BFHandle::~BFHandle() noexcept { delete m_pImpl; }



BFHandle::eNotFoundBehaviourResult BFHandle::getNotFoundBehaviourResult() const noexcept { return m_pImpl->getNotFoundBehaviourResult(); }

const std::filesystem::path& BFHandle::path() const noexcept { return m_pImpl->path(); }

BFMapBase BFHandle::map(size_t offset, size_t size) const { return map_ro(offset, size); }
BFMapBase BFHandle::map(size_t offset, size_t size) { return map_rw(offset, size); }



void BFHandle::resize(size_t new_size) { impldefault_resize(new_size); }
size_t BFHandle::size() const { return impldefault_size(); }

BFMapRO BFHandle::map_ro(size_t offset, size_t size) const { return impldefault_map_ro(offset, size); }
BFMapRW BFHandle::map_rw(size_t offset, size_t size) { return impldefault_map_rw(offset, size); }
