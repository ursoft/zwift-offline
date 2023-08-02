#pragma once
#include "AK/SoundEngine/Common/AkMemoryMgr.h"
#include "AK/SoundEngine/Common/AkModule.h"
inline int g_NumSoundObjects, g_audio_queue_failsafe;
inline AkMemSettings g_memSettings;
inline const char *g_GlobalMouseOverSID = "Play_SFX_UI_MOUSEOVER_1", *g_GlobalToggleOnSID = "Play_SFX_UI_RADIOBUTTON_ON_1",
    *g_GlobalToggleOffSID = "Play_SFX_UI_RADIOBUTTON_OFF_1", *g_GlobalSelectSID = "Play_SFX_UI_Menu_Select_1";

void AUDIO_Init();
void AUDIO_Shutdown();
struct GameWorld;
struct Camera;
void GAME_AudioUpdate(GameWorld *not_used, Camera *a2, float a3);
void AUDIO_Event(const char *name, int a2, bool vital);
void AUDIO_SetVariable(const char *name, float val);
bool AUDIO_PlayFlatFile(const char *name, float val);
void AUDIO_SetObjectPosition(int, const VEC3 &, const VEC3 &);
void AUDIO_SetListenerPosition(int, const VEC3 &, const VEC3 &);
void AUDIO_UpdatePosition(int, const VEC3 &);
void AUDIO_Update(const VEC3 &, const VEC3 &, float);
void AUDIO_UnregisterObject(int);
void AUDIO_UnloadLevel(int);
void AUDIO_StopEvent(const char *, int, uint32_t);
void AUDIO_StopCapture();
//void AUDIO_StartCapture(std::weak_ptr<AUDIO_CaptureHandler> const &)
void AUDIO_SetVolume(int, float);
void AUDIO_SetVariableOnObject(const char *, float, uint32_t);
void AUDIO_SetPitch(int, float);
void AUDIO_ResumeEvent(const char *, int, uint32_t);
void AUDIO_ResumeAllAudio();
void AUDIO_RegisterNewObject(const char *);
//void AUDIO_Queue(float, char const *, int, uint, void *, char const *);
void AUDIO_PauseEvent(const char *, int, uint32_t);
void AUDIO_PauseAllAudio();
//void AUDIO_Log(AK::Monitor::ErrorCode, char const *, AK::Monitor::ErrorLevel, uint, ulong)
void AUDIO_LoadLevel(int);
void AUDIO_LoadData();
void AUDIO_HandleFailsafe();
void AUDIO_GetNumObjects();
void AUDIO_Event(uint32_t, int);