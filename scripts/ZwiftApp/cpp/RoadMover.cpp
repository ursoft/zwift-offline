#include "ZwiftApp.h"
void RoadMover::FastSinCosNorm(float angle) {
    if (angle <= std::numbers::pi_v<float>) {
        if (angle < -std::numbers::pi_v<float>)
            angle += 2 * std::numbers::pi_v<float>;
    } else {
        angle -= 2 * std::numbers::pi_v<float>;
    }
    MU_FastSinCos(m_field_C.m_data, m_field_C.m_data + 2, angle);
    m_field_C.Normalize();
}
