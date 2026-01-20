#include "Camera.h"
#include <iostream>

Camera::Camera(Type type)
    : type(type),
    position(glm::vec3(0.0f, 0.0f, 5.0f)),
    worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw(-90.0f),
    pitch(0.0f),
    movementSpeed(5.0f),
    mouseSensitivity(0.1f),
    zoom(45.0f),
    fov(45.0f),
    nearPlane(0.1f),
    farPlane(100.0f) {

    updateCameraVectors();
}

void Camera::update(float deltaTime) {
    updateMovement(deltaTime);
}

void Camera::setMovement(int direction, bool enable) {
    switch (direction) {
    case FORWARD:   moveForward = enable; break;
    case BACKWARD:  moveBackward = enable; break;
    case LEFT:      moveLeft = enable; break;
    case RIGHT:     moveRight = enable; break;
    case UP:        moveUp = enable; break;
    case DOWN:      moveDown = enable; break;
    }

    // Логируем изменение состояния
    const char* dirNames[] = { "FORWARD", "BACKWARD", "LEFT", "RIGHT", "UP", "DOWN" };
    // LOG_TRACE("Camera movement %s: %s", dirNames[direction], enable ? "ON" : "OFF");
}

void Camera::updateMovement(float deltaTime) {
    float velocity = movementSpeed * deltaTime;

    // Логируем состояние движения (каждые 60 кадров для отладки)
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {
        bool isMoving = moveForward || moveBackward || moveLeft || moveRight || moveUp || moveDown;
        if (isMoving) {
            // LOG_TRACE("Camera moving. Pos: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
        }
    }

    if (moveForward)
        position += front * velocity;
    if (moveBackward)
        position -= front * velocity;
    if (moveLeft)
        position -= right * velocity;
    if (moveRight)
        position += right * velocity;
    if (moveUp)
        position += worldUp * velocity;
    if (moveDown)
        position -= worldUp * velocity;

    // Обновляем векторы после движения
    updateCameraVectors();
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Ограничиваем угол наклона
    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();

    // Логируем изменение угла
    // LOG_TRACE("Camera look: yaw=%.1f, pitch=%.1f", yaw, pitch);
}

void Camera::processMouseScroll(float yoffset) {
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    if (type == Type::Perspective) {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
    else {
        float halfHeight = zoom;
        float halfWidth = halfHeight * aspectRatio;
        return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
    }
}

void Camera::updateCameraVectors() {
    // Вычисляем новый вектор фронта
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // Пересчитываем правый и верхний векторы
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}