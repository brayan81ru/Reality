#include "BaseGameObject.h"
#include "TransformComponent.h"
#include <algorithm>

#include "DiligentTools/ThirdParty/args/args.hxx"

namespace Reality {
    BaseGameObject::BaseGameObject()
        : m_parent(nullptr), m_transform(nullptr), m_name("GameObject"),
          m_tag("Untagged"), m_layer(0), m_active(true), m_started(false) {
        // Every GameObject has a TransformComponent
        m_transform = AddComponent<TransformComponent>();
    }

    BaseGameObject::~BaseGameObject() {
        OnDestroy();
    }

    void BaseGameObject::Start() {
        if (!m_started) {
            m_started = true;

            // Start all components
            for (auto& pair : m_components) {
                for (BaseComponent* component : pair.second) {
                    if (component->IsEnabled()) {
                        component->Start();
                    }
                }
            }

            // Start children recursively
            for (BaseGameObject* child : m_children) {
                child->Start();
            }
        }
    }

    void BaseGameObject::Update(const float deltaTime) {
        if (!m_active) return;

        // Update all components
        for (auto& pair : m_components) {
            for (BaseComponent* component : pair.second) {
                if (component->IsEnabled()) {
                    component->Update(deltaTime);
                }
            }
        }

        // Update children recursively
        UpdateChildren(deltaTime);
    }

    void BaseGameObject::OnDestroy() {
        // Destroy all components
        for (auto& pair : m_components) {
            for (BaseComponent* component : pair.second) {
                component->OnDestroy();
                delete component;
            }
        }
        m_components.clear();

        // Remove from parent
        if (m_parent) {
            m_parent->RemoveChild(this);
        }

        // Destroy children recursively
        for (BaseGameObject* child : m_children) {
            child->OnDestroy();
            delete child;
        }
        m_children.clear();
    }

    void BaseGameObject::SetActive(const bool active) {
        if (m_active != active) {
            m_active = active;

            if (active) {
                // Re-enable components
                for (auto& pair : m_components) {
                    for (BaseComponent* component : pair.second) {
                        if (component->IsEnabled()) {
                            component->OnEnable();
                        }
                    }
                }
            } else {
                // Disable components
                for (auto& pair : m_components) {
                    for (BaseComponent* component : pair.second) {
                        if (component->IsEnabled()) {
                            component->OnDisable();
                        }
                    }
                }
            }
        }
    }

    bool BaseGameObject::IsActiveInHierarchy() const {
        if (!m_active) return false;
        if (m_parent) return m_parent->IsActiveInHierarchy();
        return true;
    }

    std::vector<BaseComponent*> BaseGameObject::GetComponents() {
        std::vector<BaseComponent*> components;
        // Iterate through all component types in the map
        for (auto& pair : m_components) {
            // pair is a std::pair<std::type_index, std::vector<BaseComponent*>>
            // pair.second is the std::vector<BaseComponent*>
            for (BaseComponent* component : pair.second) {
                components.push_back(component);
            }
        }
        return components;
    }

    void BaseGameObject::SetParent(BaseGameObject* parent) {
        if (m_parent == parent) return;

        // Remove from current parent
        if (m_parent) {
            m_parent->RemoveChild(this);
        }

        // Set new parent
        m_parent = parent;

        // Add to new parent
        if (m_parent) {
            m_parent->AddChild(this);
        }
    }

    void BaseGameObject::AddChild(BaseGameObject* child) {
        if (child && std::find(m_children.begin(), m_children.end(), child) == m_children.end()) {
            m_children.push_back(child);
            // If this game object has started, start the child
            if (m_started) {
                child->Start();
            }
        }
    }

    void BaseGameObject::RemoveChild(BaseGameObject* child) {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end()) {
            m_children.erase(it);
        }
    }

    void BaseGameObject::UpdateChildren(const float deltaTime) {
        for (BaseGameObject* child : m_children) {
            child->Update(deltaTime);
        }
    }

    // Template implementations remain the same...
    template<typename T>
    T* BaseGameObject::AddComponent() {
        static_assert(std::is_base_of_v<BaseComponent, T>,
                     "T must inherit from BaseComponent");

        // Special handling for TransformComponent
        if (std::is_same_v<T, TransformComponent> && m_transform != nullptr) {
            return m_transform;
        }

        // Create component
        T* component = new T();
        component->SetGameObject(this);

        // Add to component map
        std::type_index type = typeid(T);
        m_components[type].push_back(component);

        // If it's a TransformComponent, store the reference
        if (std::is_same_v<T, TransformComponent>) {
            m_transform = static_cast<TransformComponent*>(component);
        }

        // Start the component if the game object has started
        if (m_started) {
            component->Start();
        }

        return component;
    }

    template<typename T>
    T* BaseGameObject::GetComponent() const {
        static_assert(std::is_base_of_v<BaseComponent, T>,
                     "T must inherit from BaseComponent");

        // Special handling for TransformComponent
        if (std::is_same_v<T, TransformComponent>) {
            return m_transform;
        }

        std::type_index type = typeid(T);
        auto it = m_components.find(type);
        if (it != m_components.end()) {
            for (BaseComponent* component : it->second) {
                T* casted = dynamic_cast<T*>(component);
                if (casted) {
                    return casted;
                }
            }
        }
        return nullptr;
    }

    template<typename T>
    std::vector<T*> BaseGameObject::GetComponents() const {
        static_assert(std::is_base_of_v<BaseComponent, T>,
                     "T must inherit from BaseComponent");

        std::vector<T*> result;
        std::type_index type = typeid(T);
        auto it = m_components.find(type);
        if (it != m_components.end()) {
            for (BaseComponent* component : it->second) {
                T* casted = dynamic_cast<T*>(component);
                if (casted) {
                    result.push_back(casted);
                }
            }
        }
        return result;
    }

    template<typename T>
    bool BaseGameObject::HasComponent() const {
        static_assert(std::is_base_of_v<BaseComponent, T>,
                     "T must inherit from BaseComponent");

        // Special handling for TransformComponent
        if (std::is_same_v<T, TransformComponent>) {
            return m_transform != nullptr;
        }

        std::type_index type = typeid(T);
        auto it = m_components.find(type);
        return it != m_components.end() && !it->second.empty();
    }

    template<typename T>
    void BaseGameObject::RemoveComponent() {
        static_assert(std::is_base_of_v<BaseComponent, T>,
                     "T must inherit from BaseComponent");

        // Cannot remove TransformComponent
        if (std::is_same_v<T, TransformComponent>) {
            return;
        }

        std::type_index type = typeid(T);
        auto it = m_components.find(type);
        if (it != m_components.end()) {
            for (auto componentIt = it->second.begin(); componentIt != it->second.end(); ) {
                T* casted = dynamic_cast<T*>(*componentIt);
                if (casted) {
                    casted->OnDestroy();
                    delete casted;
                    componentIt = it->second.erase(componentIt);
                } else {
                    ++componentIt;
                }
            }
        }
    }

    // Explicit template instantiations for common types
    template TransformComponent* BaseGameObject::AddComponent<TransformComponent>();
    template TransformComponent* BaseGameObject::GetComponent<TransformComponent>() const;
    template std::vector<TransformComponent*> BaseGameObject::GetComponents<TransformComponent>() const;
    template bool BaseGameObject::HasComponent<TransformComponent>() const;
    template void BaseGameObject::RemoveComponent<TransformComponent>();
}
