#pragma once
#include "rendering/Renderer.h"
#include "core/Shader.h"

namespace Reality {

    class PrimitiveRenderer {
    public:
        explicit PrimitiveRenderer(Renderer* renderer);

        void Render() const;

    private:
        Renderer* m_pRenderer;
        Shader * m_pShader;
    };

} // namespace Reality