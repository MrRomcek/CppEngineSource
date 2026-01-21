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
// ==================== Класс GameObject (Игровой Объект) ====================
// Основной класс для представления любой сущности в игровом мире
// Реализует компонентный архитектурный паттерн (Entity-Component-System)
class GameObject {
private:
    std::string name;                    // Имя объекта для идентификации
    bool active = true;                  // Флаг активности объекта (включен/выключен)
    GameObject* parent = nullptr;        // Указатель на родительский объект в иерархии
    Transform* transform = nullptr;      // Указатель на компонент Transform (обязательный)

    // Коллекция дочерних объектов (владеем ими через unique_ptr)
    std::vector<std::unique_ptr<GameObject>> children;

    // Все компоненты объекта (владеем ими)
    std::vector<std::unique_ptr<Component>> allComponents;

    // Быстрый доступ к компонентам по их типу (type_index -> список компонентов)
    std::unordered_map<std::type_index, std::vector<Component*>> componentsByType;

    // Инициализация Transform компонента (гарантирует наличие Transform у каждого объекта)
    void initializeTransform() {
        if (!transform) {
            transform = addComponent<Transform>(); // Создаем Transform если его нет
        }
    }

public:
    // ==================== Конструкторы и деструктор ====================

    // Основной конструктор
    GameObject(const std::string& name = "GameObject") : name(name) {
        initializeTransform(); // Гарантируем наличие Transform у каждого GameObject
    }

    // Деструктор - очищает все связи и ресурсы
    virtual ~GameObject() {
        // Разрываем связи с дочерними объектами
        for (auto& child : children) {
            child->parent = nullptr;
        }
        children.clear();
        componentsByType.clear();
        allComponents.clear();
    }

    // Запрещаем копирование (из-за уникальных указателей)
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    // Разрешаем перемещение для оптимизации
    GameObject(GameObject&& other) noexcept
        : name(std::move(other.name)),
        active(other.active),
        parent(other.parent),
        transform(other.transform),
        children(std::move(other.children)),
        allComponents(std::move(other.allComponents)),
        componentsByType(std::move(other.componentsByType)) {

        // Обнуляем указатели у исходного объекта
        other.parent = nullptr;
        other.transform = nullptr;

        // Обновляем ссылки на родителя у перемещенных детей
        for (auto& child : children) {
            child->parent = this;
        }
    }

    // Оператор перемещающего присваивания
    GameObject& operator=(GameObject&& other) noexcept {
        if (this != &other) {
            // Перемещаем все данные
            name = std::move(other.name);
            active = other.active;
            parent = other.parent;
            transform = other.transform;
            children = std::move(other.children);
            allComponents = std::move(other.allComponents);
            componentsByType = std::move(other.componentsByType);

            // Обнуляем указатели у исходного объекта
            other.parent = nullptr;
            other.transform = nullptr;

            // Обновляем ссылки на родителя у перемещенных детей
            for (auto& child : children) {
                child->parent = this;
            }
        }
        return *this;
    }

    // ==================== Управление активностью ====================

    // Установка активности объекта (включает/выключает объект и всех его потомков)
    void setActive(bool isActive) {
        active = isActive;
        // Рекурсивно применяем состояние активности ко всем детям
        for (auto& child : children) {
            child->setActive(isActive);
        }
    }

    // Проверка активности объекта
    bool isActive() const { return active; }

    // ==================== Управление компонентами ====================

    // Шаблонный метод для добавления компонента любого типа
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        // Проверка, что T наследуется от Component
        static_assert(std::is_base_of<Component, T>::value,
            "T должен наследоваться от Component");

        // Создаем компонент с переданными аргументами
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->setGameObject(this); // Устанавливаем ссылку на GameObject
        T* ptr = component.get(); // Сохраняем сырой указатель

        // Добавляем в типизированный список для быстрого доступа
        std::type_index typeIdx = typeid(T);
        auto& list = componentsByType[typeIdx];
        list.push_back(ptr);

        // Добавляем в общий список владения
        allComponents.push_back(std::move(component));

        // Вызываем метод start, если объект активен
        if (isActive()) {
            ptr->start();
        }

