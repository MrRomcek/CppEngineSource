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

    // ��������� ����� ������� (���� ����� �������������� 3D)
    glEnable(GL_DEPTH_TEST);

    // ����� ���������� � OpenGL
    CoreUtils::printGLInfo();

    initialized = true;
    std::cout << "Core initialized successfully!" << std::endl;
    return true;
}
/*
const char* fragmentShaderSource = "#version 330 core\n"
    "uniform vec3 colorA;\n"
    "uniform vec3 colorB;\n"
    "uniform vec3 colorC;\n"
    "in float vertexColorMix;\n"  // ����� ����� ���������� �� ���������� �������
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   // ��������� ����� � ����������� �� �������\n"
    "   vec3 color = mix(mix(colorA, colorB, vertexColorMix), colorC, vertexColorMix);\n"
    "   FragColor = vec4(color, 1.0);\n"
    "}\0";


    // ��������� uniform ����������
glUseProgram(shaderProgram);
int colorALoc = glGetUniformLocation(shaderProgram, "colorA");
int colorBLoc = glGetUniformLocation(shaderProgram, "colorB");
int colorCLoc = glGetUniformLocation(shaderProgram, "colorC");

glUniform3f(colorALoc, 1.0f, 0.0f, 0.0f);  // �������
glUniform3f(colorBLoc, 0.0f, 1.0f, 0.0f);  // �������
glUniform3f(colorCLoc, 0.0f, 0.0f, 1.0f);  // �����
*/




/*
// ������:
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

// � �������� �����:
glUseProgram(shaderProgram);

// ����� ������� (���� ��� �� ����)
glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

// ������ 1
glm::mat4 model1 = glm::mat4(1.0f);
model1 = glm::translate(model1, glm::vec3(-1.0f, 0.0f, 0.0f));
model1 = glm::rotate(model1, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
glBindVertexArray(VAO1);
glDrawArrays(GL_TRIANGLES, 0, 3);

// ������ 2
glm::mat4 model2 = glm::mat4(1.0f);
model2 = glm::translate(model2, glm::vec3(1.0f, 0.0f, 0.0f));
model2 = glm::rotate(model2, (float)glfwGetTime() * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
glBindVertexArray(VAO2);
glDrawArrays(GL_TRIANGLES, 0, 3);

glBindVertexArray(0);


*/

void Core::run() {
    if (!initialized || !window) {
        std::cerr << "Core not initialized!" << std::endl;
        return;
    }


    // �������
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


    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 ourColor;\n"                   // ��������� ����������������� ����
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(ourColor, 1.0);\n" // ���������� ���������� ����
        "}\0";


    // �������� � ���������� ���������� �������
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // �������� ������ ���������� �������
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // �������� � ���������� ������������ �������
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // �������� ������ ������������ �������
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // �������� ��������� ���������
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // �������� ������ �������� ���������
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // �������� �������� (��� ��� � ���������)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    

    running = true;
    lastFrame = static_cast<float>(glfwGetTime());

 
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glm::mat4 view;
    glm::mat4 projection;
    // �������� ���� ����������
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

        // ������� �������
        //glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ��� 3D

        // ============ ���������������� ��������� ����� ============
        glUseProgram(shaderProgram);

        view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
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

/*
 double mouseX, mouseY;
    double lastMouseX, lastMouseY;
    bool firstMouse = true;

    // Callback ��� ������� ����
    static void mousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
        // �������� ��������� �� ��������� Core
        Core* instance = static_cast<Core*>(glfwGetWindowUserPointer(window));
        if (instance) {
            instance->processMousePosition(xpos, ypos);
        }
    }

    void processMousePosition(double xpos, double ypos) {
        if (firstMouse) {
            lastMouseX = xpos;
            lastMouseY = ypos;
            firstMouse = false;
        }

        // ���������� �������� ������������ ���������� �����
        double xoffset = xpos - lastMouseX;
        double yoffset = lastMouseY - ypos; // ����������� Y

        lastMouseX = xpos;
        lastMouseY = ypos;

        // ��������� ������ ��� ������ ������ ��������
        // camera.processMouseMovement(xoffset, yoffset);

        mouseX = xpos;
        mouseY = ypos;
    }





        // ������������� callback
        glfwSetCursorPosCallback(window, mousePositionCallback);

        // �����������: ������ � ��������� ������
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


          // ����� ��� ��������� ������� ����
    glm::vec2 getMousePosition() const {
        return glm::vec2(mouseX, mouseY);
    }

    // ����� ��� ��������� �������� ����
    glm::vec2 getMouseOffset() {
        if (firstMouse) return glm::vec2(0.0f);

        double xoffset = mouseX - lastMouseX;
        double yoffset = lastMouseY - mouseY;

        // ���������� ��� ���������� �����
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        return glm::vec2(xoffset, yoffset);
    }

    glm::vec2 getMouseNormalized() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // ����������� � �������� [-1, 1]
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height; // ����������� Y

    return glm::vec2(x, y);
}
*/