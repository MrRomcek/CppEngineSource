#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class Rigidbody;

class PhysicsEngine {
public:
    static PhysicsEngine& getInstance() {
        static PhysicsEngine instance;
        return instance;
    }

    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;

    void initialize();
    void update(float deltaTime);
    void setGravity(const glm::vec3& gravity) { this->gravity = gravity; }
    const glm::vec3& getGravity() const { return gravity; }

    void registerRigidbody(Rigidbody* rigidbody);
    void unregisterRigidbody(Rigidbody* rigidbody);

private:
    PhysicsEngine() = default;
    ~PhysicsEngine() = default;

    void applyGravity(float deltaTime);
    void resolveCollisions(float deltaTime);

    glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    std::vector<Rigidbody*> rigidbodies;
};
