#pragma once
struct SteeringModule {
    static SteeringModule *Self();
    void Init(BikeEntity *mb);
};
inline SteeringModule g_SteeringModule;
