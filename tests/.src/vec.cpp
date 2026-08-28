#include <tests/tests/vec.hpp>

#include <snvoxeng/snvoxeng/math/vec.hpp>

// === Tests Context ===

void Test_vec::SetUp() {}
void Test_vec::TearDown() {}

// === Tests ===

TEST_F(Test_vec, LayoutAndAlignment)
{
    static_assert(sizeof(sn::voxeng::math::vec2) == 8u);
    static_assert(alignof(sn::voxeng::math::vec2) == 8u);
    static_assert(sizeof(sn::voxeng::math::vec4) == 16u);
    static_assert(alignof(sn::voxeng::math::vec4) == 16u);

    EXPECT_TRUE(std::is_trivially_default_constructible_v<sn::voxeng::math::vec4>);
}

TEST_F(Test_vec, MemberAccess)
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
