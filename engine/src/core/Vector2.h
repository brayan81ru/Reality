#pragma once
#include "BasicMath.hpp"

namespace Reality {
    class Vector2 {
    public:
        float x, y;

        // Constructors
        Vector2() : x(0), y(0) {}
        Vector2(float x, float y) : x(x), y(y) {}
        explicit Vector2(const Diligent::float2& v) : x(v.x), y(v.y) {}

        // Conversions
        explicit operator Diligent::float2() const { return {x, y}; }
        [[nodiscard]] Diligent::float2 ToDiligent() const { return {x, y}; }


        // Basic operations


        // More complex operations
        [[nodiscard]] float Dot(const Vector2& rhs) const { return x*rhs.x + y*rhs.y; }

        [[nodiscard]] float Length() const { return sqrtf(x*x + y*y); }


    };
}
