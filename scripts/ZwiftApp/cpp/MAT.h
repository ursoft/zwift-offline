#pragma once
template <class T, int dim>
struct tVEC {
    T m_data[dim];
    T lenSquared() { 
        T ret{};
        for (auto i : m_data)
            ret += i * i;
        return ret;
    }
    float len() { return sqrt(lenSquared()); }
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
using VEC2  = tVEC2<float>;
using VEC4i = tVEC4<int>;
using VEC3i = tVEC3<int>;
using VEC2i = tVEC2<int>;
struct MATRIX44 { VEC4 m_data[4]; };
struct MATRIX43 { VEC4 m_data[3]; };
struct MATRIX33 { VEC3 m_data[3]; };

void MAT_GetEulerZXY(VEC3 *, const MATRIX33 &);
void MAT_GetRotX(const MATRIX33 &);
void MAT_GetRotZ(const MATRIX33 &);
void MAT_InvLookAt(MATRIX44 *, const VEC3 &, const VEC3 &, const VEC3 &);
void MAT_Invert44(MATRIX44 *, const MATRIX44 &);
void MAT_MulMat(MATRIX44 *, const MATRIX44 &, const MATRIX44 &);
//void MAT_MulMat43<MATRIX44, MATRIX44, MATRIX44>(MATRIX44 *, const MATRIX44 &, const MATRIX44 &);
void MAT_MulMat44(MATRIX44 *, const MATRIX44 &, const MATRIX44 &);
void MAT_MulVecXYZW(VEC4 *, const VEC4 &, const MATRIX44 &);
void MAT_Set33_EulerXY(MATRIX33 *, float, float);
void MAT_Set33_Zero(MATRIX33 *);
void MAT_Set43(MATRIX43 *, const MATRIX44 &);
void MAT_Set43_Identity(MATRIX43 *);
void MAT_Set43_Zero(MATRIX43 *);
void MAT_Set44_AxisAngle(MATRIX44 *, float, float, float, float);
void MAT_Set44_AxisAngle(MATRIX44 *, float, float, float, float);
void MAT_Set44_AxisAngleSinCos(MATRIX44 *, float, float, float, float, float);
void MAT_Set44_EulerXY(MATRIX44 *, float, float);
void MAT_Set44_Identity(MATRIX44 *);
void MAT_Set44_RotX(MATRIX44 *, float);
void MAT_Set44_RotY(MATRIX44 *, float);
void MAT_Set44_RotZ(MATRIX44 *, float);
void MAT_Set44_Translate(MATRIX44 *, const VEC3 &);
void MAT_Set44_Zero(MATRIX44 *);