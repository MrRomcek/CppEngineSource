#include "Core.h"
#include "GameObject.h"
#include "Shader.h"
#include <iostream>
#include <windows.h> 
#include <chrono>
#include <thread>

// ==================== Вспомогательные функции ====================
namespace CoreUtils {
    void printGLInfo() {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "==========================" << std::endl;
    }
}

// ==================== Конструктор Core ====================
Core::Core() {
    // Создаем логгер
    logger = LogManager::getInstance().createConsoleLogger("Core");
}

// ==================== Деструктор Core ====================
Core::~Core() {
    shutdown();
}

// ==================== Инициализация движка ====================
bool Core::initialize(const Config& config) {
    if (initialized) {
        LOG_WARNING("Движок уже инициализирован!");
        return false;
    }


    this->config = config;
    logger->setLevel(config.logLevel);

    LOG_INFO("Инициализация движка...");
    LOG_INFO("Конфигурация: %dx%d, заголовок: %s",
        config.width, config.height, config.title.c_str());

    // Инициализация GLFW
    if (!glfwInit()) {
        LOG_ERROR("Не удалось инициализировать GLFW");
        return false;
    }

    LOG_DEBUG("GLFW инициализирован");

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
        LOG_ERROR("Не удалось создать окно GLFW");
        glfwTerminate();
        return false;
    }

    LOG_INFO("Окно создано: %dx%d", config.width, config.height);

    // Настройка контекста OpenGL
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Настройка VSync
    glfwSwapInterval(config.vsync ? 1 : 0);
    LOG_DEBUG("VSync: %s", config.vsync ? "включен" : "выключен");

    // Настройка callback'ов GLFW
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    LOG_DEBUG("Callback'и GLFW установлены");

    // Инициализация GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Не удалось инициализировать GLAD");
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    LOG_DEBUG("GLAD инициализирован");

    // Настройка OpenGL
    glViewport(0, 0, config.width, config.height);
    glClearColor(
        config.clearColor.r,
        config.clearColor.g,
        config.clearColor.b,
        config.clearColor.a
    );

    // Включаем тест глубины для 3D
    glEnable(GL_DEPTH_TEST);
    LOG_DEBUG("Тест глубины включен");

    // Включаем смешивание цветов для прозрачности
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    LOG_DEBUG("Смешивание цветов включено");

    camera = new Camera();
    camera->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    LOG_DEBUG("Камера создана и инициализирована");


    // Вывод информации о OpenGL
    CoreUtils::printGLInfo();

    // Инициализация менеджера шейдеров
    shaderManager = std::make_unique<ShaderManager>();

    // Настройка многопоточности
    if (config.multithreaded) {
        multithreadingEnabled = true;
        LOG_INFO("Многопоточность включена (%d потоков)", config.maxThreads);

        // Создаем рабочие потоки
        for (int i = 0; i < config.maxThreads; ++i) {
            if (i == 0) {
                // Первый поток для обновления
                workerThreads.emplace_back(&Core::updateThreadFunction, this);
                LOG_DEBUG("Создан поток обновления #%d", i);
            }
            else {
                // Остальные потоки для рендеринга
                workerThreads.emplace_back(&Core::renderThreadFunction, this);
                LOG_DEBUG("Создан поток рендеринга #%d", i);
            }
        }
    }
    else {
        LOG_INFO("Многопоточность выключена");
    }

    initialized = true;
    LOG_INFO("Движок успешно инициализирован!");
    return true;
}

