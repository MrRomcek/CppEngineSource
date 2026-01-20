#include "Core.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include <iostream>
#include <locale>
#include <memory>

class MyApplication {
public:
    void run() {
        // Устанавливаем локаль
        std::locale::global(std::locale("ru_RU.UTF-8"));
        std::wcout.imbue(std::locale("ru_RU.UTF-8"));

        // ==================== Конфигурация приложения ====================
        Core::Config config;
        config.width = 1280;
        config.height = 720;
        config.title = "Мой Движок";
        config.multithreaded = false;
        config.logLevel = LogLevel::TRACE;
        config.clearColor = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);

        LOG_INFO("=== Запуск приложения ===");
        LOG_INFO("Управление:");
        LOG_INFO("  WASD - движение камеры");
        LOG_INFO("  Space/Shift - вверх/вниз");
        LOG_INFO("  Правая кнопка мыши + движение - поворот камеры");
        LOG_INFO("  ESC - выход");

        // ==================== Инициализация ядра ====================
        Core& core = Core::getInstance();
        if (!core.initialize(config)) {
            LOG_CRITICAL("Не удалось инициализировать движок!");
            return;
        }

        // ==================== Создание игровых объектов ====================
        LOG_INFO("Создание игровых объектов...");

        // Создаем пол (большой квадрат)
        auto floor = std::make_unique<GameObject>("Пол");
        floor->getTransform()->position = glm::vec3(0.0f, -2.0f, 0.0f);
        floor->getTransform()->scale = glm::vec3(10.0f, 0.1f, 10.0f);
        auto floorRenderer = floor->addComponent<MeshRenderer>();
        floorRenderer->setMesh(Mesh::createCube());
        floor->getComponent<MeshRenderer>()->getMesh()->render(); // Генерируем буферы

        // Создаем центральный куб
        auto centerCube = std::make_unique<GameObject>("Центральный куб");
        centerCube->getTransform()->position = glm::vec3(0.0f, 0.0f, 0.0f);
        auto cubeRenderer = centerCube->addComponent<MeshRenderer>();
        cubeRenderer->setMesh(Mesh::createCube());
        centerCube->getComponent<MeshRenderer>()->getMesh()->render();

        // Создаем несколько объектов вокруг
        for (int i = 0; i < 5; i++) {
            float angle = (float)i * glm::radians(72.0f);
            float radius = 3.0f;

            auto obj = std::make_unique<GameObject>("Объект " + std::to_string(i + 1));
            obj->getTransform()->position = glm::vec3(
                cos(angle) * radius,
                0.0f,
                sin(angle) * radius
            );
            obj->getTransform()->scale = glm::vec3(0.5f, 1.0f + i * 0.2f, 0.5f);

            auto renderer = obj->addComponent<MeshRenderer>();
            renderer->setMesh(Mesh::createCube());
            obj->getComponent<MeshRenderer>()->getMesh()->render();

            gameObjects.push_back(std::move(obj));
        }

        gameObjects.push_back(std::move(floor));
        gameObjects.push_back(std::move(centerCube));

        // ==================== Настройка callback'ов ====================
        core.setKeyCallback([&](int key, int action) {
            onKey(key, action);
            });

        core.setMouseCallback([&](double x, double y) {
            onMouseMove(x, y);
            });

        core.setMouseButtonCallback([&](int button, int action) {
            onMouseButton(button, action);
            });

        core.setResizeCallback([&](int width, int height) {
            onResize(width, height);
            });

        core.setUpdateCallback([&](float deltaTime) {
            onUpdate(deltaTime);
            });

        // Добавляем рендер-коллбэк
        core.addRenderCallback([&]() {
            onRender();
            });

        // ==================== Инициализация объектов ====================
        for (auto& obj : gameObjects) {
            obj->start();
        }

        LOG_INFO("=== Запуск главного цикла ===");
        core.run();
        LOG_INFO("=== Главный цикл завершен ===");
    }

private:
    void onKey(int key, int action) {
        if (action == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_ESCAPE:
                LOG_INFO("Выход из приложения...");
                Core::getInstance().stop();
                break;

            case GLFW_KEY_F1:
                LOG_INFO("Помощь по управлению:");
                LOG_INFO("WASD - движение");
                LOG_INFO("Space/Shift - вверх/вниз");
                LOG_INFO("Правая кнопка мыши - поворот");
                break;
            }
        }
    }

    void onMouseMove(double x, double y) {
        // Движение обрабатывается в Core
    }

    void onMouseButton(int button, int action) {
        // Кнопки обрабатываются в Core
    }

    void onResize(int width, int height) {
        LOG_INFO("Размер окна изменен: %dx%d", width, height);
    }

    void onUpdate(float deltaTime) {
        static float time = 0.0f;
        time += deltaTime;

        // Вращаем центральный куб
        if (gameObjects.size() > 1) {
            auto& centerCube = gameObjects.back();
            Transform* transform = centerCube->getTransform();
            if (transform) {
                transform->rotate(45.0f * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
                transform->rotate(20.0f * deltaTime, glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }

        // Плавное движение объектов вокруг
        for (size_t i = 0; i < gameObjects.size() - 2; i++) { // -2 чтобы исключить пол и центральный куб
            auto& obj = gameObjects[i];
            Transform* transform = obj->getTransform();
            if (transform) {
                transform->position.y = sinf(time + i) * 0.5f;
                transform->rotate(30.0f * deltaTime * (i + 1), glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }
    }

    void onRender() {
        // Рендеринг всех объектов
        for (auto& obj : gameObjects) {
            if (obj->isActive()) {
                obj->render();
            }
        }
    }

    std::vector<std::unique_ptr<GameObject>> gameObjects;
};

int main() {
    try {
        std::locale::global(std::locale("ru_RU.UTF-8"));
        std::wcout.imbue(std::locale("ru_RU.UTF-8"));
        Logger* logger = getEngineLogger();
        logger->setLevel(LogLevel::TRACE);
        MyApplication app;
        app.run();
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Критическая ошибка: %s", e.what());
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        LOG_CRITICAL("Неизвестная критическая ошибка");
        std::cerr << "Неизвестная критическая ошибка" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}