#pragma once
inline CFont2D *g_pGUI_GlobalFont;

void GUI_Initialize(/*void (*func)(char const *),*/ bool testServer);
inline void GUI_SetDefaultFont(CFont2D *v) { g_pGUI_GlobalFont = v; }