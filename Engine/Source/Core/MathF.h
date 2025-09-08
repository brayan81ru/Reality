#pragma once
#include <cmath>
#include <cstring>
#include <cassert>

namespace Reality {
    // Forward declarations
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Matrix3x3;
    struct Matrix4x4;
    struct Quaternion;

    // Constants
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float HALF_PI = 0.5f * PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float EPSILON = 1e-6f;

    // Math utility functions
    inline float Sqrt(float x) {
        return std::sqrtf(x);
    }

    inline float Sin(float x) {
        return std::sinf(x);
    }

    inline float Cos(float x) {
        return std::cosf(x);
    }

    inline float Tan(float x) {
        return std::tanf(x);
    }

    inline float ASin(float x) {
        return std::asinf(x);
    }

    inline float ACos(float x) {
        return std::acosf(x);
    }

    inline float ATan(float x) {
        return std::atanf(x);
    }

    inline float ATan2(float y, float x) {
        return std::atan2f(y, x);
    }

    inline float Abs(float x) {
        return std::fabsf(x);
    }

    inline float Min(float a, float b) {
        return a < b ? a : b;
    }

    inline float Max(float a, float b) {
        return a > b ? a : b;
    }

    inline float Clamp(float value, float min, float max) {
        return Min(Max(value, min), max);
    }

    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    inline bool IsApproximatelyEqual(float a, float b, float epsilon = EPSILON) {
        return Abs(a - b) < epsilon;
    }

    inline float DegreesToRadians(float degrees) {
        return degrees * DEG_TO_RAD;
    }

    inline float RadiansToDegrees(float radians) {
        return radians * RAD_TO_DEG;
    }

    // Vector2 - 2D vector
    struct Vector2 {
        float x, y;

        // Constructors
        Vector2() : x(0), y(0) {}
        explicit Vector2(float s) : x(s), y(s) {}
        Vector2(float x, float y) : x(x), y(y) {}

        // Access operators
        float& operator[](int index) {
            assert(index >= 0 && index < 2);
            return (&x)[index];
        }

        const float& operator[](int index) const {
            assert(index >= 0 && index < 2);
            return (&x)[index];
        }

        // Unary operators
        Vector2 operator-() const {
            return Vector2(-x, -y);
        }

        // Binary operators
        Vector2 operator+(const Vector2& v) const {
            return Vector2(x + v.x, y + v.y);
        }

        Vector2 operator-(const Vector2& v) const {
            return Vector2(x - v.x, y - v.y);
        }

        Vector2 operator*(float s) const {
            return Vector2(x * s, y * s);
        }

        Vector2 operator/(float s) const {
            float inv = 1.0f / s;
            return Vector2(x * inv, y * inv);
        }

        Vector2 operator*(const Vector2& v) const {
            return Vector2(x * v.x, y * v.y);
        }

        Vector2 operator/(const Vector2& v) const {
            return Vector2(x / v.x, y / v.y);
        }

        // Assignment operators
        Vector2& operator+=(const Vector2& v) {
            x += v.x;
            y += v.y;
            return *this;
        }

        Vector2& operator-=(const Vector2& v) {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        Vector2& operator*=(float s) {
            x *= s;
            y *= s;
            return *this;
        }

        Vector2& operator/=(float s) {
            float inv = 1.0f / s;
            x *= inv;
            y *= inv;
            return *this;
        }

        Vector2& operator*=(const Vector2& v) {
            x *= v.x;
            y *= v.y;
            return *this;
        }

        Vector2& operator/=(const Vector2& v) {
            x /= v.x;
            y /= v.y;
            return *this;
        }

        // Comparison operators
        bool operator==(const Vector2& v) const {
            return x == v.x && y == v.y;
        }

        bool operator!=(const Vector2& v) const {
            return !(*this == v);
        }

        // Vector operations
        float Length() const {
            return Sqrt(x * x + y * y);
        }

        float LengthSquared() const {
            return x * x + y * y;
        }

        Vector2 Normalized() const {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                return Vector2(x * inv, y * inv);
            }
            return Vector2(0, 0);
        }

