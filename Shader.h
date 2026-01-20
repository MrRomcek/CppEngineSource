#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Класс для отдельного шейдера (вершинный, фрагментный, геометрический)
class Shader {
private:
    unsigned int shaderID;
    GLenum shaderType;
    std::string shaderSource;
    bool compiled = false;

public:
    // Конструкторы
    Shader() : shaderID(0), shaderType(0) {}
    Shader(GLenum type, const std::string& source) : shaderType(type) {
        create(type, source);
    }

    // Деструктор
    ~Shader() {
        if (shaderID != 0) {
            glDeleteShader(shaderID);
        }
    }

    // Запрещаем копирование
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Разрешаем перемещение
    Shader(Shader&& other) noexcept
        : shaderID(other.shaderID),
        shaderType(other.shaderType),
        shaderSource(std::move(other.shaderSource)),
        compiled(other.compiled) {
        other.shaderID = 0;
        other.compiled = false;
    }

    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            if (shaderID != 0) {
                glDeleteShader(shaderID);
            }

            shaderID = other.shaderID;
            shaderType = other.shaderType;
            shaderSource = std::move(other.shaderSource);
            compiled = other.compiled;

            other.shaderID = 0;
            other.compiled = false;
        }
        return *this;
    }

    // Создание шейдера из исходного кода
    bool create(GLenum type, const std::string& source) {
        shaderType = type;
        shaderSource = source;

        // Создаем шейдер
        shaderID = glCreateShader(type);
        if (shaderID == 0) {
            std::cerr << "Failed to create shader" << std::endl;
            return false;
        }

        // Компилируем шейдер
        const char* sourceCode = source.c_str();
        glShaderSource(shaderID, 1, &sourceCode, nullptr);
        glCompileShader(shaderID);

        // Проверяем ошибки компиляции
        return checkCompileErrors();
    }

    // Загрузка шейдера из файла
    bool loadFromFile(GLenum type, const std::string& filepath) {
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            shaderFile.open(filepath);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();

            return create(type, shaderStream.str());
        }
        catch (std::ifstream::failure& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: "
                << filepath << std::endl;
            return false;
        }
    }

    // Получение ID шейдера
    unsigned int getID() const { return shaderID; }

    // Проверка скомпилирован ли шейдер
    bool isCompiled() const { return compiled; }

    // Получение типа шейдера
    GLenum getType() const { return shaderType; }

    // Получение исходного кода
    const std::string& getSource() const { return shaderSource; }

private:
    // Проверка ошибок компиляции
    bool checkCompileErrors() {
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shaderID, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                << infoLog << std::endl;
            compiled = false;
            return false;
        }

        compiled = true;
        return true;
    }
};

// Класс для шейдерной программы (линковка нескольких шейдеров)
class ShaderProgram {
private:
    unsigned int programID;
    bool linked = false;
    bool inUse = false;

    // Кэш для location uniform-переменных
    std::unordered_map<std::string, GLint> uniformLocations;

public:
    // Конструкторы
    ShaderProgram() : programID(0) {}

    // Деструктор
    ~ShaderProgram() {
        if (programID != 0) {
            glDeleteProgram(programID);
        }
    }

    // Запрещаем копирование
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    // Разрешаем перемещение
    ShaderProgram(ShaderProgram&& other) noexcept
        : programID(other.programID),
        linked(other.linked),
        inUse(other.inUse),
        uniformLocations(std::move(other.uniformLocations)) {
        other.programID = 0;
        other.linked = false;
        other.inUse = false;
    }

    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            if (programID != 0) {
                glDeleteProgram(programID);
            }

            programID = other.programID;
            linked = other.linked;
            inUse = other.inUse;
            uniformLocations = std::move(other.uniformLocations);

