#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
    glm::vec3 position = glm::vec3(0.0f);      // Позиция в мировом пространстве
    glm::vec3 scale = glm::vec3(1.0f);         // Масштаб
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Вращение (кватернион)

    // Конструкторы
    Transform() = default;
    Transform(const glm::vec3& pos) : position(pos) {}
    Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl)
        : position(pos), rotation(rot), scale(scl) {
    }

    // Получение матрицы модели
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);          // Применяем перемещение
        model = model * glm::mat4_cast(rotation);        // Применяем вращение
        model = glm::scale(model, scale);                // Применяем масштаб
        return model;
    }

    // Методы перемещения
    void translate(const glm::vec3& translation) {
        position += translation;
    }

    void translateLocal(const glm::vec3& translation) {
        position += rotation * translation;  // Перемещение в локальных координатах
    }

    // Методы вращения
    void rotate(float angle, const glm::vec3& axis) {
        rotation = glm::rotate(rotation, glm::radians(angle), axis);
    }

    void rotateEuler(const glm::vec3& eulerAngles) {
        // Порядок вращения: Yaw (Y), Pitch (X), Roll (Z)
        glm::quat qYaw = glm::angleAxis(glm::radians(eulerAngles.y), glm::vec3(0, 1, 0));
        glm::quat qPitch = glm::angleAxis(glm::radians(eulerAngles.x), glm::vec3(1, 0, 0));
        glm::quat qRoll = glm::angleAxis(glm::radians(eulerAngles.z), glm::vec3(0, 0, 1));
        rotation = qYaw * qPitch * qRoll * rotation;
    }

    void setRotationEuler(const glm::vec3& eulerAngles) {
        rotation = glm::quat(glm::vec3(
            glm::radians(eulerAngles.x),  // Pitch
            glm::radians(eulerAngles.y),  // Yaw
            glm::radians(eulerAngles.z)   // Roll
        ));
    }

    // Получение направляющих векторов
    glm::vec3 getForward() const {
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);  // Вектор "вперед"
    }

    glm::vec3 getRight() const {
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);   // Вектор "вправо"
    }

    glm::vec3 getUp() const {
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);   // Вектор "вверх"
    }

    // Получение углов Эйлера
    glm::vec3 getEulerAngles() const {
        return glm::degrees(glm::eulerAngles(rotation));
    }

    // Сброс трансформации
    void reset() {
        position = glm::vec3(0.0f);
        rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        scale = glm::vec3(1.0f);
    }
};