#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFMap.hpp>

#include <filesystem>
#include <mutex>

namespace sn::voxeng::io
{
    class SNVOXENG_API BFHandle
    {
    private:
        class impl;
        impl* m_pImpl;

        friend class BFMapBase;
        void onMapRelease() const noexcept;

    protected:
        void impldefault_resize(size_t new_size);
        size_t impldefault_size() const;

        BFMapRO impldefault_map_ro(size_t offset = 0, size_t size = BFMapBase::s_entire_file) const;
        BFMapRW impldefault_map_rw(size_t offset = 0, size_t size = BFMapBase::s_entire_file);

    public:
        enum class eNotFoundBehaviour : uint8_t
        {
            eThrow = 0,
            eCreateFile,
            eCreatePathAndFile,
            
            MAX_ENUM
        };
        enum class eNotFoundBehaviourResult : uint8_t
        {
            eFileFound = 0,
            eFileCreated,
            ePathAndFileCreated,

            MAX_ENUM
        };

        BFHandle(const std::filesystem::path& path, eNotFoundBehaviour not_found_behaviour = eNotFoundBehaviour::eCreatePathAndFile);
        ~BFHandle() noexcept;

        eNotFoundBehaviourResult getNotFoundBehaviourResult() const noexcept;

        BFHandle(const BFHandle&) = delete;
        BFHandle& operator=(const BFHandle&) = delete;
        BFHandle(BFHandle&&) = delete;
        BFHandle& operator=(BFHandle&&) = delete;

        const std::filesystem::path& path() const noexcept;

        BFMapBase map(size_t offset = 0, size_t size = BFMapBase::s_entire_file) const;
        BFMapBase map(size_t offset = 0, size_t size = BFMapBase::s_entire_file);

        virtual void resize(size_t new_size);
        virtual size_t size() const;

        virtual BFMapRO map_ro(size_t offset = 0, size_t size = BFMapBase::s_entire_file) const;
        virtual BFMapRW map_rw(size_t offset = 0, size_t size = BFMapBase::s_entire_file);
    };
}
