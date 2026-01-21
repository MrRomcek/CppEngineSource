#pragma once
#include "Scene.h"
#include "Shader.h"
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <glm/glm.hpp>

// Структура рендер-команды - базовый элемент системы рендеринга
struct RenderCommand {
    // Типы рендер-команд, которые может выполнять система
    enum class Type {
        DrawMesh,          // Отрисовка меша (основная команда)
        Clear,             // Очистка буферов (цвет, глубина, трафарет)
        SetViewport,       // Установка области вывода
        SetClearColor,     // Установка цвета очистки
        EnableDepthTest,   // Включение теста глубины
        DisableDepthTest   // Выключение теста глубины
    };

    Type type;                    // Тип команды
    std::function<void()> execute; // Функция для выполнения команды
    int priority = 0;             // Приоритет для сортировки команд
    // (меньше = выполняется раньше)
};

// Класс очереди рендеринга - накапливает и выполняет команды
class RenderQueue {
public:
    // Добавить команду в очередь
    void push(const RenderCommand& cmd);

    // Выполнить все команды в очереди
    void execute();

    // Очистить очередь команд
    void clear();

private:
    std::vector<RenderCommand> commands; // Вектор для хранения команд
};

// Основной класс рендерера - управляет всем процессом отрисовки
class Renderer {
public:
    Renderer();   // Конструктор
    ~Renderer();  // Деструктор

    // Инициализация рендерера (создание контекста, загрузка ресурсов)
    bool initialize();

    // Рендеринг сцены (основной метод)
    void renderScene(Scene* scene);

    // Установка области вывода (размер окна/экрана)
    void setViewport(int x, int y, int width, int height);

    // Установка цвета очистки экрана
    void setClearColor(const glm::vec4& color);

    // Очистка буферов (цвет, глубина)
    void clear();

    // ========== Управление шейдерами ==========

    // Загрузка шейдерной программы из файлов
    bool loadShader(const std::string& name,
        const std::string& vertPath,  // Путь к вершинному шейдеру
        const std::string& fragPath); // Путь к фрагментному шейдеру

    // Получение указателя на шейдерную программу по имени
    ShaderProgram* getShader(const std::string& name);

    // ========== Состояния рендеринга ==========

    // Включение/выключение теста глубины (z-buffer)
    void enableDepthTest(bool enable = true);

    // Включение/выключение смешивания цветов (прозрачность)
    void enableBlending(bool enable = true);

    // Включение/выключение отсечения задних граней (оптимизация)
    void enableFaceCulling(bool enable = true);

private:
    // Обработка и выполнение команд из очереди рендеринга
    void processRenderCommands();

    // Настройка стандартных шейдеров (базовый, текстурированный и т.д.)
    void setupDefaultShaders();

    // Очередь рендер-команд для отложенного выполнения
    RenderQueue renderQueue;

    // Коллекция загруженных шейдерных программ (ключ - имя шейдера)
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shaders;

    // Цвет очистки буфера цвета (фон)
    glm::vec4 clearColor = { 0.1f, 0.1f, 0.2f, 1.0f }; // Темно-синий

    // Размеры области вывода
    int viewportWidth = 800;
    int viewportHeight = 600;

    // ========== Текущие состояния рендеринга ==========

    bool depthTestEnabled = true;    // Тест глубины включен по умолчанию
    bool blendingEnabled = false;    // Смешивание выключено по умолчанию
    bool faceCullingEnabled = true;  // Отсечение граней включено по умолчанию
};