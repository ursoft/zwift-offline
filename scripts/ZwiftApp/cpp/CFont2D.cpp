#include "ZwiftApp.h"
bool CFont2D::LoadFont(const char *name) {
    bool result = false;
    auto name_ = GAMEPATH(name);
    FILE *f = fopen(name_, "rb");
    if (f) {
        uint16_t ver;
        fread(&ver, sizeof(ver), 1, f);
        if (ver == 0x100) {
            m_info.m_fileHdrV1.m_version = ver;
            fread(((uint8_t *)&m_info.m_fileHdrV1) + sizeof(ver), sizeof(CFont2D_fileHdrV1) - sizeof(ver), 1, f);
            fread(m_info.m_field_190 + m_info.m_fileHdrV1.m_from, sizeof(CFont2D_v1_8b), m_info.m_fileHdrV1.m_reserve, f);
            if (g_fontShader == -1)
                g_fontShader = GFX_CreateShaderFromFile("GFXDRAW_FontW", -1);
            auto cb = m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_width * 4;
            m_RGBAv1 = malloc(cb);
            //memset(m_RGBAv1, 17, 4 * m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_height); //QUEST why not m_height * m_width ???
            //assert(m_info.m_fileHdrV1.m_width >= m_info.m_fileHdrV1.m_height);
            auto r = fread(m_RGBAv1, 1, cb, f);
            assert(r == cb); (void)r;
            LoadFontV1((uint8_t *)m_RGBAv1);
            m_info.m_field_190[33].m_field_4 *= 0.7f;
            result = true;
            m_loadedV1 = 1;
        } else {
            if (ver == 0x300) {
                //inlined CFont2D::LoadFontV3(name):
                if (m_loadedV3 && strstr(m_fileName.c_str(), name_)) {
                    result = true;
                } else {
                    m_fileName = name_;
                    static_assert(sizeof(CFont2D_fileHdrV3) == 0xF0);
                    m_info.m_fileHdrV3.m_version = ver;
                    fread(((uint8_t *)&m_info.m_fileHdrV3) + sizeof(ver), sizeof(CFont2D_fileHdrV3) - sizeof(ver), 1, f);
                    memset(m_info.m_glyphIndexes, 255, sizeof(m_info.m_glyphIndexes)); //last not used?
                    if (m_glyphs)
                        free(m_glyphs);
                    m_glyphs = (CFont2D_glyph *)malloc(sizeof(CFont2D_glyph) * m_info.m_fileHdrV3.m_usedGlyphs);
                    fread(m_glyphs, sizeof(CFont2D_glyph), m_info.m_fileHdrV3.m_usedGlyphs, f);
                    for (int g = 0; g < m_info.m_fileHdrV3.m_usedGlyphs; ++g) {
                        auto codePoint = m_glyphs[g].m_codePoint;
                        if (codePoint == 0xFFFF) {
                            Log("Loading font with codepoint beyond max: %d\n", codePoint);
                        } else {
                            m_info.m_glyphIndexes[codePoint] = g;
                            if (uint8_t(m_glyphs[g].m_cnt - 1) <= 2) {
                                bool found = false;
                                auto &vect = m_struc24x4[m_glyphs[g].m_cnt].m_cont;
                                for (auto x : vect) {
                                    if (x == m_glyphs[g].m_kernIdx)
                                        found = true;
                                }
                                if (!found)
                                    vect.push_back(m_glyphs[g].m_kernIdx);
                            }
                            if (codePoint == 33)
                                m_glyphs[g].m_width *= 0.7f;
                        }
                    }
                    char tname[MAX_PATH], *pDest = tname;
                    auto pSrc = m_fileName.c_str();
                    while (*pSrc) {
                        if (*pSrc == '_' || *pSrc == '.')
                            break;
                        *pDest++ = *pSrc++;
                    }
                    strcpy_s(pDest, sizeof(tname) - (pDest - tname), "0.tga");
                    m_info.m_fileHdrV3.m_tex1 = GFX_CreateTextureFromTGAFile(tname, -1, true);
                    if (g_fontWShader == -1)
                        g_fontWShader = GFX_CreateShaderFromFile("GFXDRAW_FontW", -1);
                    LoadLanguageTextures(LOC_GetLanguageIndex());
                    result = true;
                    m_loadedV3 = true;
                }
            } else {
                Log("Unknown font version number from %s(%x)\n", name_, ver);
            }
        }
    }
    if (f)
        fclose(f);
    return result;
}
void CFont2D::LoadLanguageTextures(LOC_LANGS left) {
    GFX_SetLoadedAssetMode(true);
    m_texSuffix = LID_LAT;
    switch (left) {
        default:
            break;
        case LOC_JAPAN:
            m_texSuffix = LID_JAPAN;
            break;
        case LOC_KOREAN:
            m_texSuffix = LID_KOREAN;
            break;
        case LOC_CHINESE:
            m_texSuffix = LID_CHINESE;
            break;
    }
    if (g_fontWShader == -1)
        g_fontWShader = GFX_CreateShaderFromFile("GFXDRAW_FontW", -1);
    if (m_info.m_fileHdrV3.m_tex2 != -1) {
        GFX_UnloadTexture(m_info.m_fileHdrV3.m_tex2);
        m_info.m_fileHdrV3.m_tex2 = -1;
    }
    char       tname[MAX_PATH], *dest = tname;
    const char *src = m_fileName.c_str();
    tname[0] = 0;
    while (*src) {
        if (*src == '.' || *src == '_') {
            *dest++ = '0' + m_texSuffix;
            strcpy_s(dest, sizeof(tname) - (dest - tname), ".tga");
            break;
        }
        *dest++ = *src++;
    }
    m_info.m_langId = m_texSuffix;
    if (m_texSuffix)
        m_info.m_fileHdrV3.m_tex2 = GFX_CreateTextureFromTGAFile(tname, -1, true);
    m_lineHeight = 0;
    for (int i = 0; i < m_info.m_fileHdrV3.m_usedGlyphs; i++) {
        auto v23 = int(0.5f + m_info.m_fileHdrV3.m_kern[m_glyphs[i].m_kernIdx] * m_glyphs[i].m_height);
        if (v23 > m_lineHeight)
            m_lineHeight = v23;
    }
    if (g_CurrentAssetCategory == AC_2)
        GFX_SetLoadedAssetMode(false);
}
bool CFont2D::LoadFontFromWad(const char *name) {
    auto wh = g_WADManager.GetWadFileHeaderByItemName(name + 5, WAD_ASSET_TYPE::GLOBAL, nullptr);
    bool result = false;
    static_assert(sizeof(CFont2D_fileHdrV1) == 0x9C);
    if (wh) {
        CFont2D_fileHdrV1 *pFontHeaderV1 = (CFont2D_fileHdrV1 *)wh->FirstChar();
        if (pFontHeaderV1->m_version == 0x100) {
            m_info.m_fileHdrV1 = *pFontHeaderV1;
            auto cb_3 = sizeof(CFont2D_v1_8b) * m_info.m_fileHdrV1.m_reserve;
            memmove(m_info.m_field_190 + m_info.m_fileHdrV1.m_from, pFontHeaderV1 + 1, cb_3);
            //OMIT m_RGBAv1 = malloc(m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_width * 4);
            //memset(m_RGBAv1, 0x11, 4 * m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_height); //QUEST why not m_height * m_width ???
            //assert(m_info.m_fileHdrV1.m_width >= m_info.m_fileHdrV1.m_height);
            LoadFontV1((uint8_t *)(pFontHeaderV1 + 1) + cb_3);
            m_info.m_field_190[33].m_field_4 *= 0.7f;
            result = true;
            m_loadedV1 = 1;
        } else {
            if (pFontHeaderV1->m_version == 0x300) {
                //inlined LoadFontFromWadV3(name);
                if (m_loadedV3 && strstr(m_fileName.c_str(), name)) {
                    result = true;
                } else {
                    m_fileName = name;
                    CFont2D_fileHdrV3 *pFontHeaderV3 = (CFont2D_fileHdrV3 *)pFontHeaderV1;
                    m_info.m_fileHdrV3 = *pFontHeaderV3;
                    memset(m_info.m_glyphIndexes, 255, sizeof(m_info.m_glyphIndexes)); //last not used?
                    if (m_glyphs)
                        free(m_glyphs);
                    auto gsz = sizeof(CFont2D_glyph) * m_info.m_fileHdrV3.m_usedGlyphs;
                    m_glyphs = (CFont2D_glyph *)malloc(gsz);
                    memmove(m_glyphs, pFontHeaderV3 + 1, gsz);
                    for (int g = 0; g < m_info.m_fileHdrV3.m_usedGlyphs; ++g) {
                        auto codePoint = m_glyphs[g].m_codePoint;
                        if (codePoint == 0xFFFF) {
                            Log("Loading font with codepoint beyond max: %d\n", codePoint);
                        } else {
                            m_info.m_glyphIndexes[codePoint] = g;
                            if (uint8_t(m_glyphs[g].m_cnt - 1) <= 2) {
                                bool found = false;
                                auto &vect = m_struc24x4[m_glyphs[g].m_cnt].m_cont;
                                for (auto x : vect) {
                                    if (x == m_glyphs[g].m_kernIdx)
                                        found = true;
                                }
                                if (!found)
                                    vect.push_back(m_glyphs[g].m_kernIdx);
                            }
                            if (codePoint == 33)
                                m_glyphs[g].m_width *= 0.7f;
                        }
                    }
                    char tname[MAX_PATH], *pDest = tname;
                    auto pSrc = m_fileName.c_str();
                    while (*pSrc) {
                        if (*pSrc == '_')
                            break;
                        *pDest++ = *pSrc++;
                    }
                    strcpy(pDest, "0.tga");
                    m_info.m_fileHdrV3.m_tex1 = GFX_CreateTextureFromTGAFile(tname, -1, true);
                    if (g_fontWShader == -1)
                        g_fontWShader = GFX_CreateShaderFromFile("GFXDRAW_FontW", -1);
                    LoadLanguageTextures(LOC_GetLanguageIndex());
                    result = true;
                    m_loadedV3 = true;
                }
            } else {
                Log("Unknown font version number from %s (%x)\n", name, pFontHeaderV1->m_version);
            }
        }
    }
    return result;
}
void CFont2D::LoadFontV1(const uint8_t *data) {
    if (g_fontShader == -1)
        g_fontShader = GFX_CreateShaderFromFile("GFXDRAW_FontW", -1);
    m_tex = GFXAPI_CreateTextureFromRGBA(m_info.m_fileHdrV1.m_width, m_info.m_fileHdrV1.m_height, data, true);
    m_lineHeight = 0;
    int16_t v6 = 0;
    for (auto x = m_info.m_fileHdrV1.m_from; x < m_info.m_fileHdrV1.m_to; x++) {
        static_assert(sizeof(CFont2D_v1_8b) == 8);
        auto v9 = m_info.m_field_190[x].m_field_6;
        if (v9 > v6) {
            v6 = v9;
            m_lineHeight = v9;
        }
    }
}
bool CFont2D::LoadDirect(const char *name) { return LoadFontFromWad(name) || LoadFont(name); }
void CFont2D::LoadDirectEast(const char *name1, const char *name2) {
    LoadDirect(name1);
    if (!m_loadedV3)
        LoadDirect(name2);
}
void CFont2D::Load(FONT_STYLE s) {
    switch (s) {
        case FS_SMALL:
            LoadDirect("data/Fonts/smallfont.bin");
            break;
        case FS_SANSERIF:
            LoadDirect("data/Fonts/sanserif14.bin");
            break;
        case FS_FONDO_MED:
            if (LOC_GetLanguageIndex() == LOC_CHINESE) {
                LoadDirectEast("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_C.bin", "data/Fonts/ZwiftFondoMedium54ptW_EFIGS_JK.bin");
            } else if (LOC_GetLanguageIndex() == LOC_JAPAN) {
                LoadDirectEast("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_J.bin", "data/Fonts/ZwiftFondoMedium54ptW_EFIGS_JK.bin");
            } else {
                LoadDirect("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_K.bin");
                m_field_20A54 = 1.98f;
                if (!m_loadedV3)
                    LoadDirect("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_JK.bin");
            }
            break;
        case FS_FONDO_BLACK:
            if (LOC_GetLanguageIndex() == LOC_CHINESE) {
                LoadDirectEast("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_C.bin", "data/Fonts/ZwiftFondoBlack105ptW_EFIGS_JK.bin");
            } else if (LOC_GetLanguageIndex() == LOC_JAPAN) {
                LoadDirectEast("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_J.bin", "data/Fonts/ZwiftFondoBlack105ptW_EFIGS_JK.bin");
            } else {
                LoadDirect("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_K.bin");
                m_field_20A54 = 1.5f;
                if (!m_loadedV3)
                    LoadDirect("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_JK.bin");
            }
            break;
        default:
            Log("Trying to Load unknown font");
            break;
    }
}
void CFont2D::SetScaleAndKerning(float scale, float kerning) {
    m_scale = scale;
    m_kerning = kerning;
}
CFont2D::CFont2D() {
    m_field_20A50 = 0;
    m_field_20A54 = 1.0f;
    m_glyphs = nullptr;
    m_RGBAv1 = nullptr;
    m_field_20A3C = 1.0f;
    m_scale = 1.0f;
    m_kerning = 1.0f;
    m_cache = nullptr;
    m_cacheCnt = 0;
    m_cacheCntUsed = 0;
    m_loadedV1 = false;
    m_loadedV3 = false;
    m_info.m_langId = LID_CNT;
    m_texSuffix = LID_LAT;
    m_kern[0] = 1.0f;
    m_kern[1] = 1.08935f;
    m_kern[2] = 1.0f;
    m_kern[3] = 1.08435f;
    m_headLine = 22.0f;
    m_baseLine = 10.0f;
}
CFont2D::~CFont2D() {
    free(m_RGBAv1);
    m_RGBAv1 = nullptr;
    free(m_glyphs);
    m_glyphs = nullptr;
}
void CFont2D::SetHeadAndBaseLines(float headLine, float baseLine) {
    m_headLine = headLine;
    m_baseLine = baseLine;
}
void CFont2D::SetLanguageKerningScalar(LANGUAGE_IDS lid, float kern) {
    if (lid < LID_CNT)
        m_kern[lid] = kern;
}
void CFont2D::StartCaching(uint32_t cnt) {
    if (g_bSupportFontCaching && cnt) {
        static_assert(sizeof(CFont2D_cache) == 192);
        auto v4 = GFX_DrawMalloc(sizeof(CFont2D_cache) * cnt, 4);
        m_cache = (CFont2D_cache *)v4;
        if (v4 != nullptr) {
            m_cacheCnt = cnt;
        } else {
            m_cacheCnt = 0;
        }
        m_cacheCntUsed = 0;
        m_curCache = 0;
    }
}
void CFont2D::RenderAllCachedContent(bool uiProjection) {
    auto cacheCnt = m_cacheCnt;
    auto v4 = EndCachingAndRender(uiProjection);
    StartCaching(cacheCnt);
    m_curCache = v4;
}
int CFont2D::EndCachingAndRender(bool uiProjection) {
    if (m_cache && m_cacheCntUsed) {
        auto sh = GFX_GetCurrentShaderHandle();
        if (m_loadedV3) {
            GFX_SetShader(g_fontWShader);
            GFX_ActivateTexture(m_info.m_fileHdrV3.m_tex1, 0, nullptr, TWM_CLAMP_TO_EDGE);
            GFX_ActivateTexture((m_info.m_fileHdrV3.m_tex2 != -1) ? m_info.m_fileHdrV3.m_tex2 : g_WhiteHandle, 2, nullptr, TWM_CLAMP_TO_EDGE);
            GFX_SetTextureFilter(2, GFF_LINEAR_MIPMAP_NEAREST);
            GFX_ActivateTextureEx(2, -0.5f);
            GFX_SetTextureFilter(0, GFF_LINEAR_MIPMAP_NEAREST);
            GFX_ActivateTextureEx(0, -0.5f);
        } else {
            GFX_SetShader(g_fontShader);
            GFX_ActivateTexture(m_tex, 0xFFFFFFFF, 0, TWM_CLAMP_TO_EDGE);
        }
        GFX_SetAlphaBlendEnable(true);
        GFX_SetBlendFunc(GBO_FUNC_ADD, GB_SRC_ALPHA, GB_ONE_MINUS_SRC_ALPHA);
        GFX_MatrixMode(GMT_2);
        GFX_PushMatrix();
        GFX_LoadIdentity();
        auto pRT = VRAM_GetCurrentRT();
        if (uiProjection)
            GFX_SetupUIProjection();
        else
            GFX_Ortho(0.0f, (float)pRT->m_dw_width, (float)pRT->m_dw_height, 0.0f, -1.0f, 1.0f);
        GFX_MatrixMode(GMT_1);
        GFX_PushMatrix();
        GFX_LoadIdentity();
        GFX_MatrixMode(GMT_0);
        GFX_PushMatrix();
        GFX_LoadIdentity();
        GFX_UpdateMatrices(false);
        GFX_DrawPrimitive(GPT_TRIANGLES, (DRAW_VERT_POS_COLOR_UV *)m_cache, 6 * m_cacheCntUsed);
        GFX_ActivateTextureEx(2, 0.0f);
        GFX_MatrixMode(GMT_2);
        GFX_PopMatrix();
        GFX_MatrixMode(GMT_1);
        GFX_PopMatrix();
        GFX_MatrixMode(GMT_0);
        GFX_PopMatrix();
        GFX_ActivateTextureEx(0, 0.0f);
        GFX_SetShader(sh);
    }
    auto result = m_curCache + m_cacheCntUsed;
    m_cache = nullptr;
    m_cacheCnt = 0;
    m_cacheCntUsed = 0;
    m_curCache = 0;
    return result;
}
CFont2D_cache *CFont2D::CacheMallocCharacters(uint32_t cnt, bool uiProjection) {
    if (m_cacheCntUsed + cnt > m_cacheCnt)
        RenderAllCachedContent(uiProjection);
    if (m_cacheCntUsed + cnt <= m_cacheCnt) {
        auto ret = m_cache + m_cacheCntUsed;
        m_cacheCntUsed += cnt;
        return ret;
    } else {
        zassert(0);
        return nullptr;
    }
}
bool CFont2D::PopHeadBase() {
    if (m_hbStack.empty())
        return false;
    const auto top = m_hbStack.back();
    m_headLine = top.m_data[0];
    m_baseLine = top.m_data[1];
    m_hbStack.pop_back();
    return true;
}
void CFont2D::PushHeadBase(float head, float base) {
    auto mult = m_field_20A3C * m_scale;
    m_hbStack.emplace_back(VEC2{ mult *m_headLine, mult * m_baseLine });
    m_headLine = head;
    m_baseLine = base;
}
float CFont2D::GetBaseline(float mult) { return m_baseLine * m_field_20A3C * m_scale * mult; }
float CFont2D::GetHeadline(float mult) { return m_headLine * m_field_20A3C * m_scale * mult; }
float CFont2D::GetHeadToBase(float mult) { return (GetHeight() - m_field_20A3C * m_baseLine * m_scale - m_field_20A3C * m_headLine * m_scale) * mult; }
float CFont2D::StringWidth(const char *text) {
    BufSafeToUTF8 buf;
    return StringWidthW(SafeToUTF8(text, &buf));
}
float CFont2D::StringWidthW(const UChar *uText) { return CFont2D::StringWidthW(uText, u_strlen(uText)); }
float CFont2D::StringWidthW(const UChar *uText, uint32_t textLen) {
    static_assert(sizeof(CFont2D_info) == 0x20990);
    static_assert(sizeof(CFont2D_glyph) == 20);

    if (!m_loadedV3 || !uText || !*uText || !textLen)
        return 0.0f;
    float       ret = 0.0f;
    const UChar *uEnd = uText + textLen;
    do {
        auto gidx = m_info.m_glyphIndexes[*uText];
        if (gidx != 0xFFFF) {
            float mult = (*uText == 32) ? this->m_field_20A54 : 1.0f;
            auto  kidx = m_glyphs[gidx].m_kernIdx;
            ret += m_kern[kidx] * m_kerning * m_info.m_fileHdrV3.m_kern[kidx] * m_glyphs[gidx].m_width * mult;
        }
    } while (++uText != uEnd);
    return ret * m_scale;
}
float CFont2D::GetBoundedScaleW(float w, float h, const UChar *uText) {
    auto needWidth = (float)StringWidthW(uText);
    auto needHeight = GetHeight();
    if (needWidth == 0.0f || needHeight == 0.0f)
        return 1.0f;
    else
        return fminf(w / needWidth, h / needHeight);
}
float CFont2D::GetHeight() {
    auto ret = m_lineHeight * m_field_20A3C * m_scale;
    if (ret > 0.0f)
        return ret;
    else
        return 1.0f;
}
UChar *SafeToUTF8(const char *ansi, BufSafeToUTF8 *buf) {
    if (!ansi) {
        static bool alreadyReported0;
        if (alreadyReported0)
            return 0i64;
        alreadyReported0 = true;
        LogTyped(LOG_ERROR, "SafeToUTF8--Passing null as the source text, %s", ansi);
        return 0i64;
    }
    if (buf->m_usageCounter++ != 0)
        buf = nullptr;
    if (buf)
        return ToUTF8(ansi, buf->m_buf, _countof(buf->m_buf));
    static bool alreadyReportedW;
    if (alreadyReportedW)
        return nullptr;
    alreadyReportedW = true;
    LogTyped(LOG_ERROR, "SafeToUTF8--Temp buffer already written, %s", ansi);
    return nullptr;
}
int CFont2D::GetParagraphLineCount(float w, const char *str, float, float, bool) {
    //TODO
    assert(false);
    return 0;
}
int CFont2D::FitCharsToWidthW(const UChar *str, int w) {
    if (!str || !m_loadedV3)
        return 0;
    int  len = u_strlen(str);
    auto v8 = 0.0f;
    if (!len)
        return len;
    int i = 0;
    while (true) {
        auto v11 = m_info.m_glyphIndexes[str[i]];
        if (v11 != 0xFFFF) {
            auto kernIdx = m_glyphs[v11].m_kernIdx;
            v8 += m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * m_glyphs[v11].m_width * m_scale * (str[i] == 32 ? m_field_20A54 : 1.0);
        }
        if (v8 >= w)
            break;
        if (++i >= len)
            return len;
    }
    return i;
}
int CFont2D::FitWordsToWidthW(const UChar *str, int w, float a4) {
    if (!str || !m_loadedV3)
        return 0;
    int  len = u_strlen(str);
    int  v9 = 0, v11 = 0, v12 = 0, v14 = 0;
    auto v15 = 0.0f;
    auto goalWidth = w - a4;
    bool v27 = false;
    for (; v11 <= len && !v27; ++v11) {
        auto v16 = 0.0f;
        for (; v11 < len; ++v11) {
            if (str[v11] == 32)
                break;
            if (str[v11] == 10)
                break;
        }
        while (str[v11] == 32)
            ++v11;
        for (int i = 0; i < v11; i++) {
            auto v21 = m_info.m_glyphIndexes[str[i]];
            if (v21 != 0xFFFF) {
                auto kernIdx = m_glyphs[v21].m_kernIdx;
                v16 += m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * m_glyphs[v21].m_width * m_scale * (str[i] == 32 ? m_field_20A54 : 1.0f);
            }
        }
        ++v14;
        if (str[v11] == 10) {
            if (goalWidth <= v16) {
                if (v14 == 1)
                    v12 = 1;
            } else {
                v9 = v11 + 1;
            }
            v27 = true;
            continue;
        }
        if (goalWidth <= v16) {
            if (v14 == 1) {
                auto v28 = 0;
                v16 = 0.0;
                if (v11) {
                    auto v29 = str;
                    while (1) {
                        auto v30 = m_info.m_glyphIndexes[*v29];
                        if (v30 != 0xFFFF) {
                            auto v33 = m_glyphs[v30].m_kernIdx;
                            v16 += m_kern[v33] * m_kerning * m_info.m_fileHdrV3.m_kern[v33] * m_glyphs[v30].m_width * m_scale * (*v29 == 32 ? m_field_20A54 : 1.0f);
                        }
                        if (v16 > goalWidth)
                            break;
                        ++v28;
                        ++v29;
                        if (v28 >= v11) {
                            v9 = v11;
                            v27 = true;
                            continue;
                        }
                    }
                    v11 = v28;
                }
                v9 = v11;
            }
            v27 = true;
            continue;
        }
        v27 = false;
        v9 = v11;
    }
    if (v12) {
        for (int v35 = 0; v35 < v11; ++v35) {
            auto v36 = str[v35];
            auto v37 = m_info.m_glyphIndexes[v36];
            if (v37 != 0xFFFF) {
                auto v40 = m_glyphs[v37].m_kernIdx;
                v15 += m_kern[v40] * m_kerning * m_info.m_fileHdrV3.m_kern[v40] * m_glyphs[v37].m_width * m_scale * ((v36 == 32) ? m_field_20A54 : 1.0f);
            }
            if (v15 > goalWidth) {
                v11 = v35;
                break;
            }
        }
        v9 = v11;
    }
    if (len < v9)
        v9 = len;
    if (v9)
        return v9;
    else
        return FitCharsToWidthW(str, goalWidth);
}
int CFont2D::GetParagraphLineCountW(float w, const UChar *str, float a4, float a5, bool a6) {
    if (!m_loadedV3 || !str || !*str)
        return 0;
    int len = u_strlen(str);
    if (!a6 && len > 0x400)
        return 1;
    int ret = 0, wordsLen;
    for (auto *end = str + len; str < end; str += wordsLen) {
        wordsLen = FitWordsToWidthW(str, int(1.0f / a4 * w), 1.0f / a4 * a5);
        a5 = 0.0f;
        if (!wordsLen)
            wordsLen = len;
        ++ret;
    }
    return ret;
}
int CFont2D::RenderParagraph(float a2, float a3, float a4, float a5, const char *str, uint32_t color, uint32_t flags, float a9, bool a10, float a11, float a12, float cropTop /*= 0.0f*/) {
    BufSafeToUTF8 buf;
    return RenderParagraphW(a2, a3, a4, a5, SafeToUTF8(str, &buf), color, flags, a9, a10, a11, a12);
}
int CFont2D::RenderParagraphW(float left, float top, float width, float height, const UChar *str, uint32_t color, uint32_t flags, float a9, bool needShadow, float a11, float a12, float cropTop /*= 0.0f*/) {
    if (!m_loadedV3 || left > GFX_UI_GetCurrentSpaceWidth() || left + width < 0.0f)
        return 0;
    auto ret = 0;
    auto v14 = str;
    auto v15 = a9 * m_field_20A3C * GetHeight() * a11;
    auto v16 = top;
    auto v17 = a12;
    auto v18 = u_strlen(str);
    if (flags & RF_CY_ISCENTER)
        v16 = height * 0.5f + top - v15 * GetParagraphLineCountW(width, str, a9, (1.0f / a9) * a12, 0) * 0.5f;
    else if (flags & RF_CY_ISTOP)
        v16 = top + height - v15 * GetParagraphLineCountW(width, str, a9, (1.0f / a9) * a12, 0);
    if (top + height <= v16)
        return ret;
    auto  v23 = &str[v18];
    UChar text[328];
    do {
        if (v14 >= v23)
            break;
        auto v24 = 1.0f / a9 * v17;
        auto v25 = 1.0f / a9 * width;
        auto v26 = FitWordsToWidthW(v14, v25, v24);
        auto v27 = 255;
        if (v26 < 0xFF)
            v27 = FitWordsToWidthW(v14, v25, v24);
        for (int i = 0; i < v27; ++i)
            text[i] = v14[i];
        text[v27] = 0;
        if (v16 >= cropTop)
            RenderWString(left + v17, v16, text, color, flags & ~(RF_CY_ISTOP | RF_CY_ISCENTER), a9, needShadow, 0, 1);
        v17 = 0.0f;
        v14 += v27;
        ++ret;
        v16 += v15;
    } while (top + height > v16);
    return ret;
}
void CFont2D::RenderWString(float cx, float cy, const char *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc) {
    BufSafeToUTF8 buf;
    auto          u8_text = SafeToUTF8(text, &buf);
    RenderWString(cx, cy, u8_text, color, flags, scale, needShadow, forceDrawMalloc, true);
}
void CFont2D::RenderWString(float cx, float cy, const UChar *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc, bool uiProjection) { //RenderWString_utf
    if (!m_loadedV3 || !text || !*text)
        return;
    auto v17 = m_scale * scale;
    auto v16 = m_lineHeight * v17;
    if (uiProjection && (-v16 > cy || cy > 1280.0f / VRAM_GetUIAspectRatio() + 10.0f))
        return;
    if (needShadow) {
        auto shadowOffset = fmaxf(1.0f, v16 / 25.0f);
        RenderWString(shadowOffset + cx, shadowOffset + cy, text, (color >> 1) & 0x7F000000, flags, scale, false, forceDrawMalloc, uiProjection);
    }
    if (flags & RF_CX_ISCENTER)
        cx -= StringWidthW(text) * scale * 0.5f;
    else if (flags & 4)
        cx -= StringWidthW(text) * scale;
    if (flags & RF_CY_ISCENTER)
        cy -= GetHeight() * scale * m_field_20A3C * 0.5f;
    else if (flags & RF_CY_ISTOP)
        cy -= GetHeight() * scale * m_field_20A3C;
    auto v15 = m_field_20A3C * v17;
    auto v25 = cy + v15 * m_field_20A50;
    auto textLen = u_strlen(text);
    auto v29 = (!m_cache || forceDrawMalloc) ? (CFont2D_cache *)GFX_DrawMalloc(sizeof(CFont2D_cache) * textLen, 4) : CacheMallocCharacters(textLen, uiProjection);
    if (v29) {
        int  v31 = -1;
        auto pflt = (float *)v29;
        if (textLen) {
            for (int i = 0; i < textLen; ++i, ++text) {
                auto v35 = v31;
                auto v36 = m_info.m_glyphIndexes[*text];
                if (v36 == 0xFFFF) {
                    if (m_cache && !forceDrawMalloc && m_cacheCntUsed) {
                        ++m_curCache;
                        m_cacheCntUsed--;
                    }
                } else {
                    auto       v39 = color;
                    const auto &g = m_glyphs[v36];
                    //if (g.m_kernIdx && g.m_kernIdx != m_texSuffix)
                    //    v39 = 0;
                    v31 = g.m_kernIdx;
                    if (v35 != -1)
                        v31 = v35;
                    auto  v42 = m_info.m_fileHdrV3.m_kern[g.m_kernIdx];
                    float dx;
                    if (*text == 32) {
                        dx = g.m_width * v42 * v17 * m_kerning * m_kern[g.m_kernIdx] * m_field_20A54;
                        if (m_cache && !forceDrawMalloc && m_cacheCntUsed) {
                            ++m_curCache;
                            m_cacheCntUsed--;
                        }
                    } else {
                        auto pint = (uint32_t *)pflt;
                        pflt[0] = cx;
                        pflt[1] = v25;
                        //QUEST: is it OK? auto v33 = pflt + 7;
                        pint[2] = 0;
                        pint[3] = v39;
                        pflt[4] = g.m_field_4 + 0.001f;
                        pflt[5] = g.m_field_8 + 0.001f;
                        pflt[9] = v25;
                        auto v45 = g.m_width - 0.002f + g.m_field_4 + 0.001f;
                        pflt[13] = g.m_field_8 + 0.001f;
                        pint[7] = 0;
                        pint[10] = 0;
                        pint[11] = v39;
                        pflt[12] = v45;
                        auto v46 = g.m_width * v42 * v17;
                        auto v47 = fminf(1.0, (float)g.m_kernIdx);
                        pflt[6] = v47;
                        auto v48 = g.m_height - 0.002f + g.m_field_8 + 0.001f;
                        pflt[8] = v46 + cx;
                        pflt[14] = v47;
                        pflt[16] = v46 + cx;
                        auto v49 = g.m_height * v42 * v15 + v25;
                        pflt[21] = v48;
                        pflt[20] = v45;
                        pint[15] = 0;
                        pint[18] = 0;
                        pflt[17] = v49;
                        pint[19] = v39;
                        pflt[29] = v48;
                        pflt[22] = v47;
                        pflt[24] = cx;
                        pflt[25] = v49;
                        pflt[28] = g.m_field_4 + 0.001f;
                        pint[23] = 0;
                        pint[26] = 0;
                        pint[27] = v39;
                        pflt[37] = v48;
                        pflt[30] = v47;
                        pflt[32] = v46 + cx;
                        pflt[33] = v49;
                        pflt[36] = v45;
                        pint[31] = 0;
                        pint[34] = 0;
                        pint[35] = v39;
                        pint[43] = v39;
                        pflt[38] = v47;
                        pflt[40] = cx;
                        pflt[41] = v25;
                        pflt[44] = g.m_field_4 + 0.001f;
                        pflt[45] = g.m_field_8 + 0.001f;
                        pint[39] = 0;
                        pint[42] = 0;
                        pflt[46] = v47;
                        pint[47] = 0;
                        dx = m_kerning * m_kern[g.m_kernIdx] * v46;
                        pflt += 48;
                    }
                    cx += dx;
                }
            }
        }
        if (!m_cache || forceDrawMalloc) {
            auto CurrentShaderHandle = GFX_GetCurrentShaderHandle();
            GFX_SetShader(g_fontWShader);
            GFX_SetAlphaBlendEnable(true);
            GFX_SetBlendFunc(GBO_FUNC_ADD, GB_SRC_ALPHA, GB_ONE_MINUS_SRC_ALPHA);
            if (m_loadedV3) {
                GFX_ActivateTexture(m_info.m_fileHdrV3.m_tex1, 0, nullptr, TWM_CLAMP_TO_EDGE);
                GFX_ActivateTexture((m_info.m_fileHdrV3.m_tex2 != -1) ? m_info.m_fileHdrV3.m_tex2 : g_WhiteHandle, 2, nullptr, TWM_CLAMP_TO_EDGE);
            }
            GFX_SetTextureFilter(2, GFF_LINEAR_MIPMAP_NEAREST);
            GFX_ActivateTextureEx(2, -0.5f);
            GFX_SetTextureFilter(0, GFF_LINEAR_MIPMAP_NEAREST);
            GFX_ActivateTextureEx(0, -0.5f);
            GFX_MatrixMode(GMT_2);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            auto CurrentRT = VRAM_GetCurrentRT();
            if (uiProjection)
                GFX_SetupUIProjection();
            else
                GFX_Ortho(0.0, (float)CurrentRT->m_dw_width, (float)CurrentRT->m_dw_height, 0.0f, -1.0f, 1.0f);
            GFX_MatrixMode(GMT_0);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_MatrixMode(GMT_1);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_UpdateMatrices(0);
            GFX_DrawPrimitive(GPT_TRIANGLES, (DRAW_VERT_POS_COLOR_UV *)v29, ((char *)pflt - (char *)v29) >> 5);
            GFX_MatrixMode(GMT_2);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_0);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_1);
            GFX_PopMatrix();
            GFX_ActivateTextureEx(2, -0.5f);
            GFX_ActivateTextureEx(0, -0.5f);
            GFX_SetShader(CurrentShaderHandle);
        }
    }
}
