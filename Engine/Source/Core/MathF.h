#pragma once
#include <DiligentCore/Common/interface/BasicMath.hpp>
#include <xmmintrin.h>  // SSE
#include <smmintrin.h>  // SSE4.1
#include <cmath>
#include <cassert>

namespace Reality::MathF {
    // Forward declarations
    class Vector2f;
    class Vector3f;
    class Vector4f;
    class Matrix4x4;
    class Quaternion;

    // ==================== Vector2f ====================
    class alignas(8) Vector2f {
        public:
        // Constructors
        constexpr Vector2f() : x(0), y(0) {}
        constexpr Vector2f(const float x, const float y) : x(x), y(y) {}
        explicit Vector2f(const Diligent::float2& v) : x(v.x), y(v.y) {}

        // Conversion to Diligent type
        explicit operator Diligent::float2() const { return {x, y}; }

        // Component access
        float& operator[](const size_t i) { assert(i < 2); return (&x)[i]; }
        const float& operator[](const size_t i) const { assert(i < 2); return (&x)[i]; }

        // Vector operations
        Vector2f operator+(const Vector2f& other) const {
            return Vector2f(x + other.x, y + other.y);
        }
        Vector2f operator-(const Vector2f& other) const {
            return Vector2f(x - other.x, y - other.y);
        }
        Vector2f operator*(const float scalar) const {
            return Vector2f(x * scalar, y * scalar);
        }
        Vector2f operator/(const float scalar) const {
            assert(scalar != 0.0f);
            const float inv = 1.0f / scalar;
            return Vector2f(x * inv, y * inv);
        }

        // Dot product
        float Dot(const Vector2f& other) const {
            return x * other.x + y * other.y;
        }

        // Length and normalization
        float Length() const { return std::sqrt(x * x + y * y); }
        float LengthSquared() const { return x * x + y * y; }
        Vector2f Normalized() const {
            const float len = Length();
            return (len > 0.0f) ? (*this / len) : Vector2f();
        }

        // Data members
        float x, y;
    };

    // ==================== Vector3f ====================
    class alignas(16) Vector3f {
        public:
        // Constructors
        constexpr Vector3f() : x(0), y(0), z(0), padding(0) {}
        constexpr Vector3f(const float x, const float y, const float z) : x(x), y(y), z(z), padding(0) {}
        explicit Vector3f(const Diligent::float3& v) : x(v.x), y(v.y), z(v.z), padding(0) {}
        explicit Vector3f(const __m128 vec) { _mm_store_ps(&x, vec); }

        // Conversion to Diligent type
        explicit operator Diligent::float3() const { return {x, y, z}; }

        // Component access
        float& operator[](const size_t i) { assert(i < 3); return (&x)[i]; }
        const float& operator[](const size_t i) const { assert(i < 3); return (&x)[i]; }

        // SIMD load/store
        __m128 Load() const { return _mm_load_ps(&x); }
        void Store(const __m128 vec) { _mm_store_ps(&x, vec); }

        // Vector operations (SIMD optimized)
        Vector3f operator+(const Vector3f& other) const {
            return Vector3f(_mm_add_ps(Load(), other.Load()));
        }
        Vector3f operator-(const Vector3f& other) const {
            return Vector3f(_mm_sub_ps(Load(), other.Load()));
        }
        Vector3f operator*(const float scalar) const {
            return Vector3f(_mm_mul_ps(Load(), _mm_set1_ps(scalar)));
        }
        Vector3f operator/(const float scalar) const {
            assert(scalar != 0.0f);
            return Vector3f(_mm_div_ps(Load(), _mm_set1_ps(scalar)));
        }

        // Dot product (SIMD optimized)
        float Dot(const Vector3f& other) const {
            __m128 prod = _mm_mul_ps(Load(), other.Load());
            prod = _mm_hadd_ps(prod, prod);
            prod = _mm_hadd_ps(prod, prod);
            return _mm_cvtss_f32(prod);
        }

