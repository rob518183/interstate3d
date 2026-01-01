#include <gtest/gtest.h>
#include "../src/physics/PhysicsWorld.hpp"

class PhysicsWorldTest : public ::testing::Test {
protected:
    void SetUp() override {
        world = std::make_unique<PhysicsWorld>();
    }

    void TearDown() override {
        if (world) {
            world->shutdown();
        }
    }

    std::unique_ptr<PhysicsWorld> world;
};

TEST_F(PhysicsWorldTest, Initialization) {
    EXPECT_TRUE(world->initialize());
}

TEST_F(PhysicsWorldTest, Update) {
    ASSERT_TRUE(world->initialize());
    world->update(0.016f); // Simulate one frame at 60 FPS
    // Add assertions for expected behavior after update
    EXPECT_TRUE(true); // Placeholder
}

TEST_F(PhysicsWorldTest, Gravity) {
    ASSERT_TRUE(world->initialize());
    glm::vec3 testGravity(0.0f, -15.0f, 0.0f);
    world->setGravity(testGravity);
    // Note: Testing internal state would require exposing getters
    EXPECT_TRUE(true); // Placeholder
}

// Add more tests for raycasting, collision, etc.
TEST_F(PhysicsWorldTest, Raycast) {
    ASSERT_TRUE(world->initialize());
    glm::vec3 from(0.0f, 10.0f, 0.0f);
    glm::vec3 to(0.0f, -10.0f, 0.0f);
    auto result = world->raycast(from, to);
    // In empty world, should not hit
    EXPECT_FALSE(result.hit);
}
