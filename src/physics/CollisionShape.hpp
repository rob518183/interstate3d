#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Forward declarations for Bullet Physics
class btCollisionShape;
class btBoxShape;
class btSphereShape;
class btCylinderShape;
class btConvexHullShape;

class RigidBody;
class PhysicsWorld;

class CollisionShape {
public:
    enum class Type {
        BOX,
        SPHERE,
        CAPSULE,
        CYLINDER,
        CONE,
        MESH,
        HEIGHTFIELD
    };
    
    CollisionShape();
    ~CollisionShape();
    
    // Shape creation
    bool createBoxShape(const glm::vec3& halfExtents);
    bool createSphereShape(float radius);
    bool createCapsuleShape(float radius, float height);
    bool createCylinderShape(const glm::vec3& halfExtents);
    bool createConeShape(float radius, float height);
    bool createMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices);
    bool createHeightfieldShape(int width, int height, const std::vector<float>& heights, 
                               float minHeight, float maxHeight, const glm::vec3& scale);
    
    // Getters
    Type getType() const { return m_type; }
    btCollisionShape* getBulletShape() const { return m_shape; }
    glm::vec3 getLocalScaling() const;
    
    // Setters
    void setLocalScaling(const glm::vec3& scaling);
    void setMargin(float margin);
    
    // Utility
    float getVolume() const;
    glm::vec3 getInertia(float mass) const;
    
private:
    btCollisionShape* m_shape = nullptr;
    Type m_type = Type::BOX;
    
    // Shape parameters (for recreation if needed)
    glm::vec3 m_boxHalfExtents;
    float m_sphereRadius;
    float m_capsuleRadius;
    float m_capsuleHeight;
    glm::vec3 m_cylinderHalfExtents;
    float m_coneRadius;
    float m_coneHeight;
    
    void cleanup();
};
