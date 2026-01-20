#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>


#include "Logger.h"
#include "Camera.h"


class Camera;
class Core;

// Предварительные объявления
class Scene;
class Renderer;
class Camera;
class ShaderManager;
class GameObject;

// Типы callback'ов
using KeyCallback = std::function<void(int, int)>;
using MouseCallback = std::function<void(double, double)>;
using MouseButtonCallback = std::function<void(int, int)>;
using ResizeCallback = std::function<void(int, int)>;
using UpdateCallback = std::function<void(float)>;

class Core {
public:
    // ==================== Структура конфигурации ====================
    struct Config {
        unsigned int width = 800;
        unsigned int height = 600;
        std::string title = "Engine";
        int glMajorVersion = 4;
        int glMinorVersion = 6;
        glm::vec4 clearColor = { 0.1f, 0.1f, 0.2f, 1.0f };
        bool vsync = true;
        bool resizable = true;
        bool multithreaded = true;
        int maxThreads = 4;
        LogLevel logLevel = LogLevel::INFO;
    };

    // ==================== Singleton Pattern ====================
    static Core& getInstance() {
        static Core instance;
        return instance;
    }

    // Удаляем копирование и присваивание
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;

    // ==================== Основные методы ====================
    bool initialize(const Config& config = Config());
    void run();
    void stop();

    // ==================== Установка callback'ов ====================
    void setKeyCallback(KeyCallback callback);
    void setMouseCallback(MouseCallback callback);
    void setMouseButtonCallback(MouseButtonCallback callback);
    void setResizeCallback(ResizeCallback callback);
    void setUpdateCallback(UpdateCallback callback);

    // ==================== Геттеры ====================
    GLFWwindow* getWindow() const { return window; }
    const Config& getConfig() const { return config; }
    bool isRunning() const { return running; }
    float getDeltaTime() const { return deltaTime; }
    Logger* getLogger() const { return logger; }
    Camera* getCamera() const { return camera; }


    // ==================== Изменение параметров во время выполнения ====================
    void setClearColor(const glm::vec4& color);
    void setWindowSize(unsigned int width, unsigned int height);
    void setWindowTitle(const std::string& title);
    void setLogLevel(LogLevel level);
    void setVsync(bool vsync = true);

    void addRenderCallback(std::function<void()> callback);


private:
    Core();
    ~Core();

    // ==================== Callback-функции GLFW ====================
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    std::vector<std::function<void()>> renderCallbacks;
    

    // ==================== Внутренние методы ====================
    void processInput();
    void shutdown();
    void updateThreadFunction();
    void renderThreadFunction();
    void initializeDefaultShaders();

    // ==================== Вспомогательные функции ====================
    bool checkGLError(const char* function, const char* file, int line);
    void printGLInfo();

    // ==================== Члены класса ====================
    GLFWwindow* window = nullptr;
    Config config;
    bool initialized = false;
    bool running = false;
    Logger* logger = nullptr;

    // Callback'и
    KeyCallback keyCallbackFunc;
    MouseCallback mouseCallbackFunc;
    MouseButtonCallback mouseButtonCallbackFunc;
    ResizeCallback resizeCallbackFunc;
    UpdateCallback updateCallbackFunc;

    // Состояние ввода
    std::unordered_map<int, bool> keyPressed;
    glm::dvec2 mousePosition = { 0.0, 0.0 };
    glm::dvec2 lastMousePosition = { 0.0, 0.0 };
    bool mouseButtonPressed = false;

    // Тайминг
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float fps = 0.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;

    Camera* camera;
    bool firstMouse = true;
    double lastX = 0.0;
    double lastY = 0.0;


    // Многопоточность
    bool multithreadingEnabled = false;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> stopThreads{ false };
    std::mutex renderMutex;
    std::condition_variable renderCV;
    std::atomic<bool> renderReady{ false };
    std::atomic<bool> updateReady{ false };

    // Ресурсы
    std::unique_ptr<ShaderManager> shaderManager;
};

// ==================== Макрос для проверки ошибок OpenGL ====================
#ifdef _DEBUG
#define GL_CHECK() Core::getInstance().checkGLError(__FUNCTION__, __FILE__, __LINE__)
#else
#define GL_CHECK() ((void)0)
#endif