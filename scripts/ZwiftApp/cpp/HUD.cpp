#include "ZwiftApp.h"
void HUD_UpdateChatFont() {
    g_ChatFontGW = (LOC_GetLanguageIndex() == LOC_JAPAN) ? &g_LargeFontW : &g_GiantFontW;
}
void HUD_Initialize() {
    //TODO
}
