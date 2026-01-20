#pragma once
#include <string>
#include <memory>

class GameObject;

class Component {
public:
    virtual ~Component() = default;

    virtual void start() {}
    virtual void update(float deltaTime) {}
    virtual void render() {}
    virtual void onDestroy() {}

    // Serialization
    virtual std::string getTypeName() const = 0;
    virtual void serialize(std::ostream& os) const {}
    virtual void deserialize(std::istream& is) {}

    // Getters
    GameObject* getGameObject() const { return gameObject; }
    void setGameObject(GameObject* obj) { gameObject = obj; }

    bool isEnabled() const { return enabled; }
    void setEnabled(bool enable) { enabled = enable; }

protected:
    GameObject* gameObject = nullptr;
    bool enabled = true;
};

// Макрос для регистрации компонентов
#define REGISTER_COMPONENT(TYPE) \
    static std::string getStaticTypeName() { return #TYPE; } \
    virtual std::string getTypeName() const override { return #TYPE; }