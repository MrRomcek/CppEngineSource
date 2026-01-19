#include "Core.h"
#include <iostream>
#include <chrono>

// ============== Деструктор Core ==============

Core::~Core() {
    shutdown();
}

bool Core::initialize(const Config& config) {
    if (initialized) {
        std::cerr << "Core already initialized!" << std::endl;
        return false;
    }

    this->config = config;

    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.glMajorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.glMinorVersion);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // Создание окна
    window = glfwCreateWindow(
        config.width,
        config.height,
        config.title.c_str(),
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Делаем окно текущим контекстом
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Настройка VSync
    glfwSwapInterval(config.vsync ? 1 : 0);

    // Настройка callback'ов GLFW
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Инициализация GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    // Настройка OpenGL
    glViewport(0, 0, config.width, config.height);
    glClearColor(
        config.clearColor.r,
        config.clearColor.g,
        config.clearColor.b,
        config.clearColor.a
    );

    // Включаем тест глубины (если планируется работа с 3D)
    glEnable(GL_DEPTH_TEST);

    // Вывод информации о OpenGL
    CoreUtils::printGLInfo();

    initialized = true;
    std::cout << "Core initialized successfully!" << std::endl;
    return true;
}

void Core::run() {
    if (!initialized || !window) {
        std::cerr << "Core not initialized!" << std::endl;
        return;
    }

    // Исходный код вершинного шейдера
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

    // Исходный код фрагментного шейдера
    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 ourColor;\n"                   // Входящий цвет из вершинного шейдера
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(ourColor, 1.0);\n" // Используем интерполированный цвет
        "}\0";

    // Создание и компиляция вершинного шейдера
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Проверка компиляции вершинного шейдера
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Создание и компиляция фрагментного шейдера
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Проверка компиляции фрагментного шейдера
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Создание шейдерной программы
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Проверка линковки шейдерной программы
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Удаляем шейдеры (они уже залинкованы в программу)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    running = true;
    lastFrame = static_cast<float>(glfwGetTime());

    // Создание игровых объектов
    GameObject obj(shaderProgram);
    GameObject obj2(shaderProgram);
    obj.model = glm::translate(obj.model, glm::vec3(1, 1, 0));
    obj2.model = glm::translate(obj2.model, glm::vec3(-1, -1, 0));

    // Получение location uniform-переменных
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glm::mat4 view;
    glm::mat4 projection;

    // Главный игровой цикл
    while (running && !glfwWindowShouldClose(window)) {
        // Вычисление deltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Обработка ввода
        processInput();
        glfwPollEvents();

        // Вызов пользовательского update callback
        if (updateCallbackFunc) {
            updateCallbackFunc(deltaTime);
        }

        // Очистка буферов
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Рендеринг сцены
        glUseProgram(shaderProgram);

        // Настройка матриц вида и проекции
        view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),   // Позиция камеры
            glm::vec3(0.0f, 0.0f, 0.0f),   // Точка наведения
            glm::vec3(0.0f, 1.0f, 0.0f));  // Вектор "вверх"

        projection = glm::perspective(
            glm::radians(45.0f),           // Угол обзора
            (float)config.width / config.height, // Соотношение сторон
            0.1f,                          // Ближняя плоскость отсечения
            100.0f);                       // Дальняя плоскость отсечения

        // Передача матриц в шейдер
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Рендеринг игровых объектов
        obj.render();
        obj2.render();

        glBindVertexArray(0);

        // Проверка ошибок OpenGL
        GL_CHECK();

        // Обмен буферов (отображение результата)
        glfwSwapBuffers(window);
    }

    shutdown();
}

void Core::stop() {
    running = false;
}

void Core::setKeyCallback(KeyCallback callback) {
    keyCallbackFunc = std::move(callback);
}

void Core::setResizeCallback(ResizeCallback callback) {
    resizeCallbackFunc = std::move(callback);
}

void Core::setUpdateCallback(UpdateCallback callback) {
    updateCallbackFunc = std::move(callback);
}

void Core::setClearColor(const glm::vec4& color) {
    config.clearColor = color;
    glClearColor(color.r, color.g, color.b, color.a);
}

void Core::setWindowSize(unsigned int width, unsigned int height) {
    if (window) {
        glfwSetWindowSize(window, width, height);
        config.width = width;
        config.height = height;
    }
}

void Core::setWindowTitle(const std::string& title) {
    if (window) {
        glfwSetWindowTitle(window, title.c_str());
        config.title = title;
    }
}

void Core::processInput() {
    // Обработка стандартных клавиш
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Вызов пользовательского key callback
    if (keyCallbackFunc) {
        // Проверяем все зажатые клавиши
        for (const auto& [key, pressed] : keyPressed) {
            if (pressed) {
                keyCallbackFunc(key, GLFW_PRESS);
            }
        }
    }
}

void Core::shutdown() {
    if (!initialized) return;

    std::cout << "Shutting down Core..." << std::endl;

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
    initialized = false;
    running = false;

    std::cout << "Core shutdown complete." << std::endl;
}

// ============== Callback-функции GLFW ==============

void Core::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    // Обновляем конфигурацию
    core->config.width = width;
    core->config.height = height;

    // Обновляем viewport
    glViewport(0, 0, width, height);

    // Вызываем пользовательский callback
    if (core->resizeCallbackFunc) {
        core->resizeCallbackFunc(width, height);
    }

    std::cout << "Window resized to: " << width << "x" << height << std::endl;
}

void Core::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    // Обновляем состояние клавиш
    core->keyPressed[key] = (action != GLFW_RELEASE);

    // Вызываем пользовательский callback
    if (core->keyCallbackFunc) {
        core->keyCallbackFunc(key, action);
    }
}

// ============== Вспомогательные функции ==============

namespace CoreUtils {

    bool checkGLError(const char* function, const char* file, int line) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL Error (" << error << "): "
                << function << " in " << file << ":" << line << std::endl;
            return false;
        }
        return true;
    }

    void printGLInfo() {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "==========================" << std::endl;
    }
}