        // Cross product (SIMD optimized)
        Vector3f Cross(const Vector3f& other) const {
            const __m128 a = Load();
            const __m128 b = other.Load();

            // Shuffle components for cross product
            __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
            const __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
            const __m128 c1 = _mm_mul_ps(a, b_yzx);

            const __m128 a_zxy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
            __m128 b_zxy = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
            const __m128 c2 = _mm_mul_ps(a_zxy, b);

            return Vector3f(_mm_sub_ps(c1, c2));
        }

        // Length and normalization (SIMD optimized)
        float Length() const { return std::sqrt(LengthSquared()); }
        float LengthSquared() const { return Dot(*this); }
        Vector3f Normalized() const {
            const __m128 v = Load();
            const __m128 len = _mm_sqrt_ps(_mm_dp_ps(v, v, 0x71));
            return Vector3f(_mm_div_ps(v, len));
        }

        // Data members
        float x, y, z;
        float padding;
        // For SIMD alignment

        // Unary minus operator
        Vector3f operator-() const {
            return Vector3f(-x, -y, -z);
        }


    };

    // ==================== Vector4f ====================
    class alignas(16) Vector4f {
        public:
        // Constructors
        constexpr Vector4f() : x(0), y(0), z(0), w(0) {}
        constexpr Vector4f(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
        explicit Vector4f(const Diligent::float4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
        explicit Vector4f(const Vector3f& v, const float w = 0.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
        explicit Vector4f(const __m128 vec) { _mm_store_ps(&x, vec); }
        // Conversion to Vector3f (ignores w component)
        explicit operator Vector3f() const { return Vector3f(x, y, z); }

        // Conversion to Diligent type
        explicit operator Diligent::float4() const { return {x, y, z, w}; }

        // Component access
        float& operator[](const size_t i) { assert(i < 4); return (&x)[i]; }
        const float& operator[](const size_t i) const { assert(i < 4); return (&x)[i]; }

        // SIMD load/store
        __m128 Load() const { return _mm_load_ps(&x); }
        void Store(const __m128 vec) { _mm_store_ps(&x, vec); }

        // Vector operations (SIMD optimized)
        Vector4f operator+(const Vector4f& other) const {
            return Vector4f(_mm_add_ps(Load(), other.Load()));
        }
        Vector4f operator-(const Vector4f& other) const {
            return Vector4f(_mm_sub_ps(Load(), other.Load()));
        }
        Vector4f operator*(const float scalar) const {
            return Vector4f(_mm_mul_ps(Load(), _mm_set1_ps(scalar)));
        }
        Vector4f operator/(const float scalar) const {
            assert(scalar != 0.0f);
            return Vector4f(_mm_div_ps(Load(), _mm_set1_ps(scalar)));
        }

        // Dot product (SIMD optimized)
        float Dot(const Vector4f& other) const {
            __m128 prod = _mm_mul_ps(Load(), other.Load());
            prod = _mm_hadd_ps(prod, prod);
            prod = _mm_hadd_ps(prod, prod);
            return _mm_cvtss_f32(prod);
        }

        // Unary minus operator
        Vector4f operator-() const {
            return Vector4f(-x, -y, -z, -w);
        }

        // Data members
        float x, y, z, w;
    };

    // ==================== Matrix4x4 ====================
    class alignas(16) Matrix4x4 {
        public:

        // Constructors
        constexpr Matrix4x4() {
            // Identity matrix
            m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = 0;
            m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = 0;
            m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
            m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
        }

        explicit Matrix4x4(const Diligent::float4x4& mat) {
            for (int i = 0; i < 16; ++i) {
                m[i] = (&mat._11)[i];
            }
        }

        // Inverse of the matrix
        Matrix4x4 Inverse() const {
            Matrix4x4 result;

            // Calculate the determinant
            const float det =
                m[0] * (m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[6] * m[9] * m[15] +
                        m[6] * m[11] * m[13] + m[7] * m[9] * m[14] - m[7] * m[10] * m[13]) -
                m[1] * (m[4] * m[10] * m[15] - m[4] * m[11] * m[14] - m[6] * m[8] * m[15] +
                        m[6] * m[11] * m[12] + m[7] * m[8] * m[14] - m[7] * m[10] * m[12]) +
                m[2] * (m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[5] * m[8] * m[15] +
                        m[5] * m[11] * m[12] + m[7] * m[8] * m[13] - m[7] * m[9] * m[12]) -
                m[3] * (m[4] * m[9] * m[14] - m[4] * m[10] * m[13] - m[5] * m[8] * m[14] +
                        m[5] * m[10] * m[12] + m[6] * m[8] * m[13] - m[6] * m[9] * m[12]);

            // If determinant is zero, the matrix is not invertible
            if (std::abs(det) < 1e-6) {
                return Matrix4x4(); // Return identity matrix as fallback
            }

            const float invDet = 1.0f / det;

            // Calculate the adjugate matrix
            result.m[0] = (m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[6] * m[9] * m[15] +
                           m[6] * m[11] * m[13] + m[7] * m[9] * m[14] - m[7] * m[10] * m[13]) * invDet;
            result.m[1] = (m[1] * m[10] * m[15] - m[1] * m[11] * m[14] - m[2] * m[9] * m[15] +
                           m[2] * m[11] * m[13] + m[3] * m[9] * m[14] - m[3] * m[10] * m[13]) * -invDet;
            result.m[2] = (m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[2] * m[5] * m[15] +
                           m[2] * m[7] * m[13] + m[3] * m[5] * m[14] - m[3] * m[6] * m[13]) * invDet;
            result.m[3] = (m[1] * m[6] * m[11] - m[1] * m[7] * m[10] - m[2] * m[5] * m[11] +
                           m[2] * m[7] * m[9] + m[3] * m[5] * m[10] - m[3] * m[6] * m[9]) * -invDet;
            result.m[4] = (m[4] * m[10] * m[15] - m[4] * m[11] * m[14] - m[6] * m[8] * m[15] +
                           m[6] * m[11] * m[12] + m[7] * m[8] * m[14] - m[7] * m[10] * m[12]) * -invDet;
            result.m[5] = (m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[2] * m[8] * m[15] +
                           m[2] * m[11] * m[12] + m[3] * m[8] * m[14] - m[3] * m[10] * m[12]) * invDet;
            result.m[6] = (m[0] * m[6] * m[15] - m[0] * m[7] * m[14] - m[2] * m[4] * m[15] +
                           m[2] * m[7] * m[12] + m[3] * m[4] * m[14] - m[3] * m[6] * m[12]) * -invDet;
            result.m[7] = (m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[2] * m[4] * m[11] +
                           m[2] * m[7] * m[8] + m[3] * m[4] * m[10] - m[3] * m[6] * m[8]) * invDet;
            result.m[8] = (m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[5] * m[8] * m[15] +
                           m[5] * m[11] * m[12] + m[7] * m[8] * m[13] - m[7] * m[9] * m[12]) * invDet;
            result.m[9] = (m[0] * m[9] * m[15] - m[0] * m[11] * m[13] - m[1] * m[8] * m[15] +
                           m[1] * m[11] * m[12] + m[3] * m[8] * m[13] - m[3] * m[9] * m[12]) * -invDet;
            result.m[10] = (m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[1] * m[4] * m[15] +
                            m[1] * m[7] * m[12] + m[3] * m[4] * m[13] - m[3] * m[5] * m[12]) * invDet;
            result.m[11] = (m[0] * m[5] * m[11] - m[0] * m[7] * m[9] - m[1] * m[4] * m[11] +
                            m[1] * m[7] * m[8] + m[3] * m[4] * m[9] - m[3] * m[5] * m[8]) * -invDet;
            result.m[12] = (m[4] * m[9] * m[14] - m[4] * m[10] * m[13] - m[5] * m[8] * m[14] +
                            m[5] * m[10] * m[12] + m[6] * m[8] * m[13] - m[6] * m[9] * m[12]) * -invDet;
            result.m[13] = (m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[1] * m[8] * m[14] +
                            m[1] * m[10] * m[12] + m[2] * m[8] * m[13] - m[2] * m[9] * m[12]) * invDet;
            result.m[14] = (m[0] * m[5] * m[14] - m[0] * m[6] * m[13] - m[1] * m[4] * m[14] +
                            m[1] * m[6] * m[12] + m[2] * m[4] * m[13] - m[2] * m[5] * m[12]) * -invDet;
            result.m[15] = (m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[1] * m[4] * m[10] +
                            m[1] * m[6] * m[8] + m[2] * m[4] * m[9] - m[2] * m[5] * m[8]) * invDet;

            return result;
        }

        // Conversion to Diligent type
        explicit operator Diligent::float4x4() const {
            Diligent::float4x4 result;
            for (int i = 0; i < 16; ++i) {
                (&result._11)[i] = m[i];
            }
            return result;
        }

        // Column access
        Vector4f Column(const int col) const {
            assert(col >= 0 && col < 4);
            const int idx = col * 4;
            return Vector4f(m[idx], m[idx+1], m[idx+2], m[idx+3]);
        }

        // Row access
        Vector4f Row(const int row) const {
            assert(row >= 0 && row < 4);
            return Vector4f(m[row], m[row+4], m[row+8], m[row+12]);
        }

        static Matrix4x4 Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane, bool isGL = false) {
            Matrix4x4 result;
            result.m[0] = 2.0f / (right - left);
            result.m[5] = 2.0f / (top - bottom);
            result.m[10] = (isGL ? 2.0f : -2.0f) / (farPlane - nearPlane);
            result.m[12] = -(right + left) / (right - left);
            result.m[13] = -(top + bottom) / (top - bottom);
            result.m[14] = (isGL ? -(farPlane + nearPlane) : farPlane + nearPlane) / (farPlane - nearPlane);
            result.m[15] = 1.0f;
            return result;
        }

        static Matrix4x4 RotationArbitrary(const Vector3f& axis, float angle) {
            const Vector3f normAxis = axis.Normalized();
            const float c = std::cos(angle);
            const float s = std::sin(angle);
            const float t = 1.0f - c;
            const float x = normAxis.x;
            const float y = normAxis.y;
            const float z = normAxis.z;

            Matrix4x4 result;
            result.m[0] = t*x*x + c;    result.m[4] = t*x*y - s*z;  result.m[8] = t*x*z + s*y;  result.m[12] = 0;
            result.m[1] = t*x*y + s*z;  result.m[5] = t*y*y + c;    result.m[9] = t*y*z - s*x;  result.m[13] = 0;
            result.m[2] = t*x*z - s*y;  result.m[6] = t*y*z + s*x;  result.m[10] = t*z*z + c;   result.m[14] = 0;
            result.m[3] = 0;            result.m[7] = 0;            result.m[11] = 0;           result.m[15] = 1;
            return result;
        }

        // Matrix multiplication (SIMD optimized)
        Matrix4x4 operator*(const Matrix4x4& other) const {
            Matrix4x4 result;

            // Load columns of the right matrix
            __m128 col0 = _mm_load_ps(other.m);
            __m128 col1 = _mm_load_ps(other.m + 4);
            __m128 col2 = _mm_load_ps(other.m + 8);
            __m128 col3 = _mm_load_ps(other.m + 12);

            for (int i = 0; i < 4; ++i) {
                __m128 row = _mm_set1_ps(m[i*4]);
                __m128 mul0 = _mm_mul_ps(row, col0);

                row = _mm_set1_ps(m[i*4+1]);
                __m128 mul1 = _mm_mul_ps(row, col1);

                row = _mm_set1_ps(m[i*4+2]);
                __m128 mul2 = _mm_mul_ps(row, col2);

                row = _mm_set1_ps(m[i*4+3]);
                __m128 mul3 = _mm_mul_ps(row, col3);

                __m128 sum = _mm_add_ps(_mm_add_ps(mul0, mul1), _mm_add_ps(mul2, mul3));
                _mm_store_ps(result.m + i*4, sum);
            }

            return result;
        }

        // Vector transformation (SIMD optimized)
        Vector4f operator*(const Vector4f& vec) const {
            const __m128 v = vec.Load();

            const __m128 row0 = _mm_load_ps(m);
            const __m128 row1 = _mm_load_ps(m + 4);
            const __m128 row2 = _mm_load_ps(m + 8);
            const __m128 row3 = _mm_load_ps(m + 12);

            const __m128 res = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)), row0),
                    _mm_mul_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)), row1)
                ),
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)), row2),
                    _mm_mul_ps(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)), row3)
                )
            );

            return Vector4f(res);
        }

        // Factory methods
        static Matrix4x4 Identity() { return Matrix4x4(); }
        static Matrix4x4 Translation(const Vector3f& translation) {
            Matrix4x4 result = Identity();
            result.m[12] = translation.x;
            result.m[13] = translation.y;
            result.m[14] = translation.z;
            return result;
        }
        static Matrix4x4 Scale(const Vector3f& scale) {
            Matrix4x4 result = Identity();
            result.m[0] = scale.x;
            result.m[5] = scale.y;
            result.m[10] = scale.z;
            return result;
        }
        static Matrix4x4 RotationX(const float angle) {
            const float c = std::cos(angle);
            const float s = std::sin(angle);
            Matrix4x4 result = Identity();
            result.m[5] = c;
            result.m[6] = s;
            result.m[9] = -s;
            result.m[10] = c;
            return result;
        }
        static Matrix4x4 RotationY(const float angle) {
            const float c = std::cos(angle);
            const float s = std::sin(angle);
            Matrix4x4 result = Identity();
            result.m[0] = c;
            result.m[2] = -s;
            result.m[8] = s;
            result.m[10] = c;
            return result;
        }
        static Matrix4x4 RotationZ(const float angle) {
            const float c = std::cos(angle);
            const float s = std::sin(angle);
            Matrix4x4 result = Identity();
            result.m[0] = c;
            result.m[1] = s;
            result.m[4] = -s;
            result.m[5] = c;
            return result;
        }
        static Matrix4x4 Perspective(const float fov, const float aspect, const float nearPlane, const float farPlane) {
            const float tanHalfFov = std::tan(fov / 2.0f);
            Matrix4x4 result;
            result.m[0] = 1.0f / (aspect * tanHalfFov);
            result.m[5] = 1.0f / tanHalfFov;
            result.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
            result.m[11] = -1.0f;
            result.m[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
            result.m[15] = 0.0f;
            return result;
        }
        static Matrix4x4 LookAt(const Vector3f& eye, const Vector3f& target, const Vector3f& up) {
            const Vector3f f = (target - eye).Normalized();
            const Vector3f s = f.Cross(up).Normalized();
            const Vector3f u = s.Cross(f);

            Matrix4x4 result;
            result.m[0] = s.x;
            result.m[4] = s.y;
            result.m[8] = s.z;
            result.m[1] = u.x;
            result.m[5] = u.y;
            result.m[9] = u.z;
            result.m[2] = -f.x;
            result.m[6] = -f.y;
            result.m[10] = -f.z;
            result.m[12] = -s.Dot(eye);
            result.m[13] = -u.Dot(eye);
            result.m[14] = f.Dot(eye);
            return result;
        }

        // Data access
        float& operator()(const int row, const int col) {
            assert(row >= 0 && row < 4 && col >= 0 && col < 4);
            return m[col * 4 + row];
        }
        const float& operator()(const int row, const int col) const {
            assert(row >= 0 && row < 4 && col >= 0 && col < 4);
            return m[col * 4 + row];
        }

        // Raw data access
        float* Data() { return m; }
        const float* Data() const { return m; }

        private:
        float m[16];  // Column-major storage


    };

    // ==================== Quaternion ====================
    class alignas(16) Quaternion {
        public:
        // Constructors
        constexpr Quaternion() : x(0), y(0), z(0), w(1) {}
        constexpr Quaternion(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
        explicit Quaternion(const Diligent::Quaternion<float>& q) : x(q.q.x), y(q.q.y), z(q.q.z), w(q.q.w) {}

        // Conversion to Diligent type
        explicit operator Diligent::Quaternion<float>() const {
            Diligent::Quaternion<float> result;
            result.q.x = x;
            result.q.y = y;
            result.q.z = z;
            result.q.w = w;
            return result;
        }

        // Factory methods
        static Quaternion Identity() { return Quaternion(); }
        static Quaternion FromAxisAngle(const Vector3f& axis, const float angle) {
            const Vector3f normAxis = axis.Normalized();
            const float halfAngle = angle * 0.5f;
            const float s = std::sin(halfAngle);
            return Quaternion(
                normAxis.x * s,
                normAxis.y * s,
                normAxis.z * s,
                std::cos(halfAngle)
            );
        }
        static Quaternion FromEuler(const float pitch, const float yaw, const float roll) {
            const float cy = std::cos(yaw * 0.5f);
            const float sy = std::sin(yaw * 0.5f);
            const float cp = std::cos(pitch * 0.5f);
            const float sp = std::sin(pitch * 0.5f);
            const float cr = std::cos(roll * 0.5f);
            const float sr = std::sin(roll * 0.5f);

            return Quaternion(
                cy * cp * sr + sy * sp * cr,
                sy * cp * cr - cy * sp * sr,
                cy * sp * cr + sy * cp * sr,
                cy * cp * cr - sy * sp * sr
            );
        }

        // Add this static method to the Quaternion class
        static Quaternion FromMatrix(const Matrix4x4& matrix) {
            const float m00 = matrix(0,0);
            const float m01 = matrix(0,1);
            const float m02 = matrix(0,2);
            const float m10 = matrix(1,0);
            const float m11 = matrix(1,1);
            const float m12 = matrix(1,2);
            const float m20 = matrix(2,0);
            const float m21 = matrix(2,1);
            const float m22 = matrix(2,2);

            const float trace = m00 + m11 + m22;
            Quaternion q;

            if (trace > 0) {
                const float s = 0.5f / sqrtf(trace + 1.0f);
                q.w = 0.25f / s;
                q.x = (m21 - m12) * s;
                q.y = (m02 - m20) * s;
                q.z = (m10 - m01) * s;
            } else {
                if (m00 > m11 && m00 > m22) {
                    const float s = 2.0f * sqrtf(1.0f + m00 - m11 - m22);
                    q.w = (m21 - m12) / s;
                    q.x = 0.25f * s;
                    q.y = (m01 + m10) / s;
                    q.z = (m02 + m20) / s;
                } else if (m11 > m22) {
                    const float s = 2.0f * sqrtf(1.0f + m11 - m00 - m22);
                    q.w = (m02 - m20) / s;
                    q.x = (m01 + m10) / s;
                    q.y = 0.25f * s;
                    q.z = (m12 + m21) / s;
                } else {
                    const float s = 2.0f * sqrtf(1.0f + m22 - m00 - m11);
                    q.w = (m10 - m01) / s;
                    q.x = (m02 + m20) / s;
                    q.y = (m12 + m21) / s;
                    q.z = 0.25f * s;
                }
            }

            return q.Normalized();
        }

        // Quaternion operations
        Quaternion operator*(const Quaternion& other) const {
            return Quaternion(
                w * other.x + x * other.w + y * other.z - z * other.y,
                w * other.y - x * other.z + y * other.w + z * other.x,
                w * other.z + x * other.y - y * other.x + z * other.w,
                w * other.w - x * other.x - y * other.y - z * other.z
            );
        }

        Vector3f operator*(const Vector3f& vec) const {
            // Extract vector part
            const Vector3f u(x, y, z);
            const float s = w;

            // Quaternion rotation formula: v' = v + 2u × (u × v + s*v)
            const Vector3f vprime = vec + u.Cross(u.Cross(vec) + vec * s) * 2.0f;
            return vprime;
        }

        // Normalization
        Quaternion Normalized() const {
            const float len = std::sqrt(x*x + y*y + z*z + w*w);
            if (len > 0.0f) {
                const float inv = 1.0f / len;
                return Quaternion(x * inv, y * inv, z * inv, w * inv);
            }
            return Identity();
        }

        // Conjugate
        Quaternion Conjugate() const {
            return Quaternion(-x, -y, -z, w);
        }

        // To matrix
        Matrix4x4 ToMatrix() const {
            const float xx = x * x;
            const float xy = x * y;
            const float xz = x * z;
            const float xw = x * w;
            const float yy = y * y;
            const float yz = y * z;
            const float yw = y * w;
            const float zz = z * z;
            const float zw = z * w;

            Matrix4x4 result = Matrix4x4::Identity();
            result(0,0) = 1 - 2 * (yy + zz);
            result(1,0) = 2 * (xy - zw);
            result(2,0) = 2 * (xz + yw);
            result(0,1) = 2 * (xy + zw);
            result(1,1) = 1 - 2 * (xx + zz);
            result(2,1) = 2 * (yz - xw);
            result(0,2) = 2 * (xz - yw);
            result(1,2) = 2 * (yz + xw);
            result(2,2) = 1 - 2 * (xx + yy);
            return result;
        }

        // Data members
        float x, y, z, w;
    };

    // ==================== Utility Functions ====================
    inline float DegreesToRadians(const float degrees) {
        return degrees * (3.14159265358979323846f / 180.0f);
    }

    inline float RadiansToDegrees(const float radians) {
        return radians * (180.0f / 3.14159265358979323846f);
    }

    // Linear interpolation
    template<typename T>
    inline T Lerp(const T& a, const T& b, float t) {
        return a + (b - a) * t;
    }

    // Spherical linear interpolation for quaternions
    inline Quaternion Slerp(const Quaternion& a, const Quaternion& b, const float t) {
        // Calculate cosine
        float cosTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

        // Adjust for shortest path
        const float sign = (cosTheta < 0.0f) ? -1.0f : 1.0f;
        cosTheta *= sign;

        // Calculate interpolation factors
        float c1 = 1.0f - t;
        float c2 = t;

        // Use linear interpolation for small angles
        if (cosTheta > 0.9995f) {
            const Quaternion result(
                c1 * a.x + c2 * b.x * sign,
                c1 * a.y + c2 * b.y * sign,
                c1 * a.z + c2 * b.z * sign,
                c1 * a.w + c2 * b.w * sign
            );
            return result.Normalized();
        }

        // Calculate spherical interpolation
        const float theta = std::acos(cosTheta);
        const float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
        c1 = std::sin((1.0f - t) * theta) / sinTheta;
        c2 = std::sin(t * theta) / sinTheta;

        const Quaternion result(
            c1 * a.x + c2 * b.x * sign,
            c1 * a.y + c2 * b.y * sign,
            c1 * a.z + c2 * b.z * sign,
            c1 * a.w + c2 * b.w * sign
        );

        return result;
    }
}