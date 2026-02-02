#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Mesh;
class Material;

// ==================== Model Loader ====================
class ModelLoader {
public:
    // Загрузить модель из файла OBJ
    static std::shared_ptr<Mesh> loadOBJ(const std::string& filepath);

    // Загрузить материал из файла MTL
    static std::shared_ptr<Material> loadMTL(const std::string& filepath);

    // Получить директорию из пути файла
    static std::string getDirectory(const std::string& filepath);

private:
    ModelLoader() = default;
};

// ==================== Model Asset Manager ====================
class ModelManager {
public:
    static ModelManager& getInstance() {
        static ModelManager instance;
        return instance;
    }

    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    // Загрузить или получить модель из кэша
    std::shared_ptr<Mesh> loadModel(const std::string& filepath);

    // Очистить кэш
    void clearCache();
    void removeModel(const std::string& filepath);

private:
    ModelManager() = default;
    ~ModelManager() = default;

    std::unordered_map<std::string, std::shared_ptr<Mesh>> modelCache;
};
