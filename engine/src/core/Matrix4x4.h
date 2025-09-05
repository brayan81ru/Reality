#pragma once
#include <DiligentCore/Common/interface/BasicMath.hpp>
#include <core/Vector3.h>
#include <core/Quaternion.h>
#include <cstring>

namespace Reality {
    class Matrix4x4 {
        float m[16]{}; // Column-major storage (matches Diligent)

    public:
        // ================ CONSTRUCTORS ================
        Matrix4x4() { SetIdentity(); }

        explicit Matrix4x4(const Diligent::float4x4& mat) {
            memcpy(m, &mat._11, 16 * sizeof(float));
        }

        // ================ FACTORY METHODS ================
        static Matrix4x4 Identity() {
            return {};
        }

        static Matrix4x4 Translation(const Vector3& translation) {
            Matrix4x4 result;
            result.m[12] = translation.x;
            result.m[13] = translation.y;
            result.m[14] = translation.z;
            return result;
        }

        static Matrix4x4 Rotation(const Quat& rotation) {
            return Matrix4x4(rotation.ToDiligentMatrix());
        }

        static Matrix4x4 Scale(const Vector3& scale) {
            Matrix4x4 result;
            result.m[0] = scale.x;
            result.m[5] = scale.y;
            result.m[10] = scale.z;
            return result;
        }

        static Matrix4x4 TRS(const Vector3& translation, const Quat& rotation, const Vector3& scale) {
            return Translation(translation) * Rotation(rotation) * Scale(scale);
        }

        static Matrix4x4 Perspective(const float fovDegrees, const float aspectRatio, const float nearClip, const float farClip) {
            Matrix4x4 result;
            const float fovRad = fovDegrees * (3.14159265358979323846f / 180.0f);
            const float tanHalfFov = tanf(fovRad / 2.0f);

            result.m[0] = 1.0f / (aspectRatio * tanHalfFov);
            result.m[5] = 1.0f / tanHalfFov;
            result.m[10] = -(farClip + nearClip) / (farClip - nearClip);
            result.m[11] = -1.0f;
            result.m[14] = -(2.0f * farClip * nearClip) / (farClip - nearClip);
            result.m[15] = 0.0f;

            return result;
        }

        static Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
            const Vector3 z = (eye - target).Normalized();
            const Vector3 x = up.Cross(z).Normalized();
            const Vector3 y = z.Cross(x);

            Matrix4x4 result;
            result.m[0] = x.x; result.m[4] = x.y; result.m[8] = x.z;  result.m[12] = -x.Dot(eye);
            result.m[1] = y.x; result.m[5] = y.y; result.m[9] = y.z;  result.m[13] = -y.Dot(eye);
            result.m[2] = z.x; result.m[6] = z.y; result.m[10] = z.z; result.m[14] = -z.Dot(eye);
            result.m[3] = 0;   result.m[7] = 0;   result.m[11] = 0;   result.m[15] = 1.0f;

            return result;
        }

        // ================ OPERATIONS ================
        Matrix4x4 operator*(const Matrix4x4& rhs) const {
            Matrix4x4 result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.m[i*4+j] = 0;
                    for (int k = 0; k < 4; ++k) {
                        result.m[i*4+j] += m[k*4+j] * rhs.m[i*4+k];
                    }
                }
            }
            return result;
        }

        [[nodiscard]] Vector3 MultiplyPoint(const Vector3& point) const {
            const float x = m[0]*point.x + m[4]*point.y + m[8]*point.z + m[12];
            const float y = m[1]*point.x + m[5]*point.y + m[9]*point.z + m[13];
            const float z = m[2]*point.x + m[6]*point.y + m[10]*point.z + m[14];
            const float w = m[3]*point.x + m[7]*point.y + m[11]*point.z + m[15];

            if (w != 0.0f) {
                return {x/w, y/w, z/w};
            }
            return {x, y, z};
        }

        [[nodiscard]] Vector3 MultiplyVector(const Vector3& vector) const {
            return {
                m[0]*vector.x + m[4]*vector.y + m[8]*vector.z,
                m[1]*vector.x + m[5]*vector.y + m[9]*vector.z,
                m[2]*vector.x + m[6]*vector.y + m[10]*vector.z
            };
        }

        [[nodiscard]] Matrix4x4 Transposed() const {
            Matrix4x4 result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.m[j*4+i] = m[i*4+j];
                }
            }
            return result;
        }

        // ================ CONVERSIONS ================
        operator Diligent::float4x4() const {
            Diligent::float4x4 result;
            memcpy(&result._11, m, 16 * sizeof(float));
            return result;
        }

        // ================ ACCESSORS ================
        float* Data() { return m; }
        [[nodiscard]] const float* Data() const { return m; }
        float& operator()(const int row, const int col) { return m[col*4+row]; }
        const float& operator()(const int row, const int col) const { return m[col*4+row]; }

    private:
        void SetIdentity() {
            m[0]=1; m[4]=0; m[8]=0;  m[12]=0;
            m[1]=0; m[5]=1; m[9]=0;  m[13]=0;
            m[2]=0; m[6]=0; m[10]=1; m[14]=0;
            m[3]=0; m[7]=0; m[11]=0; m[15]=1;
        }
    };
}