#include "ZwiftApp.h"
void make_sound(const char *snd) {
    AUDIO_Event(snd, 1, false);
}
void GUIDRAW_LoadAssets(bool testServer) {
    //TODO
}
void GUI_Initialize(/*void (*mkSound)(char const *),*/ bool testServer) {
    //TODO
    GUIDRAW_LoadAssets(testServer);
    //qword_7FF6CA8C8DA0 = 0i64;
    //xmmword_7FF6CA8C8D90 = 0i64;
    //GUI_Obj::SetUIText(g_GUIRoot, "ROOT");
    //dword_7FF6CA8C8D7C = 0;
}
void GUI_INTERNAL_PlaySound(const char *snd) { make_sound(snd); }
bool GUI_Key(int a1, int a2) {
    //TODO
    return true;
}