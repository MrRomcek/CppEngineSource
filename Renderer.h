#pragma once
#include "Scene.h"
#include "Shader.h"
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <glm/glm.hpp>

struct RenderCommand {
    enum class Type {
        DrawMesh,
        Clear,
        SetViewport,
        SetClearColor,
        EnableDepthTest,
        DisableDepthTest
    };

    Type type;
    std::function<void()> execute;
    int priority = 0; // Для сортировки
};

class RenderQueue {
public:
    void push(const RenderCommand& cmd);
    void execute();
    void clear();

private:
    std::vector<RenderCommand> commands;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool initialize();
    void renderScene(Scene* scene);
    void setViewport(int x, int y, int width, int height);
    void setClearColor(const glm::vec4& color);
    void clear();

    // Shader management
    bool loadShader(const std::string& name,
        const std::string& vertPath,
        const std::string& fragPath);
    ShaderProgram* getShader(const std::string& name);

    // Render states
    void enableDepthTest(bool enable = true);
    void enableBlending(bool enable = true);
    void enableFaceCulling(bool enable = true);

private:
    void processRenderCommands();
    void setupDefaultShaders();

    RenderQueue renderQueue;
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shaders;
    glm::vec4 clearColor = { 0.1f, 0.1f, 0.2f, 1.0f };
    int viewportWidth = 800;
    int viewportHeight = 600;

    // Render states
    bool depthTestEnabled = true;
    bool blendingEnabled = false;
    bool faceCullingEnabled = true;
};