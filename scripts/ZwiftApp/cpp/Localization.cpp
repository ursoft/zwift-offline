#include "ZwiftApp.h"
void LOC_Shutdown() {
    //TODO
}
const char *g_langs[LOC_CNT] = { "en", "fr", "it", "de", "es", "ja", "nl", "ko", "zh" };
LOC_LANGS LOC_GetLanguageFromString(const char *lang) {
    for (int i = 0; i < _countof(g_langs); i++) {
        if (nullptr != strstr(lang, g_langs[i]))
            return (LOC_LANGS)i;
    }
    return LOC_CNT;
}
void LOC_SetLanguageFromEnum(LOC_LANGS l, bool cb) {
    if (g_CurrentLanguage != l) {
        g_CurrentLanguage = l;
        /*TODO if (g_GiantFontW[(unsigned int)&elf_gnu_hash_bucket[16902] + 1])
            CFont2D::LoadLanguageTextures((CFont2D *)g_GiantFontW, LanguageFromString);
        if (g_LargeFontW[0].n64_u8[(unsigned int)&elf_gnu_hash_bucket[16902] + 1])
            CFont2D::LoadLanguageTextures((CFont2D *)g_LargeFontW, g_CurrentLanguage);*/
        if (g_languageChangeCallback && cb)
            g_languageChangeCallback(l);
    }
}
bool LOC_SetLanguageFromString(const char *lang, bool cb) {
    auto nl = LOC_GetLanguageFromString(lang);
    if (nl != LOC_CNT) {
        LOC_SetLanguageFromEnum(nl, cb);
    }
    return nl != LOC_CNT;
}
void LOC_Init() {
    auto lange = g_UserConfigDoc.FindElement("ZWIFT\\CONFIG\\LANGUAGE", false);
    const char *lang;
    if (lange && (lang = lange->GetText())) {
        LOC_SetLanguageFromString(lang, false);
    } else {
        const char *oslang = OS_GetLanguage();
        LOC_SetLanguageFromString(oslang, false);
        g_UserConfigDoc.SetCStr("ZWIFT\\CONFIG\\LANGUAGE", oslang, true);
    }
}
void LOC_SetLanguageChangeCallback(void (*cb)(LOC_LANGS)) {
    g_languageChangeCallback = cb;
}
void LanguageChangeCallback(LOC_LANGS lang) {
    //TODO
}
void LOC_Initialize(void *data, size_t length, uint32_t what) {
    //TODO
}
void LOC_Initialize(const char *fileName, uint32_t what) {
    //TODO
}
void LOC_LoadStringTable(const char *fileName) {
    //TODO
}
const char *GetText(const char *loc_name) {
    //TODO
    return "TODO: GetText";
}