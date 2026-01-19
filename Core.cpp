#include "Core.h"
#include <iostream>
#include <chrono>

// ============== Реализация Core ==============

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

    // Настройка контекста
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Настройка VSync
    glfwSwapInterval(config.vsync ? 1 : 0);

    // Установка callback'ов GLFW
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

    // Включение теста глубины (если будет использоваться 3D)
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
    float vertices[] = {
        // Позиции          // Цвета
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // левый нижний угол (красный)
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // правый нижний угол (зеленый)
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f   // верхняя вершина (синий)
    };

    // Шейдеры
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"  // Добавляем атрибут цвета
        "out vec3 vertexColor;\n"                  // Передаем цвет в фрагментный шейдер
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "   vertexColor = aColor;\n"               // Передаем цвет дальше
        "}\0";

    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 vertexColor;\n"                   // Принимаем интерполированный цвет
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(vertexColor, 1.0);\n" // Используем переданный цвет
        "}\0";


    // Создание и компиляция вершинного шейдера
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Проверка ошибок вершинного шейдера
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

    // Проверка ошибок фрагментного шейдера
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

    // Проверка ошибок линковки программы
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Удаление шейдеров (они уже в программе)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Настройка VBO (Vertex Buffer Object) и VAO (Vertex Array Object)
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Привязка VAO
    glBindVertexArray(VAO);

    // Копирование вершин в буфер
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Установка атрибутов вершин
    // Атрибут 0: позиции (3 float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут 1: цвета (3 float) - смещение на 3 float
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Отвязка
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    running = true;
    lastFrame = static_cast<float>(glfwGetTime());


    // Основной цикл рендеринга
    while (running && !glfwWindowShouldClose(window)) {
        // Расчет deltaTime
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
        //glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Для 3D

        // ============ Пользовательский рендеринг здесь ============
        glUseProgram(shaderProgram);

        // Рисование треугольника
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);


        // Проверка ошибок OpenGL
        GL_CHECK();

        // Обмен буферов
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
    // Обработка глобальных горячих клавиш
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Пользовательская обработка через callback
    if (keyCallbackFunc) {
        // Можно добавить проверку конкретных клавиш
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

    // Обновление конфигурации
    core->config.width = width;
    core->config.height = height;

    // Обновление viewport
    glViewport(0, 0, width, height);

    // Вызов пользовательского callback'а
    if (core->resizeCallbackFunc) {
        core->resizeCallbackFunc(width, height);
    }

    std::cout << "Window resized to: " << width << "x" << height << std::endl;
}

void Core::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    // Обновление состояния клавиш
    core->keyPressed[key] = (action != GLFW_RELEASE);

    // Вызов пользовательского callback'а
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