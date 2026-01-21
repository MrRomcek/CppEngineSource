#pragma once
#include "Shader.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <string>

struct MaterialProperties {
    glm::vec3 ambient = glm::vec3(0.2f);   // Фоновое освещение
    glm::vec3 diffuse = glm::vec3(0.8f);   // Рассеянный свет
    glm::vec3 specular = glm::vec3(1.0f);  // Зеркальное отражение
    float shininess = 32.0f;               // Степень блеска (экспонента)
    float opacity = 1.0f;                  // Прозрачность (1 = полностью непрозрачный)
    bool useTexture = false;               // Использовать ли текстуры
};

class Material {
public:
    Material(const std::string& shaderName = "basic");
    ~Material();

    // Установка свойств
    void setProperty(const std::string& name, float value);
    void setProperty(const std::string& name, const glm::vec2& value);
    void setProperty(const std::string& name, const glm::vec3& value);
    void setProperty(const std::string& name, const glm::vec4& value);
    void setProperty(const std::string& name, const glm::mat4& value);
    void setProperty(const std::string& name, int value);
    void setProperty(const std::string& name, bool value);

    // Текстуры
    void setTexture(const std::string& slot, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(const std::string& slot) const;

    // Шейдер
    void setShader(const std::string& shaderName);
    ShaderProgram* getShader() const { return shader; }

    // Применение материала (биндинг uniform'ов)
    void apply();

    // Геттеры
    const std::string& getName() const { return name; }
    const MaterialProperties& getProperties() const { return properties; }

private:
    std::string name;
    ShaderProgram* shader = nullptr;
    MaterialProperties properties;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, int> intProperties;
    std::unordered_map<std::string, float> floatProperties;
    std::unordered_map<std::string, glm::vec2> vec2Properties;
    std::unordered_map<std::string, glm::vec3> vec3Properties;
    std::unordered_map<std::string, glm::vec4> vec4Properties;
    std::unordered_map<std::string, glm::mat4> mat4Properties;
    std::unordered_map<std::string, bool> boolProperties;
};