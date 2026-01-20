#include "MeshRenderer.h"
#include "GameObject.h"
#include <iostream>

// ==================== Реализация Mesh ====================

Mesh::~Mesh() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

bool Mesh::createFromVertices(const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices) {
    if (vertices.empty()) return false;

    setupMesh(vertices, indices);
    return true;
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices) {
    // Генерируем буферы
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Привязываем VAO
    glBindVertexArray(VAO);

    // Копируем данные вершин в буфер
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_STATIC_DRAW);

    // Настраиваем атрибуты вершин
    // Позиция
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Цвет
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Текстурные координаты
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    // Нормали
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);

    // Настройка EBO (если есть индексы)
    if (!indices.empty()) {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            indices.data(), GL_STATIC_DRAW);
        indexCount = static_cast<unsigned int>(indices.size());
    }

    // Отвязываем VAO
    glBindVertexArray(0);

    vertexCount = static_cast<unsigned int>(vertices.size());
}

void Mesh::render() const {
    if (VAO == 0) return;

    glBindVertexArray(VAO);
    if (indexCount > 0) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    glBindVertexArray(0);
}

// Создание треугольника
std::shared_ptr<Mesh> Mesh::createTriangle() {
    auto mesh = std::make_shared<Mesh>();

    std::vector<Mesh::Vertex> vertices = {
        // Позиции          // Цвета           // Текстурные координаты  // Нормали
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // Нижний левый
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // Нижний правый
        {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}}   // Верхний
    };

    mesh->createFromVertices(vertices);
    return mesh;
}

// Создание квадрата
std::shared_ptr<Mesh> Mesh::createQuad(float size) {
    auto mesh = std::make_shared<Mesh>();

    float halfSize = size * 0.5f;
    std::vector<Mesh::Vertex> vertices = {
        // Позиции                    // Цвета                // Текстурные координаты  // Нормали
        {{-halfSize, -halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfSize, -halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfSize,  halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-halfSize,  halfSize, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };

    mesh->createFromVertices(vertices, indices);
    return mesh;
}

// Создание куба
std::shared_ptr<Mesh> Mesh::createCube(float size) {
    auto mesh = std::make_shared<Mesh>();

    float halfSize = size * 0.5f;
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

    std::vector<unsigned int> indices = {
        // Передняя грань
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

    mesh->createFromVertices(vertices);
    return mesh;
}

// ==================== Реализация MeshRenderer ====================

void MeshRenderer::start() {
    // Создаем простой шейдер для рендеринга
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

    const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

    // Создаем и компилируем шейдерную программу
    ShaderProgram program;
    if (!program.create()) {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }

    if (!program.attachShader(GL_VERTEX_SHADER, vertexShaderSource) ||
        !program.attachShader(GL_FRAGMENT_SHADER, fragmentShaderSource) ||
        !program.link()) {
        std::cerr << "Failed to compile/link shader program" << std::endl;
        return;
    }
    
    // Создаем shared_ptr из временного объекта
    shaderProgram = std::make_shared<ShaderProgram>(std::move(program));
}

void MeshRenderer::render() {
    if (!mesh || !gameObject || !shaderProgram) return;

    // Получаем Transform компонент
    Transform* transform = gameObject->getComponent<Transform>();
    if (!transform) return;

    // Получаем камеру из Core
    Core& core = Core::getInstance();
    Camera* camera = core.getCamera();
    if (!camera) return;

    // Используем шейдер
    shaderProgram->use();

    // Устанавливаем uniform переменные
    glm::mat4 model = transform->getModelMatrix();
    
    // Получаем матрицы из камеры
    glm::mat4 view = camera->getViewMatrix();
    
    Core::Config config = core.getConfig();
    float aspectRatio = static_cast<float>(config.width) / static_cast<float>(config.height);
    glm::mat4 projection = camera->getProjectionMatrix(aspectRatio);

    shaderProgram->setMat4("model", model);
    shaderProgram->setMat4("view", view);
    shaderProgram->setMat4("projection", projection);

    // Рендерим меш
    mesh->render();
}