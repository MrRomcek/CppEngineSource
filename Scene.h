#pragma once
#include "GameObject.h"
#include "Camera.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>

class Scene
{
public:
    Scene(const std::string& name);
    ~Scene();

    // Object management
    GameObject* createGameObject(const std::string& name = "GameObject");
    GameObject* createGameObject(const std::string& name, GameObject* parent);
    void destroyGameObject(GameObject* obj);
    GameObject* findByName(const std::string& name);
    GameObject* findWithComponent(const std::string& componentType);
    std::vector<GameObject*> getAllWithComponent(const std::string& componentType);

    // Scene graph
    void addGameObject(std::unique_ptr<GameObject> obj);
    void update(float deltaTime);
    void render();

    // Camera management
    Camera* createCamera(const std::string& name = "Camera");
    void setActiveCamera(Camera* camera);
    Camera* getActiveCamera() const { return activeCamera; }

    // Getters
    const std::string& getName() const { return name; }
    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return objects; }
    const std::vector<std::unique_ptr<Camera>>& getCameras() const { return cameras; }

    // Events
    using SceneEvent = std::function<void(Scene&)>;
    SceneEvent onLoad;
    SceneEvent onUnload;

private:
    std::string name;
    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<std::unique_ptr<Camera>> cameras;
    Camera* activeCamera = nullptr;
    std::unordered_map<std::string, std::vector<GameObject*>> componentCache;

    void rebuildComponentCache();
};