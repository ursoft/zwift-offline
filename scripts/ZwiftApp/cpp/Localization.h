#pragma once
enum LOC_LANGS { LOC_CNT = 9 };
extern LOC_LANGS g_CurrentLanguage;
extern void (*g_languageChangeCallback) (LOC_LANGS val);
void LOC_Init();
void LOC_Shutdown();
bool LOC_SetLanguageFromString(const char *lang, bool cb);
void LOC_SetLanguageFromEnum(LOC_LANGS, bool cb);
LOC_LANGS LOC_GetLanguageFromString(const char *lang);
