#include "MeshRenderer.h"
#include "GameObject.h"
#include <iostream>

// ==================== Реализация класса Mesh ====================

// Деструктор Mesh: освобождает ресурсы OpenGL
Mesh::~Mesh() {
    if (VBO) glDeleteBuffers(1, &VBO);   // Удаляем Vertex Buffer
    if (EBO) glDeleteBuffers(1, &EBO);   // Удаляем Element Buffer (если есть)
    if (VAO) glDeleteVertexArrays(1, &VAO);  // Удаляем Vertex Array
}

// Создание меша из массивов вершин и индексов
bool Mesh::createFromVertices(const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices) {
    if (vertices.empty()) return false;  // Проверка на пустой массив вершин

    setupMesh(vertices, indices);  // Настраиваем OpenGL буферы
    return true;  // Успешное создание
}

// Настройка OpenGL буферов для меша
void Mesh::setupMesh(const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices) {

    // Генерируем идентификаторы буферов
    glGenVertexArrays(1, &VAO);  // Создаем Vertex Array Object
    glGenBuffers(1, &VBO);       // Создаем Vertex Buffer Object

    // Привязываем VAO для настройки атрибутов
    glBindVertexArray(VAO);

    // Копируем данные вершин в буфер VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_STATIC_DRAW);  // GL_STATIC_DRAW - данные не будут меняться часто

    // ============= НАСТРОЙКА АТРИБУТОВ ВЕРШИН =============

    // Атрибут 0: Позиция вершины (3 компонента float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);  // Включаем атрибут

    // Атрибут 1: Цвет вершины (3 компонента float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, color));  // offsetof вычисляет смещение поля в структуре
    glEnableVertexAttribArray(1);

    // Атрибут 2: Текстурные координаты (2 компонента float)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    // Атрибут 3: Нормаль вершины (3 компонента float)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);

    // ============= НАСТРОЙКА EBO (ИНДЕКСОВ) =============
    if (!indices.empty()) {
        glGenBuffers(1, &EBO);  // Создаем Element Buffer Object
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            indices.data(), GL_STATIC_DRAW);
        indexCount = static_cast<unsigned int>(indices.size());  // Сохраняем количество индексов
    }

    // Отвязываем VAO (защита от случайных изменений)
    glBindVertexArray(0);

    // Сохраняем количество вершин
    vertexCount = static_cast<unsigned int>(vertices.size());
}

// Отрисовка меша
void Mesh::render() const {
    if (VAO == 0) return;  // Проверка на инициализацию

    glBindVertexArray(VAO);  // Привязываем VAO для отрисовки

    if (indexCount > 0) {
        // Отрисовка с использованием индексов
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    else {
        // Отрисовка без индексов (по вершинам)
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);  // Отвязываем VAO
}

// ============= РЕАЛИЗАЦИЯ МЕТОДОВ СОЗДАНИЯ ПРИМИТИВОВ =============

// Создание треугольника
std::shared_ptr<Mesh> Mesh::createTriangle() {
    auto mesh = std::make_shared<Mesh>();  // Создаем новый меш

    std::vector<Mesh::Vertex> vertices = {
        // Позиции          // Цвета (красный)  // Текстурные координаты  // Нормали (направлена по Z)
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // Нижний левый угол
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // Нижний правый угол
        {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}}   // Верхняя вершина
    };

    mesh->createFromVertices(vertices);  // Создаем меш из вершин
    return mesh;  // Возвращаем shared_ptr на меш
}

