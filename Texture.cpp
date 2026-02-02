#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include "Logger.h"
#include "stb_image.h"
#include <iostream>

// ==================== Texture Class ====================
Texture::~Texture() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

bool Texture::loadFromFile(const std::string& filepath, Type textureType) {
    auto logger = LogManager::getInstance().createConsoleLogger("Texture");

    int channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    if (!data) {
        LOG_ERROR("Failed to load texture: %s", filepath.c_str());
        return false;
    }

    type = textureType;
    path = filepath;

    bool success = create(width, height, getGLFormat(channels), data);
    stbi_image_free(data);

    if (success) {
        LOG_INFO("Loaded texture: %s (%dx%d)", filepath.c_str(), width, height);
    }

    return success;
}

bool Texture::create(int w, int h, GLenum format, const unsigned char* pixels) {
    width = w;
    height = h;

    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Параметры текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Загружаем данные
    GLenum dataFormat = format;
    if (format == GL_RGBA) {
        dataFormat = GL_RGBA;
    }
    else if (format == GL_RGB) {
        dataFormat = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLenum Texture::getGLFormat(int channels) const {
    switch (channels) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGBA;
    }
}

// ==================== TextureManager ====================
std::shared_ptr<Texture> TextureManager::loadTexture(const std::string& filepath, Texture::Type type) {
    auto it = textureCache.find(filepath);
    if (it != textureCache.end()) {
        return it->second;
    }

    auto texture = std::make_shared<Texture>();
    if (texture->loadFromFile(filepath, type)) {
        textureCache[filepath] = texture;
        return texture;
    }

    return nullptr;
}

void TextureManager::clearCache() {
    textureCache.clear();
}

void TextureManager::removeTexture(const std::string& filepath) {
    auto it = textureCache.find(filepath);
    if (it != textureCache.end()) {
        textureCache.erase(it);
    }
}
