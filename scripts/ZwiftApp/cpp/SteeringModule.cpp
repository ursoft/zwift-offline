#include "ZwiftApp.h"
SteeringModule *SteeringModule::Self() { return &g_SteeringModule; }
void SteeringModule::Init(BikeEntity *mb) {
    //TODO
}
bool SteeringModule::ShouldUseDroneCam() {
    return m_field_20 == 2 && !m_bike->m_field_8BB && m_field_40 >= 1.0;
}
