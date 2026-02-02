#include "Material.h"
#include "Shader.h"
#include "Logger.h"

Material::Material(const std::string& shaderName) : name(shaderName) {
    // Шейдер устанавливается через setShader
}

Material::~Material() {
    // Шейдер управляется ShaderManager, не удаляем
}

void Material::setProperty(const std::string& name, float value) {
    floatProperties[name] = value;
}

void Material::setProperty(const std::string& name, const glm::vec2& value) {
    vec2Properties[name] = value;
}

void Material::setProperty(const std::string& name, const glm::vec3& value) {
    vec3Properties[name] = value;
}

void Material::setProperty(const std::string& name, const glm::vec4& value) {
    vec4Properties[name] = value;
}

void Material::setProperty(const std::string& name, const glm::mat4& value) {
    mat4Properties[name] = value;
}

void Material::setProperty(const std::string& name, int value) {
    intProperties[name] = value;
}

void Material::setProperty(const std::string& name, bool value) {
    boolProperties[name] = value;
}

void Material::setTexture(const std::string& slot, std::shared_ptr<Texture> texture) {
    textures[slot] = texture;
}

std::shared_ptr<Texture> Material::getTexture(const std::string& slot) const {
    auto it = textures.find(slot);
    if (it != textures.end()) {
        return it->second;
    }
    return nullptr;
}

void Material::setShader(const std::string& shaderName) {
    name = shaderName;
    // Шейдер будет установлен при применении материала
}

void Material::apply() {
    if (!shader) return;

    shader->use();

    // Применяем свойства материала
    shader->setVec3("uAmbient", properties.ambient);
    shader->setVec3("uDiffuse", properties.diffuse);
    shader->setVec3("uSpecular", properties.specular);
    shader->setFloat("uShininess", properties.shininess);
    shader->setFloat("uOpacity", properties.opacity);

    // Применяем пользовательские свойства
    for (const auto& prop : floatProperties) {
        shader->setFloat(prop.first, prop.second);
    }
    for (const auto& prop : intProperties) {
        shader->setInt(prop.first, prop.second);
    }
    for (const auto& prop : vec2Properties) {
        shader->setVec2(prop.first, prop.second);
    }
    for (const auto& prop : vec3Properties) {
        shader->setVec3(prop.first, prop.second);
    }
    for (const auto& prop : vec4Properties) {
        shader->setVec4(prop.first, prop.second);
    }
    for (const auto& prop : mat4Properties) {
        shader->setMat4(prop.first, prop.second);
    }

    // Привязываем текстуры
    int textureSlot = 0;
    for (const auto& tex : textures) {
        if (tex.second) {
            tex.second->bind(textureSlot);
            shader->setInt(tex.first, textureSlot);
            textureSlot++;
        }
    }
}
