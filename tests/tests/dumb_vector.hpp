#pragma once

#include <gtest/gtest.h>

class Test_dumb_vector : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};
