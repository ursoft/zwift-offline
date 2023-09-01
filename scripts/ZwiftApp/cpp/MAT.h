#pragma once //READY for testing
template <class T, int dim>
struct tVEC {
    T m_data[dim];
    T lenSquared() const { 
        T ret{};
        for (auto i : m_data)
            ret += i * i;
        return ret;
    }
    T operator *(const tVEC &mul) const {
        T ret{};
        for (int i = 0; i < dim; i++)
            ret += m_data[i] * mul.m_data[i];
        return ret;
    }
    tVEC &operator *=(const T mul) {
        for (auto &i : m_data)
            i *= mul;
        return *this;
    }
    tVEC &operator /=(const T div) {
        for (auto &i : m_data)
            i /= div;
        return *this;
    }
    tVEC &operator -=(const tVEC &b) {
        for (int i = 0; i < dim; i++)
            m_data[i] -= b.m_data[i];
        return *this;
    }
    float len() const { return sqrt(lenSquared()); }
    void Normalize() {
        auto lsq = lenSquared();
        if (lsq != 0.0f) {
            lsq = sqrtf(lsq);
            (*this) /= lsq;
        }
    }
    tVEC Normalized() const {
        tVEC ret(*this);
        auto lsq = ret.lenSquared();
        if (lsq != 0.0f) {
            lsq = sqrtf(lsq);
            ret /= lsq;
        }
        return ret;
    }
    tVEC operator -() const {
        tVEC ret;
        for (int i = 0; i < dim; i++)
            ret.m_data[i] = -m_data[i];
        return ret;
    }
    tVEC operator *(float m) const {
        tVEC ret;
        for (int i = 0; i < dim; i++)
            ret.m_data[i] = m * m_data[i];
        return ret;
    }
    bool Empty() const {
        for (auto i : m_data)
            if (i != 0.0f)
                return false;
        return true;
    }
};
template <class T, int dim>
tVEC<T, dim> operator-(const tVEC<T, dim> &a, const tVEC<T, dim> &b) { 
    tVEC<T, dim> ret;
    for (int i = 0; i < dim; i++)
        ret.m_data[i] = a.m_data[i] - b.m_data[i];
    return ret;
}
template <class T, int dim>
tVEC<T, dim> operator+(const tVEC<T, dim> &a, const tVEC<T, dim> &b) { 
    tVEC<T, dim> ret;
    for (int i = 0; i < dim; i++)
        ret.m_data[i] = a.m_data[i] + b.m_data[i];
    return ret;
}
template<class T> using tVEC2 = tVEC<T, 2>;
template<class T> using tVEC3 = tVEC<T, 3>;
template<class T> using tVEC4 = tVEC<T, 4>;
using VEC4  = tVEC4<float>;
using VEC3  = tVEC3<float>;
inline VEC3 SafeNormalized(VEC3 *src) {
    if (src->Empty())
        src->m_data[0] = 1.0;
    return src->Normalized();
}
using VEC2  = tVEC2<float>;
using VEC4i = tVEC4<int>;
using VEC3i = tVEC3<int>;
using VEC2i = tVEC2<int>;
struct MATRIX44 { VEC4 m_data[4]; };
struct MATRIX43 { VEC4 m_data[3]; };
struct MATRIX33 { VEC3 m_data[3]; };
inline VEC3 operator *(const MATRIX44 &mx, const VEC3 &v) {
    return VEC3{ v.m_data[0] * mx.m_data[0].m_data[0] + mx.m_data[1].m_data[0] * v.m_data[1] + mx.m_data[2].m_data[0] * v.m_data[2] + mx.m_data[3].m_data[0],
                 v.m_data[0] * mx.m_data[0].m_data[1] + mx.m_data[1].m_data[1] * v.m_data[1] + mx.m_data[2].m_data[1] * v.m_data[2] + mx.m_data[3].m_data[1],
                 v.m_data[0] * mx.m_data[0].m_data[2] + mx.m_data[1].m_data[2] * v.m_data[1] + mx.m_data[2].m_data[2] * v.m_data[2] + mx.m_data[3].m_data[2] };
}
struct Sphere { //16 bytes
    VEC3 m_center;
    float m_radius;
};
struct MU_OscillatorVector {
    VEC3 m_vec;
};
template <class T> struct tViewport {
    T m_left, m_top, m_width, m_height;
};
using RECT2 = tViewport<float>;
void MAT_GetEulerZXY(VEC3 *, const MATRIX33 &);
float MAT_GetRotX(const MATRIX33 &);
float MAT_GetRotZ(const MATRIX33 &);
void MAT_InvLookAt(MATRIX44 *, const VEC3 &, const VEC3 &, const VEC3 &);
bool MAT_Invert44(MATRIX44 *, const MATRIX44 &);
void MAT_MulMat(MATRIX44 *, const MATRIX44 &, const MATRIX44 &);
void MAT_MulVecXYZW(VEC4 *, const VEC4 &, const MATRIX44 &);
void MAT_Set33_EulerXY(MATRIX33 *, float, float);
void MAT_Set33_Zero(MATRIX33 *);
void MAT_Set43(MATRIX43 *, const MATRIX44 &);
void MAT_Set43_Identity(MATRIX43 *);
void MAT_Set43_Zero(MATRIX43 *);
void MAT_Set44_AxisAngle(MATRIX44 *, float, float, float, float);
void MAT_Set44_EulerXY(MATRIX44 *, float, float);
void MAT_Set44_Identity(MATRIX44 *);
void MAT_Set44_RotX(MATRIX44 *, float);
void MAT_Set44_RotY(MATRIX44 *, float);
void MAT_Set44_RotZ(MATRIX44 *, float);
void MAT_Set44_Translate(MATRIX44 *, const VEC3 &);
void MAT_Set44_Zero(MATRIX44 *);
void MU_FastSinCos(float *pSin, float *pCos, float angle);
bool MAT_IsOrthonormal(const MATRIX33 & mat);
bool MAT_IsOrthonormal(const MATRIX44 &mat);
void QUAT_SetMat(VEC4 *dest, const MATRIX33 &mat);
void QUAT_SetMat(VEC4 *dest, const MATRIX44 &mat);
void QUAT_MulQuat(VEC4 *dest, const VEC4 &a2, const VEC4 &a3);
void QUAT_Normalize(VEC4 *dest);
void QUAT_SetEulerXYZ(VEC4 *dest, const VEC3 &src);
void QUAT_Slerp(VEC4 *dest, const VEC4 &a2, const VEC4 &a3, float a4);
void QUAT_TransformVector(VEC3 *dest, const VEC4 &a2, const VEC3 &a3);
float MU_GetAngleBetweenVectors(const VEC3 &a1, const VEC3 &a2);
void MU_ClosestPointOnLineToPoint(VEC3 *dest, const VEC3 &a2, const VEC3 &a3, const VEC3 &a4);
float MU_GetSineValue(float a1, float a2, float a3);
float VEC_Length(const VEC3 &src);
float MU_EstCubeRootPositive(float x);  
float MU_FastSin(float angle);
void MU_FastSinCos(float *pSin, float *pCos, float angle);
void MU_AssertNAN(const VEC3 &v);
void VEC_GetPerp(VEC3 *dest, const VEC3 &a, const VEC3 &b);
void VEC_GetPerp(VEC3 *dest, const VEC3 &a);
void VEC_RotateXYZ_AroundAxis(VEC3 *dest, const VEC3 &a, const VEC3 &b, float angle);
void VEC_RotateXYZ_AroundAxis(VEC3 *dest, const VEC3 &a, const VEC3 &b, float sinx, float cosx);
void CheckedSincos(float angle, float *sinx, float *cosx);
void VEC_RotateXYZ_AroundAxis(VEC3 *dest, const VEC3 &a, const VEC3 &b, float angle);
void MU_ClosestPointOnLineToLine(VEC3 *dest, const VEC3 &a2, const VEC3 &a, float a4, const VEC3 &a5, const VEC3 &b, float a7);
void MU_DampedSpringSlerp3D(VEC3 *dest, float *a2, const VEC3 &a, float a4, float a5);
void VEC_SlerpXYZ(VEC3 *dest, const VEC3 &a, const VEC3 &b, float a4);
void VEC_SlerpXY(VEC2 *dest, const VEC2 &a2, const VEC2 &a3, float a4);
void VEC_SetXYZ(VEC3 *dest, float x, float y, float z);
void VEC_DirToYawPitch(float *a1, float *a2, const VEC3 &a3);
void MU_OscillatorResetRandom(MU_OscillatorVector *ov);
void QUAT_SetAxisAngle(VEC4 *dest, const VEC3 &a2, float angle2);
void QUAT_Add(VEC4 *dest, const VEC4 &a, const VEC4 &b);
void QUAT_Unpack(uint32_t src, VEC4 *dest);
uint32_t QUAT_Pack(const VEC4 &src);
void QUAT_SetXYZW(VEC4 *dest, float x, float y, float z, float w);
void QUAT_SetEulerZXY(VEC4 *dest, const VEC3 &src);
void QUAT_Scale(VEC4 *dest, const VEC4 &src, float mul);
void QUAT_MulQuatConjugate(VEC4 *dest, const VEC4 &a, const VEC4 &b);
float QUAT_Magnitude(const VEC4 &v);
void QUAT_Invert(VEC4 *dest, const VEC4 &src);
void QUAT_GetEulerZXY(VEC3 *dest, const VEC4 &src);
void QUAT_GetEulerXYZ(VEC3 *dest, const VEC4 &src);
void QUAT_GetAxisAngle(VEC3 *dest, float *a2, const VEC4 &src);
void QUAT_Exponent(VEC4 *dest, const VEC4 &src, float a3);
float QUAT_Dot(const VEC4 &a, const VEC4 &b);
void QUAT_Conjugate(VEC4 *dest, const VEC4 &src);
float QUAT_AngleDiff(const VEC4 &a, const VEC4 &b);
float MU_ClampedASIN(float sinx);
uint32_t MU_OffsetHash(uint32_t a1, int a2, int a3);
float MU_NormalizeAngle(float rad);
