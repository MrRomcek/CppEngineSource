#pragma once
#include "Component.h"
#include "Transform.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <typeindex>

// Предварительное объявление
class GameObject;

// ==================== Класс GameObject ====================
class GameObject {
private:
    std::string name;
    bool active = true;
    GameObject* parent = nullptr;
    Transform* transform = nullptr;

    std::vector<std::unique_ptr<GameObject>> children;
    std::vector<std::unique_ptr<Component>> allComponents;
    std::unordered_map<std::type_index, std::vector<Component*>> componentsByType;

    // Инициализация Transform компонента
    void initializeTransform() {
        if (!transform) {
            transform = addComponent<Transform>();
        }
    }

public:
    // ==================== Конструкторы и деструктор ====================
    GameObject(const std::string& name = "GameObject") : name(name) {
        initializeTransform();
    }

    virtual ~GameObject() {
        // Очищаем связи
        for (auto& child : children) {
            child->parent = nullptr;
        }
        children.clear();
        componentsByType.clear();
        allComponents.clear();
    }

    // Запрещаем копирование
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    // Разрешаем перемещение
    GameObject(GameObject&& other) noexcept
        : name(std::move(other.name)),
        active(other.active),
        parent(other.parent),
        transform(other.transform),
        children(std::move(other.children)),
        allComponents(std::move(other.allComponents)),
        componentsByType(std::move(other.componentsByType)) {

        other.parent = nullptr;
        other.transform = nullptr;

        // Обновляем ссылки на родителя у детей
        for (auto& child : children) {
            child->parent = this;
        }
    }

    GameObject& operator=(GameObject&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            active = other.active;
            parent = other.parent;
            transform = other.transform;
            children = std::move(other.children);
            allComponents = std::move(other.allComponents);
            componentsByType = std::move(other.componentsByType);

            other.parent = nullptr;
            other.transform = nullptr;

            // Обновляем ссылки на родителя у детей
            for (auto& child : children) {
                child->parent = this;
            }
        }
        return *this;
    }

    // ==================== Управление активностью ====================
    void setActive(bool isActive) {
        active = isActive;
        // Пропускаем состояние детям
        for (auto& child : children) {
            child->setActive(isActive);
        }
    }

    bool isActive() const { return active; }

    // ==================== Управление компонентами ====================
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value,
            "T должен наследоваться от Component");

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->setGameObject(this);
        T* ptr = component.get();

        // Добавляем в типизированный список
        std::type_index typeIdx = typeid(T);
        auto& list = componentsByType[typeIdx];
        list.push_back(ptr);

        // Добавляем в общий список
        allComponents.push_back(std::move(component));

        // Вызываем start при добавлении
        if (isActive()) {
            ptr->start();
        }

        return ptr;
    }

    template<typename T>
    T* getComponent() {
        std::type_index typeIdx = typeid(T);
        auto it = componentsByType.find(typeIdx);
        if (it != componentsByType.end() && !it->second.empty()) {
            return static_cast<T*>(it->second.front());
        }
        return nullptr;
    }

    template<typename T>
    std::vector<T*> getComponents() {
        std::vector<T*> result;
        std::type_index typeIdx = typeid(T);
        auto it = componentsByType.find(typeIdx);
        if (it != componentsByType.end()) {
            for (auto* comp : it->second) {
                result.push_back(static_cast<T*>(comp));
            }
        }
        return result;
    }

    template<typename T>
    bool hasComponent() {
        return getComponent<T>() != nullptr;
    }

    template<typename T>
    void removeComponent() {
        std::type_index typeIdx = typeid(T);
        auto it = componentsByType.find(typeIdx);
        if (it != componentsByType.end()) {
            // Удаляем из общего списка
            allComponents.erase(
                std::remove_if(allComponents.begin(), allComponents.end(),
                    [](const std::unique_ptr<Component>& comp) {
                        return dynamic_cast<T*>(comp.get()) != nullptr;
                    }),
                allComponents.end()
            );

            // Удаляем из типизированного списка
            componentsByType.erase(it);
        }
    }

    // ==================== Управление иерархией ====================
    void addChild(std::unique_ptr<GameObject> child) {
        if (!child) return;

        child->parent = this;
        children.push_back(std::move(child));
    }

    GameObject* createChild(const std::string& name = "") {
        auto child = std::make_unique<GameObject>(name);
        GameObject* ptr = child.get();
        addChild(std::move(child));
        return ptr;
    }

    GameObject* getParent() const { return parent; }

    const std::vector<std::unique_ptr<GameObject>>& getChildren() const {
        return children;
    }

    // ==================== Поиск объектов ====================
    GameObject* findByName(const std::string& targetName) {
        if (name == targetName) return this;

        for (auto& child : children) {
            GameObject* found = child->findByName(targetName);
            if (found) return found;
        }
        return nullptr;
    }

    template<typename T>
    GameObject* findWithComponent() {
        if (hasComponent<T>()) return this;

        for (auto& child : children) {
            GameObject* found = child->findWithComponent<T>();
            if (found) return found;
        }
        return nullptr;
    }

    // ==================== Методы жизненного цикла ====================
    void start() {
        if (!active) return;

        // Вызываем start у всех компонентов
        for (auto& component : allComponents) {
            component->start();
        }

        // Рекурсивно вызываем у детей
        for (auto& child : children) {
            child->start();
        }
    }

    void update(float deltaTime) {
        if (!active) return;

        // Вызываем update у всех компонентов
        for (auto& component : allComponents) {
            component->update(deltaTime);
        }

        // Рекурсивно вызываем у детей
        for (auto& child : children) {
            child->update(deltaTime);
        }
    }

    void render() {
        if (!active) return;

        // Вызываем render у всех компонентов
        for (auto& component : allComponents) {
            component->render();
        }

        // Рекурсивно вызываем у детей
        for (auto& child : children) {
            child->render();
        }
    }

    // ==================== Геттеры и сеттеры ====================
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    Transform* getTransform() {
        if (!transform) {
            transform = getComponent<Transform>();
            if (!transform) {
                transform = addComponent<Transform>();
            }
        }
        return transform;
    }

    // ==================== Паттерн Builder ====================
    class Builder {
    private:
        std::unique_ptr<GameObject> gameObject;

    public:
        Builder() : gameObject(std::make_unique<GameObject>()) {}
        explicit Builder(const std::string& name) : gameObject(std::make_unique<GameObject>(name)) {}

        Builder& withTransform(const glm::vec3& pos = glm::vec3(0.0f),
            const glm::vec3& scl = glm::vec3(1.0f),
            const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
            auto transform = gameObject->addComponent<Transform>();
            transform->position = pos;
            transform->scale = scl;
            transform->rotation = rot;
            return *this;
        }

        template<typename T, typename... Args>
        Builder& withComponent(Args&&... args) {
            gameObject->addComponent<T>(std::forward<Args>(args)...);
            return *this;
        }

        std::unique_ptr<GameObject> build() {
            return std::move(gameObject);
        }

        GameObject* buildRaw() {
            return gameObject.release();
        }
    };
};