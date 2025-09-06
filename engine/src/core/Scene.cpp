// Scene.cpp
#include "Scene.h"
#include "BaseComponent.h"
#include <algorithm>

namespace Reality {
    Scene::Scene() = default;

    Scene::~Scene() = default;

    void Scene::Initialize() {
        // Process any pending game objects
        ProcessPendingGameObjects();

        // Call Start on all components
        for (const auto& gameObject : m_gameObjects) {
            for (const auto& component : gameObject->GetComponents()) {
                component->Start();
            }
        }

        m_initialized = true;
    }

    void Scene::Update(float deltaTime) {
        // Process any pending game objects
        ProcessPendingGameObjects();

        // Process destroyed game objects
        ProcessDestroyedGameObjects();

        // Update all components
        for (const auto& gameObject : m_gameObjects) {
            for (const auto& component : gameObject->GetComponents()) {
                component->Update(deltaTime);
            }
        }
    }

    void Scene::Shutdown() {
        // Clear all game objects
        m_gameObjects.clear();
        m_pendingGameObjects.clear();
        m_destroyedGameObjects.clear();

        m_initialized = false;
    }

    void Scene::DestroyGameObject(BaseGameObject* gameObject) {
        if (!gameObject) return;

        // Check if it's already marked for destruction
        const auto it = std::find(m_destroyedGameObjects.begin(),
                           m_destroyedGameObjects.end(),
                           gameObject);

        if (it == m_destroyedGameObjects.end()) {
            m_destroyedGameObjects.push_back(gameObject);
        }
    }

    BaseGameObject* Scene::FindGameObject(const std::string& name) {
        for (const auto& gameObject : m_gameObjects) {
            if (gameObject->GetName() == name) {
                return gameObject.get();
            }
        }
        return nullptr;
    }

    void Scene::ProcessPendingGameObjects() {
        if (m_pendingGameObjects.empty()) return;

        // Move all pending game objects to the main list
        for (auto& gameObject : m_pendingGameObjects) {
            // If the scene is already initialized, call Start on components
            if (m_initialized) {
                for (const auto& component : gameObject->GetComponents()) {
                    component->Start();
                }
            }
            m_gameObjects.push_back(std::move(gameObject));
        }

        m_pendingGameObjects.clear();
    }

    void Scene::ProcessDestroyedGameObjects() {
        if (m_destroyedGameObjects.empty()) return;

        // Remove all destroyed game objects
        for (auto gameObject : m_destroyedGameObjects) {
            auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
                [gameObject](const std::unique_ptr<BaseGameObject>& obj) {
                    return obj.get() == gameObject;
                });

            if (it != m_gameObjects.end()) {
                m_gameObjects.erase(it);
            }
        }

        m_destroyedGameObjects.clear();
    }
}