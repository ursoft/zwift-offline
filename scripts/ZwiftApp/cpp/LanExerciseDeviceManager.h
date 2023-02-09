#pragma once
class LanExerciseDeviceManager { //1 byte
public:
    LanExerciseDeviceManager() {}
    static void Initialize();
};
inline LanExerciseDeviceManager *g_LanExerciseDeviceManager;
