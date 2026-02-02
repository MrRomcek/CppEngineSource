// ============================================================================
// Main.cpp - Glavnyj fajl prilozheniya
// ============================================================================

#include "Core.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "PhysicsEngine.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "CharacterController.h"
#include "PrefabFactory.h"
#include <iostream>
#include <memory>

class MyApplication {
public:
    void run() {
        // Konfiguraciya prilozheniya
        Core::Config config;
        config.width = 1280;
        config.height = 720;
        config.title = "Game Engine";
        config.multithreaded = false;
        config.logLevel = LogLevel::TRACE;
        config.clearColor = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);

        LOG_INFO("=== Zapusk prilozheniya ===");
        LOG_INFO("Upravlenie:");
        LOG_INFO("  WASD - dvizhenie");
        LOG_INFO("  Space - pryzhok");
        LOG_INFO("  Mouse - povorot kamery");
        LOG_INFO("  ESC - vyhod");

        // Inicializaciya yadra
        Core& core = Core::getInstance();
        if (!core.initialize(config)) {
            LOG_CRITICAL("Oshibka inicializacii!");
            return;
        }

        // Inicializaciya fiziki
        PhysicsEngine::getInstance().initialize();
        LOG_INFO("Fizicheskij dvizhok inicializirovan");

        // Sozdanie igrovyh objektov
        LOG_INFO("Sozdanie igrovyh objektov...");

        // Sozdaem pol
        auto floor = PrefabFactory::createFloor("Floor");
        floor->getComponent<MeshRenderer>()->getMesh()->render();
        gameObjects.push_back(std::move(floor));

        // Sozdaem personazha
        character = PrefabFactory::createCharacter("Player");
        character->getTransform()->position = glm::vec3(0.0f, 2.0f, 0.0f);
        character->getComponent<MeshRenderer>()->getMesh()->render();

        // Nastrojka kamery ot pervogo lica
        Camera* camera = core.getCamera();
        camera->setMode(Camera::Mode::FirstPerson);
        camera->setTarget(character->getTransform());
        camera->setThirdPersonOffset(glm::vec3(0.0f, 0.8f, 0.0f));

        // Privyazyvaem kameru k kontrolleru personazha
        auto controller = character->getComponent<CharacterController>();
        if (controller) {
            controller->setCamera(camera);
        }
        LOG_INFO("Kamera privyazana k personazhu");

        // Sozdaem dinamicheskie objekty
        for (int i = 0; i < 5; i++) {
            float angle = (float)i * glm::radians(72.0f);
            float radius = 4.0f;

            auto obj = PrefabFactory::createDynamicObject("Object_" + std::to_string(i + 1));
            obj->getTransform()->position = glm::vec3(
                cos(angle) * radius,
                3.0f + i,
                sin(angle) * radius
            );
            obj->getTransform()->scale = glm::vec3(0.5f, 0.5f, 0.5f);
            obj->getComponent<MeshRenderer>()->getMesh()->render();

            gameObjects.push_back(std::move(obj));
        }

        // Sozdaem staticheskie prepyatstviya
        for (int i = 0; i < 3; i++) {
            auto obstacle = PrefabFactory::createStaticObject("Obstacle_" + std::to_string(i + 1));
            obstacle->getTransform()->position = glm::vec3(-3.0f + i * 3.0f, -1.0f, 5.0f);
            obstacle->getTransform()->scale = glm::vec3(1.0f, 2.0f, 1.0f);
            obstacle->getComponent<MeshRenderer>()->getMesh()->render();

            gameObjects.push_back(std::move(obstacle));
        }

        // Nastrojka callback-funkcij
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

        core.addRenderCallback([&]() {
            onRender();
        });

        // Inicializaciya objektov
        for (auto& obj : gameObjects) {
            obj->start();
        }
        if (character) {
            character->start();
        }

        LOG_INFO("=== Zapusk glavnogo cikla ===");
        core.run();
        LOG_INFO("=== Glavnyj cikl zavershen ===");
    }

private:
    // Obrabotka klavish
    void onKey(int key, int action) {
        auto controller = character ? character->getComponent<CharacterController>() : nullptr;

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (controller) {
                switch (key) {
                case GLFW_KEY_W:
                    controller->moveForward(1.0f);
                    break;
                case GLFW_KEY_S:
                    controller->moveForward(-1.0f);
                    break;
                case GLFW_KEY_A:
                    controller->moveRight(-1.0f);
                    break;
                case GLFW_KEY_D:
                    controller->moveRight(1.0f);
                    break;
                case GLFW_KEY_SPACE:
                    controller->jump(6.0f);
                    break;
                }
            }
        }

        if (action == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_ESCAPE:
                LOG_INFO("Vyhod iz prilozheniya...");
                Core::getInstance().stop();
                break;

            case GLFW_KEY_F1:
                LOG_INFO("Upravlenie:");
                LOG_INFO("WASD - dvizhenie");
                LOG_INFO("Space - pryzhok");
                LOG_INFO("ESC - vyhod");
                break;

            case GLFW_KEY_R:
                if (character) {
                    character->getTransform()->position = glm::vec3(0.0f, 2.0f, 0.0f);
                    auto rb = character->getComponent<Rigidbody>();
                    if (rb) rb->setVelocity(glm::vec3(0.0f));
                    LOG_INFO("Poziciya personazha sbroshena");
                }
                break;
            }
        }
    }

    // Obrabotka dvizheniya myshi
    void onMouseMove(double x, double y) {
    }

    // Obrabotka knopok myshi
    void onMouseButton(int button, int action) {
    }

    // Obrabotka izmeneniya razmera okna
    void onResize(int width, int height) {
        LOG_INFO("Razmer okna izmenen: %dx%d", width, height);
    }

    // Obnovlenie kazhdyj kadr
    void onUpdate(float deltaTime) {
        // Obnovlyaem fiziku
        PhysicsEngine::getInstance().update(deltaTime);

        // Obnovlyaem vse objekty
        for (auto& obj : gameObjects) {
            obj->update(deltaTime);
        }

        // Obnovlyaem personazha
        if (character) {
            character->update(deltaTime);
        }
    }

    // Rendering kazhdyj kadr
    void onRender() {
        // Renderim vse objekty
        for (auto& obj : gameObjects) {
            if (obj->isActive()) {
                obj->render();
            }
        }
    }

    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unique_ptr<GameObject> character;
};

// Tochka vhoda
int main() {
    try {
        Logger* logger = getEngineLogger();
        logger->setLevel(LogLevel::TRACE);
        MyApplication app;
        app.run();
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Kriticheskaya oshibka: %s", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        LOG_CRITICAL("Neizvestnaya oshibka");
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
