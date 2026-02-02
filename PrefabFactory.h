#pragma once
#include <memory>
#include <string>

class GameObject;

// ==================== Prefab Factory ====================
// Фабрика для создания предустановленных игровых объектов
class PrefabFactory {
public:
    // Создать персонажа с полным набором компонентов
    static std::unique_ptr<GameObject> createCharacter(const std::string& name = "Character");

    // Создать статический объект для столкновения
    static std::unique_ptr<GameObject> createStaticObject(const std::string& name = "StaticObject");

    // Создать динамический объект
    static std::unique_ptr<GameObject> createDynamicObject(const std::string& name = "DynamicObject");

    // Создать пол
    static std::unique_ptr<GameObject> createFloor(const std::string& name = "Floor");

private:
    PrefabFactory() = default;
};
