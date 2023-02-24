#pragma once
class SteeringModule {
public:
    static SteeringModule *Self();
    void Init(BikeEntity *mb);
};
inline SteeringModule g_SteeringModule;
