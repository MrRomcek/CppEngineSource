// ============================================================================
// PhysicsEngine.cpp - Fizicheskij dvizhok
// ============================================================================

#include "PhysicsEngine.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "Transform.h"
#include "GameObject.h"
#include "Logger.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

// Inicializaciya fizicheskogo dvizhka
void PhysicsEngine::initialize() {
    auto logger = LogManager::getInstance().createConsoleLogger("PhysicsEngine");
    LOG_INFO("Physics Engine initialized");
}

// Glavnyj cikl obnovleniya fiziki
void PhysicsEngine::update(float deltaTime) {
    if (deltaTime <= 0.0f) return;

    // Fiksirovannyj shag vremeni dlya stabilnoj simulyacii
    float fixedDeltaTime = glm::min(deltaTime, 0.016f);

    // Primenyaem gravitaciyu
    applyGravity(fixedDeltaTime);

    // Obnovlyaem pozicii na osnove skorosti
    for (auto rigidbody : rigidbodies) {
        if (rigidbody && rigidbody->isEnabled() && rigidbody->getType() != Rigidbody::Type::Static) {
            auto transform = rigidbody->getGameObject()->getTransform();
            if (transform) {
                transform->position += rigidbody->velocity * fixedDeltaTime;
            }
        }
    }

    // Razreshaem stolknoveniya posle dvizheniya
    resolveCollisions(fixedDeltaTime);

    // Primenyaem soprotivlenie vozduha
    for (auto rigidbody : rigidbodies) {
        if (rigidbody && rigidbody->isEnabled() && rigidbody->getType() != Rigidbody::Type::Static) {
            float dragFactor = 1.0f - (rigidbody->getDrag() * fixedDeltaTime);
            dragFactor = glm::clamp(dragFactor, 0.0f, 1.0f);
            rigidbody->velocity *= dragFactor;

            // Obnulyaem ochen malenkuyu skorost
            if (glm::length(rigidbody->velocity) < 0.001f) {
                rigidbody->velocity = glm::vec3(0.0f);
            }
        }
    }
}

// Primenenie gravitacii k dinamicheskim objektam
void PhysicsEngine::applyGravity(float deltaTime) {
    for (auto rigidbody : rigidbodies) {
        if (rigidbody && rigidbody->isEnabled() &&
            rigidbody->getType() == Rigidbody::Type::Dynamic &&
            rigidbody->getUseGravity() && !rigidbody->isGrounded()) {
            rigidbody->velocity += gravity * deltaTime;
        }
    }
}

// Struktura ogranichivayushchego pryamougolnika (AABB)
struct AABB {
    glm::vec3 min;  // Minimalnaya tochka
    glm::vec3 max;  // Maksimalnaya tochka
};

// Poluchenie AABB dlya objekta s uchetom ego masshtaba
static AABB getAABB(Transform* transform, glm::vec3 size = glm::vec3(1.0f)) {
    AABB aabb;
    glm::vec3 halfSize = size * transform->scale * 0.5f;
    aabb.min = transform->position - halfSize;
    aabb.max = transform->position + halfSize;
    return aabb;
}

// Proverka peresecheniya dvuh AABB i raschet glubiny proniknoveniya
static bool checkAABBCollision(const AABB& a, const AABB& b, glm::vec3& penetration) {
    // Proveryaem perekrytie po vsem osyam
    glm::vec3 overlapMin = glm::max(a.min, b.min);
    glm::vec3 overlapMax = glm::min(a.max, b.max);

    if (overlapMin.x < overlapMax.x && overlapMin.y < overlapMax.y && overlapMin.z < overlapMax.z) {
        // Rasschityvaem glubinu proniknoveniya po kazhdoj osi
        glm::vec3 aCenter = (a.min + a.max) * 0.5f;
        glm::vec3 bCenter = (b.min + b.max) * 0.5f;
        glm::vec3 aHalf = (a.max - a.min) * 0.5f;
        glm::vec3 bHalf = (b.max - b.min) * 0.5f;

        glm::vec3 delta = bCenter - aCenter;
        glm::vec3 overlap = aHalf + bHalf - glm::abs(delta);

        // Nahodim os s minimalnym proniknoveniem
        if (overlap.x < overlap.y && overlap.x < overlap.z) {
            penetration = glm::vec3(delta.x > 0 ? overlap.x : -overlap.x, 0, 0);
        }
        else if (overlap.y < overlap.z) {
            penetration = glm::vec3(0, delta.y > 0 ? overlap.y : -overlap.y, 0);
        }
        else {
            penetration = glm::vec3(0, 0, delta.z > 0 ? overlap.z : -overlap.z);
        }
        return true;
    }
    return false;
}

