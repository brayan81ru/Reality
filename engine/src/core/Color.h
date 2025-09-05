#pragma once
#include "BasicMath.hpp"

namespace Reality {
    class Color {
    public:
        float r, g, b, a;

        // Constructors
        Color() : r(0), g(0), b(0), a(1) {}
        Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
        explicit Color(const Diligent::float4& c) : r(c.x), g(c.y), b(c.z), a(c.w) {}

        // Conversions
        operator Diligent::float4() const { return {r, g, b, a}; }
        Diligent::float4 ToDiligent() const { return {r, g, b, a}; }

        // Color operations
        Color operator*(float scalar) const { return {r*scalar, g*scalar, b*scalar, a}; }
        Color operator+(const Color& rhs) const { return {r+rhs.r, g+rhs.g, b+rhs.b, a}; }

        // Common colors
        static Color White() { return {1,1,1,1}; }
        static Color Black() { return {0,0,0,1}; }
        static Color Red() { return {1,0,0,1}; }
        static Color Green() { return {0,1,0,1}; }
        static Color Blue() { return {0,0,1,1}; }
        static Color Clear() { return {0,0,0,0}; }
    };
}
