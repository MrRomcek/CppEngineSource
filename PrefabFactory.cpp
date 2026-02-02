// ============================================================================
// PrefabFactory.cpp - Fabrika prefabov (gotovyh objektov)
// ============================================================================

#include "PrefabFactory.h"
#include "GameObject.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "CharacterController.h"
#include "MeshRenderer.h"
#include "Collider.h"
#include "Logger.h"

// Sozdanie personazha s fizikoj i upravleniem
std::unique_ptr<GameObject> PrefabFactory::createCharacter(const std::string& name) {
    auto logger = LogManager::getInstance().createConsoleLogger("PrefabFactory");

    // Sozdaem bazovyj GameObject
    auto character = std::make_unique<GameObject>(name);

    // Ustanavlivaem poziciyu i masshtab
    auto transform = character->getTransform();
    transform->position = glm::vec3(0.0f, 2.0f, 0.0f);
    transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Dobavlyaem Rigidbody
    auto rigidbody = character->addComponent<Rigidbody>();
    rigidbody->setType(Rigidbody::Type::Dynamic);
    rigidbody->setMass(1.0f);
    rigidbody->setUseGravity(true);
    rigidbody->setDrag(0.1f);
    rigidbody->setFriction(0.8f);
    rigidbody->setBounciness(0.0f);
    rigidbody->setFreezeRotation(true);

    // Dobavlyaem BoxCollider dlya personazha
    auto collider = character->addComponent<BoxCollider>();
    collider->setSize(glm::vec3(1.0f, 1.0f, 1.0f));

    // Dobavlyaem CharacterController dlya upravleniya
    auto controller = character->addComponent<CharacterController>();
    controller->setMoveSpeed(5.0f);
    controller->setJumpForce(8.0f);

    // Dobavlyaem MeshRenderer dlya otobrazheniya
    auto renderer = character->addComponent<MeshRenderer>();
    renderer->setMesh(Mesh::createCube());

    LOG_INFO("Created character: %s", name.c_str());
    return character;
}

// Sozdanie staticheskogo objekta (ne dvigaetsya)
std::unique_ptr<GameObject> PrefabFactory::createStaticObject(const std::string& name) {
    auto obj = std::make_unique<GameObject>(name);

    auto transform = obj->getTransform();
    transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
    transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Dobavlyaem staticheskij Rigidbody
    auto rigidbody = obj->addComponent<Rigidbody>();
    rigidbody->setType(Rigidbody::Type::Static);
    rigidbody->setFriction(0.6f);
    rigidbody->setBounciness(0.0f);

    // Dobavlyaem kollajder
    auto collider = obj->addComponent<BoxCollider>();
    collider->setSize(glm::vec3(1.0f, 1.0f, 1.0f));

    // Dobavlyaem renderer
    auto renderer = obj->addComponent<MeshRenderer>();
    renderer->setMesh(Mesh::createCube());

    return obj;
}

// Sozdanie dinamicheskogo objekta (padaet pod gravitaciej)
std::unique_ptr<GameObject> PrefabFactory::createDynamicObject(const std::string& name) {
    auto obj = std::make_unique<GameObject>(name);

    auto transform = obj->getTransform();
    transform->position = glm::vec3(0.0f, 1.0f, 0.0f);
    transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Dobavlyaem dinamicheskij Rigidbody
    auto rigidbody = obj->addComponent<Rigidbody>();
    rigidbody->setType(Rigidbody::Type::Dynamic);
    rigidbody->setMass(1.0f);
    rigidbody->setUseGravity(true);
    rigidbody->setDrag(0.05f);
    rigidbody->setFriction(0.5f);
    rigidbody->setBounciness(0.0f);  // Bez otskokov

    // Dobavlyaem kollajder
    auto collider = obj->addComponent<BoxCollider>();
    collider->setSize(glm::vec3(1.0f, 1.0f, 1.0f));

    // Dobavlyaem renderer
    auto renderer = obj->addComponent<MeshRenderer>();
    renderer->setMesh(Mesh::createCube());

    return obj;
}

// Sozdanie pola (bolshaya staticheskaya ploskost)
std::unique_ptr<GameObject> PrefabFactory::createFloor(const std::string& name) {
    auto floor = std::make_unique<GameObject>(name);

    auto transform = floor->getTransform();
    transform->position = glm::vec3(0.0f, -2.0f, 0.0f);
    transform->scale = glm::vec3(20.0f, 1.0f, 20.0f);

    // Dobavlyaem staticheskij Rigidbody
    auto rigidbody = floor->addComponent<Rigidbody>();
    rigidbody->setType(Rigidbody::Type::Static);
    rigidbody->setFriction(0.7f);
    rigidbody->setBounciness(0.0f);

    // Dobavlyaem bolshoj BoxCollider
    auto collider = floor->addComponent<BoxCollider>();
    collider->setSize(glm::vec3(1.0f, 1.0f, 1.0f));

    // Dobavlyaem renderer
    auto renderer = floor->addComponent<MeshRenderer>();
    renderer->setMesh(Mesh::createCube());

    return floor;
}
