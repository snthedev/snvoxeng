#include <tests/tests/dumb_vector.hpp>

#include <snvoxeng/snvoxeng/utils/dumb_vector.hpp>

// === Tests Context ===

void Test_dumb_vector::SetUp() {}
void Test_dumb_vector::TearDown() {}

// === Tests ===

TEST_F(Test_dumb_vector, DefaultState)
{
    sn::voxeng::dumb_vector<int> v;
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
    EXPECT_TRUE(v.empty());
}

TEST_F(Test_dumb_vector, ReserveThenEmplace)
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

TEST_F(Test_dumb_vector, EmplaceBackReturnsReference)
{
    sn::voxeng::dumb_vector<std::string> v;
    v.reserve(2);

    std::string& ref = v.emplace_back("abc");
    EXPECT_EQ(ref, "abc");
    EXPECT_EQ(&ref, &v.back());       // returned reference points into the buffer

    v.emplace_back(std::string("def"));
    EXPECT_EQ(v.back(), "def");
}

TEST_F(Test_dumb_vector, ClearKeepsCapacity)
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

TEST_F(Test_dumb_vector, MoveSemantics)
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

TEST_F(Test_dumb_vector, SpanConversion)
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

TEST_F(Test_dumb_vector, NonTrivialElementType)
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
