// ============================================================================
// snvoxeng_tests.cpp — headless GoogleTest suite for snvoxeng utilities.
// No window, no GPU work: everything here must run in CI.
// ============================================================================

#include <gtest/gtest.h>

#include <snvoxeng/snvoxeng.hpp>

#include <string>
#include <type_traits>
#include <utility>

// ---------------------------------------------------------------------------
// dumb_vector
// ---------------------------------------------------------------------------

TEST(DumbVector, DefaultState)
{
    sn::voxeng::dumb_vector<int> v;
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
    EXPECT_TRUE(v.empty());
}

TEST(DumbVector, ReserveThenEmplace)
{
    sn::voxeng::dumb_vector<int> v;
    v.reserve(4);
    EXPECT_EQ(v.capacity(), 4u);
    EXPECT_EQ(v.size(), 0u);          // reserve() allocates but stays empty

    for (int i = 0; i < 4; ++i)
        v.emplace_back(i * 10);

    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[3], 30);

    int sum = 0;
    for (int x : v) sum += x;         // iteration over live elements only
    EXPECT_EQ(sum, 0 + 10 + 20 + 30);
}

TEST(DumbVector, EmplaceBackReturnsReference)
{
    sn::voxeng::dumb_vector<std::string> v;
    v.reserve(2);

    std::string& ref = v.emplace_back("abc");
    EXPECT_EQ(ref, "abc");
    EXPECT_EQ(&ref, &v.back());       // returned reference points into the buffer

    v.emplace_back(std::string("def"));
    EXPECT_EQ(v.back(), "def");
}

TEST(DumbVector, ClearKeepsCapacity)
{
    sn::voxeng::dumb_vector<int> v;
    v.reserve(3);
    v.emplace_back(1);
    v.emplace_back(2);

    v.clear();
    EXPECT_EQ(v.size(), 0u);
    EXPECT_TRUE(v.empty());

    // storage is still there: can fill it again without re-reserving
    v.emplace_back(7);
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 7);
}

TEST(DumbVector, MoveSemantics)
{
    sn::voxeng::dumb_vector<int> a;
    a.reserve(2);
    a.emplace_back(42);

    sn::voxeng::dumb_vector<int> b{ std::move(a) };
    ASSERT_EQ(b.size(), 1u);
    EXPECT_EQ(b[0], 42);
    EXPECT_EQ(a.size(), 0u);          // moved-out source is empty

    sn::voxeng::dumb_vector<int> c;
    c = std::move(b);
    ASSERT_EQ(c.size(), 1u);
    EXPECT_EQ(c[0], 42);
}

TEST(DumbVector, SpanConversion)
{
    sn::voxeng::dumb_vector<float> v;
    v.reserve(3);
    v.emplace_back(1.f);
    v.emplace_back(2.f);

    std::span<const float> s = v;
    ASSERT_EQ(s.size(), 2u);
    EXPECT_EQ(s[0], 1.f);
    EXPECT_EQ(s[1], 2.f);
}

TEST(DumbVector, NonTrivialElementType)
{
    // elements are properly constructed/destroyed via placement new
    sn::voxeng::dumb_vector<std::string> v;
    v.reserve(2);
    v.emplace_back(4u, 'x');
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v.front(), "xxxx");

    v.clear();                        // destructors run here
    v.emplace_back("reused");
    EXPECT_EQ(v.back(), "reused");
}

// ---------------------------------------------------------------------------
// math::vec
// ---------------------------------------------------------------------------

TEST(Vec, LayoutAndAlignment)
{
    static_assert(sizeof(sn::voxeng::math::vec2) == 8u);
    static_assert(alignof(sn::voxeng::math::vec2) == 8u);
    static_assert(sizeof(sn::voxeng::math::vec4) == 16u);
    static_assert(alignof(sn::voxeng::math::vec4) == 16u);

    EXPECT_TRUE(std::is_trivially_default_constructible_v<sn::voxeng::math::vec4>);
}

TEST(Vec, MemberAccess)
{
    sn::voxeng::math::vec4 v{ 1.f, 2.f, 3.f, 4.f };
    EXPECT_EQ(v.x, 1.f);
    EXPECT_EQ(v.y, 2.f);
    EXPECT_EQ(v.z, 3.f);
    EXPECT_EQ(v.w, 4.f);

    sn::voxeng::math::vec2 p{ 5.f, 6.f };
    p.x += 1.f;
    EXPECT_EQ(p.x, 6.f);
    EXPECT_EQ(p.y, 6.f);
}
