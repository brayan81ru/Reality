#include "ImguiBackend.h"
#include "imgui.h"
#include "ImGuiUtils.hpp"
#include "SDL_events.h"

namespace Reality {

    ImguiBackend::ImguiBackend() = default;

    ImguiBackend::~ImguiBackend() {
        Shutdown();
    }

    bool ImguiBackend::Initialize(Diligent::IRenderDevice* device,Diligent::IDeviceContext* context, const Diligent::ISwapChain* swapChain,const char* fontPath){

        if (m_Initialized) {
            return false;
        }

        // Initialize ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup style
        ImGui::StyleColorsDiligent();

        // Initialize Diligent ImGui implementation
        const auto& SCDesc = swapChain->GetDesc();
        Diligent::ImGuiDiligentCreateInfo CI;
        CI.pDevice = device;
        CI.BackBufferFmt = SCDesc.ColorBufferFormat;
        CI.DepthBufferFmt = SCDesc.DepthBufferFormat;
        CI.InitialVertexBufferSize = 1024;  // Adjust as needed
        CI.InitialIndexBufferSize = 2048;   // Adjust as needed

        m_ImGuiImpl = std::make_unique<Diligent::ImGuiImplDiligent>(CI);

        // Load custom font if specified
        if (fontPath) {
            const ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f);
            IM_ASSERT(font != nullptr);
            // Recreate device objects if font was loaded
            //m_ImGuiImpl->CreateDeviceObjects();
        }

        m_ImGuiImpl->CreateDeviceObjects();

        m_Initialized = true;

        return m_Initialized;
    }

    void ImguiBackend::BeginFrame(const Diligent::ISwapChain* swapChain) const {
        if (!m_Initialized) return;

        // Ensure previous frame was properly ended
        if (ImGui::GetCurrentContext() && ImGui::GetFrameCount() > 0) {
            if (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard) {
                ImGui::EndFrame();
            }
        }

        // Update display size
        const auto& SCDesc = swapChain->GetDesc();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
            static_cast<float>(SCDesc.Width),
            static_cast<float>(SCDesc.Height)
        );

        // Begin new frame
        m_ImGuiImpl->NewFrame(
            SCDesc.Width,
            SCDesc.Height,
            SCDesc.PreTransform
        );
    }

    void ImguiBackend::EndFrame(Diligent::IDeviceContext* context) const {
        if (!m_Initialized) return;

        // Only render if we have a valid frame
        if (ImGui::GetCurrentContext() && ImGui::GetFrameCount() > 0) {
            ImGui::Render();

            ImDrawData* draw_data = ImGui::GetDrawData();

            draw_data->ScaleClipRects(ImGui::GetIO().DisplayFramebufferScale);

            if (ImGui::GetDrawData()) {
                m_ImGuiImpl->Render(context);
            }
        }
    }


    void ImguiBackend::ProcessSDLEvent(const SDL_Event* event) const {
        if (!m_Initialized) return;

        ImGuiIO& io = ImGui::GetIO();

        switch (event->type) {
            case SDL_MOUSEMOTION: {
                io.AddMousePosEvent(
                    event->motion.x,
                    event->motion.y
                );
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                int button = -1;
                if (event->button.button == SDL_BUTTON_LEFT) button = 0;
                if (event->button.button == SDL_BUTTON_RIGHT) button = 1;
                if (event->button.button == SDL_BUTTON_MIDDLE) button = 2;
                if (button != -1)
                    io.AddMouseButtonEvent(button, event->type == SDL_MOUSEBUTTONDOWN);
                break;
            }
            case SDL_MOUSEWHEEL:
                io.AddMouseWheelEvent(static_cast<float>(event->wheel.x), static_cast<float>(event->wheel.y));
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Handle keyboard events
                break;
            case SDL_TEXTINPUT:
                io.AddInputCharactersUTF8(event->text.text);
                break;
            default: break;
        }
    }

    void ImguiBackend::Shutdown() {
        if (!m_Initialized) return;
        m_ImGuiImpl->InvalidateDeviceObjects();
        m_ImGuiImpl.reset();
        ImGui::DestroyContext();
        m_Initialized = false;
    }
} // namespace REngine