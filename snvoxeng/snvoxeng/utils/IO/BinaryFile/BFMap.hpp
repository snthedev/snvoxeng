#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/utils/IO/BinaryFile/BFView.hpp>

namespace sn::voxeng::io
{
    class BFHandle;

    enum class eBFMapAccessFlags : uint8_t
    {
        NONE = 0,

        eRead  = 1 << 0,
        eWrite = 1 << 1,

        // hi and lo
        EXTREME_BITS,
        ALL_BITS = (EXTREME_BITS << 1) - 0b11
    };
    constexpr eBFMapAccessFlags operator&(eBFMapAccessFlags lhs, eBFMapAccessFlags rhs) noexcept
    {
        return static_cast<eBFMapAccessFlags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }
    constexpr eBFMapAccessFlags operator|(eBFMapAccessFlags lhs, eBFMapAccessFlags rhs) noexcept
    {
        return static_cast<eBFMapAccessFlags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }
    constexpr eBFMapAccessFlags operator^(eBFMapAccessFlags lhs, eBFMapAccessFlags rhs) noexcept
    {
        return static_cast<eBFMapAccessFlags>(static_cast<uint8_t>(lhs) ^ static_cast<uint8_t>(rhs));
    }
    constexpr eBFMapAccessFlags operator~(eBFMapAccessFlags val) noexcept
    {
        return static_cast<eBFMapAccessFlags>(~static_cast<uint8_t>(val) & static_cast<uint8_t>(eBFMapAccessFlags::ALL_BITS));
    }
    constexpr eBFMapAccessFlags& operator&=(eBFMapAccessFlags& lhs, eBFMapAccessFlags rhs) noexcept
    {
        return lhs = lhs & rhs;
    }
    constexpr eBFMapAccessFlags& operator|=(eBFMapAccessFlags& lhs, eBFMapAccessFlags rhs) noexcept
    {
        return lhs = lhs | rhs;
    }
    constexpr eBFMapAccessFlags& operator^=(eBFMapAccessFlags& lhs, eBFMapAccessFlags rhs) noexcept
    {
        return lhs = lhs ^ rhs;
    }

    class SNVOXENG_API BFMapBase
    {
    public:
        static constexpr size_t s_entire_file = ~static_cast<size_t>(0);

    private:
        friend class BFHandle;

        const BFHandle* m_pHandle;
        size_t m_offset;
        size_t m_size;

    protected:
        BFMapBase(const BFHandle& handle, size_t offset = 0, size_t size = s_entire_file);

    public:
        BFMapBase(const BFMapBase&) = delete;
        BFMapBase& operator=(const BFMapBase&) = delete;
        BFMapBase(BFMapBase&& other) noexcept;
        BFMapBase& operator=(BFMapBase&& other) noexcept;

        ~BFMapBase() noexcept;

        const BFHandle& getHandle() const noexcept;

        size_t offset() const noexcept;
        size_t size() const noexcept;

        virtual eBFMapAccessFlags accessFlags() const noexcept;

        virtual BFViewRO viewRO() const;
        virtual BFViewRW viewRW();
    };

    class SNVOXENG_API BFMapRO : public BFMapBase
    {
        class impl;
        impl* m_pImpl;

    public:
        BFMapRO(BFMapBase& base);
        BFMapRO(BFMapBase&& base);

        BFMapRO(const BFMapRO&) = delete;
        BFMapRO& operator=(const BFMapRO&) = delete;
        BFMapRO(BFMapRO&& other) noexcept;
        BFMapRO& operator=(BFMapRO&& other) noexcept;

        ~BFMapRO() noexcept;

        eBFMapAccessFlags accessFlags() const noexcept override;
        BFViewRO viewRO() const override;
    };

    class SNVOXENG_API BFMapRW : public BFMapBase
    {
        class impl;
        impl* m_pImpl;

    public:
        BFMapRW(BFMapBase& base);
        BFMapRW(BFMapBase&& base);

        BFMapRW(const BFMapRW&) = delete;
        BFMapRW& operator=(const BFMapRW&) = delete;
        BFMapRW(BFMapRW&& other) noexcept;
        BFMapRW& operator=(BFMapRW&& other) noexcept;

        ~BFMapRW() noexcept;

        BFHandle& getHandle() noexcept;

        eBFMapAccessFlags accessFlags() const noexcept override;
        BFViewRO viewRO() const override;
        BFViewRW viewRW() override;

        void sync();
    };
}
