#pragma once

#include <gtest/gtest.h>

#include <filesystem>

class Test_ShaderCompiler : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    std::filesystem::path m_temp_dir;
    std::filesystem::path m_temp_shader;
};