// ==================== Запуск главного цикла ====================
void Core::run() {
    if (!initialized || !window) {
        LOG_ERROR("Движок не инициализирован!");
        return;
    }

    LOG_INFO("Запуск главного цикла...");
    running = true;

    // Используем high_resolution_clock для точного времени
    using Clock = std::chrono::high_resolution_clock;
    auto lastTime = Clock::now();
    float deltaTime = 0.0f;

    // Для FPS
    float fpsTimer = 0.0f;
    int frameCount = 0;
    float fps = 0.0f;

    // ==================== Главный игровой цикл ====================
    while (running && !glfwWindowShouldClose(window)) {
        // ТОЧНОЕ вычисление deltaTime
        auto currentTime = Clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        deltaTime = elapsed.count();
        lastTime = currentTime;

        // Ограничиваем deltaTime (защита от "зависаний")
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f; // Максимум 100 мс
            LOG_WARNING("Большой deltaTime: %.3f мс, ограничено до 100 мс", deltaTime * 1000.0f);
        }

        // Сохраняем для доступа извне
        this->deltaTime = deltaTime;

        // Расчет FPS
        frameCount++;
        fpsTimer += deltaTime;

        // Обновляем FPS каждую секунду
        if (fpsTimer >= 1.0f) {
            fps = frameCount / fpsTimer;
            this->fps = fps; // Сохраняем
            frameCount = 0;
            fpsTimer = 0.0f;

            // Выводим FPS в заголовок окна
            std::string newTitle = config.title +
                " | FPS: " + std::to_string(static_cast<int>(fps)) +
                " | Delta: " + std::to_string(deltaTime * 1000.0f).substr(0, 6) + " ms";
            glfwSetWindowTitle(window, newTitle.c_str());

            LOG_TRACE("FPS: %.1f, DeltaTime: %.3f ms", fps, deltaTime * 1000.0f);
        }

        // Обработка ввода
        processInput();
        glfwPollEvents();

        // Обновление состояния
        if (multithreadingEnabled) {
            // ... многопоточный код ...
        }
        else {
            // Последовательное обновление
            if (updateCallbackFunc) {
                updateCallbackFunc(deltaTime);
            }

            // Последовательный рендеринг
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Устанавливаем цвет очистки
            glClearColor(config.clearColor.r, config.clearColor.g,
                config.clearColor.b, config.clearColor.a);

            // Настройки OpenGL
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // Вызываем все рендер-коллбэки
            for (auto& callback : renderCallbacks) {
                callback();
            }

            // Проверка ошибок OpenGL
            GL_CHECK();
        }

        // Обмен буферов
        glfwSwapBuffers(window);
    }

    // Остановка потоков
    if (multithreadingEnabled) {
        LOG_INFO("Остановка рабочих потоков...");
        stopThreads = true;
        renderCV.notify_all();

        for (auto& thread : workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        workerThreads.clear();
        LOG_INFO("Все потоки остановлены");
    }

    LOG_INFO("Главный цикл завершен");
    shutdown();
}

// ==================== Функция потока обновления ====================
void Core::updateThreadFunction() {
    LOG_DEBUG("Поток обновления запущен");

    while (!stopThreads) {
        std::unique_lock<std::mutex> lock(renderMutex);
        renderCV.wait(lock, [this]() {
            return updateReady.load() || stopThreads.load();
            });

        if (stopThreads) break;

        // Выполняем обновление
        if (updateCallbackFunc) {
            updateCallbackFunc(deltaTime);
        }

        updateReady = false;
        renderReady = true;
        renderCV.notify_one();
    }

    LOG_DEBUG("Поток обновления завершен");
}

// ==================== Функция потока рендеринга ====================
void Core::renderThreadFunction() {
    LOG_DEBUG("Поток рендеринга запущен");

    while (!stopThreads) {
        std::unique_lock<std::mutex> lock(renderMutex);
        renderCV.wait(lock, [this]() {
            return renderReady.load() || stopThreads.load();
            });

        if (stopThreads) break;

        // Выполняем рендеринг
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GL_CHECK();

        renderReady = true;
        renderCV.notify_one();
    }

    LOG_DEBUG("Поток рендеринга завершен");
}

// ==================== Обработка ввода ====================
void Core::processInput() {
    // Обработка стандартных клавиш
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        LOG_INFO("Клавиша ESC нажата - завершение работы");
        glfwSetWindowShouldClose(window, true);
    }

    // Вызов пользовательского callback'а для клавиш
    if (keyCallbackFunc) {
        for (const auto& pair : keyPressed) {
            if (pair.second) {
                keyCallbackFunc(pair.first, GLFW_PRESS);
            }
        }
    }
    if (camera) {
        camera->updateMovement(deltaTime);
    }
}