// Создание квадрата
std::shared_ptr<Mesh> Mesh::createQuad(float size) {
    auto mesh = std::make_shared<Mesh>();

    float halfSize = size * 0.5f;  // Половина размера для центрирования
    std::vector<Mesh::Vertex> vertices = {
        // Позиции                    // Цвета (белый)     // Текстурные координаты  // Нормали
        {{-halfSize, -halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfSize, -halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfSize,  halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-halfSize,  halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
    };

    // Индексы для формирования двух треугольников
    std::vector<unsigned int> indices = {
        0, 1, 2,  // Первый треугольник (нижний правый)
        2, 3, 0   // Второй треугольник (верхний левый)
    };

    mesh->createFromVertices(vertices, indices);  // Создаем меш с индексами
    return mesh;
}

// Создание куба
std::shared_ptr<Mesh> Mesh::createCube(float size) {
    auto mesh = std::make_shared<Mesh>();

    float halfSize = size * 0.5f;
    // Массив из 24 вершин (4 вершины на каждую из 6 граней)
    std::vector<Mesh::Vertex> vertices = {
        // Передняя грань
        {{-halfSize, -halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfSize, -halfSize,  halfSize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfSize,  halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-halfSize,  halfSize,  halfSize}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        // Задняя грань
        {{-halfSize, -halfSize, -halfSize}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{ halfSize, -halfSize, -halfSize}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{ halfSize,  halfSize, -halfSize}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {{-halfSize,  halfSize, -halfSize}, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},

        // Верхняя грань
        {{-halfSize,  halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{ halfSize,  halfSize,  halfSize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{ halfSize,  halfSize, -halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-halfSize,  halfSize, -halfSize}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},

        // Нижняя грань
        {{-halfSize, -halfSize,  halfSize}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{ halfSize, -halfSize,  halfSize}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{ halfSize, -halfSize, -halfSize}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
        {{-halfSize, -halfSize, -halfSize}, {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},

        // Правая грань
        {{ halfSize, -halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ halfSize, -halfSize, -halfSize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ halfSize,  halfSize, -halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{ halfSize,  halfSize,  halfSize}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

        // Левая грань
        {{-halfSize, -halfSize,  halfSize}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-halfSize,  halfSize,  halfSize}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-halfSize,  halfSize, -halfSize}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-halfSize, -halfSize, -halfSize}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}
    };

    // Индексы для 6 граней (по 2 треугольника на грань = 12 треугольников)
    std::vector<unsigned int> indices = {
        // Передняя грань (2 треугольника)
        0, 1, 2, 2, 3, 0,
        // Задняя грань
        4, 5, 6, 6, 7, 4,
        // Верхняя грань
        8, 9, 10, 10, 11, 8,
        // Нижняя грань
        12, 13, 14, 14, 15, 12,
        // Правая грань
        16, 17, 18, 18, 19, 16,
        // Левая грань
        20, 21, 22, 22, 23, 20
    };

    mesh->createFromVertices(vertices, indices);
    return mesh;
}

// Создание линии
std::shared_ptr<Mesh> Mesh::createLine(const glm::vec3& start,
    const glm::vec3& end,
    const glm::vec3& color) {
    auto mesh = std::make_shared<Mesh>();

    std::vector<Mesh::Vertex> vertices = {
        {{start.x, start.y, start.z}, {color.r, color.g, color.b}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{end.x, end.y, end.z}, {color.r, color.g, color.b}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
    };

    mesh->createFromVertices(vertices);  // Создаем меш из двух вершин
    return mesh;
}

// ==================== Реализация класса MeshRenderer ====================

// Инициализация компонента MeshRenderer
void MeshRenderer::start() {
    // Исходный код вершинного шейдера в виде строки
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;        // Атрибут позиции (связывается с location = 0)
layout (location = 1) in vec3 aColor;      // Атрибут цвета (связывается с location = 1)

uniform mat4 model;        // Матрица модели (преобразования объекта)
uniform mat4 view;         // Матрица вида (преобразования камеры)
uniform mat4 projection;   // Матрица проекции (перспектива)

out vec3 ourColor;         // Выходная переменная для передачи цвета во фрагментный шейдер

void main() {
    // Преобразование позиции из локальных координат в экранные
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Передача цвета дальше
    ourColor = aColor;
}
)";

    // Исходный код фрагментного шейдера в виде строки
    const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;          // Входная переменная цвета из вершинного шейдера
out vec4 FragColor;        // Выходной цвет пикселя

void main() {
    FragColor = vec4(ourColor, 1.0);  // Устанавливаем цвет с альфа-каналом = 1.0
}
)";

    // Создаем и компилируем шейдерную программу
    ShaderProgram program;  // Создаем временный объект
    if (!program.create()) {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }

    // Присоединяем и компилируем шейдеры
    if (!program.attachShader(GL_VERTEX_SHADER, vertexShaderSource) ||
        !program.attachShader(GL_FRAGMENT_SHADER, fragmentShaderSource) ||
        !program.link()) {
        std::cerr << "Failed to compile/link shader program" << std::endl;
        return;
    }

    // Создаем shared_ptr из временного объекта (перемещаем)
    shaderProgram = std::make_shared<ShaderProgram>(std::move(program));
}

// Отрисовка компонента MeshRenderer
void MeshRenderer::render() {
    // Проверка необходимых условий для рендеринга
    if (!mesh || !gameObject || !shaderProgram) return;

    // Получаем Transform компонент игрового объекта
    Transform* transform = gameObject->getComponent<Transform>();
    if (!transform) return;  // Если нет Transform, нечего отрисовывать

    // Получаем синглтон Core для доступа к камере
    Core& core = Core::getInstance();
    Camera* camera = core.getCamera();
    if (!camera) return;  // Если нет камеры, нечего отрисовывать

    // Активируем шейдерную программу
    shaderProgram->use();

    // Получаем матрицу модели из Transform компонента
    glm::mat4 model = transform->getModelMatrix();

    // Получаем матрицы вида и проекции из камеры
    glm::mat4 view = camera->getViewMatrix();  // Матрица вида камеры

    // Получаем соотношение сторон окна для расчета проекции
    Core::Config config = core.getConfig();
    float aspectRatio = static_cast<float>(config.width) / static_cast<float>(config.height);
    glm::mat4 projection = camera->getProjectionMatrix(aspectRatio);  // Матрица проекции

    // Устанавливаем uniform-переменные в шейдере
    shaderProgram->setMat4("model", model);          // Матрица модели
    shaderProgram->setMat4("view", view);           // Матрица вида
    shaderProgram->setMat4("projection", projection); // Матрица проекции

    // Отрисовываем меш
    mesh->render();
}