        Vector2& Normalize() {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                x *= inv;
                y *= inv;
            }
            return *this;
        }

        float Dot(const Vector2& v) const {
            return x * v.x + y * v.y;
        }

        // Static methods
        static Vector2 Zero() { return Vector2(0, 0); }
        static Vector2 One() { return Vector2(1, 1); }
        static Vector2 UnitX() { return Vector2(1, 0); }
        static Vector2 UnitY() { return Vector2(0, 1); }
    };

    // Vector3 - 3D vector
    struct Vector3 {
        float x, y, z;

        // Constructors
        Vector3() : x(0), y(0), z(0) {}
        explicit Vector3(float s) : x(s), y(s), z(s) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
        explicit Vector3(const Vector2& v, float z) : x(v.x), y(v.y), z(z) {}

        // Access operators
        float& operator[](int index) {
            assert(index >= 0 && index < 3);
            return (&x)[index];
        }

        const float& operator[](int index) const {
            assert(index >= 0 && index < 3);
            return (&x)[index];
        }

        // Unary operators
        Vector3 operator-() const {
            return Vector3(-x, -y, -z);
        }

        // Binary operators
        Vector3 operator+(const Vector3& v) const {
            return Vector3(x + v.x, y + v.y, z + v.z);
        }

        Vector3 operator-(const Vector3& v) const {
            return Vector3(x - v.x, y - v.y, z - v.z);
        }

        Vector3 operator*(float s) const {
            return Vector3(x * s, y * s, z * s);
        }

        Vector3 operator/(float s) const {
            float inv = 1.0f / s;
            return Vector3(x * inv, y * inv, z * inv);
        }

        Vector3 operator*(const Vector3& v) const {
            return Vector3(x * v.x, y * v.y, z * v.z);
        }

        Vector3 operator/(const Vector3& v) const {
            return Vector3(x / v.x, y / v.y, z / v.z);
        }

        // Assignment operators
        Vector3& operator+=(const Vector3& v) {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        Vector3& operator-=(const Vector3& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        Vector3& operator*=(float s) {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        Vector3& operator/=(float s) {
            float inv = 1.0f / s;
            x *= inv;
            y *= inv;
            z *= inv;
            return *this;
        }

        Vector3& operator*=(const Vector3& v) {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }

        Vector3& operator/=(const Vector3& v) {
            x /= v.x;
            y /= v.y;
            z /= v.z;
            return *this;
        }

        // Comparison operators
        bool operator==(const Vector3& v) const {
            return x == v.x && y == v.y && z == v.z;
        }

        bool operator!=(const Vector3& v) const {
            return !(*this == v);
        }

        // Vector operations
        float Length() const {
            return Sqrt(x * x + y * y + z * z);
        }

        float LengthSquared() const {
            return x * x + y * y + z * z;
        }

        Vector3 Normalized() const {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                return Vector3(x * inv, y * inv, z * inv);
            }
            return Vector3(0, 0, 0);
        }

        Vector3& Normalize() {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                x *= inv;
                y *= inv;
                z *= inv;
            }
            return *this;
        }

        float Dot(const Vector3& v) const {
            return x * v.x + y * v.y + z * v.z;
        }

        Vector3 Cross(const Vector3& v) const {
            return Vector3(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            );
        }

        // Static methods
        static Vector3 Zero() { return Vector3(0, 0, 0); }
        static Vector3 One() { return Vector3(1, 1, 1); }
        static Vector3 UnitX() { return Vector3(1, 0, 0); }
        static Vector3 UnitY() { return Vector3(0, 1, 0); }
        static Vector3 UnitZ() { return Vector3(0, 0, 1); }
        static Vector3 Up() { return Vector3(0, 1, 0); }
        static Vector3 Down() { return Vector3(0, -1, 0); }
        static Vector3 Left() { return Vector3(-1, 0, 0); }
        static Vector3 Right() { return Vector3(1, 0, 0); }
        static Vector3 Forward() { return Vector3(0, 0, 1); }
        static Vector3 Backward() { return Vector3(0, 0, -1); }
    };

    // Vector4 - 4D vector
    struct Vector4 {
        float x, y, z, w;

        // Constructors
        Vector4() : x(0), y(0), z(0), w(0) {}
        explicit Vector4(float s) : x(s), y(s), z(s), w(s) {}
        Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        explicit Vector4(const Vector3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
        explicit Vector4(const Vector2& v, float z, float w) : x(v.x), y(v.y), z(z), w(w) {}

        // Access operators
        float& operator[](int index) {
            assert(index >= 0 && index < 4);
            return (&x)[index];
        }

        const float& operator[](int index) const {
            assert(index >= 0 && index < 4);
            return (&x)[index];
        }

        // Unary operators
        Vector4 operator-() const {
            return Vector4(-x, -y, -z, -w);
        }

        // Binary operators
        Vector4 operator+(const Vector4& v) const {
            return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
        }

        Vector4 operator-(const Vector4& v) const {
            return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
        }

        Vector4 operator*(float s) const {
            return Vector4(x * s, y * s, z * s, w * s);
        }

        Vector4 operator/(float s) const {
            float inv = 1.0f / s;
            return Vector4(x * inv, y * inv, z * inv, w * inv);
        }

        Vector4 operator*(const Vector4& v) const {
            return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
        }

        Vector4 operator/(const Vector4& v) const {
            return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
        }

        // Assignment operators
        Vector4& operator+=(const Vector4& v) {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            return *this;
        }

        Vector4& operator-=(const Vector4& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
            return *this;
        }

        Vector4& operator*=(float s) {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }

        Vector4& operator/=(float s) {
            float inv = 1.0f / s;
            x *= inv;
            y *= inv;
            z *= inv;
            w *= inv;
            return *this;
        }

        Vector4& operator*=(const Vector4& v) {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            w *= v.w;
            return *this;
        }

        Vector4& operator/=(const Vector4& v) {
            x /= v.x;
            y /= v.y;
            z /= v.z;
            w /= v.w;
            return *this;
        }

        // Comparison operators
        bool operator==(const Vector4& v) const {
            return x == v.x && y == v.y && z == v.z && w == v.w;
        }

        bool operator!=(const Vector4& v) const {
            return !(*this == v);
        }

        // Vector operations
        float Length() const {
            return Sqrt(x * x + y * y + z * z + w * w);
        }

        float LengthSquared() const {
            return x * x + y * y + z * z + w * w;
        }

        Vector4 Normalized() const {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                return Vector4(x * inv, y * inv, z * inv, w * inv);
            }
            return Vector4(0, 0, 0, 0);
        }

        Vector4& Normalize() {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                x *= inv;
                y *= inv;
                z *= inv;
                w *= inv;
            }
            return *this;
        }

        float Dot(const Vector4& v) const {
            return x * v.x + y * v.y + z * v.z + w * v.w;
        }

        // Static methods
        static Vector4 Zero() { return Vector4(0, 0, 0, 0); }
        static Vector4 One() { return Vector4(1, 1, 1, 1); }
        static Vector4 UnitX() { return Vector4(1, 0, 0, 0); }
        static Vector4 UnitY() { return Vector4(0, 1, 0, 0); }
        static Vector4 UnitZ() { return Vector4(0, 0, 1, 0); }
        static Vector4 UnitW() { return Vector4(0, 0, 0, 1); }
    };

    // Matrix3x3 - 3x3 matrix
    struct Matrix3x3 {
        float m[3][3];

        // Constructors
        Matrix3x3() {
            memset(m, 0, sizeof(m));
        }

        explicit Matrix3x3(float s) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    m[i][j] = (i == j) ? s : 0.0f;
                }
            }
        }

        Matrix3x3(
            float m00, float m01, float m02,
            float m10, float m11, float m12,
            float m20, float m21, float m22) {
            m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
            m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
            m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
        }

        // Access operators
        float* operator[](int row) {
            assert(row >= 0 && row < 3);
            return m[row];
        }

        const float* operator[](int row) const {
            assert(row >= 0 && row < 3);
            return m[row];
        }

        // Matrix operations
        Matrix3x3 Transposed() const {
            return Matrix3x3(
                m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]
            );
        }

        Matrix3x3& Transpose() {
            float tmp;
            tmp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = tmp;
            tmp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = tmp;
            tmp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = tmp;
            return *this;
        }

        float Determinant() const {
            return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                   m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                   m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        }

        Matrix3x3 Inverse() const {
            float det = Determinant();
            if (Abs(det) < EPSILON) {
                return Matrix3x3(); // Return zero matrix if not invertible
            }

            float invDet = 1.0f / det;
            return Matrix3x3(
                (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet,
                (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet,
                (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet,
                (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet,
                (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet,
                (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet,
                (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet,
                (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet,
                (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet
            );
        }

        // Static methods
        static Matrix3x3 Zero() {
            return Matrix3x3();
        }

        static Matrix3x3 Identity() {
            return Matrix3x3(1.0f);
        }

        // Factory methods
        static Matrix3x3 RotationX(float angle) {
            float c = Cos(angle);
            float s = Sin(angle);
            return Matrix3x3(
                1.0f, 0.0f, 0.0f,
                0.0f, c,    s,
                0.0f, -s,   c
            );
        }

        static Matrix3x3 RotationY(float angle) {
            float c = Cos(angle);
            float s = Sin(angle);
            return Matrix3x3(
                c,    0.0f, -s,
                0.0f, 1.0f, 0.0f,
                s,    0.0f, c
            );
        }

        static Matrix3x3 RotationZ(float angle) {
            float c = Cos(angle);
            float s = Sin(angle);
            return Matrix3x3(
                c,    s,    0.0f,
                -s,   c,    0.0f,
                0.0f, 0.0f, 1.0f
            );
        }

        static Matrix3x3 RotationAxis(const Vector3& axis, float angle) {
            Vector3 a = axis.Normalized();
            float c = Cos(angle);
            float s = Sin(angle);
            float t = 1.0f - c;

            return Matrix3x3(
                t * a.x * a.x + c,          t * a.x * a.y - s * a.z,    t * a.x * a.z + s * a.y,
                t * a.x * a.y + s * a.z,    t * a.y * a.y + c,          t * a.y * a.z - s * a.x,
                t * a.x * a.z - s * a.y,    t * a.y * a.z + s * a.x,    t * a.z * a.z + c
            );
        }

        static Matrix3x3 Scale(float s) {
            return Matrix3x3(
                s, 0.0f, 0.0f,
                0.0f, s, 0.0f,
                0.0f, 0.0f, s
            );
        }

        static Matrix3x3 Scale(const Vector3& s) {
            return Matrix3x3(
                s.x, 0.0f, 0.0f,
                0.0f, s.y, 0.0f,
                0.0f, 0.0f, s.z
            );
        }
    };

    // Matrix4x4 - 4x4 matrix
    struct Matrix4x4 {
        float m[4][4];

        // Constructors
        Matrix4x4() {
            memset(m, 0, sizeof(m));
        }

        explicit Matrix4x4(float s) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    m[i][j] = (i == j) ? s : 0.0f;
                }
            }
        }

        Matrix4x4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33) {
            m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
            m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
            m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
            m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
        }

        // Access operators
        float* operator[](int row) {
            assert(row >= 0 && row < 4);
            return m[row];
        }

        const float* operator[](int row) const {
            assert(row >= 0 && row < 4);
            return m[row];
        }

        // Binary operators
        Matrix4x4 operator*(const Matrix4x4& other) const {
            Matrix4x4 result;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result.m[i][j] = 0.0f;
                    for (int k = 0; k < 4; k++) {
                        result.m[i][j] += m[i][k] * other.m[k][j];
                    }
                }
            }
            return result;
        }

        Vector4 operator*(const Vector4& v) const {
            return Vector4(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
            );
        }

        Vector3 operator*(const Vector3& v) const {
            Vector4 result = *this * Vector4(v, 1.0f);
            return Vector3(result.x, result.y, result.z);
        }

        // Matrix operations
        Matrix4x4 Transposed() const {
            return Matrix4x4(
                m[0][0], m[1][0], m[2][0], m[3][0],
                m[0][1], m[1][1], m[2][1], m[3][1],
                m[0][2], m[1][2], m[2][2], m[3][2],
                m[0][3], m[1][3], m[2][3], m[3][3]
            );
        }

        Matrix4x4& Transpose() {
            float tmp;
            for (int i = 0; i < 4; i++) {
                for (int j = i + 1; j < 4; j++) {
                    tmp = m[i][j];
                    m[i][j] = m[j][i];
                    m[j][i] = tmp;
                }
            }
            return *this;
        }

        Matrix4x4 Inverse() const {
            // Compute the inverse of a 4x4 matrix using Gaussian elimination
            Matrix4x4 result = Matrix4x4::Identity();
            Matrix4x4 temp = *this;

            for (int i = 0; i < 4; i++) {
                // Find pivot
                int pivot = i;
                for (int j = i + 1; j < 4; j++) {
                    if (Abs(temp.m[j][i]) > Abs(temp.m[pivot][i])) {
                        pivot = j;
                    }
                }

                // Swap rows if needed
                if (pivot != i) {
                    for (int j = 0; j < 4; j++) {
                        float tmp = temp.m[i][j];
                        temp.m[i][j] = temp.m[pivot][j];
                        temp.m[pivot][j] = tmp;

                        tmp = result.m[i][j];
                        result.m[i][j] = result.m[pivot][j];
                        result.m[pivot][j] = tmp;
                    }
                }

                // Singular matrix
                if (Abs(temp.m[i][i]) < EPSILON) {
                    return Matrix4x4::Identity(); // Return identity if not invertible
                }

                // Normalize pivot row
                float invPivot = 1.0f / temp.m[i][i];
                for (int j = 0; j < 4; j++) {
                    temp.m[i][j] *= invPivot;
                    result.m[i][j] *= invPivot;
                }

                // Eliminate other rows
                for (int j = 0; j < 4; j++) {
                    if (j != i && Abs(temp.m[j][i]) > EPSILON) {
                        float factor = temp.m[j][i];
                        for (int k = 0; k < 4; k++) {
                            temp.m[j][k] -= factor * temp.m[i][k];
                            result.m[j][k] -= factor * result.m[i][k];
                        }
                    }
                }
            }

            return result;
        }

        // Static methods
        static Matrix4x4 Zero() {
            return Matrix4x4();
        }

        static Matrix4x4 Identity() {
            return Matrix4x4(1.0f);
        }

        // Factory methods
        static Matrix4x4 Translation(const Vector3& t) {
            return Matrix4x4(
                1.0f, 0.0f, 0.0f, t.x,
                0.0f, 1.0f, 0.0f, t.y,
                0.0f, 0.0f, 1.0f, t.z,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        static Matrix4x4 RotationX(float angle) {
            float c = Cos(angle);
            float s = Sin(angle);
            return Matrix4x4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, c,    s,    0.0f,
                0.0f, -s,   c,    0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        static Matrix4x4 RotationY(float angle) {
            float c = Cos(angle);
            float s = Sin(angle);
            return Matrix4x4(
                c,    0.0f, -s,   0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                s,    0.0f, c,    0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        static Matrix4x4 RotationZ(float angle) {
            float c = Cos(angle);
            float s = Sin(angle);
            return Matrix4x4(
                c,    s,    0.0f, 0.0f,
                -s,   c,    0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        static Matrix4x4 RotationAxis(const Vector3& axis, float angle) {
            Vector3 a = axis.Normalized();
            float c = Cos(angle);
            float s = Sin(angle);
            float t = 1.0f - c;

            return Matrix4x4(
                t * a.x * a.x + c,          t * a.x * a.y - s * a.z,    t * a.x * a.z + s * a.y,    0.0f,
                t * a.x * a.y + s * a.z,    t * a.y * a.y + c,          t * a.y * a.z - s * a.x,    0.0f,
                t * a.x * a.z - s * a.y,    t * a.y * a.z + s * a.x,    t * a.z * a.z + c,          0.0f,
                0.0f,                      0.0f,                      0.0f,                      1.0f
            );
        }

        static Matrix4x4 Scale(float s) {
            return Matrix4x4(
                s, 0.0f, 0.0f, 0.0f,
                0.0f, s, 0.0f, 0.0f,
                0.0f, 0.0f, s, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        static Matrix4x4 Scale(const Vector3& s) {
            return Matrix4x4(
                s.x, 0.0f, 0.0f, 0.0f,
                0.0f, s.y, 0.0f, 0.0f,
                0.0f, 0.0f, s.z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        }

        // Projection matrices
        static Matrix4x4 Perspective(float fovy, float aspect, float nearPlane, float farPlane) {
            float f = 1.0f / Tan(fovy * 0.5f);
            float nf = 1.0f / (nearPlane - farPlane);

            return Matrix4x4(
                f / aspect, 0.0f, 0.0f,                   0.0f,
                0.0f,       f,    0.0f,                   0.0f,
                0.0f,       0.0f, (farPlane + nearPlane) * nf,     2.0f * farPlane * nearPlane * nf,
                0.0f,       0.0f, -1.0f,                  0.0f
            );
        }

        static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
            float rl = 1.0f / (right - left);
            float tb = 1.0f / (top - bottom);
            float fn = 1.0f / (farPlane - nearPlane);

            return Matrix4x4(
                2.0f * rl, 0.0f,      0.0f,       -(right + left) * rl,
                0.0f,      2.0f * tb, 0.0f,       -(top + bottom) * tb,
                0.0f,      0.0f,      -2.0f * fn, -(farPlane + nearPlane) * fn,
                0.0f,      0.0f,      0.0f,       1.0f
            );
        }

        static Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
            Vector3 zAxis = (eye - target).Normalized();
            Vector3 xAxis = up.Cross(zAxis).Normalized();
            Vector3 yAxis = zAxis.Cross(xAxis);

            return Matrix4x4(
                xAxis.x, xAxis.y, xAxis.z, -xAxis.Dot(eye),
                yAxis.x, yAxis.y, yAxis.z, -yAxis.Dot(eye),
                zAxis.x, zAxis.y, zAxis.z, -zAxis.Dot(eye),
                0.0f,    0.0f,    0.0f,    1.0f
            );
        }
    };

    // Quaternion - for rotations
    struct Quaternion {
        float x, y, z, w;

        // Constructors
        Quaternion() : x(0), y(0), z(0), w(1) {}
        Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        explicit Quaternion(const Vector3& axis, float angle) {
            Vector3 a = axis.Normalized();
            float s = Sin(angle * 0.5f);
            x = a.x * s;
            y = a.y * s;
            z = a.z * s;
            w = Cos(angle * 0.5f);
        }

        // Access operators
        float& operator[](int index) {
            assert(index >= 0 && index < 4);
            return (&x)[index];
        }

        const float& operator[](int index) const {
            assert(index >= 0 && index < 4);
            return (&x)[index];
        }

        // Unary operators
        Quaternion operator-() const {
            return Quaternion(-x, -y, -z, -w);
        }

        // Binary operators
        Quaternion operator+(const Quaternion& q) const {
            return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
        }

        Quaternion operator-(const Quaternion& q) const {
            return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
        }

        Quaternion operator*(const Quaternion& q) const {
            return Quaternion(
                w * q.x + x * q.w + y * q.z - z * q.y,
                w * q.y + y * q.w + z * q.x - x * q.z,
                w * q.z + z * q.w + x * q.y - y * q.x,
                w * q.w - x * q.x - y * q.y - z * q.z
            );
        }

        Quaternion operator*(float s) const {
            return Quaternion(x * s, y * s, z * s, w * s);
        }

        Vector3 operator*(const Vector3& v) const {
            // Convert quaternion to matrix and multiply
            return ToMatrix4x4() * v;
        }

        // Assignment operators
        Quaternion& operator+=(const Quaternion& q) {
            x += q.x;
            y += q.y;
            z += q.z;
            w += q.w;
            return *this;
        }

        Quaternion& operator-=(const Quaternion& q) {
            x -= q.x;
            y -= q.y;
            z -= q.z;
            w -= q.w;
            return *this;
        }

        Quaternion& operator*=(const Quaternion& q) {
            *this = *this * q;
            return *this;
        }

        Quaternion& operator*=(float s) {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }

        // Quaternion operations
        float Length() const {
            return Sqrt(x * x + y * y + z * z + w * w);
        }

        float LengthSquared() const {
            return x * x + y * y + z * z + w * w;
        }

        Quaternion Normalized() const {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                return Quaternion(x * inv, y * inv, z * inv, w * inv);
            }
            return Quaternion(0, 0, 0, 1);
        }

        Quaternion& Normalize() {
            float len = Length();
            if (len > EPSILON) {
                float inv = 1.0f / len;
                x *= inv;
                y *= inv;
                z *= inv;
                w *= inv;
            }
            return *this;
        }

        Quaternion Conjugate() const {
            return Quaternion(-x, -y, -z, w);
        }

        Quaternion Inverse() const {
            float lenSq = LengthSquared();
            if (lenSq > EPSILON) {
                float inv = 1.0f / lenSq;
                return Quaternion(-x * inv, -y * inv, -z * inv, w * inv);
            }
            return Quaternion(0, 0, 0, 1);
        }

        float Dot(const Quaternion& q) const {
            return x * q.x + y * q.y + z * q.z + w * q.w;
        }

        // Conversion functions
        Matrix3x3 ToMatrix3x3() const {
            float xx = x * x;
            float xy = x * y;
            float xz = x * z;
            float xw = x * w;
            float yy = y * y;
            float yz = y * z;
            float yw = y * w;
            float zz = z * z;
            float zw = z * w;
            float ww = w * w;

            return Matrix3x3(
                1.0f - 2.0f * (yy + zz), 2.0f * (xy - zw),         2.0f * (xz + yw),
                2.0f * (xy + zw),         1.0f - 2.0f * (xx + zz), 2.0f * (yz - xw),
                2.0f * (xz - yw),         2.0f * (yz + xw),         1.0f - 2.0f * (xx + yy)
            );
        }

        Matrix4x4 ToMatrix4x4() const {
            float xx = x * x;
            float xy = x * y;
            float xz = x * z;
            float xw = x * w;
            float yy = y * y;
            float yz = y * z;
            float yw = y * w;
            float zz = z * z;
            float zw = z * w;
            float ww = w * w;

            return Matrix4x4(
                1.0f - 2.0f * (yy + zz), 2.0f * (xy - zw),         2.0f * (xz + yw),         0.0f,
                2.0f * (xy + zw),         1.0f - 2.0f * (xx + zz), 2.0f * (yz - xw),         0.0f,
                2.0f * (xz - yw),         2.0f * (yz + xw),         1.0f - 2.0f * (xx + yy), 0.0f,
                0.0f,                     0.0f,                     0.0f,                     1.0f
            );
        }

        // Static methods
        static Quaternion Identity() {
            return Quaternion(0, 0, 0, 1);
        }

        // Factory methods
        static Quaternion FromEulerAngles(float pitch, float yaw, float roll) {
            float cp = Cos(pitch * 0.5f);
            float sp = Sin(pitch * 0.5f);
            float cy = Cos(yaw * 0.5f);
            float sy = Sin(yaw * 0.5f);
            float cr = Cos(roll * 0.5f);
            float sr = Sin(roll * 0.5f);

            return Quaternion(
                cy * sp * cr + sy * cp * sr,
                sy * cp * cr - cy * sp * sr,
                cy * cp * sr - sy * sp * cr,
                cy * cp * cr + sy * sp * sr
            );
        }

        static Quaternion FromMatrix3x3(const Matrix3x3& m) {
            float trace = m[0][0] + m[1][1] + m[2][2];
            Quaternion q;

            if (trace > 0.0f) {
                float s = Sqrt(trace + 1.0f);
                float invS = 0.5f / s;
                q.w = s * 0.5f;
                q.x = (m[2][1] - m[1][2]) * invS;
                q.y = (m[0][2] - m[2][0]) * invS;
                q.z = (m[1][0] - m[0][1]) * invS;
            } else {
                int i = 0;
                if (m[1][1] > m[0][0]) i = 1;
                if (m[2][2] > m[i][i]) i = 2;

                int j = (i + 1) % 3;
                int k = (j + 1) % 3;

                float s = Sqrt(m[i][i] - m[j][j] - m[k][k] + 1.0f);
                float invS = 0.5f / s;

                q[i] = s * 0.5f;
                q.w = (m[k][j] - m[j][k]) * invS;
                q[j] = (m[j][i] + m[i][j]) * invS;
                q[k] = (m[k][i] + m[i][k]) * invS;
            }

            return q;
        }

        static Quaternion FromMatrix4x4(const Matrix4x4& m) {
            return FromMatrix3x3(Matrix3x3(
                m[0][0], m[0][1], m[0][2],
                m[1][0], m[1][1], m[1][2],
                m[2][0], m[2][1], m[2][2]
            ));
        }

        static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t) {
            float cosTheta = a.Dot(b);

            // Take the shorter path
            if (cosTheta < 0.0f) {
                cosTheta = -cosTheta;
                return Slerp(a, -b, t);
            }

            if (cosTheta > 1.0f - EPSILON) {
                // Linear interpolation for very close quaternions
                return (a * (1.0f - t) + b * t).Normalized();
            }

            float theta = ACos(cosTheta);
            float sinTheta = Sin(theta);
            float t1 = Sin((1.0f - t) * theta) / sinTheta;
            float t2 = Sin(t * theta) / sinTheta;

            return a * t1 + b * t2;
        }
    };

    // Utility functions
    inline Vector3 TransformPoint(const Matrix4x4& m, const Vector3& p) {
        Vector4 result = m * Vector4(p, 1.0f);
        return Vector3(result.x, result.y, result.z);
    }

    inline Vector3 TransformVector(const Matrix4x4& m, const Vector3& v) {
        Vector4 result = m * Vector4(v, 0.0f);
        return Vector3(result.x, result.y, result.z);
    }

    inline Vector3 TransformNormal(const Matrix4x4& m, const Vector3& n) {
        // Use inverse transpose for normal transformation
        Matrix4x4 invTrans = m.Inverse().Transposed();
        Vector4 result = invTrans * Vector4(n, 0.0f);
        return Vector3(result.x, result.y, result.z).Normalized();
    }

    // Matrix layout conversion utilities
    namespace MatrixLayout {
        // Convert from column-major (GLM/Vulkan style) to row-major (D3D style)
        inline Matrix4x4 ColumnMajorToRowMajor(const Matrix4x4& m) {
            return m.Transposed();
        }

        // Convert from row-major (D3D style) to column-major (GLM/Vulkan style)
        inline Matrix4x4 RowMajorToColumnMajor(const Matrix4x4& m) {
            return m.Transposed();
        }
    }
}