#include "ZwiftApp.h"
int g_NumSoundObjects;
AkMemSettings g_memSettings;

void AUDIO_Init() {
	g_NumSoundObjects = 0;
	AK::MemoryMgr::GetDefaultSettings(g_memSettings);
	//TODO
}
