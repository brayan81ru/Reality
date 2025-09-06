#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <algorithm>
#include "BaseComponent.h"

namespace Reality {
    // Forward declarations
    class TransformComponent;

    class BaseGameObject {
    public:
        BaseGameObject();
        virtual ~BaseGameObject();

        // Lifecycle
        virtual void Start();
        virtual void Update(float deltaTime);
        virtual void OnDestroy();

        // Name management
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        // Component management
        template<typename T>
        T* AddComponent();

        template<typename T>
        T* GetComponent() const;

        template<typename T>
        std::vector<T*> GetComponents() const;

        template<typename T>
        bool HasComponent() const;

        template<typename T>
        void RemoveComponent();

        // Special access to Transform
        TransformComponent* GetTransform() const { return m_transform; }

        // Hierarchy
        void SetParent(BaseGameObject* parent);
        BaseGameObject* GetParent() const { return m_parent; }
        const std::vector<BaseGameObject*>& GetChildren() const { return m_children; }

        // Active state
        void SetActive(bool active);
        bool IsActive() const { return m_active; }
        bool IsActiveInHierarchy() const;

        // Tag and Layer (Unity-like)
        void SetTag(const std::string& tag) { m_tag = tag; }
        const std::string& GetTag() const { return m_tag; }

        void SetLayer(int layer) { m_layer = layer; }
        int GetLayer() const { return m_layer; }

    private:
        std::unordered_map<std::type_index, std::vector<BaseComponent*>> m_components;
        std::vector<BaseGameObject*> m_children;
        BaseGameObject* m_parent;
        TransformComponent* m_transform;
        std::string m_name;
        std::string m_tag;
        int m_layer;
        bool m_active;
        bool m_started;

        // Internal methods
        void AddChild(BaseGameObject* child);
        void RemoveChild(BaseGameObject* child);
        void UpdateChildren(float deltaTime);
    };
}