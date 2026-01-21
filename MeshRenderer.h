#pragma once
#include "Component.h"
#include "Transform.h"
#include "Shader.h"
#include <glad/glad.h>        // Библиотека GLAD для загрузки функций OpenGL
#include <GLFW/glfw3.h>       // Библиотека GLFW для создания окон и контекста
#include <glm/glm.hpp>        // Математическая библиотека GLM для работы с векторами и матрицами
#include <glm/gtc/matrix_transform.hpp>  // Функции преобразований (translate, rotate, scale)
#include <glm/gtc/type_ptr.hpp>          // Функции для преобразования GLM типов в указатели
#include <vector>
#include <memory>

#include "Core.h"
#include "Camera.h"

// Предварительные объявления классов (forward declarations)
class Camera;
class Core;

// ==================== Класс Mesh (Геометрический объект) ====================
// Класс для представления 3D-мешей (геометрических объектов)
class Mesh {
public:
    // Структура вершины, содержащая все необходимые атрибуты
    struct Vertex {
        glm::vec3 position;   // Позиция вершины в локальном пространстве (x, y, z)
        glm::vec3 color;      // Цвет вершины (RGB)
        glm::vec2 texCoord;   // Текстурные координаты (u, v) для наложения текстур
        glm::vec3 normal;     // Нормаль вершины для расчета освещения
    };

    Mesh() = default;  // Конструктор по умолчанию
    ~Mesh();           // Деструктор для очистки ресурсов OpenGL

    // Создание меша из массивов вершин и индексов
    bool createFromVertices(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices = {});

    // Отрисовка меша на экране
    void render() const;

    // ============= Статические методы для создания примитивов =============

    // Создание треугольника
    static std::shared_ptr<Mesh> createTriangle();

    // Создание квадрата (квада) с заданным размером
    static std::shared_ptr<Mesh> createQuad(float size = 1.0f);

    // Создание куба с заданным размером
    static std::shared_ptr<Mesh> createCube(float size = 1.0f);

    // Создание линии между двумя точками с заданным цветом
    static std::shared_ptr<Mesh> createLine(const glm::vec3& start,
        const glm::vec3& end,
        const glm::vec3& color = glm::vec3(1.0f));

    // Геттеры для получения внутренних данных
    unsigned int getVAO() const { return VAO; }           // Получение Vertex Array Object
    unsigned int getVertexCount() const { return vertexCount; }  // Количество вершин
    unsigned int getIndexCount() const { return indexCount; }    // Количество индексов

private:
    // Идентификаторы OpenGL объектов
    unsigned int VAO = 0;      // Vertex Array Object (хранит конфигурацию атрибутов)
    unsigned int VBO = 0;      // Vertex Buffer Object (хранит данные вершин)
    unsigned int EBO = 0;      // Element Buffer Object (хранит индексы вершин)

    // Количество элементов
    unsigned int vertexCount = 0;  // Общее количество вершин
    unsigned int indexCount = 0;   // Общее количество индексов (0 если рисуем без индексов)

    // Настройка меша: создание и конфигурация буферов OpenGL
    void setupMesh(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);
};

// ==================== Компонент MeshRenderer ====================
// Компонент для отрисовки мешей на игровом объекте
class MeshRenderer : public Component {
public:
    MeshRenderer() = default;  // Конструктор по умолчанию
    explicit MeshRenderer(std::shared_ptr<Mesh> mesh) : mesh(mesh) {}  // Конструктор с мешем

    // Методы жизненного цикла компонента
    void start() override;  // Инициализация (вызывается один раз)
    void render() override; // Отрисовка (вызывается каждый кадр)

    // Сеттеры и геттеры
    void setMesh(std::shared_ptr<Mesh> newMesh) { mesh = newMesh; }
    std::shared_ptr<Mesh> getMesh() const { return mesh; }

    void setShaderProgram(std::shared_ptr<ShaderProgram> program) { shaderProgram = program; }
    std::shared_ptr<ShaderProgram> getShaderProgram() const { return shaderProgram; }

    // Макрос для регистрации компонента в системе (нужен для рефлексии/фабрики)
    REGISTER_COMPONENT(MeshRenderer)

private:
    std::shared_ptr<Mesh> mesh;                // Меш для отрисовки
    std::shared_ptr<ShaderProgram> shaderProgram;  // Шейдерная программа для рендеринга
};
