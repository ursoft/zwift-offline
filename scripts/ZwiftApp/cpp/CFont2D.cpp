#include "ZwiftApp.h" //READY for testing
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
            fread(m_info.m_v1gls + m_info.m_fileHdrV1.m_from, sizeof(tViewport<uint16_t>), m_info.m_fileHdrV1.m_chars, f);
            if (g_fontShader == -1)
                g_fontShader = GFX_CreateShaderFromFile("GFXDRAW_FontW", -1);
            auto cb = m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_width * 4;
            m_RGBAv1 = malloc(cb);
            //memset(m_RGBAv1, 17, 4 * m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_height); //QUEST why not m_height * m_width ???
            //assert(m_info.m_fileHdrV1.m_width >= m_info.m_fileHdrV1.m_height);
            auto r = fread(m_RGBAv1, 1, cb, f);
            assert(r == cb); (void)r;
            LoadFontV1((uint8_t *)m_RGBAv1);
            m_info.m_v1gls[33].m_width *= 0.7f;
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
                                m_glyphs[g].m_view.m_width *= 0.7f;
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
                    m_info.m_fileHdrV3.m_tex[0] = GFX_CreateTextureFromTGAFile(tname, -1, true);
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
    if (m_info.m_fileHdrV3.m_tex[1] != -1) {
        GFX_UnloadTexture(m_info.m_fileHdrV3.m_tex[1]);
        m_info.m_fileHdrV3.m_tex[1] = -1;
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
        m_info.m_fileHdrV3.m_tex[1] = GFX_CreateTextureFromTGAFile(tname, -1, true);
    m_lineHeight = 0;
    for (int i = 0; i < m_info.m_fileHdrV3.m_usedGlyphs; i++) {
        auto v23 = int(0.5f + m_info.m_fileHdrV3.m_kern[m_glyphs[i].m_kernIdx] * m_glyphs[i].m_view.m_height);
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
            auto cb_3 = sizeof(tViewport<uint16_t>) * m_info.m_fileHdrV1.m_chars;
            memmove(m_info.m_v1gls + m_info.m_fileHdrV1.m_from, pFontHeaderV1 + 1, cb_3);
            //OMIT m_RGBAv1 = malloc(m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_width * 4);
            //memset(m_RGBAv1, 0x11, 4 * m_info.m_fileHdrV1.m_height * m_info.m_fileHdrV1.m_height); //QUEST why not m_height * m_width ???
            //assert(m_info.m_fileHdrV1.m_width >= m_info.m_fileHdrV1.m_height);
            LoadFontV1((uint8_t *)(pFontHeaderV1 + 1) + cb_3);
            m_info.m_v1gls[33].m_width *= 0.7f;
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
                                m_glyphs[g].m_view.m_width *= 0.7f;
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
                    m_info.m_fileHdrV3.m_tex[0] = GFX_CreateTextureFromTGAFile(tname, -1, true);
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
        static_assert(sizeof(tViewport<uint16_t>) == 8);
        auto v9 = m_info.m_v1gls[x].m_height;
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
                m_spaceScale = 1.98f;
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
                m_spaceScale = 1.5f;
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
    m_verticalOffset = 0.0f;
    m_spaceScale = 1.0f;
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
float CFont2D::GetHeadOffset(float a2, float a3) {
    return a3 - m_headLine * m_field_20A3C * m_scale * a2;
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
            GFX_ActivateTexture(m_info.m_fileHdrV3.m_tex[0], 0, nullptr, TWM_CLAMP_TO_EDGE);
            GFX_ActivateTexture((m_info.m_fileHdrV3.m_tex[1] != -1) ? m_info.m_fileHdrV3.m_tex[1] : g_WhiteHandle, 2, nullptr, TWM_CLAMP_TO_EDGE);
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
float CFont2D::StringWidthW_c(const char *text) {
    BufSafeToUTF8 buf;
    return StringWidthW_u(SafeToUTF8(text, &buf));
}
float CFont2D::StringWidthW_u(const UChar *uText) { return StringWidthW_ulen(uText, u_strlen(uText)); }
float CFont2D::StringWidthW_ulen(const UChar *uText, uint32_t textLen) {
    static_assert(sizeof(CFont2D_info) == 0x20990);
    static_assert(sizeof(CFont2D_glyph) == 20);
    if (!m_loadedV3 || !uText || !*uText || !textLen)
        return 0.0f;
    float       ret = 0.0f;
    const UChar *uEnd = uText + textLen;
    do {
        auto gidx = m_info.m_glyphIndexes[*uText];
        if (gidx != 0xFFFF) {
            float mult = (*uText == 32) ? this->m_spaceScale : 1.0f;
            auto  kidx = m_glyphs[gidx].m_kernIdx;
            ret += m_kern[kidx] * m_kerning * m_info.m_fileHdrV3.m_kern[kidx] * m_glyphs[gidx].m_view.m_width * mult;
        }
    } while (++uText != uEnd);
    return ret * m_scale;
}
float CFont2D::GetBoundedScaleW(float w, float h, const UChar *uText) {
    auto needWidth = (float)StringWidthW_u(uText);
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
            v8 += m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * m_glyphs[v11].m_view.m_width * m_scale * (str[i] == 32 ? m_spaceScale : 1.0);
        }
        if (v8 >= w)
            break;
        if (++i >= len)
            return len;
    }
    return i;
}
int CFont2D::FitWordsToWidthW(const UChar *str, int w, float marg) {
    if (!str || !m_loadedV3)
        return 0;
    int  len = u_strlen(str);
    int  v9 = 0, v11 = 0, v12 = 0, v14 = 0;
    auto v15 = 0.0f;
    auto goalWidth = w - marg;
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
                v16 += m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * m_glyphs[v21].m_view.m_width * m_scale * (str[i] == 32 ? m_spaceScale : 1.0f);
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
                            v16 += m_kern[v33] * m_kerning * m_info.m_fileHdrV3.m_kern[v33] * m_glyphs[v30].m_view.m_width * m_scale * (*v29 == 32 ? m_spaceScale : 1.0f);
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
                v15 += m_kern[v40] * m_kerning * m_info.m_fileHdrV3.m_kern[v40] * m_glyphs[v37].m_view.m_width * m_scale * ((v36 == 32) ? m_spaceScale : 1.0f);
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
int CFont2D::GetParagraphLineCountW(float w, const UChar *str, float scale, float marg, bool unlim) {
    if (!m_loadedV3 || !str || !*str)
        return 0;
    int len = u_strlen(str);
    if (!unlim && len > 0x400)
        return 1;
    int ret = 0, wordsLen;
    for (auto *end = str + len; str < end; str += wordsLen) {
        scale = 1.0f / scale;
        wordsLen = FitWordsToWidthW(str, int(scale * w), scale * marg);
        marg = 0.0f;
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
    else if (flags & RF_CY_ISBOTTOM)
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
            RenderWString_u(left + v17, v16, text, color, flags & ~(RF_CY_ISBOTTOM | RF_CY_ISCENTER), a9, needShadow, 0, 1);
        v17 = 0.0f;
        v14 += v27;
        ++ret;
        v16 += v15;
    } while (top + height > v16);
    return ret;
}
void CFont2D::RenderWString_c(float cx, float cy, const char *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc) {
    BufSafeToUTF8 buf;
    auto          u8_text = SafeToUTF8(text, &buf);
    RenderWString_u(cx, cy, u8_text, color, flags, scale, needShadow, forceDrawMalloc, true);
}
void CFont2D::RenderString(float cx, float cy, const char *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc, bool uiProjection) {
    zassert(m_loadedV1);
    if (m_loadedV1 && text && *text) {
        auto v15 = scale * m_field_20A3C * m_scale;
        auto v17 = m_scale * scale;
        if (needShadow) {
            auto shadowOffset = fmaxf(1.0f, m_lineHeight * v17 / 25.0f);
            RenderString(shadowOffset + cx, shadowOffset + cy, text, (color >> 1) & 0x7F000000, flags, scale, false, forceDrawMalloc, uiProjection);
        }
        if (flags & RF_CX_ISCENTER)
            cx -= StringWidth(text) * scale * 0.5f;
        else if (flags & RF_CX_ISRIGHT)
            cx -= StringWidth(text) * scale;
        if (flags & RF_CY_ISCENTER)
            cy -= GetHeight() * scale * m_field_20A3C * 0.5f;
        else if (flags & RF_CY_ISBOTTOM)
            cy -= GetHeight() * scale * m_field_20A3C;
        auto len = (uint32_t)strlen(text);
        auto v24 = (!m_cache || forceDrawMalloc) ? (CFont2D_cache *)GFX_DrawMalloc(sizeof(CFont2D_cache) * len, 4) : CacheMallocCharacters(len, uiProjection);
        memset(v24, 0, sizeof(CFont2D_cache) * len);
        if (!v24)
            return;
        auto v25 = (DRAW_VERT_POS_COLOR_UV * )v24;
        auto v26 = v25;
        cy += v15 * m_verticalOffset;
        auto v23 = cx;
        while (*text) {
            auto v29 = *text++;
            if (v29 < m_info.m_fileHdrV1.m_from || v29 > m_info.m_fileHdrV1.m_to) {
                if (m_cache) {
                    if (!forceDrawMalloc) {
                        if (m_cacheCntUsed) {
                            ++m_curCache;
                            m_cacheCntUsed--;
                        }
                    }
                }
            } else {
                auto v30 = m_info.m_v1gls[v29];
                if (v29 == 32) {
                    v23 += v30.m_width * v17 * m_kerning * m_spaceScale;
                    if (m_cache) {
                        if (!forceDrawMalloc) {
                            if (m_cacheCntUsed) {
                                ++m_curCache;
                                m_cacheCntUsed--;
                            }
                        }
                    }
                } else {
                    auto width = this->m_info.m_fileHdrV1.m_width;
                    auto height = this->m_info.m_fileHdrV1.m_height;
                    v26[0].m_pos.m_data[0] = v26[3].m_pos.m_data[0] = v26[5].m_pos.m_data[0] = v23; //27[-7,17,33]
                    v26[0].m_pos.m_data[1] = v26[1].m_pos.m_data[1] = v26[5].m_pos.m_data[1] = cy; //27[-6,2,34]
                    v26[0].m_color = color;
                    v26[1].m_color = color;
                    v26[2].m_color = color;
                    v26[3].m_color = color;
                    v26[4].m_color = color;
                    v26[5].m_color = color;
                    auto v36 = v30.m_left / width + 0.001f;
                    auto v39 = v30.m_width / width - 0.002f;
                    auto v40 = v30.m_width * v17;
                    auto v41 = v39 + v36;
                    auto v42 = v30.m_height * v15 + cy;
                    v26[0].m_field_10 = v26[3].m_field_10 = v26[5].m_field_10 = v36; //27[-3,21,37]
                    v26[1].m_field_10 = v26[2].m_field_10 = v26[4].m_field_10 = v41; //27[5,13,29]
                    v26[2].m_field_14 = v26[3].m_field_14 = v26[4].m_field_14 = v30.m_height / height - 0.002f + v30.m_top / height + 0.001f; //27*[14,22,30]
                    v26[0].m_field_14 = v26[1].m_field_14 = v26[5].m_field_14 = v30.m_top / height + 0.001f; //27[-2,6,38]
                    v26[1].m_pos.m_data[0] = v26[2].m_pos.m_data[0] = v26[4].m_pos.m_data[0] = v40 + v23; //27[1,9,25]
                    v26[2].m_pos.m_data[1] = v26[3].m_pos.m_data[1] = v26[4].m_pos.m_data[1] = v42; //27[10,18,26]
                    v26 += 6;
                    v23 += v40 * m_kerning;
                }
            }
        }
        if (!m_cache || forceDrawMalloc) {
            auto CurrentShaderHandle = GFX_GetCurrentShaderHandle();
            GFX_SetShader(g_fontShader);
            GFX_SetAlphaBlendEnable(true);
            GFX_SetBlendFunc(GBO_FUNC_ADD, GB_SRC_ALPHA, GB_ONE_MINUS_SRC_ALPHA);
            GFX_ActivateTexture(m_tex, -1, nullptr, TWM_CLAMP_TO_EDGE);
            GFX_MatrixMode(GMT_2);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            auto CurrentRT = VRAM_GetCurrentRT();
            if (uiProjection)
                GFX_SetupUIProjection();
            else
                GFX_Ortho(0.0f, (float)CurrentRT->m_dw_width, (float)CurrentRT->m_dw_height, 0.0f, -1.0f, 1.0f);
            GFX_MatrixMode(GMT_1);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_MatrixMode(GMT_0);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_Translate({ cx, cy, 0.0f });
            GFX_RotateZ(uiProjection * 0.017453292f);
            GFX_Translate({-cx, -cy, 0.0f});
            GFX_UpdateMatrices(false);
            GFX_SetTextureFilter(0, GFF_LINEAR_MIPMAP_NEAREST);
            GFX_ActivateTextureEx(0, -0.5f);
            GFX_DrawPrimitive<DRAW_VERT_POS_COLOR_UV>(GPT_TRIANGLES, v25, v26 - v25);
            GFX_ActivateTextureEx(0, 0.0f);
            GFX_MatrixMode(GMT_2);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_1);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_0);
            GFX_PopMatrix();
            GFX_SetShader(CurrentShaderHandle);
        }
    }
}
void CFont2D::RenderWString_u(float cx, float cy, const UChar *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc, bool uiProjection) {
    if (!m_loadedV3 || !text || !*text)
        return;
    auto v17 = m_scale * scale;
    auto v16 = m_lineHeight * v17;
    if (uiProjection && (-v16 > cy || cy > 1280.0f / VRAM_GetUIAspectRatio() + 10.0f))
        return;
    if (needShadow) {
        auto shadowOffset = fmaxf(1.0f, v16 / 25.0f);
        RenderWString_u(shadowOffset + cx, shadowOffset + cy, text, (color >> 1) & 0x7F000000, flags, scale, false, forceDrawMalloc, uiProjection);
    }
    if (flags & RF_CX_ISCENTER)
        cx -= StringWidthW_u(text) * scale * 0.5f;
    else if (flags & RF_CX_ISRIGHT)
        cx -= StringWidthW_u(text) * scale;
    if (flags & RF_CY_ISCENTER)
        cy -= GetHeight() * scale * m_field_20A3C * 0.5f;
    else if (flags & RF_CY_ISBOTTOM)
        cy -= GetHeight() * scale * m_field_20A3C;
    auto v15 = m_field_20A3C * v17;
    auto v25 = cy + v15 * m_verticalOffset;
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
                        dx = g.m_view.m_width * v42 * v17 * m_kerning * m_kern[g.m_kernIdx] * m_spaceScale;
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
                        pflt[4] = g.m_view.m_left + 0.001f;
                        pflt[5] = g.m_view.m_top + 0.001f;
                        pflt[9] = v25;
                        auto v45 = g.m_view.m_width - 0.002f + g.m_view.m_left + 0.001f;
                        pflt[13] = g.m_view.m_top + 0.001f;
                        pint[7] = 0;
                        pint[10] = 0;
                        pint[11] = v39;
                        pflt[12] = v45;
                        auto v46 = g.m_view.m_width * v42 * v17;
                        auto v47 = fminf(1.0, (float)g.m_kernIdx);
                        pflt[6] = v47;
                        auto v48 = g.m_view.m_height - 0.002f + g.m_view.m_top + 0.001f;
                        pflt[8] = v46 + cx;
                        pflt[14] = v47;
                        pflt[16] = v46 + cx;
                        auto v49 = g.m_view.m_height * v42 * v15 + v25;
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
                        pflt[28] = g.m_view.m_left + 0.001f;
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
                        pflt[44] = g.m_view.m_left + 0.001f;
                        pflt[45] = g.m_view.m_top + 0.001f;
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
            GFX_ActivateTexture(m_info.m_fileHdrV3.m_tex[0], 0, nullptr, TWM_CLAMP_TO_EDGE);
            GFX_ActivateTexture((m_info.m_fileHdrV3.m_tex[1] != -1) ? m_info.m_fileHdrV3.m_tex[1] : g_WhiteHandle, 2, nullptr, TWM_CLAMP_TO_EDGE);
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
                GFX_Ortho(0.0f, (float)CurrentRT->m_dw_width, (float)CurrentRT->m_dw_height, 0.0f, -1.0f, 1.0f);
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
int CFont2D::GetTexture(uint32_t digit) {
    if (digit < 10) {
        if (m_loadedV1)
            return m_tex;
        if (m_loadedV3)
            return m_info.m_fileHdrV3.m_tex[digit];
    }
    return -1;
}
void CFont2D::GetGlyphUVView(uint32_t ch, tViewport<float> *vwp, uint32_t *kernIdx) {
    if (ch < 0xFFFF) {
        if (m_loadedV1) {
            if (ch >= m_info.m_fileHdrV1.m_chars)
                return;
            if (kernIdx)
                *kernIdx = 0;
            vwp->m_left = (m_info.m_v1gls[ch].m_left + 0.5f) / m_info.m_fileHdrV1.m_width;
            vwp->m_top = (m_info.m_v1gls[ch].m_top + 0.5f) / m_info.m_fileHdrV1.m_height;
            vwp->m_width = (m_info.m_v1gls[ch].m_width - 1.0f) / m_info.m_fileHdrV1.m_width;
            vwp->m_height = (m_info.m_v1gls[ch].m_height - 1.0f) / m_info.m_fileHdrV1.m_height;
        } else if (m_loadedV3) {
            auto v8 = m_info.m_glyphIndexes[ch];
            if (v8 != 0xFFFF) {
                if (kernIdx)
                    *kernIdx = m_glyphs[v8].m_kernIdx;
                auto v9 = 1.0f / m_info.m_fileHdrV3.m_kern[m_glyphs[v8].m_kernIdx];
                vwp->m_left = 0.5f * v9 + m_glyphs[v8].m_view.m_left;
                vwp->m_top = 0.5f * v9 + m_glyphs[v8].m_view.m_top;
                vwp->m_width = m_glyphs[v8].m_view.m_width - v9;
                vwp->m_height = m_glyphs[v8].m_view.m_height - v9;
            }
        }
    }
}
bool CFont2D::GetGlyphView(uint32_t ch, tViewport<int> *viewPort) {
    if (ch >= 0xFFFF)
        return false;
    if (m_loadedV1) {
        if (ch < m_info.m_fileHdrV1.m_chars) {
            auto v1_glyph = m_info.m_v1gls[ch];
            viewPort->m_left = v1_glyph.m_left;
            viewPort->m_top = v1_glyph.m_top;
            viewPort->m_width = v1_glyph.m_width;
            viewPort->m_height = v1_glyph.m_height;
            return true;
        }
    } else if (m_loadedV3) {
        auto v8 = m_info.m_glyphIndexes[ch];
        if (v8 != 0xFFFF) {
            auto kernIdx = m_glyphs[v8].m_kernIdx;
            if (kernIdx <= 1) {
                auto v10 = m_info.m_fileHdrV3.m_kern[kernIdx];
                viewPort->m_left = int(v10 * m_glyphs[v8].m_view.m_left);
                viewPort->m_top = int(v10 * m_glyphs[v8].m_view.m_top);
                viewPort->m_width = int(v10 * m_glyphs[v8].m_view.m_width);
                viewPort->m_height = int(v10 * m_glyphs[v8].m_view.m_height);
                return true;
            }
        }
    }
    return false;
}
float CFont2D::GetTopToBase(float a2) {
    return (GetHeight() - m_baseLine * m_field_20A3C * m_scale) * a2;
}
float CFont2D::GetBoundedScale(float totalW, float totalH, const char *txt) {
    auto ret = 1.0f;
    for (int i = 0; i < 10; ++i) {
        if (GetHeight() * GetParagraphLineCount(totalW, txt, ret, 0.0f, false) * ret < totalH)
            break;
        ret *= 0.95f;
    }
    return ret;
}
float CFont2D::CharWidthW(UChar ch) {
    if (!m_loadedV3)
        return 0.0f;
    auto gi = m_info.m_glyphIndexes[ch];
    if (gi == 0xFFFF)
        return 0.0f;
    auto kernIdx = m_glyphs[gi].m_kernIdx;
    auto ret = m_scale * m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * m_glyphs[gi].m_view.m_width;
    if (ch == 32)
        ret *= m_spaceScale;
    return ret;
}
void CFont2D::RenderStringRT(float cx, float cy, const char *txt, uint32_t color, int flags, float scale, bool needShadow) {
    BufSafeToUTF8 buf;
    RenderWString_u(cx, cy, SafeToUTF8(txt, &buf), color, flags, scale, needShadow, false, false);
}
void CFont2D::RenderBorderedString(float cx, float cy, const char *txt, uint32_t color, int flags, float scale, bool needShadow, bool fdm, uint32_t clr2) {
    BufSafeToUTF8 buf;
    RenderBorderedWString(cx, cy, SafeToUTF8(txt, &buf), color, flags, scale, needShadow, fdm, clr2);
}
char g_ellipsizeBuf[1024];
const char *CFont2D::Ellipsize(float scale, const char *txt, float maxW) {
    zassert(m_loadedV3);
    if (m_loadedV3) {
        if (maxW >= StringWidthW_c(txt) * scale)
            return txt;
        auto d3w = StringWidthW_c("...") * scale;
        if (d3w <= maxW) {
            strcpy_s(g_ellipsizeBuf, txt);
            auto v9 = strlen(g_ellipsizeBuf) - 1;
            while (true) {
                g_ellipsizeBuf[v9--] = 0; //QUEST: зачем такой медленный алгоритм
                if (v9 < 0)
                    break;
                auto w = StringWidth(g_ellipsizeBuf) * scale + d3w;
                if (w <= maxW) {
                    strcat_s(g_ellipsizeBuf, "...");
                    return g_ellipsizeBuf;
                }
            }
        }
    }
    return "";
}
float CFont2D::StringWidth(const char *text) {
    zassert(m_loadedV1);
    auto ret = 0.0f;
    if (text) {
        auto mult = m_scale * m_kerning;
        while (*text) {
            if (*text >= m_info.m_fileHdrV1.m_from && *text <= m_info.m_fileHdrV1.m_to)
                ret += m_info.m_v1gls[*text].m_width * (*text == 32 ? m_spaceScale : 1.0f) * mult;
            text++;
        }
    }
    return ret;
}
UChar g_ellipsizeBufW[1024];
const UChar *CFont2D::EllipsizeW(float scale, const UChar *txt, float maxW) {
    if (m_loadedV3) {
        if (maxW >= (StringWidthW_u(txt) * scale))
            return txt;
        auto d3w = StringWidthW_u(u"...") * scale;
        if (d3w <= maxW) {
            u_strncpy(g_ellipsizeBufW, txt, _countof(g_ellipsizeBufW));
            auto v14 = u_strlen(g_ellipsizeBufW) - 1;
            while (true) {
                g_ellipsizeBufW[v14] = 0;
                auto w = StringWidthW_u(g_ellipsizeBufW) * scale + d3w; //QUEST: зачем такой медленный алгоритм
                if (--v14 < 0)
                    break;
                if (w <= maxW) {
                    u_strcat(g_ellipsizeBufW, u"...");
                    return g_ellipsizeBufW;
                }
            }
        }
    }
    return u"";
}
float CFont2D::FitParagraphInBox(float w, float h, const UChar *txt) {
    float scale = 1.0f, ret = 0.1f, v8 = 99999.0f;
    do {
        auto v12 = GetHeight() * scale;
        auto v13 = fmaxf(0.0f, (GetParagraphLineCountW(w, txt, scale, 0.0f, true) - 1) * v12 * 1.5f) + v12;
        auto v14 = fabs(h - v13);
        if (v8 > v14 && h > v13) {
            v8 = v14;
            ret = scale;
        }
        scale -= 0.05f;
    } while (scale > 0.0f);
    ret = fmaxf(0.1f, ret);
    if (ret >= 1.0f)
        return 1.0f;
    return ret;
}
float CFont2D::RenderWStringMultisized(float cx, float cy, int nlines, const UChar **lines, uint32_t *clr, float *scales, RenderFlags hflg, RenderFlags vflg, bool needShadow, int cntLines, bool forceDrawMalloc) {
    auto ret = 0.0f, lh = GetHeight();
    if (cntLines == -1)
        cntLines = nlines;
    auto v22 = lh * scales[0];
    for (int i = 0; i < cntLines; i++)
        ret += StringWidthW_u(lines[i]) * scales[i];
    if (hflg == RF_CX_ISRIGHT)
        cx -= ret;
    else if (hflg == RF_CX_ISCENTER)
        cx -= ret * 0.5f;
    for (int i = 0; i < nlines; i++) {
        auto v27 = 0.0f;
        if (vflg == RF_CY_ISBOTTOM)
            v27 = (v22 - scales[i] * lh) * 0.75f;
        RenderWString_u(cx, v27 + cy, lines[i], clr[i], 0, scales[i], needShadow, forceDrawMalloc, true);
        cx += StringWidthW_u(lines[i]) * scales[i];
    }
    return ret;
}
float CFont2D::RenderStringMultisized(float cx, float cy, int nlines, const char **lines, uint32_t *clrs, float *scales, RenderFlags hflg, RenderFlags vflg, bool needShadow, int cntLines, bool forceDrawMalloc) {
    auto ret = 0.0f, lh = GetHeight();
    if (cntLines == -1)
        cntLines = nlines;
    auto v22 = lh * scales[0];
    for (int i = 0; i < cntLines; i++)
        ret += StringWidthW_c(lines[i]) * scales[i];
    if (hflg == RF_CX_ISRIGHT)
        cx -= ret;
    else if (hflg == RF_CX_ISCENTER)
        cx -= ret * 0.5f;
    for (int i = 0; i < nlines; i++) {
        auto v27 = 0.0f;
        if (vflg == RF_CY_ISBOTTOM)
            v27 = (v22 - scales[i] * lh) * 0.75f;
        RenderWString_c(cx, v27 + cy, lines[i], clrs[i], 0, scales[i], needShadow, forceDrawMalloc);
        cx += StringWidthW_c(lines[i]) * scales[i];
    }
    return ret;
}
void CFont2D::RenderBorderedWString(float cx, float cy, const UChar *text, uint32_t clr, int flags, float scale, bool needShadow, bool forceDrawMalloc, uint32_t clr2) {
    //auto clr2 = clr ^ (clr2 ^ clr) & 0xFFFFFF;
    auto v14 = fmaxf(1.0f, m_lineHeight * m_scale * scale / 25.0f), v15 = cy - v14, v17 = v14 + cx;
    RenderWString_u(cx - v14, v15, text, clr2, flags, scale, false, forceDrawMalloc, true);
    RenderWString_u(cx - v14, v14 + cy, text, clr2, flags, scale, false, forceDrawMalloc, true);
    RenderWString_u(v17, v15, text, clr2, flags, scale, false, forceDrawMalloc, true);
    RenderWString_u(v17, v14 + cy, text, clr2, flags, scale, false, forceDrawMalloc, true);
    RenderWString_u(cx, cy, text, clr, flags, scale, false, forceDrawMalloc, true);
}
float CFont2D::RenderBorderedWStringMultisized(float cx, float cy, int nlines, const char **lines, uint32_t *clrs, float *scales, RenderFlags hflg, RenderFlags vflg, bool needShadow, int cntLines, bool fdm) {
    auto ret = 0.0f, lh = GetHeight();
    if (cntLines == -1)
        cntLines = nlines;
    auto v22 = lh * scales[0];
    for (int i = 0; i < cntLines; i++)
        ret += StringWidthW_c(lines[i]) * scales[i];
    if (hflg == RF_CX_ISRIGHT)
        cx -= ret;
    else if (hflg == RF_CX_ISCENTER)
        cx -= ret * 0.5f;
    for (int i = 0; i < nlines; i++) {
        auto v27 = 0.0f;
        if (vflg == RF_CY_ISBOTTOM)
            v27 = (v22 - scales[i] * lh) * 0.75f;
        RenderBorderedString(cx, v27 + cy, lines[i], clrs[i], 0, scales[i], needShadow, fdm, 0xFF000000);
        cx += StringWidthW_c(lines[i]) * scales[i];
    }
    return ret;
}
int CFont2D::GetParagraphIndexByPosition(float cx, float cy, float w, float h, const UChar *str, RenderFlags flags, float scaleW, float scaleH, float *ox, float *oy) {
    int ret = 0;
    if (m_loadedV3 && str) {
        auto _str = str;
        ret = u_strlen(str);
        auto oy_ = 0.0f;
        auto v18 = GetHeight() * scaleW * m_field_20A3C * scaleH, fw = 1.0f / scaleW * w;
        if (flags & RF_CY_ISCENTER)
            oy_ = h * 0.5f - GetParagraphLineCountW(w, str, scaleW, 0.0f, false) * v18 * 0.5f;
        if (h > oy_) {
            while (_str < str + ret) {
                auto eatChars = FitWordsToWidthW(_str, fw, 0.0f);
                auto ox_ = 0.0f;
                *oy = oy_;
                *ox = ox_;
                for (int v20 = 0; v20 < eatChars; v20++) {
                    auto v24 = m_info.m_glyphIndexes[_str[v20]];
                    if (v24 != 0xFFFF) {
                        auto v25 = ox_;
                        auto v27 = m_glyphs[v24].m_kernIdx;
                        ox_ += m_kern[v27] * m_kerning * m_glyphs[v24].m_view.m_width * m_info.m_fileHdrV3.m_kern[v27] * scaleW * m_scale * ((_str[v20] == 32) ? m_spaceScale : 1.0f);
                        if (ox_ > cx && cx > v25 && cy > oy_ && (oy_ + v18) > cy) {
                            *ox = v25;
                            *oy = oy_;
                            return (_str - str) + v20;
                        }
                    }
                }
                auto v29 = oy_ + v18;
                if (cy > oy_ && v29 > cy) {
                    *ox = ox_;
                    *oy = oy_;
                    return (_str - str) + (eatChars - 1);
                }
                _str += eatChars;
                *ox = ox_;
                *oy = oy_;
                if (h <= v29)
                    return ret;
            }
        }
    }
    return ret;
}
VEC2 CFont2D::GetParagraphPositionByIndexW(int index, float w, UChar *str, float mult, float scale) {
    VEC2 ret{};
    if (m_loadedV3 && str != nullptr && *str) {
        auto _str = str;
        auto y = 0.0f, lh = GetHeight() * mult * m_field_20A3C * scale;
        w *= 1.0f / mult;
        int i = 0;
        do {
            auto v19 = FitWordsToWidthW(_str, w, 0.0f);
            auto v21 = v19 + i;
            if (v21 >= index)
                break;
            for (int v23 = v19; v23; --v23)
                if (*_str++ == 10)
                    y += lh;
            i = v21;
            if (_str[-1] != 10)
                y += lh;
        } while (*_str);
        auto v24 = mult * m_scale * m_kerning;
        while (*_str) {
            ++i;
            auto v26 = *_str++;
            auto v27 = m_info.m_glyphIndexes[v26];
            if (v27 != 0xFFFF) {
                if (v26 == 10) {
                    ret.m_data[0] = 0.0f;
                    y += lh;
                } else {
                    auto v29 = m_glyphs[v27].m_kernIdx;
                    ret.m_data[0] += m_kern[v29] * m_glyphs[v27].m_view.m_width * m_info.m_fileHdrV3.m_kern[v29] * v24 * ((v26 == 32) ? m_spaceScale : 1.0f);
                }
            }
            if (i >= index)
                break;
        }
        ret.m_data[1] = y;
    }
    return ret;
}
std::string CFont2D::ImbueCommas(uint32_t val) {
    std::string ret;
    if (m_loadedV3) {
        char cret[13], *pRet = cret + _countof(cret); //4,294,967,295
        int  cnt = 0, c3 = 0;
        do {
            if (c3 == 3) {
                c3 = 0;
                *(--pRet) = ',';
                ++cnt;
            }
            *(--pRet) = '0' + (val % 10);
            ++cnt; ++c3;
            val /= 10;
        } while (val);
        ret.assign(pRet, cnt);
    }
    return ret;
}
int CFont2D::GetParagraphLineCount(float w, const char *str, float scale, float marg, bool unlim) {
    BufSafeToUTF8 buf;
    return GetParagraphLineCountW(w, SafeToUTF8(str, &buf), scale, marg, unlim);
}

//Unit tests
TEST(SmokeTestFont, Imbue) {
    CFont2D f;
    ASSERT_EQ(std::string(), f.ImbueCommas(123));
    f.m_loadedV3 = true;
    ASSERT_EQ(std::string("0"), f.ImbueCommas(0));
    ASSERT_EQ(std::string("123"), f.ImbueCommas(123));
    ASSERT_EQ(std::string("1,234"), f.ImbueCommas(1234));
    ASSERT_EQ(std::string("4,294,967,295"), f.ImbueCommas(0xFFFFFFFF));
}