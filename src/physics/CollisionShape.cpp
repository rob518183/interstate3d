#include "physics/CollisionShape.hpp"
#include "core/Logger.hpp"
#include <numbers>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

// Define pi constant if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CollisionShape::CollisionShape() = default;

CollisionShape::~CollisionShape() {
    cleanup();
}

bool CollisionShape::createBoxShape(const glm::vec3& halfExtents) {
    cleanup();
    
    m_shape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    m_type = Type::BOX;
    m_boxHalfExtents = halfExtents;
    
    Logger::debug("Created box collision shape with half extents: " + 
                std::to_string(halfExtents.x) + ", " + std::to_string(halfExtents.y) + ", " + 
                std::to_string(halfExtents.z));
    return true;
}

bool CollisionShape::createSphereShape(float radius) {
    cleanup();
    
    m_shape = new btSphereShape(radius);
    m_type = Type::SPHERE;
    m_sphereRadius = radius;
    
    Logger::debug("Created sphere collision shape with radius: " + std::to_string(radius));
    return true;
}

bool CollisionShape::createCapsuleShape(float radius, float height) {
    cleanup();
    
    m_shape = new btCapsuleShape(radius, height);
    m_type = Type::CAPSULE;
    m_capsuleRadius = radius;
    m_capsuleHeight = height;
    
    Logger::debug("Created capsule collision shape with radius: " + std::to_string(radius) + 
                ", height: " + std::to_string(height));
    return true;
}

bool CollisionShape::createCylinderShape(const glm::vec3& halfExtents) {
    cleanup();
    
    m_shape = new btCylinderShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    m_type = Type::CYLINDER;
    m_cylinderHalfExtents = halfExtents;
    
    Logger::debug("Created cylinder collision shape with half extents: " + 
                std::to_string(halfExtents.x) + ", " + std::to_string(halfExtents.y) + ", " + 
                std::to_string(halfExtents.z));
    return true;
}

bool CollisionShape::createConeShape(float radius, float height) {
    cleanup();
    
    m_shape = new btConeShape(radius, height);
    m_type = Type::CONE;
    m_coneRadius = radius;
    m_coneHeight = height;
    
    Logger::debug("Created cone collision shape with radius: " + std::to_string(radius) + 
                ", height: " + std::to_string(height));
    return true;
}

