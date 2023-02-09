#pragma once
inline bool g_bMapSchedule_DLFailed, g_bMapSchedule_Ready, g_UseErgModeInWorkouts, g_ShowGraph;

void GAME_onFinishedDownloadingMapSchedule(const std::string &, int err);
void GAME_GetSuppressedLogTypes();
void GAME_SetUseErgModeInWorkouts(bool);
