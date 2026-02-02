#include "Collider.h"
#include "GameObject.h"
#include "Transform.h"
#include "PhysicsEngine.h"

// ==================== Collider Base Class ====================
void Collider::start() {
    // Регистрируем коллайдер в системе физики
}

void Collider::onDestroy() {
    // Удаляем коллайдер из системы физики
}

// ==================== Box Collider ====================
glm::vec3 BoxCollider::getMin() const {
    auto transform = gameObject->getTransform();
    glm::vec3 pos = transform->position + offset;
    return pos - (size * transform->scale) * 0.5f;
}

glm::vec3 BoxCollider::getMax() const {
    auto transform = gameObject->getTransform();
    glm::vec3 pos = transform->position + offset;
    return pos + (size * transform->scale) * 0.5f;
}

bool BoxCollider::checkCollision(const Collider* other) const {
    if (!other) return false;

    const BoxCollider* boxCollider = dynamic_cast<const BoxCollider*>(other);
    if (boxCollider) {
        // AABB столкновение (Axis-Aligned Bounding Box)
        glm::vec3 min1 = getMin();
        glm::vec3 max1 = getMax();
        glm::vec3 min2 = boxCollider->getMin();
        glm::vec3 max2 = boxCollider->getMax();

        return (min1.x <= max2.x && max1.x >= min2.x) &&
               (min1.y <= max2.y && max1.y >= min2.y) &&
               (min1.z <= max2.z && max1.z >= min2.z);
    }

    const SphereCollider* sphereCollider = dynamic_cast<const SphereCollider*>(other);
    if (sphereCollider) {
        // Столкновение Box-Sphere
        glm::vec3 min = getMin();
        glm::vec3 max = getMax();
        glm::vec3 center = sphereCollider->getCenter();

        glm::vec3 closest = glm::clamp(center, min, max);
        glm::vec3 delta = closest - center;

        return glm::length(delta) < sphereCollider->getRadius();
    }

    return false;
}

// ==================== Sphere Collider ====================
glm::vec3 SphereCollider::getCenter() const {
    auto transform = gameObject->getTransform();
    return transform->position + offset;
}

bool SphereCollider::checkCollision(const Collider* other) const {
    if (!other) return false;

    const SphereCollider* sphereCollider = dynamic_cast<const SphereCollider*>(other);
    if (sphereCollider) {
        // Столкновение Sphere-Sphere
        glm::vec3 center1 = getCenter();
        glm::vec3 center2 = sphereCollider->getCenter();
        float distance = glm::distance(center1, center2);
        return distance < (radius + sphereCollider->getRadius());
    }

    const BoxCollider* boxCollider = dynamic_cast<const BoxCollider*>(other);
    if (boxCollider) {
        // Столкновение Sphere-Box (делегируем к BoxCollider)
        return boxCollider->checkCollision(this);
    }

    return false;
}

// ==================== Capsule Collider ====================
bool CapsuleCollider::checkCollision(const Collider* other) const {
    if (!other) return false;

    const CapsuleCollider* capsuleCollider = dynamic_cast<const CapsuleCollider*>(other);
    if (capsuleCollider) {
        auto transform = gameObject->getTransform();
        auto otherTransform = other->getGameObject()->getTransform();

        glm::vec3 pos1 = transform->position + offset;
        glm::vec3 pos2 = otherTransform->position + capsuleCollider->offset;

        float distance = glm::distance(pos1, pos2);
        return distance < (radius + capsuleCollider->radius);
    }

    const SphereCollider* sphereCollider = dynamic_cast<const SphereCollider*>(other);
    if (sphereCollider) {
        auto transform = gameObject->getTransform();
        glm::vec3 pos1 = transform->position + offset;
        glm::vec3 pos2 = sphereCollider->getCenter();

        float distance = glm::distance(pos1, pos2);
        return distance < (radius + sphereCollider->getRadius());
    }

    return false;
}
