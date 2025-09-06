// Scene.h
#pragma once
#include <vector>
#include <memory>
#include <string>
#include "BaseGameObject.h"

namespace Reality {
    class Scene {
    public:
        Scene();
        ~Scene();

        // Scene lifecycle
        void Initialize();
        void Update(float deltaTime);
        void Shutdown();

        // GameObject management
        template<typename T, typename... Args>
        T* CreateGameObject(Args&&... args) {
            static_assert(std::is_base_of<BaseGameObject, T>::value,
                         "T must inherit from BaseGameObject");

            auto gameObject = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = gameObject.get();
            m_pendingGameObjects.push_back(std::move(gameObject));
            return ptr;
        }

        void DestroyGameObject(BaseGameObject* gameObject);
        BaseGameObject* FindGameObject(const std::string& name);

        // Component management - implementation moved to header
        template<typename T>
        std::vector<T*> FindComponentsOfType() {
            std::vector<T*> components;
            for (auto& gameObject : m_gameObjects) {
                T* component = gameObject->GetComponent<T>();
                if (component) {
                    components.push_back(component);
                }
            }
            return components;
        }

        // Scene state
        bool IsInitialized() const { return m_initialized; }

    private:
        void ProcessPendingGameObjects();
        void ProcessDestroyedGameObjects();

        std::vector<std::unique_ptr<BaseGameObject>> m_gameObjects;
        std::vector<std::unique_ptr<BaseGameObject>> m_pendingGameObjects;
        std::vector<BaseGameObject*> m_destroyedGameObjects;

        bool m_initialized = false;
    };
}