#pragma once
#include <BasicMath.hpp>
#include <core/Vector3.h>

namespace Reality {
    // Forward declaration
    template<typename T> class Quaternion;

    // Default float specialization
    using Quat = Quaternion<float>;

    template<typename T = float>

    class Quaternion {
    public:
        // Internal storage using Diligent's quaternion
        Diligent::Quaternion<T> q;

        // Constructors
        Quaternion() : q{0, 0, 0, 1} {}
        Quaternion(T x, T y, T z, T w) : q{x, y, z, w} {}
        explicit Quaternion(const Diligent::Quaternion<T>& q) : q(q) {}

        // Conversion from our Vector3 + angle
        static Quaternion FromAxisAngle(const Vector3& axis, T angle) {
            return Quaternion(Diligent::Quaternion<T>::RotationFromAxisAngle(
                {axis.x, axis.y, axis.z}, angle));
        }

        // Euler angles constructor (degrees)
        static Quaternion FromEuler(T pitch, T yaw, T roll) {
            // Convert to radians
            pitch *= T(3.14159265358979323846 / 180.0);
            yaw *= T(3.14159265358979323846 / 180.0);
            roll *= T(3.14159265358979323846 / 180.0);

            T cy = cos(yaw * T(0.5));
            T sy = sin(yaw * T(0.5));
            T cp = cos(pitch * T(0.5));
            T sp = sin(pitch * T(0.5));
            T cr = cos(roll * T(0.5));
            T sr = sin(roll * T(0.5));

            return {
                sr * cp * cy - cr * sp * sy,
                cr * sp * cy + sr * cp * sy,
                cr * cp * sy - sr * sp * cy,
                cr * cp * cy + sr * sp * sy
            };
        }

        // Operations
        Quaternion operator*(const Quaternion& rhs) const {
            return Quaternion(Diligent::Quaternion<T>::Mul(q, rhs.q));
        }

        Vector3 operator*(const Vector3& v) const {
            auto rotated = q.RotateVector({v.x, v.y, v.z});
            return Vector3(rotated.x, rotated.y, rotated.z);
        }

        // Conversions
        Diligent::Matrix4x4<T> ToRotationMatrix() const {
            auto mat = q.ToMatrix();
            return Matrix4x4(mat);
        }

        // Add this method to your Quaternion class
        [[nodiscard]] Diligent::float4x4 ToDiligentMatrix() const {
            return q.ToMatrix();
        }

        // Constants
        static Quaternion Identity() { return {0, 0, 0, 1}; }
    };
}