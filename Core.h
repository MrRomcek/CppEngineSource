#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
//$(ProjectDir)include
// Предварительное объявление
class Core;

// Типы для callback-функций
using KeyCallback = std::function<void(int, int)>;
using ResizeCallback = std::function<void(int, int)>;
using UpdateCallback = std::function<void(float)>;



class Core
{
public:
    // Конфигурация приложения
    struct Config {
        unsigned int width = 800;
        unsigned int height = 600;
        std::string title = "CppEngineSource";
        int glMajorVersion = 4;
        int glMinorVersion = 6;
        glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        bool vsync = true;
        bool resizable = true;
    };

    // Singleton с ленивой инициализацией и thread-safe (C++11)
    static Core& getInstance() {
        static Core instance;
        return instance;
    }

    // Удаляем копирование и присваивание
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;

    // Инициализация с конфигурацией
    bool initialize(const Config& config = Config());

    // Запуск основного цикла
    void run();

    // Остановка приложения
    void stop();

    // Установка callback'ов
    void setKeyCallback(KeyCallback callback);
    void setResizeCallback(ResizeCallback callback);
    void setUpdateCallback(UpdateCallback callback);

    // Утилиты
    GLFWwindow* getWindow() const { return window; }
    const Config& getConfig() const { return config; }
    bool isRunning() const { return running; }

    // Изменение параметров во время выполнения
    void setClearColor(const glm::vec4& color);
    void setWindowSize(unsigned int width, unsigned int height);
    void setWindowTitle(const std::string& title);

private:
    Core() = default;
    ~Core();

    // Callback-функции GLFW
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    // Внутренние методы
    void processInput();
    void shutdown();

    // Члены класса
    GLFWwindow* window = nullptr;
    Config config;
    bool initialized = false;
    bool running = false;

    // Callback'и
    KeyCallback keyCallbackFunc;
    ResizeCallback resizeCallbackFunc;
    UpdateCallback updateCallbackFunc;

    // Состояние клавиш
    std::unordered_map<int, bool> keyPressed;

    // Таймер для deltaTime
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
};

// Вспомогательные функции
namespace CoreUtils {
    bool checkGLError(const char* function, const char* file, int line);
    void printGLInfo();
}

// Макрос для проверки ошибок OpenGL
#ifdef _DEBUG
#define GL_CHECK() CoreUtils::checkGLError(__FUNCTION__, __FILE__, __LINE__)
#else
#define GL_CHECK() ((void)0)
#endif