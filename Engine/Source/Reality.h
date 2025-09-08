#pragma once

#include <Core/Config.h>
#include <Core/Log.h>
#include <Core/Timer.h>
#include <Core/MathF.h>

#include <Platform/DisplayManager.h>
#include <Platform/Window.h>

#include <Rendering/GraphicsTypes.h>
#include <Rendering/CommandList.h>
#include <Rendering/GraphicsDevice.h>
#include <Rendering/GraphicsFactory.h>
#include <Rendering/Resource.h>
#include <Rendering/GraphicsFactory.h>
#include <Rendering/HighLevelRenderer.h>

#include "Rendering/Backends/D3D12/D3D12Buffer.h"
#include "Rendering/Backends/D3D12/D3D12CommandList.h"
#include "Rendering/Backends/D3D12/D3D12Shader.h"
#include "Rendering/Backends/D3D12/D3D12SwapChain.h"
#include "Rendering/Backends/D3D12/D3D12PipelineState.h"
#include "Rendering/Backends/D3D12/D3D12Texture.h"

#include "RenderingBackend/RAW/DX12Renderer.h"


using Reality::DX12Renderer;

using Reality::Log;

using Reality::Config;

using Reality::Timer;

using Reality::DisplayInfo;

using Reality::Window;

using Reality::GraphicsFactory;

using Reality::HighLevelRenderer;

using Reality::D3D12Buffer;

using Reality::D3D12Texture;

using Reality::D3D12PipelineState;

using Reality::D3D12Shader;

using Reality::D3D12SwapChain;

using Reality::D3D12CommandList;

using Reality::D3D12PipelineState;

using Reality::D3D12Device;