// ==================== Завершение работы ====================
void Core::shutdown() {
    if (!initialized) {
        LOG_WARNING("Движок уже завершил работу");
        return;
    }

    LOG_INFO("Завершение работы движка...");

    // Закрываем окно
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
        LOG_DEBUG("Окно закрыто");
    }
    if (camera) {
        delete camera;
        camera = nullptr;
    }
    // Завершаем GLFW
    glfwTerminate();
    LOG_DEBUG("GLFW завершен");

    initialized = false;
    running = false;

    LOG_INFO("Движок успешно завершил работу");
}

// ==================== Callback для изменения размера окна ====================
void Core::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    // Обновляем конфигурацию
    core->config.width = width;
    core->config.height = height;

    // Обновляем viewport
    glViewport(0, 0, width, height);

    LOG_INFO("Размер окна изменен: %dx%d", width, height);

    // Вызываем пользовательский callback
    if (core->resizeCallbackFunc) {
        core->resizeCallbackFunc(width, height);
    }
}

// ==================== Callback для клавиатуры ====================
void Core::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core || !core->camera) return;

    // Обновляем состояние клавиш
    core->keyPressed[key] = (action != GLFW_RELEASE);

    // Устанавливаем флаги движения для камеры (не вызываем processKeyboard!)
    if (core->camera) {
        bool enable = (action == GLFW_PRESS || action == GLFW_REPEAT);

        switch (key) {
        case GLFW_KEY_W:      core->camera->setMovement(Camera::FORWARD, enable); break;
        case GLFW_KEY_S:      core->camera->setMovement(Camera::BACKWARD, enable); break;
        case GLFW_KEY_A:      core->camera->setMovement(Camera::LEFT, enable); break;
        case GLFW_KEY_D:      core->camera->setMovement(Camera::RIGHT, enable); break;
        case GLFW_KEY_SPACE:  core->camera->setMovement(Camera::UP, enable); break;
        case GLFW_KEY_LEFT_SHIFT: core->camera->setMovement(Camera::DOWN, enable); break;
        case GLFW_KEY_O: core->setVsync(0); break;
        }
    }

    // Логируем нажатия
    if (action == GLFW_PRESS) {
        LOG_TRACE("Клавиша нажата: %d (scancode: %d)", key, scancode);
    }
    else if (action == GLFW_RELEASE) {
        LOG_TRACE("Клавиша отпущена: %d", key);
    }

    // Вызываем пользовательский callback
    if (core->keyCallbackFunc) {
        core->keyCallbackFunc(key, action);
    }
}


// ==================== Callback для движения мыши ====================
void Core::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core || !core->camera) return;

    if (core->firstMouse) {
        core->lastX = xpos;
        core->lastY = ypos;
        core->firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - core->lastX);
    float yoffset = static_cast<float>(core->lastY - ypos); // обратный порядок для Y

    core->lastX = xpos;
    core->lastY = ypos;

    // Обрабатываем движение мыши только если зажата правая кнопка
    if (core->mouseButtonPressed) {
        core->camera->processMouseMovement(xoffset, yoffset);
    }
}

// ==================== Callback для кнопок мыши ====================
void Core::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
    if (!core) return;

    core->mouseButtonPressed = (action == GLFW_PRESS);

    // Логируем нажатия
    const char* buttonName = "";
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:   buttonName = "LEFT"; break;
    case GLFW_MOUSE_BUTTON_RIGHT:  buttonName = "RIGHT"; break;
    case GLFW_MOUSE_BUTTON_MIDDLE: buttonName = "MIDDLE"; break;
    default:                       buttonName = "OTHER"; break;
    }

    if (action == GLFW_PRESS) {
        LOG_DEBUG("Кнопка мыши нажата: %s", buttonName);
    }
    else {
        LOG_DEBUG("Кнопка мыши отпущена: %s", buttonName);
    }

    // Вызываем пользовательский callback
    if (core->mouseButtonCallbackFunc) {
        core->mouseButtonCallbackFunc(button, action);
    }
}

// ==================== Инициализация шейдеров по умолчанию ====================
void Core::initializeDefaultShaders() {
    LOG_INFO("Загрузка шейдеров по умолчанию...");

    // Базовый шейдер для цветных объектов
    const std::string basicVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)";

    const std::string basicFragmentShader = R"(
#version 330 core
in vec3 ourColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform bool useTexture;

