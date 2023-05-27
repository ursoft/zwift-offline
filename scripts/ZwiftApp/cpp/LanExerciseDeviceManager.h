#pragma once
struct LanExerciseDeviceManager { //1 byte
    LanExerciseDeviceManager() {}
    static void Initialize();
};
inline LanExerciseDeviceManager *g_LanExerciseDeviceManager;
