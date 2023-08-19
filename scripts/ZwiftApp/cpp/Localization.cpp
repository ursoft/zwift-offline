#include "ZwiftApp.h" //READY for testing
struct TextHelper {
    std::string m_ansi;
    std::u16string m_ustring;
};
typedef std::map<uint32_t, TextHelper> LocalizationMap;
LocalizationMap g_LocalizationMap[LOC_CNT];
std::map<uint32_t, std::vector<uint32_t>> g_LoadedStringTableToElementsMap;
void LOC_Shutdown() {
    for (auto &p : g_LocalizationMap) 
        p.clear();
}
const char *g_LocalizedLangs[LOC_CNT] = { "en", "fr", "it", "de", "es", "ja", "nl", "ko", "zh" };
LOC_LANGS LOC_GetLanguageFromString(const char *lang) {
    for (int i = 0; i < _countof(g_LocalizedLangs); i++) {
        if (nullptr != strstr(lang, g_LocalizedLangs[i]))
            return (LOC_LANGS)i;
    }
    return LOC_CNT;
}
const char *LOC_GetLanguage() {
    return g_LocalizedLangs[g_CurrentLanguage];
}
void LOC_SetLanguageFromEnum(LOC_LANGS l, bool cb) {
    if (g_CurrentLanguage != l) {
        g_CurrentLanguage = l;
        if (g_GiantFontW.m_loadedV3) {
            g_GiantFontW.LoadLanguageTextures(l);
            l = g_CurrentLanguage;
        }
        if (g_LargeFontW.m_loadedV3)
            g_LargeFontW.LoadLanguageTextures(l);
        if (g_languageChangeCallback && cb)
            g_languageChangeCallback(l);
    }
}
bool LOC_SetLanguageFromString(const char *lang, bool cb) {
    auto nl = LOC_GetLanguageFromString(lang);
    if (nl != LOC_CNT)
        LOC_SetLanguageFromEnum(nl, cb);
    return nl != LOC_CNT;
}
void LOC_Init() {
    auto lange = g_UserConfigDoc.FindElement("ZWIFT\\CONFIG\\LANGUAGE", false);
    const char *lang = lange ? lange->GetText() : nullptr;
    if (lang) {
        LOC_SetLanguageFromString(lang, false);
    } else {
        const char *oslang = OS_GetLanguage();
        LOC_SetLanguageFromString(oslang, false);
        g_UserConfigDoc.SetCStr("ZWIFT\\CONFIG\\LANGUAGE", oslang);
    }
}
void LOC_SetLanguageChangeCallback(void (*cb)(LOC_LANGS)) { g_languageChangeCallback = cb; }
void LanguageChangeCallback(LOC_LANGS lang) {
    g_LargeFontW.Load(FS_FONDO_MED);
    g_GiantFontW.Load(FS_FONDO_BLACK);
    g_LargeFontW.SetHeadAndBaseLines(14.0f, 20.0f);
    auto el = g_UserConfigDoc.FindElement("ZWIFT\\CONFIG\\LANGUAGE", true);
    if (el)
        el->SetText(LOC_GetLanguage());
    g_UserConfigDoc.Save();
    HUD_UpdateChatFont();
    HUD_RefreshText();
#if 0 //TODO
    v17 = 0i64;
    v9 = *((_QWORD *)&xmmword_7FF6A2346A00 + 1);
    if (*((_QWORD *)&xmmword_7FF6A2346A00 + 1))
    {
        v10 = *(_DWORD *)(*((_QWORD *)&xmmword_7FF6A2346A00 + 1) + 8i64);
        if (v10)
        {
            while (1)
            {
                v11 = v10;
                v10 = _InterlockedCompareExchange((volatile signed __int32 *)(v9 + 8), v10 + 1, v10);
                if (v11 == v10)
                    break;
                if (!v10)
                    goto LABEL_19;
            }
            v17 = pChangeLanguageSignal;
        }
    }
LABEL_19:
    if ((_QWORD)v17)
        IoCpp::Notifier::NotifyObservers(v17, v9);
#endif
    if (ZNETWORK_IsLoggedIn()) {
        auto mb = BikeManager::Instance()->m_mainBike;
        mb->m_profile.set_preferred_language(LOC_GetLanguage());
        mb->SaveProfile(true, false);
    }
    //OMIT CrashReporting::SetLanguage
}
void LOC_ProcessXmlDoc(tinyxml2::XMLDocument *doc, uint32_t fileNameCI) {
    auto &vec = g_LoadedStringTableToElementsMap[fileNameCI];
    bool fillVec = vec.empty();
    for (auto i = doc->FirstChildElement(); i; i = i->NextSiblingElement()) {
        auto val = i->Value();
        if (*val == 'Z' && val[1] == 'L' && !val[2]) {
            for (auto j = i->FirstChild(); j; j = j->NextSibling()) {
                auto v6 = j->ToElement();
                if (v6) {
                    auto v7 = v6->Attribute("LOC_ID");
                    auto v8 = SIG_CalcCaseInsensitiveSignature(v7);
                    if (fillVec)
                        vec.push_back(v8);
                    auto v17 = j->LastChildElement();
                    auto v18 = 0;
                    while (v17) {
                        if (v18 >= LOC_CNT) {
                            Log("***********LOCALIZATION DATA HAS MORE LANGUAGES THAN THE SYSTEM CAN SUPPORT. PLEASE UPDATE LOCALIZATION CODE.***********");
                        } else {
                            auto text = v17->Attribute("value");
                            g_LocalizationMap[v18][v8] = TextHelper{text, ToUTF8_ib(text)};
                            ++v18;
                        }
                        v17 = v17->PreviousSiblingElement();
                        if (!v17 && v18 != LOC_CNT)
                            break;
                    }
                }
            }
            return;
        }
    }
}
bool LOC_Initialize(void *data, size_t length, uint32_t fileNameCI) {
    CONSOLE_AddCommand("lang", CMD_SetLanguage);
    bool ret = true;
    tinyxml2::XMLDocument v25;
    auto err = v25.Parse((const char *)data, length);
    if (err != tinyxml2::XML_SUCCESS)
        LogDebug("LOC_Initialize: unable to load langData (error: %d)", (int)err);
    else
        LOC_ProcessXmlDoc(&v25, fileNameCI);
    return ret;
}
bool LOC_Initialize(const char *fileName, uint32_t fileNameCI) {
    CONSOLE_AddCommand("lang", CMD_SetLanguage);
    bool ret = true;
    tinyxml2::XMLDocument v34;
    auto err = v34.LoadFile(GAMEPATH(fileName));
    if (err != tinyxml2::XML_SUCCESS) {
        if (err != tinyxml2::XML_ERROR_FILE_NOT_FOUND && err != tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED && err != tinyxml2::XML_ERROR_FILE_READ_ERROR)
            LogDebug("LOC_Initialize: unable to load %s (error: %d)", fileName, (int)err);
        ret = false;
    } else {
        LOC_ProcessXmlDoc(&v34, fileNameCI);
    }
    return ret;
}
void LOC_LoadStringTable(const char *fileName) {
    if (fileName && *fileName) {
        auto fileNameSign = SIG_CalcCaseInsensitiveSignature(fileName);
        if (!LOC_Initialize(fileName, fileNameSign)) {
            if (strstr(fileName, "data/") == fileName) {
                auto wh = g_WADManager.GetWadFileHeaderByItemName(fileName + 5, WAD_ASSET_TYPE::GLOBAL, nullptr);
                if (wh)
                    LOC_Initialize(wh->FirstChar(), wh->m_fileLength, fileNameSign);
            }
        }
    }
}
const TextHelper &GetTextHelper(const char *loc_name, LOC_LANGS lang) {
    auto crcCI = SIG_CalcCaseInsensitiveSignature(loc_name);
    auto &result = g_LocalizationMap[lang][crcCI];
    if (result.m_ansi.empty() && lang != LOC_ENG)
        return g_LocalizationMap[LOC_ENG][crcCI];
    return result;
}
const char *GetText(const char *loc_name, LOC_LANGS lang) {
    auto &th = GetTextHelper(loc_name, lang);
    auto result = th.m_ansi.c_str();
    if (*result == '%' && result[1] == 'n' && !result[2])
        return "";
    return result;
}
const char *GetText(const char *loc_name) { return GetText(loc_name, g_CurrentLanguage); }
const UChar *GetTextW(const char *loc_name) {
    auto &th = GetTextHelper(loc_name, g_CurrentLanguage);
    auto result = th.m_ustring.c_str();
    if (*result == u'%' && result[1] == u'n' && !result[2])
        return u"";
    return result;
}
bool LOC_StringKeyExists(const char *key, LOC_LANGS lang) {
    return g_LocalizationMap[lang].contains(SIG_CalcCaseInsensitiveSignature(key));
}
void LOC_UnloadStringTable(const char *name) {
    if (name && *name) {
        auto v1 = SIG_CalcCaseInsensitiveSignature(name);
        auto f = g_LoadedStringTableToElementsMap.find(v1);
        if (f != g_LoadedStringTableToElementsMap.end()) {
            for (auto h : f->second)
                for (auto &lang : g_LocalizationMap) 
                    lang.erase(h);
            g_LoadedStringTableToElementsMap.erase(f);
        }
    }
}