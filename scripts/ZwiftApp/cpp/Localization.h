#pragma once
enum LOC_LANGS { LOC_CNT = 9 };
extern LOC_LANGS g_CurrentLanguage;
extern void (*g_languageChangeCallback) (LOC_LANGS val);
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