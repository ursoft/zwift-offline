#pragma once
struct VEC4 { float m_data[4]; };
struct VEC3 {
    float m_data[3];
    float lenSquared() { return m_data[0] * m_data[0] + m_data[1] * m_data[1] + m_data[2] * m_data[2]; }
    float len() { return sqrt(lenSquared()); }
};
inline VEC3 operator-(const VEC3 &a, const VEC3 &b) { return VEC3{a.m_data[0] - b.m_data[0], a.m_data[1] - b.m_data[1], a.m_data[2] - b.m_data[2] }; }
inline VEC3 operator+(const VEC3 &a, const VEC3 &b) { return VEC3{a.m_data[0] + b.m_data[0], a.m_data[1] + b.m_data[1], a.m_data[2] + b.m_data[2] }; }
struct VEC2 { float m_data[2]; };
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