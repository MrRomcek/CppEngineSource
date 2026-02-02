#include "CharacterController.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "GameObject.h"
#include "Transform.h"
#include "Camera.h"
#include "Logger.h"

void CharacterController::start() {
    rigidbody = gameObject->getComponent<Rigidbody>();
    collider = gameObject->getComponent<Collider>();

    if (!rigidbody) {
        auto logger = LogManager::getInstance().createConsoleLogger("CharacterController");
        LOG_WARNING("CharacterController requires a Rigidbody component!");
    }
}

void CharacterController::update(float deltaTime) {
    if (!rigidbody || !enabled) return;

    // Use grounded state from rigidbody (set by physics engine)
    grounded = rigidbody->isGrounded();

    // Reset grounded flag after reading (physics will set it again if still on ground)
    rigidbody->setGrounded(false);

    if (grounded) {
        jumping = false;
    }

    applyMovement(deltaTime);

    // Limit fall speed
    if (rigidbody->getVelocity().y < -maxFallSpeed) {
        glm::vec3 vel = rigidbody->getVelocity();
        vel.y = -maxFallSpeed;
        rigidbody->setVelocity(vel);
    }

    // Apply ground/air drag
    float drag = grounded ? groundDrag : airDrag;
    rigidbody->setDrag(drag);
}

void CharacterController::moveForward(float speed) {
    movementDirection.z = speed;
}

void CharacterController::moveRight(float speed) {
    movementDirection.x = speed;
}

void CharacterController::jump(float force) {
    if (grounded && !jumping) {
        jumping = true;
        grounded = false;
        glm::vec3 vel = rigidbody->getVelocity();
        vel.y = force;
        rigidbody->setVelocity(vel);
        rigidbody->setGrounded(false);
    }
}

void CharacterController::stop() {
    movementDirection = glm::vec3(0.0f);
}

void CharacterController::applyMovement(float deltaTime) {
    if (glm::length(movementDirection) < 0.001f) {
        return;
    }

    auto transform = gameObject->getTransform();

    // Normalize movement direction
    glm::vec3 normalizedDir = glm::normalize(movementDirection);

    glm::vec3 forward, right;

    // Если есть камера, используем её направление
    if (camera) {
        // Берем направление камеры, но игнорируем вертикальную составляющую
        forward = camera->getFront();
        forward.y = 0.0f;
        forward = glm::normalize(forward);

        right = camera->getRight();
        right.y = 0.0f;
        right = glm::normalize(right);
    }
    else {
        // Используем направление персонажа
        forward = transform->getForward();
        right = transform->getRight();
    }

    glm::vec3 movement = (forward * normalizedDir.z + right * normalizedDir.x) * moveSpeed;

    // Сохраняем текущую вертикальную скорость
    glm::vec3 vel = rigidbody->getVelocity();
    vel.x = movement.x;
    vel.z = movement.z;

    rigidbody->setVelocity(vel);

    // Поворачиваем персонажа в направлении движения
    if (glm::length(glm::vec2(movement.x, movement.z)) > 0.1f) {
        float targetYaw = atan2(movement.x, movement.z);
        transform->rotation = glm::quat(glm::vec3(0.0f, targetYaw, 0.0f));
    }

    // Сбрасываем направление для следующего кадра
    movementDirection = glm::vec3(0.0f);
}