            other.programID = 0;
            other.linked = false;
            other.inUse = false;
        }
        return *this;
    }

    // Создание программы
    bool create() {
        programID = glCreateProgram();
        if (programID == 0) {
            std::cerr << "Failed to create shader program" << std::endl;
            return false;
        }
        return true;
    }

    // Присоединение шейдера к программе
    bool attachShader(const Shader& shader) {
        if (!shader.isCompiled()) {
            std::cerr << "Cannot attach uncompiled shader" << std::endl;
            return false;
        }

        glAttachShader(programID, shader.getID());
        return true;
    }

    // Присоединение шейдера по исходному коду
    bool attachShader(GLenum type, const std::string& source) {
        Shader shader(type, source);
        if (!shader.isCompiled()) {
            return false;
        }
        return attachShader(shader);
    }

    // Присоединение шейдера из файла
    bool attachShaderFromFile(GLenum type, const std::string& filepath) {
        Shader shader;
        if (!shader.loadFromFile(type, filepath)) {
            return false;
        }
        return attachShader(shader);
    }

    // Линковка программы
    bool link() {
        glLinkProgram(programID);

        // Проверяем ошибки линковки
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(programID, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n"
                << infoLog << std::endl;
            linked = false;
            return false;
        }

        linked = true;
        uniformLocations.clear(); // Очищаем кэш при перелинковке
        return true;
    }

    // Проверка валидности программы
    bool validate() {
        if (!linked) return false;

        glValidateProgram(programID);

        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(programID, GL_VALIDATE_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(programID, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM::VALIDATION_FAILED\n"
                << infoLog << std::endl;
            return false;
        }
        return true;
    }

    // Использование программы
    void use() {
        if (!linked) {
            std::cerr << "Cannot use unlinked shader program" << std::endl;
            return;
        }

        glUseProgram(programID);
        inUse = true;
    }

    // Отключение программы
    void unuse() {
        glUseProgram(0);
        inUse = false;
    }

    // Проверка используется ли программа
    bool isInUse() const { return inUse; }

    // Проверка залинкована ли программа
    bool isLinked() const { return linked; }

    // Получение ID программы
    unsigned int getID() const { return programID; }

    // ==================== Установка uniform-переменных ====================

    // Получение location uniform-переменной (с кэшированием)
    GLint getUniformLocation(const std::string& name) {
        // Проверяем кэш
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end()) {
            return it->second;
        }

        // Получаем location
        GLint location = glGetUniformLocation(programID, name.c_str());

        // Кэшируем результат (даже если -1)
        uniformLocations[name] = location;

        if (location == -1 && linked) {
            std::cerr << "Warning: Uniform '" << name
                << "' not found in shader program" << std::endl;
        }

        return location;
    }

    // Установка bool
    void setBool(const std::string& name, bool value) {
        setInt(name, value ? 1 : 0);
    }

    // Установка int
    void setInt(const std::string& name, int value) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniform1i(location, value);
        }
    }

    // Установка float
    void setFloat(const std::string& name, float value) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniform1f(location, value);
        }
    }

    // Установка vec2
    void setVec2(const std::string& name, const glm::vec2& value) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniform2f(location, value.x, value.y);
        }
    }

    void setVec2(const std::string& name, float x, float y) {
        setVec2(name, glm::vec2(x, y));
    }

    // Установка vec3
    void setVec3(const std::string& name, const glm::vec3& value) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniform3f(location, value.x, value.y, value.z);
        }
    }

    void setVec3(const std::string& name, float x, float y, float z) {
        setVec3(name, glm::vec3(x, y, z));
    }

    // Установка vec4
    void setVec4(const std::string& name, const glm::vec4& value) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniform4f(location, value.x, value.y, value.z, value.w);
        }
    }

    void setVec4(const std::string& name, float x, float y, float z, float w) {
        setVec4(name, glm::vec4(x, y, z, w));
    }

    // Установка mat2
    void setMat2(const std::string& name, const glm::mat2& mat) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(mat));
        }
    }

    // Установка mat3
    void setMat3(const std::string& name, const glm::mat3& mat) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
        }
    }

    // Установка mat4
    void setMat4(const std::string& name, const glm::mat4& mat) {
        if (!inUse) use();
        GLint location = getUniformLocation(name);
        if (location != -1) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
        }
    }

    // ==================== Утилитные методы ====================

    // Создание программы из вершинного и фрагментного шейдера
    static std::unique_ptr<ShaderProgram> createFromSource(
        const std::string& vertexSource,
        const std::string& fragmentSource) {

        auto program = std::make_unique<ShaderProgram>();
        if (!program->create()) {
            return nullptr;
        }

        if (!program->attachShader(GL_VERTEX_SHADER, vertexSource) ||
            !program->attachShader(GL_FRAGMENT_SHADER, fragmentSource) ||
            !program->link()) {
            return nullptr;
        }

        return program;
    }

    // Создание программы из файлов
    static std::unique_ptr<ShaderProgram> createFromFiles(
        const std::string& vertexPath,
        const std::string& fragmentPath) {

        auto program = std::make_unique<ShaderProgram>();
        if (!program->create()) {
            return nullptr;
        }

        if (!program->attachShaderFromFile(GL_VERTEX_SHADER, vertexPath) ||
            !program->attachShaderFromFile(GL_FRAGMENT_SHADER, fragmentPath) ||
            !program->link()) {
            return nullptr;
        }

        return program;
    }

    // Получение информации о программе
    void printInfo() const {
        if (!linked) {
            std::cout << "Shader program is not linked" << std::endl;
            return;
        }

        std::cout << "=== Shader Program Info ===" << std::endl;
        std::cout << "Program ID: " << programID << std::endl;

        // Получаем количество активных uniform-переменных
        GLint numUniforms;
        glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numUniforms);
        std::cout << "Active uniforms: " << numUniforms << std::endl;

        // Получаем количество активных атрибутов
        GLint numAttributes;
        glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &numAttributes);
        std::cout << "Active attributes: " << numAttributes << std::endl;

        std::cout << "===========================" << std::endl;
    }

    // Получение лога программы
    std::string getInfoLog() const {
        if (!linked) return "Program not linked";

        GLint length;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);

        if (length > 0) {
            std::string log(length, '\0');
            glGetProgramInfoLog(programID, length, nullptr, &log[0]);
            return log;
        }

        return "";
    }
};

