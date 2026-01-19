#include "Core.h"
#include <iostream>
#include <chrono>

// ============== ���������� Core ==============

Core::~Core() {
    shutdown();
}

bool Core::initialize(const Config& config) {
    if (initialized) {
        std::cerr << "Core already initialized!" << std::endl;
        return false;
    }

    this->config = config;

    // ������������� GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // ��������� GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.glMajorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.glMinorVersion);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // �������� ����
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

    // ��������� ���������
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // ��������� VSync
    glfwSwapInterval(config.vsync ? 1 : 0);

    // ��������� callback'�� GLFW
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    // ������������� GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    // ��������� OpenGL
    glViewport(0, 0, config.width, config.height);
    glClearColor(
        config.clearColor.r,
        config.clearColor.g,
        config.clearColor.b,
        config.clearColor.a
    );

    // Включение теста глубины (если будет использоваться 3D)
    // glEnable(GL_DEPTH_TEST);

    // ����� ���������� � OpenGL
    CoreUtils::printGLInfo();

    initialized = true;
    std::cout << "Core initialized successfully!" << std::endl;
    return true;
}

glBindVertexArray(0);


*/

void Core::run() {
    if (!initialized || !window) {
        std::cerr << "Core not initialized!" << std::endl;
        return;
    }

    running = true;
    lastFrame = static_cast<float>(glfwGetTime());

    // Основной цикл рендеринга
    while (running && !glfwWindowShouldClose(window)) {
        // ������ deltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ��������� �����
        processInput();
        glfwPollEvents();

        // ����� ����������������� update callback
        if (updateCallbackFunc) {
            updateCallbackFunc(deltaTime);
        }

        // Очистка буферов
        glClear(GL_COLOR_BUFFER_BIT);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Для 3D

        // ============ Пользовательский рендеринг здесь ============

        view = glm::lookAt(
            glm::vec3(0.0f,0.0f,3.0f),
            glm::vec3(0.0f,0.0f,0.0f),
            glm::vec3(0.0f,1.0f,0.0f));
        projection = glm::perspective(glm::radians(45.0f), (float)config.width / config.height, 0.1f, 100.f);



        // ����� ������� (���� ��� �� ����)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // ������ 1
        glm::mat4 model1 = glm::mat4(1.0f);
        //model1 = glm::translate(model1, glm::vec3(-1.0f, 0.0f, 0.0f));
        //model1 = glm::rotate(model1, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(0);

        // �������� ������ OpenGL
        GL_CHECK();

        // ����� �������
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
    // ��������� ���������� ������� ������
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // ���������������� ��������� ����� callback
    if (keyCallbackFunc) {
        // ����� �������� �������� ���������� ������
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

// ============== Callback-������� GLFW ==============

void Core::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    // ���������� ������������
    core->config.width = width;
    core->config.height = height;

    // ���������� viewport
    glViewport(0, 0, width, height);

    // ����� ����������������� callback'�
    if (core->resizeCallbackFunc) {
        core->resizeCallbackFunc(width, height);
    }

    std::cout << "Window resized to: " << width << "x" << height << std::endl;
}

void Core::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    // ���������� ��������� ������
    core->keyPressed[key] = (action != GLFW_RELEASE);

    // ����� ����������������� callback'�
    if (core->keyCallbackFunc) {
        core->keyCallbackFunc(key, action);
    }
}

// ============== ��������������� ������� ==============

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