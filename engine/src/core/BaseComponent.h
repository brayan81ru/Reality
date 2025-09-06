#pragma once
#include <string>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

namespace Reality {
    class BaseGameObject;

    class BaseComponent {
    public:
        BaseComponent() : m_gameObject(nullptr), m_enabled(true), m_started(false) {}
        virtual ~BaseComponent() = default;

        // Lifecycle methods (similar to Unity)
        virtual void Start() { m_started = true; }
        virtual void Update(float deltaTime) {}
        virtual void OnDestroy() {}
        virtual void OnEnable() {}
        virtual void OnDisable() {}

        // GameObject association
        void SetGameObject(BaseGameObject* gameObject) { m_gameObject = gameObject; }
        BaseGameObject* GetGameObject() const { return m_gameObject; }

        // Enable/Disable
        void SetEnabled(bool enabled) {
            if (m_enabled != enabled) {
                m_enabled = enabled;
                if (enabled) OnEnable();
                else OnDisable();
            }
        }
        bool IsEnabled() const { return m_enabled; }

        // Name management
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        // Type information
        virtual const std::type_info& GetType() const = 0;

    protected:
        BaseGameObject* m_gameObject;
        std::string m_name;
        bool m_enabled;
        bool m_started;
    };
}