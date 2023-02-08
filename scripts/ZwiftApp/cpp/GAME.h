#pragma once
void GAME_onFinishedDownloadingMapSchedule(const std::string &, int err);
extern bool g_bMapSchedule_DLFailed, g_bMapSchedule_Ready;
void GAME_GetSuppressedLogTypes();
void GAME_SetUseErgModeInWorkouts(bool);
extern bool g_UseErgModeInWorkouts, g_ShowGraph;
