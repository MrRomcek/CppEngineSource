#include "Core.h"
#include <iostream>

class MyApplication {
public:
    void run() {
        // Конфигурация приложения
        Core::Config config;
        config.width = 1024;
        config.height = 768;
        config.title = "My Awesome OpenGL App";
        config.clearColor = { 0.1f, 0.1f, 0.2f, 1.0f };
        config.vsync = true;

        // Получение экземпляра Core и инициализация
        Core& core = Core::getInstance();
        if (!core.initialize(config)) {
            std::cerr << "Failed to initialize Core!" << std::endl;
            return;
        }

        // Установка callback'ов
        core.setKeyCallback([&](int key, int action) {
            onKey(key, action);
            });

        core.setResizeCallback([&](int width, int height) {
            onResize(width, height);
            });

        core.setUpdateCallback([&](float deltaTime) {
            onUpdate(deltaTime);
            });

        // Запуск приложения
        std::cout << "Starting application..." << std::endl;
        core.run();
    }

private:
    void onKey(int key, int action) {
        if (action == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_R:
                std::cout << "R pressed! Changing clear color to red." << std::endl;
                Core::getInstance().setClearColor({ 1.0f, 0.0f, 0.0f, 1.0f });
                break;
            case GLFW_KEY_G:
                std::cout << "G pressed! Changing clear color to green." << std::endl;
                Core::getInstance().setClearColor({ 0.0f, 1.0f, 0.0f, 1.0f });
                break;
            case GLFW_KEY_B:
                std::cout << "B pressed! Changing clear color to blue." << std::endl;
                Core::getInstance().setClearColor({ 0.0f, 0.0f, 1.0f, 1.0f });
                break;
            case GLFW_KEY_F11:
                std::cout << "F11 pressed! Toggling fullscreen." << std::endl;
                // Реализация переключения полноэкранного режима
                break;
            }
        }
    }

    void onResize(int width, int height) {
        std::cout << "Window resized to " << width << "x" << height << std::endl;
        // Здесь можно обновить проекционные матрицы и т.д.
    }

    void onUpdate(float deltaTime) {
        static float timeAccumulator = 0.0f;
        timeAccumulator += deltaTime;

        // Пример: мигающий цвет каждые 2 секунды
        if (timeAccumulator >= 2.0f) {
            timeAccumulator = 0.0f;
            // Автоматическое изменение цвета
        }

        // Обновление логики игры/приложения
        // updateCamera(deltaTime);
        // updatePhysics(deltaTime);
    }
};

int main() {
    try {
        MyApplication app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}