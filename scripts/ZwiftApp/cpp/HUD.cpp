#include "ZwiftApp.h"
void HUD_UpdateChatFont() {
    g_ChatFontGW = (LOC_GetLanguageIndex() == LOC_JAPAN) ? &g_LargeFontW : &g_GiantFontW;
}
void HUD_Initialize() {
    //TODO
}
void HUD_OnEscape()
{
}
int HUD_GetFlagTextureFromISO3166(uint32_t countryCode, bool) {
    //TODO
    return 0;
}
void HUD_Notify(const char *msg, float a2, HUD_NotificationType nt, int texture, uint32_t not_used1, float not_used2, int64_t playerId) {
    //TODO
}