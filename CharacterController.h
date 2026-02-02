#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Rigidbody;
class Collider;
class Camera;

class CharacterController : public Component {
public:
    CharacterController() = default;
    virtual ~CharacterController() = default;

    virtual void start() override;
    virtual void update(float deltaTime) override;
    virtual std::string getTypeName() const override { return "CharacterController"; }

    // Управление движением
    void moveForward(float speed);
    void moveRight(float speed);
    void jump(float jumpForce = 5.0f);
    void stop();

    // Параметры движения
    void setMoveSpeed(float speed) { moveSpeed = glm::max(speed, 0.0f); }
    float getMoveSpeed() const { return moveSpeed; }

    void setJumpForce(float force) { jumpForce = force; }
    float getJumpForce() const { return jumpForce; }

    void setGroundDrag(float drag) { groundDrag = glm::clamp(drag, 0.0f, 1.0f); }
    float getGroundDrag() const { return groundDrag; }

    void setAirDrag(float drag) { airDrag = glm::clamp(drag, 0.0f, 1.0f); }
    float getAirDrag() const { return airDrag; }

    // Состояние
    bool isGrounded() const { return grounded; }
    bool isJumping() const { return jumping; }
    glm::vec3 getMovementDirection() const { return movementDirection; }

    // Ground layer for collision check
    void setGroundLayer(int layer) { groundLayer = layer; }

    // Camera for movement direction
    void setCamera(Camera* cam) { camera = cam; }
    Camera* getCamera() const { return camera; }

private:
    void applyMovement(float deltaTime);

    Rigidbody* rigidbody = nullptr;
    Collider* collider = nullptr;
    Camera* camera = nullptr;

    // Movement parameters
    float moveSpeed = 5.0f;
    float jumpForce = 5.0f;
    float groundDrag = 0.9f;
    float airDrag = 0.01f;

    // State
    glm::vec3 movementDirection = glm::vec3(0.0f);
    bool grounded = false;
    bool jumping = false;

    // Layer for ground collision
    int groundLayer = 0;

    // Для ограничения скорости падения
    float maxFallSpeed = 30.0f;
};
