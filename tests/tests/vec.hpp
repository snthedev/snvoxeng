#pragma once

#include <gtest/gtest.h>

class Test_vec : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};
