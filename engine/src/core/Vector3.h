#pragma once
#include "BasicMath.hpp"

namespace Reality {
    class Vector3 {
    public:
        float x, y, z;

        // Constructors
        Vector3() : x(0), y(0), z(0) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
        explicit Vector3(const Diligent::float3& v) : x(v.x), y(v.y), z(v.z) {}

        // Conversions
        explicit operator Diligent::float3() const { return {x, y, z}; }
        [[nodiscard]] Diligent::float3 ToDiligent() const { return {x, y, z}; }
        static Vector3 FromDiligent(const Diligent::float3& v) { return {v.x, v.y, v.z}; }

        // Basic operations
        Vector3 operator+(const Vector3& rhs) const { return {x+rhs.x, y+rhs.y, z+rhs.z}; }
        Vector3 operator-(const Vector3& rhs) const { return {x-rhs.x, y-rhs.y, z-rhs.z}; }
        Vector3 operator*(float scalar) const { return {x*scalar, y*scalar, z*scalar}; }
        Vector3 operator/(float scalar) const { return {x/scalar, y/scalar, z/scalar}; }

        // More complex operations
        [[nodiscard]] float Dot(const Vector3& rhs) const { return x*rhs.x + y*rhs.y + z*rhs.z; }
        [[nodiscard]] Vector3 Cross(const Vector3& rhs) const {
            return {
                y*rhs.z - z*rhs.y,
                z*rhs.x - x*rhs.z,
                x*rhs.y - y*rhs.x
            };
        }
        [[nodiscard]] float Length() const { return sqrtf(x*x + y*y + z*z); }
        [[nodiscard]] Vector3 Normalized() const { return *this / Length(); }

        // Constants
        static Vector3 Zero() { return {0,0,0}; }
        static Vector3 One() { return {1,1,1}; }
        static Vector3 Up() { return {0,1,0}; }
        static Vector3 Forward() { return {0,0,1}; }
        static Vector3 Right() { return {1,0,0}; }
    };
}
