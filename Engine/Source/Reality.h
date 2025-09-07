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
#include "RenderingBackend/GraphicsTypes.h"
#include "RenderingBackend/GraphicsDevice.h"
#include "RenderingBackend/Resource.h"
#include "Platform/RealityWindow.h"
#include "RenderingBackend/CommandList.h"
#include "RenderingBackend/D3D12Buffer.h"
#include "RenderingBackend/D3D12Texture.h"
#include "RenderingBackend/D3D12CommandList.h"
#include "RenderingBackend/D3D12Fence.h"
#include "RenderingBackend/GraphicsFactory.h"
#include "RenderingBackend/D3D12Device.h"


namespace Reality {
    class GraphicsFactory;
    class D3D12Shader;
}

using namespace Diligent;

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

using Reality::IGraphicsDevice;

using Reality::Texture;

using Reality::GraphicsFactory;

using Reality::D3D12Buffer;

using Reality::D3D12Shader;

using Reality::D3D12Texture;

using Reality::D3D12CommandList;

using Reality::D3D12Fence;

using Reality::D3D12Device;



using namespace Reality::MathF;



