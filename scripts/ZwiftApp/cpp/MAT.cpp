#pragma once
#include "MAT.h"
void MAT_MulMat(MATRIX44 *dest, const MATRIX44 &m1, const MATRIX44 &m2) {
    auto v3 = m1.m_data[0].m_data[2], v4 = m1.m_data[0].m_data[0], v32 = m2.m_data[0].m_data[1], v35 = m2.m_data[0].m_data[2], v31 = m2.m_data[0].m_data[3], v34 = m2.m_data[1].m_data[2], v30 = m2.m_data[1].m_data[3];
    auto v33 = m2.m_data[2].m_data[2], v5 = m1.m_data[0].m_data[3], v6 = m1.m_data[0].m_data[1], v7 = m2.m_data[2].m_data[0], v8 = m2.m_data[3].m_data[0], v9 = m2.m_data[0].m_data[0], v10 = m2.m_data[1].m_data[0], v11 = m2.m_data[2].m_data[1];
    auto v12 = m2.m_data[3].m_data[1], v13 = m2.m_data[1].m_data[1], v14 = m2.m_data[3].m_data[2], v29 = m2.m_data[2].m_data[3], v28 = m2.m_data[3].m_data[3], v15 = m1.m_data[0].m_data[0] * v32;
    dest->m_data[0].m_data[0] = ((v5 * v8) + (v3 * v7)) + ((v6 * v10) + (m1.m_data[0].m_data[0] * m2.m_data[0].m_data[0]));
    dest->m_data[0].m_data[1] = ((v5 * v12) + (v3 * v11)) + ((v6 * v13) + v15);
    dest->m_data[0].m_data[2] = ((v5 * v14) + (v3 * v33)) + ((v6 * v34) + (v4 * v35));
    dest->m_data[0].m_data[3] = ((v5 * v28) + (v3 * v29)) + ((v6 * v30) + (v4 * v31));
    auto v16 = m1.m_data[1].m_data[2];
    auto v17 = m1.m_data[1].m_data[3];
    auto v18 = m1.m_data[1].m_data[0];
    auto v19 = m1.m_data[1].m_data[1];
    dest->m_data[1].m_data[0] = ((v17 * v8) + (v16 * v7)) + ((v19 * v10) + (v18 * v9));
    dest->m_data[1].m_data[1] = ((v17 * v12) + (v16 * v11)) + ((v19 * v13) + (v18 * v32));
    dest->m_data[1].m_data[3] = ((v17 * v28) + (v16 * v29)) + ((v19 * v30) + (v18 * v31));
    dest->m_data[1].m_data[2] = ((v17 * v14) + (v16 * v33)) + ((v19 * v34) + (v18 * v35));
    auto v20 = m1.m_data[2].m_data[2];
    auto v21 = m1.m_data[2].m_data[3];
    auto v22 = m1.m_data[2].m_data[0];
    auto v23 = m1.m_data[2].m_data[1];
    dest->m_data[2].m_data[0] = ((v21 * v8) + (v20 * v7)) + ((v23 * v10) + (v22 * v9));
    dest->m_data[2].m_data[1] = ((v21 * v12) + (v20 * v11)) + ((v23 * v13) + (v22 * v32));
    dest->m_data[2].m_data[2] = ((v21 * v14) + (v20 * v33)) + ((v23 * v34) + (v22 * v35));
    dest->m_data[2].m_data[3] = ((v21 * v28) + (v20 * v29)) + ((v23 * v30) + (v22 * v31));
    auto v24 = m1.m_data[3].m_data[3];
    auto v25 = m1.m_data[3].m_data[1];
    auto v26 = m1.m_data[3].m_data[2];
    auto v27 = m1.m_data[3].m_data[0];
    dest->m_data[3].m_data[0] = ((v24 * v8) + (v26 * v7)) + ((v25 * v10) + (v27 * v9));
    dest->m_data[3].m_data[1] = ((v24 * v12) + (v26 * v11)) + ((v25 * v13) + (v27 * v32));
    dest->m_data[3].m_data[3] = ((v24 * v28) + (v26 * v29)) + ((v25 * v30) + (v27 * v31));
    dest->m_data[3].m_data[2] = ((v24 * v14) + (v26 * v33)) + ((v25 * v34) + (v27 * v35));
}
void MAT_MulVecXYZW(VEC4 *dest, const VEC4 &a2, const MATRIX44 &a3) {
    auto v3 = (((a2.m_data[0] * a3.m_data[0].m_data[1]) + (a2.m_data[1] * a3.m_data[1].m_data[1])) + (a2.m_data[2] * a3.m_data[2].m_data[1])) + (a2.m_data[3] * a3.m_data[3].m_data[1]);
    auto v4 = a2.m_data[2];
    auto v5 = (((a2.m_data[0] * a3.m_data[0].m_data[2]) + (a2.m_data[1] * a3.m_data[1].m_data[2])) + (v4 * a3.m_data[2].m_data[2])) + (a2.m_data[3] * a3.m_data[3].m_data[2]);
    auto v6 = a2.m_data[3];
    auto v7 = (((a2.m_data[0] * a3.m_data[0].m_data[3]) + (a2.m_data[1] * a3.m_data[1].m_data[3])) + (v4 * a3.m_data[2].m_data[3])) + (v6 * a3.m_data[3].m_data[3]);
    dest->m_data[0] = (((a2.m_data[1] * a3.m_data[1].m_data[0]) + (a2.m_data[0] * a3.m_data[0].m_data[0])) + (v4 * a3.m_data[2].m_data[0])) + (v6 * a3.m_data[3].m_data[0]);
    dest->m_data[1] = v3;
    dest->m_data[2] = v5;
    dest->m_data[3] = v7;
}