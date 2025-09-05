#include "PrimitiveRenderer.h"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "Log.h"


namespace Reality {
    PrimitiveRenderer::PrimitiveRenderer(Renderer *renderer) {
        m_pRenderer = renderer;

        if (!m_pRenderer) {
            return;
        }

        // Create the shader.
        m_pShader = new Shader(m_pRenderer);

        // Load the shaders vs and the ps.
        if (!m_pShader->Load("Assets/Shaders/shaderTestTriangle.vs", "Assets/Shaders/shaderTestTriangle.ps")) {
            RLOG_ERROR("Failed to load shaders");
            return;
        }
    }

    void PrimitiveRenderer::Render() const {
        if (!m_pRenderer) return;
        if (!m_pShader) return;
        m_pShader->Bind();
        // Render the triangle.
        Diligent::DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // We will render 3 vertices
        m_pRenderer->GetContext()->Draw(drawAttrs);
    }
} // namespace Reality