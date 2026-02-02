#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Rigidbody : public Component {
public:
    enum class Type {
        Static,      // Does not move, but can be collided
        Dynamic,     // Moves under physics
        Kinematic    // Moves by script, but affects other objects
    };

    Rigidbody() = default;
    virtual ~Rigidbody() = default;

    // Component overrides
    virtual void start() override;
    virtual void update(float deltaTime) override;
    virtual void onDestroy() override;
    virtual std::string getTypeName() const override { return "Rigidbody"; }

    // Physics properties
    void setType(Type type) { bodyType = type; }
    Type getType() const { return bodyType; }

    void setMass(float mass) { this->mass = mass > 0 ? mass : 0.1f; }
    float getMass() const { return mass; }

    void setDrag(float drag) { this->drag = glm::clamp(drag, 0.0f, 1.0f); }
    float getDrag() const { return drag; }

    void setAngularDrag(float drag) { this->angularDrag = glm::clamp(drag, 0.0f, 1.0f); }
    float getAngularDrag() const { return angularDrag; }

    void setFriction(float f) { friction = glm::clamp(f, 0.0f, 1.0f); }
    float getFriction() const { return friction; }

    void setBounciness(float b) { bounciness = glm::clamp(b, 0.0f, 1.0f); }
    float getBounciness() const { return bounciness; }

    void setUseGravity(bool use) { useGravity = use; }
    bool getUseGravity() const { return useGravity; }

    void setFreezeRotation(bool freeze) { freezeRotation = freeze; }
    bool getFreezeRotation() const { return freezeRotation; }

    // Velocity control
    void setVelocity(const glm::vec3& vel) { velocity = vel; }
    glm::vec3 getVelocity() const { return velocity; }

    void setAngularVelocity(const glm::vec3& vel) { angularVelocity = vel; }
    glm::vec3 getAngularVelocity() const { return angularVelocity; }

    void addForce(const glm::vec3& force, bool isImpulse = false);
    void addTorque(const glm::vec3& torque);

    // State
    bool isGrounded() const { return grounded; }
    void setGrounded(bool g) { grounded = g; }

    // Access for PhysicsEngine
    friend class PhysicsEngine;

private:
    Type bodyType = Type::Dynamic;
    float mass = 1.0f;
    float drag = 0.1f;
    float angularDrag = 0.05f;
    float friction = 0.5f;      // Surface friction (0-1)
    float bounciness = 0.0f;    // Bounce coefficient (0-1)
    bool useGravity = true;
    bool freezeRotation = false;

    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 angularVelocity = glm::vec3(0.0f);
    glm::vec3 accumulatedForce = glm::vec3(0.0f);
    glm::vec3 accumulatedTorque = glm::vec3(0.0f);

    bool grounded = false;
};
