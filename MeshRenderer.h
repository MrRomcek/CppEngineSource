#pragma once
#include "Component.h"
#include "Transform.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

#include "Core.h"
#include "Camera.h"


class Camera;
class Core;

// Класс для представления меша
class Mesh {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCoord;
        glm::vec3 normal;
    };

    Mesh() = default;
    ~Mesh();

    bool createFromVertices(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices = {});

    void render() const;

    // Методы для создания примитивов
    static std::shared_ptr<Mesh> createTriangle();
    static std::shared_ptr<Mesh> createQuad(float size = 1.0f);
    static std::shared_ptr<Mesh> createCube(float size = 1.0f);
    static std::shared_ptr<Mesh> createLine(const glm::vec3& start,
        const glm::vec3& end,
        const glm::vec3& color = glm::vec3(1.0f));

    unsigned int getVAO() const { return VAO; }
    unsigned int getVertexCount() const { return vertexCount; }
    unsigned int getIndexCount() const { return indexCount; }

private:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int vertexCount = 0;
    unsigned int indexCount = 0;

    void setupMesh(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);
};

// Компонент MeshRenderer
class MeshRenderer : public Component {
public:
    MeshRenderer() = default;
    explicit MeshRenderer(std::shared_ptr<Mesh> mesh) : mesh(mesh) {}

    void start() override;
    void render() override;

    void setMesh(std::shared_ptr<Mesh> newMesh) { mesh = newMesh; }
    std::shared_ptr<Mesh> getMesh() const { return mesh; }

    void setShaderProgram(std::shared_ptr<ShaderProgram> program) { shaderProgram = program; }
    std::shared_ptr<ShaderProgram> getShaderProgram() const { return shaderProgram; }

    // Регистрация компонента
    REGISTER_COMPONENT(MeshRenderer)

private:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<ShaderProgram> shaderProgram;
};