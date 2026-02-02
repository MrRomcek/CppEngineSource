#include "ModelLoader.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>

std::string ModelLoader::getDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return "";
    }
    return filepath.substr(0, lastSlash + 1);
}

std::shared_ptr<Mesh> ModelLoader::loadOBJ(const std::string& filepath) {
    auto logger = LogManager::getInstance().createConsoleLogger("ModelLoader");

    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open OBJ file: %s", filepath.c_str());
        return nullptr;
    }

    std::vector<Mesh::Vertex> vertices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    std::string line;
    int vertexCount = 0;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // Позиция вершины
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vt") {
            // Текстурные координаты
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // Нормали
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(glm::normalize(normal));
        }
        else if (prefix == "f") {
            // Грани (треугольники)
            std::string faceData;
            while (iss >> faceData) {
                std::istringstream faceStream(faceData);
                unsigned int posIndex = 0, texIndex = 0, normalIndex = 0;
                char slash;

                // Парсим формат: position/texture/normal
                faceStream >> posIndex;
                if (faceStream.peek() == '/') {
                    faceStream >> slash;
                    if (faceStream.peek() != '/') {
                        faceStream >> texIndex;
                    }
                    if (faceStream.peek() == '/') {
                        faceStream >> slash >> normalIndex;
                    }
                }

                // OBJ индексирует с 1, нам нужно с 0
                posIndex--;
                if (texIndex > 0) texIndex--;
                if (normalIndex > 0) normalIndex--;

                Mesh::Vertex vertex;
                if (posIndex < positions.size()) {
                    vertex.position = positions[posIndex];
                }
                if (texIndex < texCoords.size()) {
                    vertex.texCoord = texCoords[texIndex];
                }
                if (normalIndex < normals.size()) {
                    vertex.normal = normals[normalIndex];
                }
                else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                
                vertex.color = glm::vec3(1.0f);

                vertices.push_back(vertex);
                indices.push_back(vertexCount++);
            }
        }
    }

    file.close();

    if (vertices.empty()) {
        LOG_ERROR("No vertices loaded from OBJ file: %s", filepath.c_str());
        return nullptr;
    }

    auto mesh = std::make_shared<Mesh>();
    if (mesh->createFromVertices(vertices, indices)) {
        LOG_INFO("Loaded OBJ model: %s (%d vertices)", filepath.c_str(), vertices.size());
        return mesh;
    }

    return nullptr;
}

std::shared_ptr<Material> ModelLoader::loadMTL(const std::string& filepath) {
    auto logger = LogManager::getInstance().createConsoleLogger("ModelLoader");
    LOG_INFO("Loading MTL file: %s", filepath.c_str());
    
    // Базовая реализация - можно расширить
    auto material = std::make_shared<Material>();
    return material;
}

// ==================== Model Manager ====================
std::shared_ptr<Mesh> ModelManager::loadModel(const std::string& filepath) {
    auto it = modelCache.find(filepath);
    if (it != modelCache.end()) {
        return it->second;
    }

    // Определяем тип файла по расширению
    size_t dotPos = filepath.find_last_of(".");
    if (dotPos == std::string::npos) {
        return nullptr;
    }

    std::string extension = filepath.substr(dotPos);
    std::shared_ptr<Mesh> mesh = nullptr;

    if (extension == ".obj") {
        mesh = ModelLoader::loadOBJ(filepath);
    }
    else {
        auto logger = LogManager::getInstance().createConsoleLogger("ModelManager");
        LOG_WARNING("Unsupported model format: %s", extension.c_str());
        return nullptr;
    }

    if (mesh) {
        modelCache[filepath] = mesh;
    }

    return mesh;
}

void ModelManager::clearCache() {
    modelCache.clear();
}

void ModelManager::removeModel(const std::string& filepath) {
    auto it = modelCache.find(filepath);
    if (it != modelCache.end()) {
        modelCache.erase(it);
    }
}