// Razreshenie stolknovenij mezhdu objektami
void PhysicsEngine::resolveCollisions(float deltaTime) {
    for (size_t i = 0; i < rigidbodies.size(); ++i) {
        for (size_t j = i + 1; j < rigidbodies.size(); ++j) {
            auto rb1 = rigidbodies[i];
            auto rb2 = rigidbodies[j];

            if (!rb1 || !rb2 || !rb1->isEnabled() || !rb2->isEnabled()) continue;

            // Propuskaem esli oba staticheskie
            if (rb1->getType() == Rigidbody::Type::Static && 
                rb2->getType() == Rigidbody::Type::Static) continue;

            auto transform1 = rb1->getGameObject()->getTransform();
            auto transform2 = rb2->getGameObject()->getTransform();

            AABB aabb1 = getAABB(transform1);
            AABB aabb2 = getAABB(transform2);

            glm::vec3 penetration;
            if (checkAABBCollision(aabb1, aabb2, penetration)) {
                bool rb1Dynamic = rb1->getType() == Rigidbody::Type::Dynamic;
                bool rb2Dynamic = rb2->getType() == Rigidbody::Type::Dynamic;

                // Rasschityvaem kombinirovannoe trenie
                float combinedFriction = (rb1->getFriction() + rb2->getFriction()) * 0.5f;

                if (rb1Dynamic && rb2Dynamic) {
                    // Oba dinamicheskie - razdelyaem popolam
                    transform1->position -= penetration * 0.5f;
                    transform2->position += penetration * 0.5f;

                    glm::vec3 normal = glm::normalize(penetration);
                    float v1n = glm::dot(rb1->velocity, normal);
                    float v2n = glm::dot(rb2->velocity, normal);

                    // Otrazhaem skorosti
                    if (v1n > 0) rb1->velocity -= normal * v1n;
                    if (v2n < 0) rb2->velocity -= normal * v2n;
                }
                else if (rb1Dynamic) {
                    // Tolko rb1 dinamicheskij
                    transform1->position -= penetration;

                    glm::vec3 normal = glm::normalize(penetration);
                    float vn = glm::dot(rb1->velocity, normal);

                    // Proveryaem stoit li na poverhnosti (stolknovenie sverhu)
                    bool isOnTop = penetration.y < 0 && 
                                   glm::abs(penetration.y) > glm::abs(penetration.x) && 
                                   glm::abs(penetration.y) > glm::abs(penetration.z);

                    if (vn > 0) {
                        // Ostanavlivaem dvizhenie v napravlenii stolknoveniya
                        rb1->velocity -= normal * vn;
                    }

                    if (isOnTop) {
                        rb1->setGrounded(true);
                        rb1->velocity.y = 0;  // Polnostyu ostanavlivaem vertikalnoe dvizhenie
                        // Primenyaem trenie k gorizontalnomu dvizheniyu
                        rb1->velocity.x *= (1.0f - combinedFriction * 0.1f);
                        rb1->velocity.z *= (1.0f - combinedFriction * 0.1f);
                    }
                }
                else if (rb2Dynamic) {
                    // Tolko rb2 dinamicheskij
                    transform2->position += penetration;

                    glm::vec3 normal = glm::normalize(penetration);
                    float vn = glm::dot(rb2->velocity, normal);

                    // Proveryaem stoit li na poverhnosti
                    bool isOnTop = penetration.y > 0 && 
                                   glm::abs(penetration.y) > glm::abs(penetration.x) && 
                                   glm::abs(penetration.y) > glm::abs(penetration.z);

                    if (vn < 0) {
                        // Ostanavlivaem dvizhenie v napravlenii stolknoveniya
                        rb2->velocity += normal * glm::abs(vn);
                    }

                    if (isOnTop) {
                        rb2->setGrounded(true);
                        rb2->velocity.y = 0;  // Polnostyu ostanavlivaem vertikalnoe dvizhenie
                        // Primenyaem trenie k gorizontalnomu dvizheniyu
                        rb2->velocity.x *= (1.0f - combinedFriction * 0.1f);
                        rb2->velocity.z *= (1.0f - combinedFriction * 0.1f);
                    }
                }
            }
        }
    }
}

// Registraciya Rigidbody v fizicheskom dvizhke
void PhysicsEngine::registerRigidbody(Rigidbody* rigidbody) {
    if (!rigidbody) return;
    auto it = std::find(rigidbodies.begin(), rigidbodies.end(), rigidbody);
    if (it == rigidbodies.end()) {
        rigidbodies.push_back(rigidbody);
    }
}

// Udalenie Rigidbody iz fizicheskogo dvizhka
void PhysicsEngine::unregisterRigidbody(Rigidbody* rigidbody) {
    if (!rigidbody) return;
    auto it = std::find(rigidbodies.begin(), rigidbodies.end(), rigidbody);
    if (it != rigidbodies.end()) {
        rigidbodies.erase(it);
    }
}
