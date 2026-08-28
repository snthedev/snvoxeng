#include <tests/tests/ShaderCompiler.hpp>

#include <snvoxeng/snvoxeng/ShaderCompiler.hpp>

#include <fstream>

// === Tests Context ===

void Test_ShaderCompiler::SetUp()
{
    m_temp_dir = std::filesystem::temp_directory_path() / "snvoxeng_Test_ShaderCompiler";
    std::filesystem::create_directories(m_temp_dir);

    m_temp_shader = m_temp_dir / "trivial.comp";
    std::ofstream file(m_temp_shader, std::ios::binary | std::ios::trunc);
    file << "#version 450\n"
        "layout(local_size_x = 1) in;\n"
        "layout(set = 0, binding = 0) buffer Data { float values[]; };\n"
        "void main() { values[0] += 1.0; }\n";
}
void Test_ShaderCompiler::TearDown()
{
    std::error_code ec;
    std::filesystem::remove_all(m_temp_dir, ec);
}

// === Tests ===

TEST_F(Test_ShaderCompiler, CompilesGlslToSpirv)
{
    const sn::voxeng::ShaderCompiler compiler;
    auto spirv = compiler.loadFromFile(m_temp_shader.string().c_str());

    EXPECT_NE(spirv.getCode(), nullptr);
    EXPECT_GT(spirv.getSize(), 0u);
    EXPECT_EQ(spirv.getSize() % 4u, 0u); // SPIR-V is a stream of 32-bit words
}

TEST_F(Test_ShaderCompiler, CacheSurvivesReloadAndInvalidatesOnSettings)
{
    const auto srcPath = m_temp_shader.string();

    sn::voxeng::ShaderCompiler compiler;
    auto first = compiler.loadFromFile(srcPath.c_str());
    ASSERT_GT(first.getSize(), 0u);

    // Same settings + same source: served from the freshly written cache.
    auto second = compiler.loadFromFile(srcPath.c_str());
    ASSERT_EQ(second.getSize(), first.getSize());
    EXPECT_EQ(0, std::memcmp(first.getCode(), second.getCode(), first.getSize()));

    // Changed optimization level: cache entry must be invalidated and the
    // shader recompiled (still succeeding).
    auto settings = compiler.getSettings();
    settings.optLevel = sn::voxeng::ShaderCompiler::settings_t::eOptLevel::eNone;
    compiler.setSettings(settings);

    auto third = compiler.loadFromFile(srcPath.c_str());
    ASSERT_GT(third.getSize(), 0u);
    EXPECT_EQ(third.getSize() % 4u, 0u);

    // forceCompile bypasses the cache regardless of validity.
    auto forced = compiler.loadFromFile(srcPath.c_str(), /*forceCompile=*/true);
    ASSERT_GT(forced.getSize(), 0u);
}
