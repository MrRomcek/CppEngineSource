#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform;

class Camera {
public:
    enum class Type {
        Perspective,
        Orthographic
    };

    enum class Mode {
        Free,           // Свободная камера
        ThirdPerson,    // Камера от третьего лица (следует за целью)
        FirstPerson     // Камера от первого лица (внутри цели)
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
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }

    // Сеттеры
    void setPosition(const glm::vec3& pos) {
        position = pos;
        updateCameraVectors();
    }

    void setFOV(float fov) { this->fov = fov; }
    void setNearPlane(float near) { nearPlane = near; }
    void setFarPlane(float far) { farPlane = far; }

    // ==================== Режим слежения ====================
    void setMode(Mode mode) { this->mode = mode; }
    Mode getMode() const { return mode; }

    // Установить цель для слежения
    void setTarget(Transform* target) { this->target = target; }
    Transform* getTarget() const { return target; }

    // Параметры третьего лица
    void setThirdPersonDistance(float distance) { thirdPersonDistance = distance; }
    void setThirdPersonHeight(float height) { thirdPersonHeight = height; }
    void setThirdPersonOffset(const glm::vec3& offset) { thirdPersonOffset = offset; }

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
    void updateFollowCamera(float deltaTime);

    Type type;
    Mode mode = Mode::Free;

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

    // Параметры слежения
    Transform* target = nullptr;
    float thirdPersonDistance = 5.0f;
    float thirdPersonHeight = 2.0f;
    glm::vec3 thirdPersonOffset = glm::vec3(0.0f, 1.0f, 0.0f);
    float followSmoothness = 10.0f;

    // Флаги движения
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
};