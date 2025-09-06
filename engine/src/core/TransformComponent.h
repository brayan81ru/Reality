#pragma once
#include "BaseComponent.h"
#include "MathF.h"

namespace Reality {
    class BaseGameObject;

    class TransformComponent : public BaseComponent {
    public:
        TransformComponent();
        ~TransformComponent() override;

        // Override base methods
        void Start() override;

        void Update(float deltaTime) override;

        const std::type_info& GetType() const override { return typeid(TransformComponent); }

        // Transform properties
        void SetPosition(const MathF::Vector3f& position);
        void SetRotation(const MathF::Quaternion& rotation);
        void SetScale(const MathF::Vector3f& scale);

        void SetLocalPosition(const MathF::Vector3f& position);
        void SetLocalRotation(const MathF::Quaternion& rotation);
        void SetLocalScale(const MathF::Vector3f& scale);

        MathF::Vector3f GetPosition() const;
        MathF::Quaternion GetRotation() const;
        MathF::Vector3f GetScale() const;

        MathF::Vector3f GetLocalPosition() const { return m_localPosition; }
        MathF::Quaternion GetLocalRotation() const { return m_localRotation; }
        MathF::Vector3f GetLocalScale() const { return m_localScale; }

        // Matrix access
        MathF::Matrix4x4 GetLocalToWorldMatrix() const;
        MathF::Matrix4x4 GetWorldToLocalMatrix() const;

        // Hierarchy
        void SetParent(TransformComponent* parent);
        TransformComponent* GetParent() const { return m_parent; }
        const std::vector<TransformComponent*>& GetChildren() const { return m_children; }

        // Direction vectors
        MathF::Vector3f GetForward() const;
        MathF::Vector3f GetRight() const;
        MathF::Vector3f GetUp() const;

        // Transform operations
        MathF::Vector3f TransformPoint(const MathF::Vector3f& point) const;
        MathF::Vector3f InverseTransformPoint(const MathF::Vector3f& point) const;
        MathF::Vector3f TransformDirection(const MathF::Vector3f& direction) const;
        MathF::Vector3f InverseTransformDirection(const MathF::Vector3f& direction) const;

    private:
        // Transform data
        MathF::Vector3f m_localPosition;
        MathF::Quaternion m_localRotation;
        MathF::Vector3f m_localScale;
        mutable MathF::Vector3f m_scale; // World scale

        // Cache
        mutable MathF::Matrix4x4 m_localToWorldMatrix;
        mutable MathF::Vector3f m_position;
        mutable MathF::Quaternion m_rotation;
        mutable bool m_transformDirty;

        // Hierarchy
        TransformComponent* m_parent;
        std::vector<TransformComponent*> m_children;

        // Internal methods
        void AddChild(TransformComponent* child);
        void RemoveChild(TransformComponent* child);
        void UpdateTransform() const;
        void MarkTransformDirty() const;
    };

    inline TransformComponent::~TransformComponent() = default;
}
