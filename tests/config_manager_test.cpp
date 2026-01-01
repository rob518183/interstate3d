#include <gtest/gtest.h>
#include "../src/core/ConfigManager.hpp"

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ConfigManagerTest, DefaultValues) {
    ConfigManager config;
    // Add test assertions here once we know the expected behavior
    EXPECT_TRUE(true); // Placeholder assertion
}

// Add more test cases as needed
