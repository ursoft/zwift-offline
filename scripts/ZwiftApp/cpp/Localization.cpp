#include "ZwiftApp.h"
struct TextHelper {
    std::string m_ansi;
    std::u16string m_ustring;
};
typedef std::map<uint32_t, TextHelper> LocalizationMap;
LocalizationMap g_LocalizationMap[LOC_CNT];
std::map<uint32_t, uint32_t> g_LoadedStringTableToElementsMap;
void LOC_Shutdown() {
    for (auto &p : g_LocalizationMap) 
        p.clear();
}
const char *g_langs[LOC_CNT] = { "en", "fr", "it", "de", "es", "ja", "nl", "ko", "zh" };
LOC_LANGS LOC_GetLanguageFromString(const char *lang) {
    for (int i = 0; i < _countof(g_langs); i++) {
        if (nullptr != strstr(lang, g_langs[i]))
            return (LOC_LANGS)i;
    }
    return LOC_CNT;
}
bool byte_7FF6A22E02F9, byte_7FF6A2342FE9; //TODO
void LOC_SetLanguageFromEnum(LOC_LANGS l, bool cb) {
    if (g_CurrentLanguage != l) {
        g_CurrentLanguage = l;
        if (byte_7FF6A22E02F9) {
            g_GiantFontW.LoadLanguageTextures(l);
            l = g_CurrentLanguage;
        }
        if (byte_7FF6A2342FE9)
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
#if 0 //TODO
    v0 = &g_LargeFontW;
    v1 = sub_7FF6A13588A0();
    Element = XMLDoc::FindElement(&g_UserConfigDoc, "ZWIFT\\CONFIG\\LANGUAGE", 1);
    if (Element)
        xml::set_text(Element, v1);
    v3 = XMLDoc::FindElement(&g_UserConfigDoc, "ZWIFT\\CONFIG\\LANGUAGE", 0);
    v4 = v3;
    if (v3)
    {
        firstChild = v3->node._firstChild;
        if (firstChild)
        {
            if ((*((__int64(__fastcall **)(XMLNode *))firstChild->vptr + 2))(firstChild))
            {
                v6 = (*((__int64(__fastcall **)(XMLNode *))v4->node._firstChild->vptr + 2))(v4->node._firstChild);
                StrPair::GetStr(v6 + 24);
            }
        }
    }
    (*((void(__fastcall **)(XMLDoc *, char *, _QWORD))g_UserConfigDoc.vptr + 2))(&g_UserConfigDoc, aCUsersBuilderD, 0i64);
    if (LOC_GetLanguageIndex() != LOC_JAPAN)
        v0 = &g_GiantFontW;
    g_ChatFontGW = v0;
    if (g_pDialogs[15])
        sub_7FF6A10485E0(g_pDialogs[15]);
    v7 = sub_7FF6A13588A0();
    v18._Bx._Ptr = 0i64;
    v18._Mysize = 0i64;
    v18._Myres = 15i64;
    v8 = -1i64;
    do
        ++v8;
    while (*((_BYTE *)v7 + v8));
    string_assign(&v18, (char *)v7, v8);
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
            v17 = xmmword_7FF6A2346A00;
        }
    }
LABEL_19:
    if ((_QWORD)v17)
        sub_7FF6A0B9C460(v17, v9);
    if (*((_QWORD *)&v17 + 1))
    {
        if (_InterlockedExchangeAdd((volatile signed __int32 *)(*((_QWORD *)&v17 + 1) + 8i64), 0xFFFFFFFF) == 1)
        {
            (***((void(__fastcall ****)(_QWORD)) & v17 + 1))(*((_QWORD *)&v17 + 1));
            if (_InterlockedExchangeAdd((volatile signed __int32 *)(*((_QWORD *)&v17 + 1) + 12i64), 0xFFFFFFFF) == 1)
                (*(void(__fastcall **)(_QWORD))(**((_QWORD **)&v17 + 1) + 8i64))(*((_QWORD *)&v17 + 1));
        }
    }
    if (ZNETWORK_IsLoggedIn()) {
        m_mainBike = BikeManager::g_pBikeManager->m_mainBike;
        *(_DWORD *)&m_mainBike->field_11F8[504] |= 0x200u;
        v14 = (_QWORD *)(*(_QWORD *)&m_mainBike->field_11F8[496] & 0xFFFFFFFFFFFFFFFCui64);
        if ((m_mainBike->field_11F8[496] & 1) != 0)
            v14 = (_QWORD *)*v14;
        sub_7FF6A1526300((unsigned __int64 *)&m_mainBike->field_13F8[208], v18._Bx._Buf, (__int64)v14);
        BikeEntity::SaveProfile(BikeManager::g_pBikeManager->m_mainBike, 1u, 0);
    }
    result = sub_7FF6A0B7A0F0(v12, (boost::math::detail *)&v18);
#endif
}
void LOC_ProcessXmlDoc(tinyxml2::XMLDocument *doc, uint32_t fileNameCI) {
    for (auto i = doc->FirstChildElement(); i; i = i->NextSiblingElement()) {
        auto val = i->Value();
        if (*val == 'Z' && val[1] == 'L' && !val[2]) {
            for (auto j = i->FirstChild(); j; j = j->NextSibling()) {
                auto v6 = j->ToElement();
                if (v6) {
                    auto v7 = v6->Attribute("LOC_ID");
                    auto v8 = SIG_CalcCaseInsensitiveSignature(v7);
                    if (fileNameCI && !g_LocalizationMap->contains(v8))
                        g_LoadedStringTableToElementsMap[fileNameCI] = v8;
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