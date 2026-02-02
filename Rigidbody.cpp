#include "Rigidbody.h"
#include "GameObject.h"
#include "Transform.h"
#include "PhysicsEngine.h"

void Rigidbody::start() {
    PhysicsEngine::getInstance().registerRigidbody(this);
}

void Rigidbody::update(float deltaTime) {
    if (!enabled || bodyType == Type::Static) return;

    // Обновляем накопленные силы в скорость
    if (mass > 0 && glm::length(accumulatedForce) > 0.001f) {
        glm::vec3 acceleration = accumulatedForce / mass;
        velocity += acceleration * deltaTime;
        accumulatedForce = glm::vec3(0.0f);
    }

    // Обновляем угловую скорость
    if (!freezeRotation && glm::length(accumulatedTorque) > 0.001f) {
        glm::vec3 angularAcceleration = accumulatedTorque / mass;
        angularVelocity += angularAcceleration * deltaTime;
        accumulatedTorque = glm::vec3(0.0f);

        // Применяем угловое сопротивление
        float angularDragFactor = 1.0f - (angularDrag * deltaTime);
        angularDragFactor = glm::clamp(angularDragFactor, 0.0f, 1.0f);
        angularVelocity *= angularDragFactor;

        // Применяем угловую скорость к трансформации
        if (glm::length(angularVelocity) > 0.001f) {
            auto transform = gameObject->getTransform();
            float angle = glm::length(angularVelocity) * deltaTime;
            glm::vec3 axis = glm::normalize(angularVelocity);
            transform->rotate(glm::degrees(angle), axis);
        }
    }
}

void Rigidbody::onDestroy() {
    PhysicsEngine::getInstance().unregisterRigidbody(this);
}

void Rigidbody::addForce(const glm::vec3& force, bool isImpulse) {
    if (bodyType == Type::Static) return;

    if (isImpulse) {
        // Импульс - мгновенное изменение скорости
        velocity += force / mass;
    }
    else {
        // Обычная сила - накапливается и применяется в следующем обновлении
        accumulatedForce += force;
    }
}

void Rigidbody::addTorque(const glm::vec3& torque) {
    if (bodyType == Type::Static || freezeRotation) return;
    accumulatedTorque += torque;
}
