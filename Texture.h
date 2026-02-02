#pragma once
#include <string>
#include <glad/glad.h>
#include <memory>
#include <unordered_map>

class Texture {
public:
    enum class Type {
        Diffuse,
        Specular,
        Normal,
        Emissive
    };

    Texture() = default;
    ~Texture();

    // Загрузка текстуры из файла (поддерживает JPG, PNG и т.д.)
    bool loadFromFile(const std::string& filepath, Type type = Type::Diffuse);

    // Создание пустой текстуры
    bool create(int width, int height, GLenum format = GL_RGBA, const unsigned char* pixels = nullptr);

    // Привязка текстуры
    void bind(unsigned int slot = 0) const;
    static void unbind();

    // Геттеры
    unsigned int getID() const { return textureID; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Type getType() const { return type; }
    const std::string& getPath() const { return path; }

private:
    unsigned int textureID = 0;
    int width = 0;
    int height = 0;
    Type type = Type::Diffuse;
    std::string path;

    GLenum getGLFormat(int channels) const;
};

// ==================== Texture Manager ====================
class TextureManager {
public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // Загрузить или получить текстуру из кэша
    std::shared_ptr<Texture> loadTexture(const std::string& filepath, 
                                         Texture::Type type = Texture::Type::Diffuse);

    // Очистить кэш
    void clearCache();
    void removeTexture(const std::string& filepath);

private:
    TextureManager() = default;
    ~TextureManager() = default;

    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
};

