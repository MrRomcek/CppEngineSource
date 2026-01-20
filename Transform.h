#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform : public Component {
public:
    // Публичные поля для упрощения доступа
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Конструкторы
    Transform() = default;
    Transform(const glm::vec3& pos, const glm::vec3& scl = glm::vec3(1.0f),
        const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
        : position(pos), scale(scl), rotation(rot) {
    }

    // Получение матрицы модели
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = model * glm::mat4_cast(rotation);
        model = glm::scale(model, scale);
        return model;
    }

    // Методы трансформации
    void translate(const glm::vec3& translation, bool local = true) {
        if (local) {
            position += rotation * translation;
        }
        else {
            position += translation;
        }
    }

    void rotate(float angle, const glm::vec3& axis) {
        rotation = glm::rotate(rotation, glm::radians(angle), axis);
    }

    // Получение направляющих векторов
    glm::vec3 getForward() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }
    glm::vec3 getRight() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
    glm::vec3 getUp() const { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

    // Регистрация компонента
    REGISTER_COMPONENT(Transform)
};