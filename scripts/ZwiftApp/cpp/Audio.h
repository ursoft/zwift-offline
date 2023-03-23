#pragma once
#include "AK/SoundEngine/Common/AkMemoryMgr.h"
#include "AK/SoundEngine/Common/AkModule.h"
inline int g_NumSoundObjects, g_audio_queue_failsafe;
inline AkMemSettings g_memSettings;
inline const char *g_GlobalMouseOverSID = "Play_SFX_UI_MOUSEOVER_1", *g_GlobalToggleOnSID = "Play_SFX_UI_RADIOBUTTON_ON_1",
    *g_GlobalToggleOffSID = "Play_SFX_UI_RADIOBUTTON_OFF_1", *g_GlobalSelectSID = "Play_SFX_UI_Menu_Select_1";

void AUDIO_Init();
void AUDIO_Shutdown();
class GameWorld;
class Camera;
void GAME_AudioUpdate(GameWorld *not_used, Camera *a2, float a3);
void AUDIO_Event(const char *name, int a2, bool vital);
void AUDIO_SetVariable(const char *name, float val);
bool AUDIO_PlayFlatFile(const char *name, float val);