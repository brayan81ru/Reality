#pragma once
#include "BaseComponent.h"
#include "MathF.h"

namespace Reality {

class TransformComponent:BaseComponent {
public:
    MathF::Vector3f position = MathF::Vector3f(0, 0, 0);
    MathF::Vector3f rotation = MathF::Vector3f(0, 0, 0);
    MathF::Vector3f scale = MathF::Vector3f(1, 1, 1);

    ~TransformComponent() override;

    void Update() override;

    void FixedUpdate() override;

    void LateUpdate() override;

    void Awake() override;

    void Start() override;
};

} // Reality


