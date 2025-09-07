#pragma once

#include <Core/Config.h>
#include <Core/Log.h>
#include <Rendering/ImguiBackend.h>
#include <Platform/DisplayManager.h>
#include <Rendering/Renderer.h>
#include <Core/Timer.h>
#include <Rendering/ShaderManager.h>
#include <Rendering/Texture.h>
#include <Core/RealityApplication.h>
#include <Rendering/PrimitiveRenderer.h>
#include <Core/Camera.h>
#include <Core/MathF.h>
#include <Core/BaseComponent.h>
#include <Core/BaseGameObject.h>
#include <Components/TransformComponent.h>
#include "Core/Scene.h"
#include "Platform/RealityWindow.h"
#include "RenderingBackend/RAW/DX12Renderer.h"

namespace Reality {
    class GraphicsFactory;
    class D3D12Shader;
}

using namespace Diligent;

using Reality::DX12Renderer;

using Reality::RealityWindow;

using Reality::Log;

using Reality::Config;

using Reality::Renderer;

using Reality::Window;

using Reality::ImguiBackend;

using Reality::Timer;

using Reality::DisplayInfo;

using Reality::ShaderManager;

using Reality::RealityApplication;

using Reality::PrimitiveRenderer;

using Reality::Camera;

using Reality::BaseGameObject;

using Reality::BaseComponent;

using Reality::TransformComponent;

using Reality::Scene;

using Reality::TransformComponent;

using Reality::Texture;

using Reality::GraphicsFactory;

using Reality::D3D12Shader;

using namespace Reality::MathF;



