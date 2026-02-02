#pragma once
#include "Component.h"
#include <glm/glm.hpp>

class Collider : public Component {
public:
    virtual ~Collider() = default;

    virtual void start() override;
    virtual void onDestroy() override;

    void setIsTrigger(bool trigger) { isTrigger = trigger; }
    bool getIsTrigger() const { return isTrigger; }

    virtual bool checkCollision(const Collider* other) const = 0;
    virtual std::string getTypeName() const override = 0;

protected:
    bool isTrigger = false;
};

// ==================== Box Collider ====================
class BoxCollider : public Collider {
public:
    BoxCollider() = default;
    virtual ~BoxCollider() = default;

    void setSize(const glm::vec3& size) { this->size = size; }
    glm::vec3 getSize() const { return size; }

    void setOffset(const glm::vec3& offset) { this->offset = offset; }
    glm::vec3 getOffset() const { return offset; }

    virtual bool checkCollision(const Collider* other) const override;
    virtual std::string getTypeName() const override { return "BoxCollider"; }

    glm::vec3 getMin() const;
    glm::vec3 getMax() const;

private:
    glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
};

// ==================== Sphere Collider ====================
class SphereCollider : public Collider {
public:
    SphereCollider() = default;
    virtual ~SphereCollider() = default;

    void setRadius(float radius) { this->radius = glm::max(radius, 0.01f); }
    float getRadius() const { return radius; }

    void setOffset(const glm::vec3& offset) { this->offset = offset; }
    glm::vec3 getOffset() const { return offset; }

    virtual bool checkCollision(const Collider* other) const override;
    virtual std::string getTypeName() const override { return "SphereCollider"; }

    glm::vec3 getCenter() const;

private:
    float radius = 0.5f;
    glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
};

// ==================== Capsule Collider ====================
class CapsuleCollider : public Collider {
public:
    CapsuleCollider() = default;
    virtual ~CapsuleCollider() = default;

    void setRadius(float radius) { this->radius = glm::max(radius, 0.01f); }
    float getRadius() const { return radius; }

    void setHeight(float height) { this->height = glm::max(height, 0.01f); }
    float getHeight() const { return height; }

    void setOffset(const glm::vec3& offset) { this->offset = offset; }
    glm::vec3 getOffset() const { return offset; }

    virtual bool checkCollision(const Collider* other) const override;
    virtual std::string getTypeName() const override { return "CapsuleCollider"; }

private:
    float radius = 0.5f;
    float height = 2.0f;
    glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
};