        return ptr; // Возвращаем указатель на созданный компонент
    }

    // Получение первого компонента указанного типа
    template<typename T>
    T* getComponent() {
        std::type_index typeIdx = typeid(T);
        auto it = componentsByType.find(typeIdx);
        if (it != componentsByType.end() && !it->second.empty()) {
            return static_cast<T*>(it->second.front()); // Безопасное приведение
        }
        return nullptr; // Компонент не найден
    }

    // Получение всех компонентов указанного типа
    template<typename T>
    std::vector<T*> getComponents() {
        std::vector<T*> result;
        std::type_index typeIdx = typeid(T);
        auto it = componentsByType.find(typeIdx);
        if (it != componentsByType.end()) {
            for (auto* comp : it->second) {
                result.push_back(static_cast<T*>(comp)); // Добавляем все компоненты
            }
        }
        return result;
    }

    // Проверка наличия компонента указанного типа
    template<typename T>
    bool hasComponent() {
        return getComponent<T>() != nullptr;
    }

    // Удаление всех компонентов указанного типа
    template<typename T>
    void removeComponent() {
        std::type_index typeIdx = typeid(T);
        auto it = componentsByType.find(typeIdx);
        if (it != componentsByType.end()) {
            // Удаляем из общего списка владения
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

    // ==================== Управление иерархией объектов ====================

    // Добавление существующего дочернего объекта
    void addChild(std::unique_ptr<GameObject> child) {
        if (!child) return; // Проверка на null

        child->parent = this; // Устанавливаем себя как родителя
        children.push_back(std::move(child)); // Перемещаем во владение
    }

    // Создание нового дочернего объекта
    GameObject* createChild(const std::string& name = "") {
        auto child = std::make_unique<GameObject>(name); // Создаем объект
        GameObject* ptr = child.get(); // Сохраняем указатель
        addChild(std::move(child)); // Добавляем как дочерний
        return ptr; // Возвращаем указатель
    }

    // Получение родительского объекта
    GameObject* getParent() const { return parent; }

    // Получение списка дочерних объектов (только для чтения)
    const std::vector<std::unique_ptr<GameObject>>& getChildren() const {
        return children;
    }

    // ==================== Поиск объектов в иерархии ====================

    // Поиск объекта по имени (рекурсивный)
    GameObject* findByName(const std::string& targetName) {
        if (name == targetName) return this; // Проверяем себя

        // Рекурсивно проверяем всех детей
        for (auto& child : children) {
            GameObject* found = child->findByName(targetName);
            if (found) return found; // Нашли в поддереве
        }
        return nullptr; // Не нашли
    }

    // Поиск объекта с определенным компонентом (рекурсивный)
    template<typename T>
    GameObject* findWithComponent() {
        if (hasComponent<T>()) return this; // Проверяем себя

        // Рекурсивно проверяем всех детей
        for (auto& child : children) {
            GameObject* found = child->findWithComponent<T>();
            if (found) return found; // Нашли в поддереве
        }
        return nullptr; // Не нашли
    }

    // ==================== Методы жизненного цикла ====================

    // Инициализация объекта (вызывается один раз при создании)
    void start() {
        if (!active) return; // Пропускаем если объект неактивен

        // Вызываем start у всех компонентов
        for (auto& component : allComponents) {
            component->start();
        }

        // Рекурсивно вызываем у всех детей
        for (auto& child : children) {
            child->start();
        }
    }

    // Обновление объекта (вызывается каждый кадр)
    void update(float deltaTime) {
        if (!active) return; // Пропускаем если объект неактивен

        // Вызываем update у всех компонентов
        for (auto& component : allComponents) {
            component->update(deltaTime);
        }

        // Рекурсивно вызываем у всех детей
        for (auto& child : children) {
            child->update(deltaTime);
        }
    }

    // Отрисовка объекта (вызывается каждый кадр)
    void render() {
        if (!active) return; // Пропускаем если объект неактивен

        // Вызываем render у всех компонентов
        for (auto& component : allComponents) {
            component->render();
        }

        // Рекурсивно вызываем у всех детей
        for (auto& child : children) {
            child->render();
        }
    }

    // ==================== Геттеры и сеттеры ====================

    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    // Получение компонента Transform (гарантирует его наличие)
    Transform* getTransform() {
        if (!transform) {
            transform = getComponent<Transform>(); // Пытаемся найти существующий
            if (!transform) {
                transform = addComponent<Transform>(); // Создаем если нет
            }
        }
        return transform;
    }

    // ==================== Паттерн Builder (Строитель) ====================
    // Позволяет удобно создавать и настраивать объекты
    class Builder {
    private:
        std::unique_ptr<GameObject> gameObject; // Создаваемый объект

    public:
        // Конструкторы Builder'а
        Builder() : gameObject(std::make_unique<GameObject>()) {}
        explicit Builder(const std::string& name) : gameObject(std::make_unique<GameObject>(name)) {}

        // Метод для настройки Transform компонента
        Builder& withTransform(const glm::vec3& pos = glm::vec3(0.0f),
            const glm::vec3& scl = glm::vec3(1.0f),
            const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) {
            auto transform = gameObject->addComponent<Transform>();
            transform->position = pos;
            transform->scale = scl;
            transform->rotation = rot;
            return *this; // Возвращаем this для цепочки вызовов
        }

        // Метод для добавления любого компонента
        template<typename T, typename... Args>
        Builder& withComponent(Args&&... args) {
            gameObject->addComponent<T>(std::forward<Args>(args)...);
            return *this; // Возвращаем this для цепочки вызовов
        }

        // Финальный метод сборки (возвращает unique_ptr)
        std::unique_ptr<GameObject> build() {
            return std::move(gameObject); // Передаем владение
        }

        // Финальный метод сборки (возвращает сырой указатель)
        GameObject* buildRaw() {
            return gameObject.release(); // Освобождаем владение
        }
    };
};