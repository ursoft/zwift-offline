#pragma once
#include "AK/SoundEngine/Common/AkMemoryMgr.h"
#include "AK/SoundEngine/Common/AkModule.h"
extern int g_NumSoundObjects;
extern AkMemSettings g_memSettings;

void AUDIO_Init();
class GameWorld;
class Camera;
void GAME_AudioUpdate(GameWorld *not_used, Camera *a2, float a3);