// ==================== Вспомогательные классы ====================

// Класс для управления несколькими шейдерными программами
class ShaderManager {
private:
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shaders;

public:
    // Добавление шейдера
    bool addShader(const std::string& name, std::unique_ptr<ShaderProgram> shader) {
        if (!shader || !shader->isLinked()) {
            return false;
        }

        shaders[name] = std::move(shader);
        return true;
    }

    // Создание и добавление шейдера из исходного кода
    bool createShaderFromSource(const std::string& name,
        const std::string& vertexSource,
        const std::string& fragmentSource) {
        auto shader = ShaderProgram::createFromSource(vertexSource, fragmentSource);
        if (!shader) {
            return false;
        }
        return addShader(name, std::move(shader));
    }

    // Создание и добавление шейдера из файлов
    bool createShaderFromFiles(const std::string& name,
        const std::string& vertexPath,
        const std::string& fragmentPath) {
        auto shader = ShaderProgram::createFromFiles(vertexPath, fragmentPath);
        if (!shader) {
            return false;
        }
        return addShader(name, std::move(shader));
    }

    // Получение шейдера
    ShaderProgram* getShader(const std::string& name) {
        auto it = shaders.find(name);
        if (it != shaders.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Удаление шейдера
    bool removeShader(const std::string& name) {
        return shaders.erase(name) > 0;
    }

    // Использование шейдера
    void useShader(const std::string& name) {
        auto shader = getShader(name);
        if (shader) {
            shader->use();
        }
    }

    // Проверка наличия шейдера
    bool hasShader(const std::string& name) const {
        return shaders.find(name) != shaders.end();
    }

    // Очистка всех шейдеров
    void clear() {
        shaders.clear();
    }

    // Получение количества шейдеров
    size_t size() const {
        return shaders.size();
    }
};