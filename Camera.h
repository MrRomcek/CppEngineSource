#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum class Type {
        Perspective,
        Orthographic
    };

    Camera(Type type = Type::Perspective);

    void update(float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    // Движение камеры
    void processKeyboard(int direction, float deltaTime);

    // Геттеры
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getRight() const { return right; }
    glm::vec3 getUp() const { return up; }

    // Сеттеры
    void setPosition(const glm::vec3& pos) {
        position = pos;
        updateCameraVectors();
    }

    void setFOV(float fov) { this->fov = fov; }
    void setNearPlane(float near) { nearPlane = near; }
    void setFarPlane(float far) { farPlane = far; }

    // Методы для непрерывного движения
    void setMovement(int direction, bool enable);
    void updateMovement(float deltaTime);

    // Управление
    enum Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

private:
    void updateCameraVectors();

    Type type;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Углы Эйлера
    float yaw;
    float pitch;

    // Параметры камеры
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // Параметры проекции
    float fov;
    float nearPlane;
    float farPlane;

    // Флаги движения
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
};