void main() {
    if (useTexture) {
        FragColor = texture(texture1, TexCoord);
    } else {
        FragColor = vec4(ourColor, 1.0);
    }
}
)";

    // Шейдер освещения по Фонгу
    const std::string phongVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

    const std::string phongFragmentShader = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

    // Шейдер для рисования линий и точек
    const std::string simpleVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 lineColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    lineColor = aColor;
}
)";

    const std::string simpleFragmentShader = R"(
#version 330 core
in vec3 lineColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(lineColor, 1.0);
}
)";

    // Загружаем шейдеры
    if (shaderManager) {
        if (shaderManager->createShaderFromSource("basic",
            basicVertexShader,
            basicFragmentShader)) {
            LOG_INFO("Базовый шейдер загружен");
        }
        else {
            LOG_ERROR("Не удалось загрузить базовый шейдер");
        }

        if (shaderManager->createShaderFromSource("phong",
            phongVertexShader,
            phongFragmentShader)) {
            LOG_INFO("Шейдер Фонга загружен");
        }
        else {
            LOG_ERROR("Не удалось загрузить шейдер Фонга");
        }

        if (shaderManager->createShaderFromSource("simple",
            simpleVertexShader,
            simpleFragmentShader)) {
            LOG_INFO("Простой шейдер для линий загружен");
        }
        else {
            LOG_ERROR("Не удалось загрузить простой шейдер");
        }
    }

    LOG_INFO("Шейдеры по умолчанию загружены");
}

// ==================== Проверка ошибок OpenGL ====================
bool Core::checkGLError(const char* function, const char* file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        const char* errorStr = "";
        switch (error) {
        case GL_INVALID_ENUM:                  errorStr = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 errorStr = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             errorStr = "GL_INVALID_OPERATION"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        case GL_OUT_OF_MEMORY:                 errorStr = "GL_OUT_OF_MEMORY"; break;
        default:                               errorStr = "Неизвестная ошибка"; break;
        }

        LOG_ERROR("Ошибка OpenGL: %s (%d) в %s (%s:%d)",
            errorStr, error, function, file, line);
        return false;
    }
    return true;
}

// ==================== Установка цвета очистки ====================
void Core::setClearColor(const glm::vec4& color) {
    config.clearColor = color;
    glClearColor(color.r, color.g, color.b, color.a);
    LOG_DEBUG("Цвет очистки установлен: (%.2f, %.2f, %.2f, %.2f)",
        color.r, color.g, color.b, color.a);
}

// ==================== Установка размера окна ====================
void Core::setWindowSize(unsigned int width, unsigned int height) {
    if (window) {
        glfwSetWindowSize(window, width, height);
        config.width = width;
        config.height = height;
        LOG_INFO("Размер окна установлен: %dx%d", width, height);
    }
}

// ==================== Установка заголовка окна ====================
void Core::setWindowTitle(const std::string& title) {
    if (window) {
        glfwSetWindowTitle(window, title.c_str());
        config.title = title;
        LOG_DEBUG("Заголовок окна установлен: %s", title.c_str());
    }
}

// ==================== Установка уровня логирования ====================
void Core::setLogLevel(LogLevel level) {
    if (logger) {
        logger->setLevel(level);
        LOG_INFO("Уровень логирования установлен: %d", static_cast<int>(level));
    }
}

void Core::setVsync(bool vsync)
{
    config.vsync = vsync;
    glfwSwapInterval(vsync);
}

// ==================== Остановка движка ====================
void Core::stop() {
    running = false;
}

// ==================== Установка callback'ов ====================
void Core::setKeyCallback(KeyCallback callback) {
    keyCallbackFunc = std::move(callback);
}

void Core::setMouseCallback(MouseCallback callback) {
    mouseCallbackFunc = std::move(callback);
}

void Core::setMouseButtonCallback(MouseButtonCallback callback) {
    mouseButtonCallbackFunc = std::move(callback);
}

void Core::setResizeCallback(ResizeCallback callback) {
    resizeCallbackFunc = std::move(callback);
}

void Core::setUpdateCallback(UpdateCallback callback) {
    updateCallbackFunc = std::move(callback);
}

void Core::addRenderCallback(std::function<void()> callback) {
    renderCallbacks.push_back(callback);
}