bool CollisionShape::createMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices) {
    cleanup();
    
    if (vertices.empty()) {
        Logger::error("Cannot create mesh shape with empty vertices");
        return false;
    }
    
    // Convert vertices to Bullet format
    btVector3* bulletVertices = new btVector3[vertices.size()];
    for (size_t i = 0; i < vertices.size(); ++i) {
        bulletVertices[i] = btVector3(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    
    // Create triangle index array
    int* triangleIndices = nullptr;
    int numTriangles = 0;
    
    if (!indices.empty()) {
        numTriangles = indices.size() / 3;
        triangleIndices = new int[indices.size()];
        for (size_t i = 0; i < indices.size(); ++i) {
            triangleIndices[i] = indices[i];
        }
    } else {
        // Generate triangles from vertices (assuming triangle strip or similar)
        numTriangles = (vertices.size() - 2) / 3;
        triangleIndices = new int[numTriangles * 3];
        for (int i = 0; i < numTriangles; ++i) {
            triangleIndices[i * 3] = i;
            triangleIndices[i * 3 + 1] = i + 1;
            triangleIndices[i * 3 + 2] = i + 2;
        }
    }
    
    // Create triangle mesh
    btTriangleMesh* triangleMesh = new btTriangleMesh();
    for (int i = 0; i < numTriangles; ++i) {
        int i0 = triangleIndices[i * 3];
        int i1 = triangleIndices[i * 3 + 1];
        int i2 = triangleIndices[i * 3 + 2];
        
        if (i0 < vertices.size() && i1 < vertices.size() && i2 < vertices.size()) {
            triangleMesh->addTriangle(bulletVertices[i0], bulletVertices[i1], bulletVertices[i2]);
        }
    }
    
    // Create mesh shape
    m_shape = new btBvhTriangleMeshShape(triangleMesh, true);
    m_type = Type::MESH;
    
    // Clean up temporary arrays
    delete[] bulletVertices;
    delete[] triangleIndices;
    
    Logger::debug("Created mesh collision shape with " + std::to_string(vertices.size()) + 
                " vertices, " + std::to_string(numTriangles) + " triangles");
    return true;
}

bool CollisionShape::createHeightfieldShape(int width, int height, const std::vector<float>& heights,
                                          float minHeight, float maxHeight, const glm::vec3& scale) {
    cleanup();
    
    if (width <= 0 || height <= 0 || heights.size() != static_cast<size_t>(width * height)) {
        Logger::error("Invalid heightfield parameters");
        return false;
    }
    
    // Create heightfield shape
    btHeightfieldTerrainShape* heightfield = new btHeightfieldTerrainShape(
        width, height, heights.data(), 
        1.0f, minHeight, maxHeight, 
        1, PHY_FLOAT, false
    );
    
    // Set local scaling
    heightfield->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
    
    m_shape = heightfield;
    m_type = Type::HEIGHTFIELD;
    
    Logger::debug("Created heightfield collision shape: " + std::to_string(width) + "x" + 
                std::to_string(height));
    return true;
}

glm::vec3 CollisionShape::getLocalScaling() const {
    if (!m_shape) {
        return glm::vec3(1.0f);
    }
    
    btVector3 scaling = m_shape->getLocalScaling();
    return glm::vec3(scaling.x(), scaling.y(), scaling.z());
}

void CollisionShape::setLocalScaling(const glm::vec3& scaling) {
    if (m_shape) {
        m_shape->setLocalScaling(btVector3(scaling.x, scaling.y, scaling.z));
    }
}

void CollisionShape::setMargin(float margin) {
    if (m_shape) {
        m_shape->setMargin(margin);
    }
}

float CollisionShape::getVolume() const {
    if (!m_shape) {
        return 0.0f;
    }
    
    switch (m_type) {
        case Type::BOX:
            return 8.0f * m_boxHalfExtents.x * m_boxHalfExtents.y * m_boxHalfExtents.z;
        case Type::SPHERE:
            return (4.0f / 3.0f) * M_PI * m_sphereRadius * m_sphereRadius * m_sphereRadius;
        case Type::CAPSULE:
            return M_PI * m_capsuleRadius * m_capsuleRadius * 
           (2.0f * m_capsuleRadius + m_capsuleHeight);
        case Type::CYLINDER:
            return M_PI * m_cylinderHalfExtents.x * m_cylinderHalfExtents.x * 
           (2.0f * m_cylinderHalfExtents.y);
        case Type::CONE:
            return (1.0f / 3.0f) * M_PI * m_coneRadius * m_coneRadius * m_coneHeight;
        default:
            return 0.0f;
    }
}

glm::vec3 CollisionShape::getInertia(float mass) const {
    if (!m_shape) {
        return glm::vec3(0.0f);
    }
    
    // Simple inertia calculation based on shape type
    switch (m_type) {
        case Type::BOX:
            return glm::vec3(
                mass * (m_boxHalfExtents.y * m_boxHalfExtents.y + m_boxHalfExtents.z * m_boxHalfExtents.z) / 3.0f,
                mass * (m_boxHalfExtents.x * m_boxHalfExtents.x + m_boxHalfExtents.z * m_boxHalfExtents.z) / 3.0f,
                mass * (m_boxHalfExtents.x * m_boxHalfExtents.x + m_boxHalfExtents.y * m_boxHalfExtents.y) / 3.0f
            );
        case Type::SPHERE:
            return glm::vec3(0.4f * mass * m_sphereRadius * m_sphereRadius);
        default:
            return glm::vec3(1.0f);
    }
}

void CollisionShape::cleanup() {
    if (m_shape) {
        delete m_shape;
        m_shape = nullptr;
    }
}
