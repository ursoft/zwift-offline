#pragma once
const char *const OS_GetUserPath();
void OS_Initialize();
bool OS_IsOnBattery();
void OS_Shutdown();
const char *OS_GetLanguage();