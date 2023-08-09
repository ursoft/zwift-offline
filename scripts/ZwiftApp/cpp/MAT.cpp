#pragma once //READY for testing
#include "MAT.h"
#include "Logging.h"
const VEC4 g_cident0{1.0f, 0.0f, 0.0f, 0.0f};
const VEC4 g_cident1{0.0f, 1.0f, 0.0f, 0.0f};
const VEC4 g_cident2{0.0f, 0.0f, 1.0f, 0.0f};
const VEC4 g_cident3{0.0f, 0.0f, 0.0f, 1.0f};
void MAT_MulMat(MATRIX44 *dest, const MATRIX44 &m1, const MATRIX44 &m2) {
    auto v3 = m1.m_data[0].m_data[2], v4 = m1.m_data[0].m_data[0], v32 = m2.m_data[0].m_data[1], v35 = m2.m_data[0].m_data[2], v31 = m2.m_data[0].m_data[3], v34 = m2.m_data[1].m_data[2], v30 = m2.m_data[1].m_data[3];
    auto v33 = m2.m_data[2].m_data[2], v5 = m1.m_data[0].m_data[3], v6 = m1.m_data[0].m_data[1], v7 = m2.m_data[2].m_data[0], v8 = m2.m_data[3].m_data[0], v9 = m2.m_data[0].m_data[0], v10 = m2.m_data[1].m_data[0], v11 = m2.m_data[2].m_data[1];
    auto v12 = m2.m_data[3].m_data[1], v13 = m2.m_data[1].m_data[1], v14 = m2.m_data[3].m_data[2], v29 = m2.m_data[2].m_data[3], v28 = m2.m_data[3].m_data[3], v15 = m1.m_data[0].m_data[0] * v32;
    dest->m_data[0].m_data[0] = v5 * v8 + v3 * v7 + v6 * v10 + m1.m_data[0].m_data[0] * m2.m_data[0].m_data[0];
    dest->m_data[0].m_data[1] = v5 * v12 + v3 * v11 + v6 * v13 + v15;
    dest->m_data[0].m_data[2] = v5 * v14 + v3 * v33 + v6 * v34 + v4 * v35;
    dest->m_data[0].m_data[3] = v5 * v28 + v3 * v29 + v6 * v30 + v4 * v31;
    auto v16 = m1.m_data[1].m_data[2];
    auto v17 = m1.m_data[1].m_data[3];
    auto v18 = m1.m_data[1].m_data[0];
    auto v19 = m1.m_data[1].m_data[1];
    dest->m_data[1].m_data[0] = v17 * v8 + v16 * v7 + v19 * v10 + v18 * v9;
    dest->m_data[1].m_data[1] = v17 * v12 + v16 * v11 + v19 * v13 + v18 * v32;
    dest->m_data[1].m_data[3] = v17 * v28 + v16 * v29 + v19 * v30 + v18 * v31;
    dest->m_data[1].m_data[2] = v17 * v14 + v16 * v33 + v19 * v34 + v18 * v35;
    auto v20 = m1.m_data[2].m_data[2];
    auto v21 = m1.m_data[2].m_data[3];
    auto v22 = m1.m_data[2].m_data[0];
    auto v23 = m1.m_data[2].m_data[1];
    dest->m_data[2].m_data[0] = v21 * v8 + v20 * v7 + v23 * v10 + v22 * v9;
    dest->m_data[2].m_data[1] = v21 * v12 + v20 * v11 + v23 * v13 + v22 * v32;
    dest->m_data[2].m_data[2] = v21 * v14 + v20 * v33 + v23 * v34 + v22 * v35;
    dest->m_data[2].m_data[3] = v21 * v28 + v20 * v29 + v23 * v30 + v22 * v31;
    auto v24 = m1.m_data[3].m_data[3];
    auto v25 = m1.m_data[3].m_data[1];
    auto v26 = m1.m_data[3].m_data[2];
    auto v27 = m1.m_data[3].m_data[0];
    dest->m_data[3].m_data[0] = v24 * v8 + v26 * v7 + v25 * v10 + v27 * v9;
    dest->m_data[3].m_data[1] = v24 * v12 + v26 * v11 + v25 * v13 + v27 * v32;
    dest->m_data[3].m_data[3] = v24 * v28 + v26 * v29 + v25 * v30 + v27 * v31;
    dest->m_data[3].m_data[2] = v24 * v14 + v26 * v33 + v25 * v34 + v27 * v35;
}
void MAT_MulVecXYZW(VEC4 *dest, const VEC4 &a2, const MATRIX44 &a3) {
    auto v3 = a2.m_data[0] * a3.m_data[0].m_data[1] + a2.m_data[1] * a3.m_data[1].m_data[1] + a2.m_data[2] * a3.m_data[2].m_data[1] + a2.m_data[3] * a3.m_data[3].m_data[1];
    auto v4 = a2.m_data[2];
    auto v5 = a2.m_data[0] * a3.m_data[0].m_data[2] + a2.m_data[1] * a3.m_data[1].m_data[2] + v4 * a3.m_data[2].m_data[2] + a2.m_data[3] * a3.m_data[3].m_data[2];
    auto v6 = a2.m_data[3];
    auto v7 = a2.m_data[0] * a3.m_data[0].m_data[3] + a2.m_data[1] * a3.m_data[1].m_data[3] + v4 * a3.m_data[2].m_data[3] + v6 * a3.m_data[3].m_data[3];
    dest->m_data[0] = a2.m_data[1] * a3.m_data[1].m_data[0] + a2.m_data[0] * a3.m_data[0].m_data[0] + v4 * a3.m_data[2].m_data[0] + v6 * a3.m_data[3].m_data[0];
    dest->m_data[1] = v3;
    dest->m_data[2] = v5;
    dest->m_data[3] = v7;
}
void MAT_Set44_AxisAngle(MATRIX44 *mx, float a2, float a3, float a4, float angle) {
    float sn, cs;
    __libm_sse2_sincosf_(angle, &sn, &cs);
    float v9 = sn * a2, v10 = sn * a3, v12 = sn * a4, csa = 1.0f - cs, v13 = csa * a3, v13a4 = v13 * a4, v14 = csa * a2;
    float v15 = v14 * a4, v17 = v14 * a3;
    mx->m_data[0].m_data[0] = v14 * a2 + cs;
    mx->m_data[0].m_data[1] = v12 + v17;
    mx->m_data[0].m_data[2] = v15 - v10;
    mx->m_data[0].m_data[3] = 0.0f;
    mx->m_data[1].m_data[0] = v17 - v12;
    mx->m_data[1].m_data[1] = v13 * a3 + cs;
    mx->m_data[1].m_data[2] = v9 + v13a4;
    mx->m_data[1].m_data[3] = 0.0f;
    mx->m_data[2].m_data[0] = v10 + v15;
    mx->m_data[2].m_data[1] = v13a4 - v9;
    mx->m_data[2].m_data[2] = csa * a4 * a4 + cs;
    mx->m_data[2].m_data[3] = 0.0f;
    mx->m_data[3].m_data[0] = 0.0f;
    mx->m_data[3].m_data[1] = 0.0f;
    mx->m_data[3].m_data[2] = 0.0f;
    mx->m_data[3].m_data[3] = 1.0f;
}
void MAT_GetEulerZXY(VEC3 *dest, const MATRIX33 &src) {
    dest->m_data[1] = atan2f(src.m_data[2].m_data[0], src.m_data[2].m_data[2]);
    float sinx, cosx;
    __libm_sse2_sincosf_(dest->m_data[1], &sinx, &cosx);
    auto v7 = sinx * src.m_data[0].m_data[0] + cosx * src.m_data[0].m_data[2];
    auto v8 = cosx * src.m_data[0].m_data[0] - sinx * src.m_data[0].m_data[2];
    dest->m_data[0] = atan2f(-src.m_data[2].m_data[1], sinx * src.m_data[2].m_data[0] + cosx * src.m_data[2].m_data[2]);
    __libm_sse2_sincosf_(dest->m_data[0], &sinx, &cosx);
    dest->m_data[2] = atan2f(v7 * sinx + src.m_data[0].m_data[1] * cosx, v8);
}
float MAT_GetRotX(const MATRIX33 &src) { return atan2f(-src.m_data[2].m_data[1], src.m_data[2].m_data[2]); }
float MAT_GetRotZ(const MATRIX33 &src) { return atan2f(src.m_data[0].m_data[1], src.m_data[0].m_data[0]); }
void MAT_InvLookAt(MATRIX44 *dest, const VEC3 &a2, const VEC3 &a3, const VEC3 &a4) {
    auto v10 = (a2 - a3).Normalized();
    VEC3 v16{
        a4.m_data[1] * v10.m_data[2] - a4.m_data[2] * v10.m_data[1],
        a4.m_data[2] * v10.m_data[0] - a4.m_data[0] * v10.m_data[2],
        a4.m_data[0] * v10.m_data[1] - a4.m_data[1] * v10.m_data[0] };
    v16.Normalize();
    MATRIX44 m1, m2;
    m1.m_data[0] = g_cident0;
    m1.m_data[1] = g_cident1;
    m1.m_data[2] = g_cident2;
    m1.m_data[3].m_data[0] = a2.m_data[0] * v16.m_data[0] + a2.m_data[1] * v16.m_data[1] + a2.m_data[2] * v16.m_data[2];
    m1.m_data[3].m_data[1] = m2.m_data[1].m_data[1] * a2.m_data[1] + m2.m_data[1].m_data[0] * a2.m_data[0] + m2.m_data[1].m_data[2] * a2.m_data[2];
    m1.m_data[3].m_data[2] = a2.m_data[1] * v10.m_data[1] + a2.m_data[0] * v10.m_data[0] + a2.m_data[2] * v10.m_data[2];
    m1.m_data[3].m_data[3] = 1.0f;
    m2.m_data[0] = { v16.m_data[0], v16.m_data[1],  v16.m_data[2], 0.0f };
    m2.m_data[1].m_data[0] = v10.m_data[1] * v16.m_data[2] - v10.m_data[2] * v16.m_data[1];
    m2.m_data[1].m_data[1] = v10.m_data[2] * v16.m_data[0] - v10.m_data[0] * v16.m_data[2];
    m2.m_data[1].m_data[2] = v10.m_data[0] * v16.m_data[1] - v10.m_data[1] * v16.m_data[0];
    m2.m_data[1].m_data[3] = 0.0f;
    m2.m_data[2] = { v10.m_data[0], v10.m_data[1],  v10.m_data[2], 0.0f };
    m2.m_data[3] = g_cident3;
    MAT_MulMat(dest, m1, m2);
}
bool MAT_Invert44(MATRIX44 *dest, const MATRIX44 &src) {
    auto v2 = src.m_data[0].m_data[3] * src.m_data[3].m_data[2];
    auto v3 = src.m_data[0].m_data[2] * src.m_data[3].m_data[3];
    auto v4 = src.m_data[2].m_data[3] * src.m_data[3].m_data[2];
    auto v5 = src.m_data[0].m_data[3] * src.m_data[2].m_data[2];
    auto v6 = src.m_data[3].m_data[1];
    auto v7 = src.m_data[2].m_data[2] * src.m_data[3].m_data[3];
    auto v8 = src.m_data[1].m_data[2] * src.m_data[3].m_data[3];
    auto v9 = src.m_data[1].m_data[3] * src.m_data[3].m_data[2];
    auto v10 = src.m_data[0].m_data[2] * src.m_data[2].m_data[3];
    auto v11 = src.m_data[0].m_data[2] * src.m_data[1].m_data[3];
    auto v12 = src.m_data[0].m_data[3] * src.m_data[1].m_data[2];
    auto v91 = src.m_data[1].m_data[2] * src.m_data[2].m_data[3];
    auto v89 = src.m_data[1].m_data[3] * src.m_data[2].m_data[2];
    MATRIX44 tmp;
    tmp.m_data[0].m_data[0] = v9 * src.m_data[2].m_data[1] + v7 * src.m_data[1].m_data[1] + v91 * v6
        - v4 * src.m_data[1].m_data[1] - v8 * src.m_data[2].m_data[1] - v89 * v6;
    auto v14 = src.m_data[0].m_data[1];
    tmp.m_data[0].m_data[1] = v4 * v14 + v3 * src.m_data[2].m_data[1] + v5 * v6 - v7 * v14 - v2 * src.m_data[2].m_data[1] - v10 * v6;
    tmp.m_data[0].m_data[2] = v8 * v14 + v2 * src.m_data[1].m_data[1] + v11 * v6 - v9 * v14 - v3 * src.m_data[1].m_data[1] + v12 * v6;
    tmp.m_data[0].m_data[3] = v89 * v14 + v10 * src.m_data[1].m_data[1] + v12 * src.m_data[2].m_data[1]
        - v91 * v14 - v5 * src.m_data[1].m_data[1] - v11 * src.m_data[2].m_data[1];
    tmp.m_data[1].m_data[0] = v4 * src.m_data[1].m_data[0] + v8 * src.m_data[2].m_data[0] + v89 * src.m_data[3].m_data[0]
        - v9 * src.m_data[2].m_data[0] - v7 * src.m_data[1].m_data[0] - v91 * src.m_data[3].m_data[0];
    auto v31 = src.m_data[1].m_data[0];
    auto v32 = src.m_data[2].m_data[0];
    tmp.m_data[1].m_data[1] = v7 * src.m_data[0].m_data[0] + v2 * v32 + v10 * src.m_data[3].m_data[0]
        - v4 * src.m_data[0].m_data[0] - v3 * v32 - v5 * src.m_data[3].m_data[0];
    auto v33 = src.m_data[3].m_data[0];
    auto v34 = src.m_data[0].m_data[0] * v6;
    auto v36 = v33 * src.m_data[2].m_data[1];
    auto v39 = v32 * v6;
    auto v40 = v32 * v14;
    auto v92 = v32 * src.m_data[1].m_data[1];
    auto v43 = v33 * src.m_data[1].m_data[1];
    auto v44 = src.m_data[0].m_data[0] * src.m_data[1].m_data[1];
    auto v46 = v31 * v6;
    auto v48 = v31 * src.m_data[2].m_data[1];
    tmp.m_data[1].m_data[3] = v91 * src.m_data[0].m_data[0] + v5 * v31 + v11 * v32 - v89 * src.m_data[0].m_data[0] - v10 * v31 - v12 * v32;
    auto v50 = v33 * v14;
    auto v52 = src.m_data[0].m_data[0] * src.m_data[2].m_data[1];
    tmp.m_data[1].m_data[2] = v9 * src.m_data[0].m_data[0] + v3 * v31 + v12 * v33 - v8 * src.m_data[0].m_data[0] - v2 * v31 + v11 * v33;
    auto v55 = v34 * src.m_data[2].m_data[3];
    auto v56 = v31 * v14;
    auto v57 = v40 * src.m_data[3].m_data[3];
    auto v58 = v50 * src.m_data[2].m_data[3];
    tmp.m_data[2].m_data[0] = v43 * src.m_data[2].m_data[3] + src.m_data[1].m_data[3] * v39 + v48 * src.m_data[3].m_data[3]
        - src.m_data[1].m_data[3] * v36 - v46 * src.m_data[2].m_data[3] - v92 * src.m_data[3].m_data[3];
    tmp.m_data[2].m_data[1] = v55 + src.m_data[0].m_data[3] * v36 + v57 - v58 - src.m_data[0].m_data[3] * v39 - v52 * src.m_data[3].m_data[3];
    tmp.m_data[2].m_data[2] = v50 * src.m_data[1].m_data[3] + src.m_data[0].m_data[3] * v46 + v44 * src.m_data[3].m_data[3]
        - v34 * src.m_data[1].m_data[3] - src.m_data[0].m_data[3] * v43 - v56 * src.m_data[3].m_data[3];
    tmp.m_data[2].m_data[3] = v92 * src.m_data[0].m_data[3] + v52 * src.m_data[1].m_data[3] + v56 * src.m_data[2].m_data[3]
        - v40 * src.m_data[1].m_data[3] - src.m_data[0].m_data[3] * v48 - v44 * src.m_data[2].m_data[3];
    auto v68 = src.m_data[0].m_data[2];
    tmp.m_data[3].m_data[0] = v92 * src.m_data[3].m_data[2] + v46 * src.m_data[2].m_data[2] + src.m_data[1].m_data[2] * v36
        - v48 * src.m_data[3].m_data[2] - src.m_data[1].m_data[2] * v39 - v43 * src.m_data[2].m_data[2];
    tmp.m_data[3].m_data[1] = v52 * src.m_data[3].m_data[2] + src.m_data[0].m_data[2] * v39 + v50 * src.m_data[2].m_data[2] 
        - v40 * src.m_data[3].m_data[2] - v34 * src.m_data[2].m_data[2] - v68 * v36;
    auto v75 = src.m_data[1].m_data[2];
    tmp.m_data[3].m_data[2] = v56 * src.m_data[3].m_data[2] + v34 * v75 + v68 * v43 - v44 * src.m_data[3].m_data[2] - v68 * v46 - v50 * v75;
    tmp.m_data[3].m_data[3] = v68 * v48 + v44 * src.m_data[2].m_data[2] + v40 * v75 - v52 * v75 - v56 * src.m_data[2].m_data[2] - v92 * v68;
    auto D = tmp.m_data[0].m_data[0] * src.m_data[0].m_data[0] + tmp.m_data[0].m_data[1] * src.m_data[1].m_data[0] 
        + tmp.m_data[0].m_data[2] * src.m_data[2].m_data[0] + src.m_data[3].m_data[0] * tmp.m_data[0].m_data[3];
    if (fabs(D) < 0.00001f)
        return false;
    D = 1.0f / D;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            dest->m_data[i].m_data[j] = D * tmp.m_data[i].m_data[j];
    return true;
}
void MAT_Set33_Zero(MATRIX33 *dest) { memset(dest, 0, sizeof(*dest)); }
void MAT_Set43_Zero(MATRIX43 *dest) { memset(dest, 0, sizeof(*dest)); }
void MAT_Set33_EulerXY(MATRIX33 *dest, float angleX, float angleY) {
    float sinx, cosx, siny, cosy;
    __libm_sse2_sincosf_(angleY, &siny, &cosy);
    __libm_sse2_sincosf_(angleX, &sinx, &cosx);
    dest->m_data[0].m_data[0] = cosy;
    dest->m_data[0].m_data[1] = 0.0f;
    dest->m_data[0].m_data[2] = -siny;
    dest->m_data[1].m_data[0] = sinx * siny;
    dest->m_data[1].m_data[1] = cosx;
    dest->m_data[1].m_data[2] = sinx * cosy;
    dest->m_data[2].m_data[0] = cosx * siny;
    dest->m_data[2].m_data[1] = -sinx;
    dest->m_data[2].m_data[2] = cosx * cosy;
}
void MAT_Set44_EulerXY(MATRIX44 *dest, float angleX, float angleY) {
    float sinx, cosx, siny, cosy;
    __libm_sse2_sincosf_(angleY, &siny, &cosy);
    __libm_sse2_sincosf_(angleX, &sinx, &cosx);
    dest->m_data[0].m_data[0] = cosy;
    dest->m_data[0].m_data[1] = 0.0f;
    dest->m_data[0].m_data[2] = -siny;
    dest->m_data[1].m_data[0] = sinx * siny;
    dest->m_data[1].m_data[1] = cosx;
    dest->m_data[1].m_data[2] = sinx * cosy;
    dest->m_data[2].m_data[0] = cosx * siny;
    dest->m_data[2].m_data[1] = -sinx;
    dest->m_data[2].m_data[2] = cosx * cosy;
}
void MAT_Set44_Identity(MATRIX44 *dest) {
    dest->m_data[0].m_data[0] = 1.0f;
    dest->m_data[0].m_data[1] = 0.0f;
    dest->m_data[0].m_data[2] = 0.0f;
    dest->m_data[0].m_data[3] = 0.0f;
    dest->m_data[1].m_data[0] = 0.0f;
    dest->m_data[1].m_data[1] = 1.0f;
    dest->m_data[1].m_data[2] = 0.0f;
    dest->m_data[1].m_data[3] = 0.0f;
    dest->m_data[2].m_data[0] = 0.0f;
    dest->m_data[2].m_data[1] = 0.0f;
    dest->m_data[2].m_data[2] = 1.0f;
    dest->m_data[2].m_data[3] = 0.0f;
    dest->m_data[3].m_data[0] = 0.0f;
    dest->m_data[3].m_data[1] = 0.0f;
    dest->m_data[3].m_data[2] = 0.0f;
    dest->m_data[3].m_data[3] = 1.0f;
}
void MAT_Set44_RotX(MATRIX44 *dest, float angle) {
    float sinx, cosx;
    __libm_sse2_sincosf_(angle, &sinx, &cosx);
    dest->m_data[0].m_data[0] = 1.0f;
    dest->m_data[0].m_data[1] = 0.0f;
    dest->m_data[0].m_data[2] = 0.0f;
    dest->m_data[0].m_data[3] = 0.0f;
    dest->m_data[1].m_data[0] = 0.0f;
    dest->m_data[1].m_data[1] = sinx;
    dest->m_data[1].m_data[2] = cosx;
    dest->m_data[1].m_data[3] = 0.0f;
    dest->m_data[2].m_data[0] = 0.0f;
    dest->m_data[2].m_data[1] = -cosx;
    dest->m_data[2].m_data[2] = sinx;
    dest->m_data[2].m_data[3] = 0.0f;
    dest->m_data[3].m_data[0] = 0.0f;
    dest->m_data[3].m_data[1] = 0.0f;
    dest->m_data[3].m_data[2] = 0.0f;
    dest->m_data[3].m_data[3] = 1.0f;
}
void MAT_Set44_RotY(MATRIX44 *dest, float angle) {
    float sinx, cosx;
    __libm_sse2_sincosf_(angle, &sinx, &cosx);
    dest->m_data[0].m_data[0] = cosx;
    dest->m_data[0].m_data[1] = 0.0f;
    dest->m_data[0].m_data[2] = -sinx;
    dest->m_data[0].m_data[3] = 0.0f;
    dest->m_data[1].m_data[0] = 0.0f;
    dest->m_data[1].m_data[1] = 1.0f;
    dest->m_data[1].m_data[2] = 0.0f;
    dest->m_data[1].m_data[3] = 0.0f;
    dest->m_data[2].m_data[0] = sinx;
    dest->m_data[2].m_data[1] = 0.0f;
    dest->m_data[2].m_data[2] = cosx;
    dest->m_data[2].m_data[3] = 0.0f;
    dest->m_data[3].m_data[0] = 0.0f;
    dest->m_data[3].m_data[1] = 0.0f;
    dest->m_data[3].m_data[2] = 0.0f;
    dest->m_data[3].m_data[3] = 1.0f;
}
void MAT_Set44_RotZ(MATRIX44 *dest, float angle) {
    float sinx, cosx;
    __libm_sse2_sincosf_(angle, &sinx, &cosx);
    dest->m_data[0].m_data[0] = cosx;
    dest->m_data[0].m_data[1] = sinx;
    dest->m_data[0].m_data[2] = 0.0f;
    dest->m_data[0].m_data[3] = 0.0f;
    dest->m_data[1].m_data[0] = -sinx;
    dest->m_data[1].m_data[1] = cosx;
    dest->m_data[1].m_data[2] = 0.0f;
    dest->m_data[1].m_data[3] = 0.0f;
    dest->m_data[2].m_data[0] = 0.0f;
    dest->m_data[2].m_data[1] = 0.0f;
    dest->m_data[2].m_data[2] = 1.0f;
    dest->m_data[2].m_data[3] = 0.0f;
    dest->m_data[3].m_data[0] = 0.0f;
    dest->m_data[3].m_data[1] = 0.0f;
    dest->m_data[3].m_data[2] = 0.0f;
    dest->m_data[3].m_data[3] = 1.0f;
}
void MAT_Set44_Translate(MATRIX44 *dest, const VEC3 &vec) {
    dest->m_data[0].m_data[0] = 1.0f;
    dest->m_data[0].m_data[1] = 0.0f;
    dest->m_data[0].m_data[2] = 0.0f;
    dest->m_data[0].m_data[3] = 0.0f;
    dest->m_data[1].m_data[0] = 0.0f;
    dest->m_data[1].m_data[1] = 1.0f;
    dest->m_data[1].m_data[2] = 0.0f;
    dest->m_data[1].m_data[3] = 0.0f;
    dest->m_data[2].m_data[0] = 0.0f;
    dest->m_data[2].m_data[1] = 0.0f;
    dest->m_data[2].m_data[2] = 1.0f;
    dest->m_data[2].m_data[3] = 0.0f;
    dest->m_data[3].m_data[0] = vec.m_data[0];
    dest->m_data[3].m_data[1] = vec.m_data[1];
    dest->m_data[3].m_data[2] = vec.m_data[2];
    dest->m_data[3].m_data[3] = 1.0f;
}
void MAT_Set44_Zero(MATRIX44 *dest) { memset(dest, 0, sizeof(*dest)); }
bool MAT_IsOrthonormal(const MATRIX33 &mat) {
    return fabs(mat.m_data[0].lenSquared() - 1.0f) <= 0.03f && fabs(mat.m_data[1].lenSquared() - 1.0f) <= 0.03f && fabs(mat.m_data[2].lenSquared() - 1.0f) <= 0.03f &&
           fabs(mat.m_data[1].m_data[1] * mat.m_data[0].m_data[1] + mat.m_data[1].m_data[0] * mat.m_data[0].m_data[0] + mat.m_data[1].m_data[2] * mat.m_data[0].m_data[2]) <= 0.07f &&
           fabs(mat.m_data[2].m_data[1] * mat.m_data[0].m_data[1] + mat.m_data[2].m_data[0] * mat.m_data[0].m_data[0] + mat.m_data[2].m_data[2] * mat.m_data[0].m_data[2]) <= 0.07f &&
           fabs(mat.m_data[2].m_data[1] * mat.m_data[1].m_data[1] + mat.m_data[2].m_data[0] * mat.m_data[1].m_data[0] + mat.m_data[2].m_data[2] * mat.m_data[1].m_data[2]) <= 0.07f;
}
bool MAT_IsOrthonormal(const MATRIX44 &mat) {
    VEC3 v0{ mat.m_data[0].m_data[0], mat.m_data[0].m_data[1], mat.m_data[0].m_data[2] };
    VEC3 v1{ mat.m_data[1].m_data[0], mat.m_data[1].m_data[1], mat.m_data[1].m_data[2] };
    VEC3 v2{ mat.m_data[2].m_data[0], mat.m_data[2].m_data[1], mat.m_data[2].m_data[2] };
    return fabs(v0.lenSquared() - 1.0f) <= 0.03f && fabs(v1.lenSquared() - 1.0f) <= 0.03f && fabs(v2.lenSquared() - 1.0f) <= 0.03f &&
           fabs(mat.m_data[1].m_data[1] * mat.m_data[0].m_data[1] + mat.m_data[1].m_data[0] * mat.m_data[0].m_data[0] + mat.m_data[1].m_data[2] * mat.m_data[0].m_data[2]) <= 0.07f &&
           fabs(mat.m_data[2].m_data[1] * mat.m_data[0].m_data[1] + mat.m_data[2].m_data[0] * mat.m_data[0].m_data[0] + mat.m_data[2].m_data[2] * mat.m_data[0].m_data[2]) <= 0.07f &&
           fabs(mat.m_data[2].m_data[1] * mat.m_data[1].m_data[1] + mat.m_data[2].m_data[0] * mat.m_data[1].m_data[0] + mat.m_data[2].m_data[2] * mat.m_data[1].m_data[2]) <= 0.07f;
}
template<class M>
void Quat_ConvertFromMatrixInternal(VEC4 *dest, const M &mat) {
    zassert(MAT_IsOrthonormal(mat));
    auto v4 = mat.m_data[1].m_data[0];
    auto v5 = mat.m_data[2].m_data[0];
    auto v6 = v5 - mat.m_data[0].m_data[0] - v4;
    auto v7 = mat.m_data[0].m_data[0] - v4 - v5;
    auto v8 = v4 + mat.m_data[0].m_data[0] + v5;
    auto v9 = v8;
    if (v7 > v8)
        v8 = v7;
    int v10 = v7 > v9;
    if (mat.m_data[1].m_data[0] - mat.m_data[0].m_data[0] - v5 > v8) {
        v8 = mat.m_data[1].m_data[0] - mat.m_data[0].m_data[0] - v5;
        v10 = 2;
    }
    if (v6 > v8) {
        v8 = v6;
        v10 = 3;
    }
    auto v11 = sqrtf(v8 + 1.0f) * 0.5f;
    auto v12 = 0.25f / v11;
    switch (v10) {
    case 0:
        dest->m_data[3] = v11;
        dest->m_data[0] = (mat.m_data[1].m_data[1] - mat.m_data[1].m_data[3]) * v12;
        dest->m_data[1] = (mat.m_data[1].m_data[2] - mat.m_data[0].m_data[2]) * v12;
        dest->m_data[2] = (mat.m_data[0].m_data[1] - mat.m_data[0].m_data[3]) * v12;
        break;
    case 1:
        dest->m_data[0] = v11;
        dest->m_data[3] = (mat.m_data[1].m_data[1] - mat.m_data[1].m_data[3]) * v12;
        dest->m_data[1] = (mat.m_data[0].m_data[3] + mat.m_data[0].m_data[1]) * v12;
        dest->m_data[2] = (mat.m_data[1].m_data[2] + mat.m_data[0].m_data[2]) * v12;
        break;
    case 2:
        dest->m_data[3] = (mat.m_data[1].m_data[2] - mat.m_data[0].m_data[2]) * v12;
        dest->m_data[1] = v11;
        dest->m_data[0] = (mat.m_data[0].m_data[3] + mat.m_data[0].m_data[1]) * v12;
        dest->m_data[2] = (mat.m_data[1].m_data[3] + mat.m_data[1].m_data[1]) * v12;
        break;
    case 3:
        dest->m_data[3] = (mat.m_data[0].m_data[1] - mat.m_data[0].m_data[3]) * v12;
        dest->m_data[0] = (mat.m_data[1].m_data[2] + mat.m_data[0].m_data[2]) * v12;
        dest->m_data[2] = v11;
        dest->m_data[1] = (mat.m_data[1].m_data[3] + mat.m_data[1].m_data[1]) * v12;
        break;
    }
}
void QUAT_SetMat(VEC4 *dest, const MATRIX33 &mat) { Quat_ConvertFromMatrixInternal(dest, mat); }
void QUAT_SetMat(VEC4 *dest, const MATRIX44 &mat) { Quat_ConvertFromMatrixInternal(dest, mat); }
void QUAT_MulQuat(VEC4 *dest, const VEC4 &a2, const VEC4 &a3) {
    dest->m_data[0] = a3.m_data[0] * a2.m_data[3] + a3.m_data[3] * a2.m_data[0] + a3.m_data[2] * a2.m_data[1] - a3.m_data[1] * a2.m_data[2];
    dest->m_data[1] = a3.m_data[1] * a2.m_data[3] - a3.m_data[2] * a2.m_data[0] + a3.m_data[3] * a2.m_data[1] + a3.m_data[0] * a2.m_data[2];
    dest->m_data[2] = a3.m_data[2] * a2.m_data[3] + a3.m_data[1] * a2.m_data[0] - a3.m_data[0] * a2.m_data[1] + a3.m_data[3] * a2.m_data[2];
    dest->m_data[3] = a3.m_data[3] * a2.m_data[3] - a3.m_data[0] * a2.m_data[0] - a3.m_data[1] * a2.m_data[1] - a3.m_data[2] * a2.m_data[2];
}
void QUAT_Normalize(VEC4 *dest) { dest->Normalize(); }
void QUAT_SetEulerXYZ(VEC4 *dest, const VEC3 &src) {
    auto v4 = src.m_data[0] * 0.5f, v5 = cosf(v4);
    auto v6 = src.m_data[1] * 0.5f, v7 = cosf(v6);
    auto v8 = src.m_data[2] * 0.5f, v9 = cosf(v8);
    auto v10 = sinf(v4), v11 = sinf(v6), v12 = sinf(v8);
    auto v13 = v11 * v10, v14 = v11 * v5;
    dest->m_data[0] = v10 * v7 * v9 - v14 * v12;
    dest->m_data[1] = v10 * v7 * v12 + v14 * v9;
    dest->m_data[3] = v13 * v12 + v7 * v5 * v9;
    dest->m_data[2] = v7 * v5 * v12 - v13 * v9;
}
void QUAT_Slerp(VEC4 *dest, const VEC4 &a2, const VEC4 &a3, float a4) {
    VEC4 v16{ a2 };
    auto v11 = v16 * a3, v12 = 1.0f - a4;
    if (v11 < 0.0f) {
        v11 = -v11;
        v16 = -a2;
    }
    if (1.0f - v11 > 0.0001f) {
        auto v13 = acosf(v11);
        auto v14 = sinf(v13);
        v12 = sinf(v12 * v13) / v14;
        a4 = sinf(v13 * a4) / v14;
    }
    for (int i = 0; i < 4; ++i)
        dest->m_data[i] = a4 * a3.m_data[i] + v12 * v16.m_data[i];
}
void QUAT_TransformVector(VEC3 *dest, const VEC4 &a2, const VEC3 &a3) {
    auto  v8 = a3.m_data[0] * a2.m_data[3] + a3.m_data[2] * a2.m_data[1] - a3.m_data[1] * a2.m_data[2];
    auto  v9 = a3.m_data[1] * a2.m_data[3] - a3.m_data[2] * a2.m_data[0] + a3.m_data[0] * a2.m_data[2];
    auto v10 = a3.m_data[2] * a2.m_data[3] + a3.m_data[1] * a2.m_data[0] - a3.m_data[0] * a2.m_data[1];
    auto v11 = a3.m_data[1] * a2.m_data[1] + a3.m_data[0] * a2.m_data[0] + a3.m_data[2] * a2.m_data[2];
    dest->m_data[0] = v8 * a2.m_data[3] - v9 * a2.m_data[2] + v10 * a2.m_data[1] + v11 * a2.m_data[0];
    dest->m_data[2] = v9 * a2.m_data[0] - v8 * a2.m_data[1] + v10 * a2.m_data[3] + v11 * a2.m_data[2];
    dest->m_data[1] = v9 * a2.m_data[3] + v8 * a2.m_data[2] - v10 * a2.m_data[0] + v11 * a2.m_data[1];
}
float MU_GetAngleBetweenVectors(const VEC3 &a1, const VEC3 &a2) {
    auto v4 = std::clamp(a1 * a2, -1.0f, 1.0f);
    v4 = acosf(v4);
    if (a2.m_data[0] * a1.m_data[2] - a1.m_data[0] * a2.m_data[2] < 0.0f)
        return -v4;
    return v4;
}
void MU_ClosestPointOnLineToPoint(VEC3 *dest, const VEC3 &a2, const VEC3 &a3, const VEC3 &a4) {
    auto v14 = (a4 - a2).lenSquared();
    if (v14 >= 0.0f) {
        auto tmp = a3 - a2;
        auto v15 = tmp.lenSquared();
        if (v15 >= 0.001f) {
            v15 = sqrtf(v15);
            auto div = 1.0f / v15, v17 = div * v14;
            if (v17 <= v15) {
                tmp *= (v17 * div);
                *dest = tmp + a2;
                return;
            }
        }
    }
    *dest = a2;
}
float MU_GetSineValue(float a1, float a2, float a3) {
    static auto t = std::chrono::steady_clock::now();
    auto dt = std::chrono::steady_clock::now() - t;
    return (a2 + a3) * 0.5f + (a3 - a2) * 0.5f * sinf(dt.count() / 1'000'0000'000.0f * a1 * (std::numbers::pi_v<float> + std::numbers::pi_v<float>));

}
float VEC_Length(const VEC3 &src) { return src.len(); }
float MU_EstCubeRootPositive(float x) {
    union { float f; uint32_t i; } u = { x }, v5, v6;
    zassert(x > 0.0f);
    uint8_t v1 = u.i >> 23;
    uint32_t v2 = (v1 + 2) / 3;
    v5.i = u.i & 0x807FFFFF | ((509 * v2 + v1 + 126) << 23);
    float v3 = v5.f * v5.f;
    v6.f = (v3 * v3 * 45.254833f + v3 * v5.f * 192.27983f + v5.f * v5.f * 119.16548f + v5.f * 13.432502f + 0.16361612f) / 
        (v3 * v3 * 14.808841f + v3 * v5.f * 151.97141f + v5.f * v5.f * 168.52544f + v5.f * 33.990593f + 1.0f);
    v6.i = v6.i & 0x7FFFFF | ((v2 + 84) << 23);
    return v6.f;
}
float MU_FastSin(float angle) {
    zassert(angle >= -std::numbers::pi_v<float> - 0.01f && angle <= std::numbers::pi_v<float> + 0.01f);
    auto v1 = angle * 0.15915494f; //1 / 2 PI
    v1 = (0.5f - fabs(v1)) * v1 * 7.5894666f;
    return (fabs(v1) + 1.6338434f) * v1;
}
void MU_FastSinCos(float *pSin, float *pCos, float angle) {
    *pSin = MU_FastSin(angle);
    if (angle > std::numbers::pi_v<float> / 2)
        angle -= std::numbers::pi_v<float> / 2;
    else
        angle += std::numbers::pi_v<float> / 2;
    *pCos = MU_FastSin(angle);
}
bool MU_IsNAN(float f) {
    return isnan(f); // mask with 0x7F800000
}
void MU_AssertNAN(const VEC3 &v) {
    zassert(!MU_IsNAN(v.m_data[0]));
    zassert(!MU_IsNAN(v.m_data[1]));
    zassert(!MU_IsNAN(v.m_data[2]));
}
void VEC_GetPerp(VEC3 *dest, const VEC3 &a, const VEC3 &b) {
    zassert((void *)dest != (void *)&a && (void *)dest != (void *)&b);
    dest->m_data[0] = b.m_data[2] * a.m_data[1] - b.m_data[1] * a.m_data[2];
    dest->m_data[1] = b.m_data[0] * a.m_data[2] - b.m_data[2] * a.m_data[0];
    dest->m_data[2] = b.m_data[1] * a.m_data[0] - b.m_data[0] * a.m_data[1];
}
void VEC_GetPerp(VEC3 *dest, const VEC3 &a) {
    auto v4 = fabsf(a.m_data[0]), v5 = fabsf(a.m_data[1]), v6 = fabsf(a.m_data[2]);
    VEC3 b{ 0.0f, 0.0f, 1.0f };
    if (v4 >= v5) {
        if (v5 < v6)
            b = { 0.0f, 1.0f, 0.0f };
    } else if (v4 < v6)
        b = { 1.0f, 0.0f, 0.0f };
    VEC_GetPerp(dest, a, b);
}
void VEC_RotateXYZ_AroundAxis(VEC3 *dest, const VEC3 &a, const VEC3 &b, float sinx, float cosx) {
    VEC3 desta;
    VEC_GetPerp(&desta, b, a);
    auto v12 = b * a;
    dest->m_data[0] = (a.m_data[0] - b.m_data[0] * v12) * cosx + b.m_data[0] * v12 + desta.m_data[0] * sinx;
    dest->m_data[1] = (a.m_data[1] - b.m_data[1] * v12) * cosx + b.m_data[1] * v12 + desta.m_data[1] * sinx;
    dest->m_data[2] = (a.m_data[2] - b.m_data[2] * v12) * cosx + b.m_data[2] * v12 + desta.m_data[2] * sinx;
}
void CheckedSincos(float angle, float *sinx, float *cosx) {
    zassert(angle >= -std::numbers::pi_v<float> - 0.01f && angle <= std::numbers::pi_v<float> + 0.01f);
    __libm_sse2_sincosf_(angle, sinx, cosx);
}
void VEC_RotateXYZ_AroundAxis(VEC3 *dest, const VEC3 &a, const VEC3 &b, float angle) {
    float sinx, cosx;
    CheckedSincos(angle, &sinx, &cosx);
    VEC_RotateXYZ_AroundAxis(dest, a, b, sinx, cosx);
}
void MU_ClosestPointOnLineToLine(VEC3 *dest, const VEC3 &a2, const VEC3 &a, float a4, const VEC3 &a5, const VEC3 &b, float a7) {
    VEC3 perp;
    VEC_GetPerp(&perp, a, b);
    auto v24 = perp.lenSquared();
    float v30;
    auto v23 = a5 - a2;
    if (v24 >= 0.000001f) {
        v30 = (1.0f / v24) * ((a.m_data[1] * perp.m_data[2] - a.m_data[2] * perp.m_data[1]) * v23.m_data[0]
                            - (a.m_data[0] * perp.m_data[2] - a.m_data[2] * perp.m_data[0]) * v23.m_data[1]
                            + (a.m_data[0] * perp.m_data[1] - a.m_data[1] * perp.m_data[0]) * v23.m_data[2]);
    } else {
        auto v28 = a * b;
        auto v29 = a * v23;
        if ((v28 > 0.0f && v29 > 0.0f) || (v28 < 0.0f && v29 < 0.0f)) {
            dest->m_data[0] = a5.m_data[0];
            return;
        }
        if ((v28 >= 0.0f || v29 <= 0.0f) && (v28 <= 0.0f || v29 >= 0.0f)) {
            dest->m_data[0] = a2.m_data[0];
            return;
        }
        v30 = -(b * v23);
    }
    v30 = std::clamp(v30, -a7, a7);
    *dest = b * v30 + a5;
}
void MU_DampedSpringSlerp3D(VEC3 *dest, float *a2, const VEC3 &a, float a4, float a5) {
    VEC3 v30;
    VEC_GetPerp(&v30, a, *dest);
    auto v20 = v30.lenSquared();
    if (v20 >= 0.000001f) {
        auto v21 = sqrtf(v20), v23 = atan2f(v21, a * (*dest)), v24 = (*a2 + v23 * a4) * a5, angle = (v23 * a5) + v24;
        *a2 = v24 - angle * a4;
        if (angle >= 0.0f) {
            v30 *= (1.0f / v21);
            VEC_RotateXYZ_AroundAxis(dest, a, v30, angle);
            return;
        }
    }
    *a2 = 0.0f;
    dest->m_data[0] = a.m_data[0];
    dest->m_data[2] = a.m_data[2];
}
void VEC_SlerpXYZ(VEC3 *dest, const VEC3 &a, const VEC3 &b, float a4) {
    VEC3 v26;
    VEC_GetPerp(&v26, a, b);
    auto v18 = v26.lenSquared();
    if (v18 >= 0.000001f) {
        auto v19 = sqrtf(v18), angle = atan2f(v19, a * b) * a4;
        v26 *= (1.0f / v19);
        VEC_RotateXYZ_AroundAxis(dest, a, v26, angle);
    } else {
        *dest = b;
    }
}
void VEC_SlerpXY(VEC2 *dest, const VEC2 &a2, const VEC2 &a3, float a4) {
    auto v9 = a2 * a3;
    if (v9 >= 0.99999f) {
        dest->m_data[0] = a3.m_data[0];
    } else {
        auto v13 = acosf(fmaxf(v9, -1.0f));
        if (a2.m_data[0] * a3.m_data[1] - a3.m_data[0] * a2.m_data[1] > 0.0f)
            v13 = -v13;
        float sinx, cosx;
        CheckedSincos(v13 * a4, &sinx, &cosx);
        dest->m_data[0] = cosx * a2.m_data[0] + sinx * a2.m_data[1];
        dest->m_data[1] = cosx * a2.m_data[1] - sinx * a2.m_data[0];
    }
}
void VEC_SetXYZ(VEC3 *dest, float x, float y, float z) { *dest = { x, y, z }; }
void VEC_DirToYawPitch(float *a1, float *a2, const VEC3 &a3) {
    *a1 = atan2f(a3.m_data[0], a3.m_data[2]);
    *a2 = -asinf(a3.m_data[1]);
}
void MU_OscillatorResetRandom(MU_OscillatorVector *ov) {
    ov->m_vec.m_data[2] = (float)rand() * 0.000061037019f - 1.0f;
    auto angle = (float)rand() * 0.00019175345f - std::numbers::pi_v<float>;
    float sinx, cosx;
    CheckedSincos(angle, &sinx, &cosx);
    auto v5 = sqrtf(1.0f - ov->m_vec.m_data[2] * ov->m_vec.m_data[2]);
    ov->m_vec.m_data[0] = cosx * v5;
    ov->m_vec.m_data[1] = sinx * v5;
}
void QUAT_SetAxisAngle(VEC4 *dest, const VEC3 &a2, float angle2) {
    float sinx, angle = angle2 * 0.5f;
    CheckedSincos(angle, &sinx, dest->m_data + 3);
    dest->m_data[0] = a2.m_data[0] * sinx;
    dest->m_data[1] = a2.m_data[1] * sinx;
    dest->m_data[2] = a2.m_data[2] * sinx;
}
void QUAT_Add(VEC4 *dest, const VEC4 &a, const VEC4 &b) {
    *dest = a + b;
}
void QUAT_Unpack(uint32_t src, VEC4 *dest) {
    dest->m_data[0] = ((src & 0x7FF) - 1024) * 0.00097656f;
    src >>= 11;
    dest->m_data[1] = ((src & 0x3FF) * 2 - 1024) * 0.00097656f;
    dest->m_data[2] = ((src >> 10) - 1024) * 0.00097656f;
    auto v5 = 1.0f - (dest->m_data[0] * dest->m_data[0] + dest->m_data[1] * dest->m_data[1] + dest->m_data[2] * dest->m_data[2]);
    if (v5 <= 0.00001f)
        dest->m_data[3] = 0.0f;
    else
        dest->m_data[3] = sqrtf(v5);
}
uint32_t QUAT_Pack(const VEC4 &src) {
    if (src.m_data[3] >= 0.0f)
        return ((int(( src.m_data[1] * 1023.9f) * 0.5f) << 11) + 0x100000) | (int( src.m_data[0] * 1023.9f) + 1024) | (int( src.m_data[2] * 1023.9f) << 21) ^ 0x80000000;
    return     ((int((-src.m_data[1] * 1023.9f) * 0.5f) << 11) + 0x100000) | (int(-src.m_data[0] * 1023.9f) + 1024) | (int(-src.m_data[2] * 1023.9f) << 21) ^ 0x80000000;
}
void QUAT_SetXYZW(VEC4 *dest, float x, float y, float z, float w) { *dest = { x, y, z, w }; }
void QUAT_SetEulerZXY(VEC4 *dest, const VEC3 &src) {
    float sinx0, cosx0, sinx1, cosx1, sinx2, cosx2;
    __libm_sse2_sincosf_(src.m_data[0], &sinx0, &cosx0);
    __libm_sse2_sincosf_(src.m_data[1], &sinx1, &cosx1);
    __libm_sse2_sincosf_(src.m_data[2], &sinx2, &cosx2);
    dest->m_data[0] = sinx0 * cosx1 * cosx2 + cosx0 * sinx1 * sinx2;
    dest->m_data[1] = cosx0 * sinx1 * cosx2 - sinx0 * cosx1 * sinx2;
    dest->m_data[2] = cosx0 * cosx1 * sinx2 - sinx0 * sinx1 * cosx2;
    dest->m_data[3] = cosx0 * cosx1 * cosx2 + sinx0 * sinx1 * sinx2;
}
void QUAT_Scale(VEC4 *dest, const VEC4 &src, float mul) { *dest = src * mul; }
void QUAT_MulQuatConjugate(VEC4 *dest, const VEC4 &a, const VEC4 &b) {
    dest->m_data[0] = a.m_data[2] * b.m_data[1] + a.m_data[0] * b.m_data[3] - a.m_data[1] * b.m_data[2] - a.m_data[3] * b.m_data[0];
    dest->m_data[1] = a.m_data[0] * b.m_data[2] + a.m_data[1] * b.m_data[3] - a.m_data[2] * b.m_data[0] - a.m_data[3] * b.m_data[1];
    dest->m_data[2] = a.m_data[1] * b.m_data[0] - a.m_data[0] * b.m_data[1] + a.m_data[2] * b.m_data[3] - a.m_data[3] * b.m_data[2];
    dest->m_data[3] = a.m_data[0] * b.m_data[0] + a.m_data[3] * b.m_data[3] + a.m_data[2] * b.m_data[2] + a.m_data[1] * b.m_data[1];
}
float QUAT_Magnitude(const VEC4 &v) { return v.len(); }
void QUAT_Invert(VEC4 *dest, const VEC4 &src) {
    auto v4 = 1.0f / src.lenSquared();
    dest->m_data[0] = v4 * src.m_data[0];
    dest->m_data[1] = v4 * src.m_data[1];
    dest->m_data[2] = -v4 * src.m_data[2];
    dest->m_data[3] = v4 * src.m_data[3];
}
void QUAT_GetEulerZXY(VEC3 *dest, const VEC4 &src) {
    auto v6 = src.m_data[3] * src.m_data[0];
    dest->m_data[0] = asinf((src.m_data[2] * src.m_data[1] - src.m_data[0] * src.m_data[3]) * -2.0f);
    dest->m_data[1] = atan2f(src.m_data[0] * src.m_data[2] + src.m_data[3] * src.m_data[1] + src.m_data[0] * src.m_data[2] + src.m_data[3] * src.m_data[1],
        src.m_data[2] * src.m_data[2] + v6 - src.m_data[1] * src.m_data[1]);
    dest->m_data[2] = atan2f(src.m_data[2] * src.m_data[3] + src.m_data[0] * src.m_data[1] + src.m_data[2] * src.m_data[3] + src.m_data[0] * src.m_data[1],
        src.m_data[3] * src.m_data[3] - src.m_data[0] * src.m_data[0] + src.m_data[1] * src.m_data[1] - src.m_data[2] * src.m_data[2]);
}
void QUAT_GetEulerXYZ(VEC3 *dest, const VEC4 &src) {
    auto v6 = src.m_data[0] * src.m_data[0];
    dest->m_data[0] = atan2f(src.m_data[1] * src.m_data[2] + src.m_data[3] * src.m_data[0] + src.m_data[1] * src.m_data[2] + src.m_data[3] * src.m_data[0],
        src.m_data[2] * src.m_data[2] + src.m_data[3] * src.m_data[3] - v6 - src.m_data[1] * src.m_data[1]);
    dest->m_data[1] = asinf((src.m_data[2] * src.m_data[0] - src.m_data[1] * src.m_data[3]) * -2.0f);
    dest->m_data[2] = atan2f(src.m_data[2] * src.m_data[3] + src.m_data[1] * src.m_data[0] + src.m_data[2] * src.m_data[3] + src.m_data[1] * src.m_data[0],
        src.m_data[3] * src.m_data[3] + v6 - src.m_data[1] * src.m_data[1] - src.m_data[2] * src.m_data[2]);
}
void QUAT_GetAxisAngle(VEC3 *dest, float *a2, const VEC4 &src) {
    auto v6 = std::clamp(src.m_data[3], -1.0f, 1.0f), v9 = acosf(v6);
    *a2 = v9 + v9;
    v9 = sqrtf(1.0f - (v6 * v6));
    if (fabsf(v9) < 0.0005f)
        v9 = 1.0f;
    else
        v9 = 1.0f / v9;
    dest->m_data[0] = src.m_data[0] * v9;
    dest->m_data[1] = src.m_data[1] * v9;
    dest->m_data[2] = src.m_data[2] * v9;
}
void QUAT_Exponent(VEC4 *dest, const VEC4 &src, float a3) {
    if (fabsf(src.m_data[3]) >= 0.99999f) {
        *dest = src;
    } else {
        auto v7 = acosf(src.m_data[3]);
        float sinx, cosx;
        __libm_sse2_sincosf_(v7 * a3, &sinx, &cosx);
        auto v9 = sinx / sinf(v7);
        dest->m_data[0] = src.m_data[0] * v9;
        dest->m_data[1] = src.m_data[1] * v9;
        dest->m_data[2] = src.m_data[2] * v9;
        dest->m_data[3] = cosx;
    }
}
float QUAT_Dot(const VEC4 &a, const VEC4 &b) { return a * b; }
void QUAT_Conjugate(VEC4 *dest, const VEC4 &src) { *dest = -src; }
float QUAT_AngleDiff(const VEC4 &a, const VEC4 &b) {
    auto v6 = acosf(std::clamp(a * b, -1.0f, 1.0f));
    return v6 + v6;
}
float MU_ClampedASIN(float sinx) {
    return asinf(std::clamp(sinx, -1.0f, 1.0f));
}
uint32_t MU_OffsetHash(uint32_t a1, int a2, int a3) {
    return (a1 + (a3 << 6)) & 0x3C0 | (a2 + a1) & 0x1Fu;
}
float MU_NormalizeAngle(float rad) {
    if (rad < -std::numbers::pi_v<float>)
        return rad + std::numbers::pi_v<float> + std::numbers::pi_v<float>;
    else if (rad >= std::numbers::pi_v<float>)
        return rad - std::numbers::pi_v<float> - std::numbers::pi_v<float>;
    return rad;
}
uint32_t MU_LerpColor(uint32_t a1, uint32_t a2, float a3) {
    union { int8_t u8[4]; uint32_t u32; } ua1{ .u32 = a1 }, ua2{ .u32 = a2 }, ret;
    for (int i = 0; i < 4; i++)
        ret.u8[i] = ua1.u8[i] + int8_t(float(ua2.u8[i] - ua1.u8[i]) * a3);
    return ret.u32;
}

/* not found:
void MU_DampedSpringSlerp3DGuided(VEC3 *, float *, const VEC3 &, const VEC3 &, float, float)
void VEC_BBoxToBSphere(VEC4 *, const VEC3 &, const VEC3 &)
void VEC_AngleBetweenVectors(const VEC3 &a, const VEC3 &b, bool)
void MAT_Set43(MATRIX43 *dest, const MATRIX44 &src);
void MAT_Set43_Identity(MATRIX43 *dest);
void MAT_Set44_AxisAngleSinCos(MATRIX44 *, float, float, float, float, float);
Quat_ConvertFromMatrixInternal<MATRIX43>(VEC4 *,MATRIX43 const&)
QUAT_SetMat(VEC4 *,MATRIX43 const&)
void MU_OscillatorResetRandom(MU_OscillatorVector *, int, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, int, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, int);
void MU_OscillatorResetRandom(MU_OscillatorVector *, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, MU_OscillatorVector *, int, float, float, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, MU_OscillatorVector *, int, float, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, MU_OscillatorVector *, int, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, MU_OscillatorVector *, float, float, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, MU_OscillatorVector *, float, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *, MU_OscillatorVector *, float, float);
void MU_OscillatorResetRandom(MU_OscillatorVector *);
void MU_OscillatorResetRandom(MU_Oscillator *, int, float, float, float, float);
void MU_OscillatorResetRandom(MU_Oscillator *, int, float, float, float);
void MU_OscillatorResetRandom(MU_Oscillator *, int, float, float);
void MU_OscillatorResetRandom(MU_Oscillator *, MU_OscillatorRange const *, int);
void MU_OscillatorResetRandom(MU_Oscillator *, FloatRange const *, int, float);
void MU_OscillatorResetRandom(MU_Oscillator *, FloatRange const *, int);
MU_UnderdampedSpringSetup(SpringCoeff *,float,float,float)
MU_SolveQuartic(float *,float,float,float,float)
MU_SCurve(float *,float *,float *,float,float)
MU_SCurve(float *,float *,float *,float)
MU_RotateDirectionVec3(VEC3 *,VEC3 const*,float,float)
MU_RandSeedDirectionVec3(VEC3 *,float,float,float,float)
MU_RandSeedDirectionVec3(VEC3 *,float,float)
MU_RandRotateBasis(VEC3 *,VEC3 *,VEC3 const*)
MU_RandPointInSphere(VEC3 *,float,float)
MU_RandPointInSphere(VEC3 *)
MU_Parabola_PrepareConfig_v(MU_ParabolaConfig *,VEC3 const*,VEC3 const*,float,float)
MU_Parabola_PrepareConfig_t(MU_ParabolaConfig *,VEC3 const*,VEC3 const*,float,float)
MU_Parabola_PrepareConfig_normal(MU_ParabolaConfig *,VEC3 const*,VEC3 const*,float)
MU_Parabola(MU_ParabolaConfig *,float)
MU_OverdampedSpringSetup(SpringCoeff *,float,float,float)
MU_InverseSCurve(float,float)
MU_GetVectorFromHash(uint)
MU_GetHashFromVector(VEC3 const*)
MU_InverseSCurve(float)
MU_FitSCurveTail(float *,float *,float *,float,float,float)
MU_FitSCurve(float *,float *,float,float,float)
MU_FastFloor2(float)
MU_EvaluateCubicDerivative(VEC4 *,float)
MU_EvaluateCubicDerivative(VEC3 *,Cubic3D *,float)
MU_EvaluateCubicDerivative(VEC2 *,Cubic2D *,float)
MU_EvaluateCubic(VEC4 const*,float)
MU_EvaluateCubic(VEC3 *,Cubic3D const*,float)
MU_EvaluateCubic(VEC2 *,Cubic2D const*,float)
MU_EstParabolaInterpolatorUpdate(MU_EstParabolaInterpolator *,float,VEC3 const*,VEC3 const*,float,float,float)
MU_EstParabolaInterpolatorReset(MU_EstParabolaInterpolator *)
MU_EstParabolaArcLength(VEC3 const*,VEC3 const*,float,float,float)
MU_DifferentiateCubic(VEC4 *,VEC4 const*)
MU_DifferentiateCubic(Cubic3D *,Cubic3D const*)
MU_DampedSpringSlerp2D(VEC2 *,float *,VEC2 const*,float,float)
MU_DampedSpringSlerp(VEC4 *,float *,VEC4 const*,float,float)
MU_DampedSpringSetup(SpringCoeff *,float,float)
MU_ClosestPointOnLineToPoint_uc(VEC3 *,VEC3 const*,VEC3 const*,VEC3 const*)
MU_ClosestPointOnLineToPoint(VEC3 *,VEC3 const*,VEC3 const*,VEC3 const*,float *)
MU_ClosestPointOnLineToPoint(VEC3 *,VEC3 const*,VEC3 const*,VEC3 const*,VEC3 const*)
MU_CalcHermiteCubic(Cubic3D *,VEC3 const*,VEC3 const*,VEC3 const*,VEC3 const*)
MU_CalcHermiteCubic(Cubic2D *,VEC2 const*,VEC2 const*,VEC2 const*,VEC2 const*)
 */