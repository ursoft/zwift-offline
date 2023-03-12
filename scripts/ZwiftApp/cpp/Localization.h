#pragma once
enum LOC_LANGS { LOC_ENG = 0, LOC_FRAN = 1, LOC_ITAL = 2, LOC_GERMAN = 3, LOC_ESPAN = 4, LOC_JAPAN = 5, LOC_NL = 6, LOC_KOREAN = 7, LOC_CHINESE = 8, LOC_CNT = 9 };
inline LOC_LANGS g_CurrentLanguage;
inline LOC_LANGS LOC_GetLanguageIndex() { return g_CurrentLanguage; }
inline void (*g_languageChangeCallback) (LOC_LANGS val);
void LOC_Init();
void LOC_Shutdown();
bool LOC_SetLanguageFromString(const char *lang, bool cb);
void LOC_SetLanguageFromEnum(LOC_LANGS, bool cb);
LOC_LANGS LOC_GetLanguageFromString(const char *lang);
void LOC_SetLanguageChangeCallback(void (*cb)(LOC_LANGS));
void LanguageChangeCallback(LOC_LANGS);
void LOC_Initialize(void *data, size_t length, uint32_t what);
void LOC_Initialize(const char *fileName, uint32_t what);
void LOC_LoadStringTable(const char *fileName);
const char *GetText(const char *loc_name);