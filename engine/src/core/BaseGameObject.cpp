//
// Created by Brayan on 06/09/2025.
//

#include "BaseGameObject.h"

#include "TransformComponent.h"

Reality::BaseGameObject::BaseGameObject() {
    auto transform = new TransformComponent();
    AddComponent(transform);
}

void Reality::BaseGameObject::AddComponent(const BaseComponent *_component) {
    m_components.push_back(*_component);
}
