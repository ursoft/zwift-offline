#include "ZwiftApp.h"
void AUDIO_Init() {
    g_NumSoundObjects = 0;
    AK::MemoryMgr::GetDefaultSettings(g_memSettings);
    //TODO
}
void AUDIO_Shutdown() {
    //TODO
}
void GAME_AudioUpdate(GameWorld *, Camera *camera, float a3) {
    if (camera && g_pGameWorld) {
        //TODO
    }
}