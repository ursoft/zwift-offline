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
            fread(m_info.m_v1gls + m_info.m_fileHdrV1.m_from, sizeof(tViewport<uint16_t>), m_info.m_fileHdrV1.m_charsCnt, f);
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
                    m_glyphs = (CFont2D_glyph *)malloc(sizeof(CFont2D_glyph) * m_info.m_fileHdrV3.m_charsCnt);
                    fread(m_glyphs, sizeof(CFont2D_glyph), m_info.m_fileHdrV3.m_charsCnt, f);
                    for (int g = 0; g < m_info.m_fileHdrV3.m_charsCnt; ++g) {
                        auto codePoint = m_glyphs[g].m_codePoint;
                        if (codePoint == 0xFFFF) {
                            Log("Loading font with codepoint beyond max: %d\n", codePoint);
                        } else {
                            m_info.m_glyphIndexes[codePoint] = g;
                            if (uint8_t(m_glyphs[g].m_lid - 1) <= 2) { //i.e. [1,2,3]
                                bool found = false;
                                auto &vect = m_lidKernIdx[m_glyphs[g].m_lid];
                                for (auto x : vect) {
                                    if (x == m_glyphs[g].m_kernIdx)
                                        found = true;
                                }
                                if (!found)
                                    vect.push_back(m_glyphs[g].m_kernIdx);
                            }
                            //if (codePoint == 33)
                            //    m_glyphs[g].m_view.m_width *= 0.7f;
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
                    static_assert(sizeof(RealKernItem) == 5);
                    RealKernItem rki;
                    for (int k = 0; k < m_info.m_fileHdrV3.m_realKerns; k++) {
                        fread(&rki, sizeof(RealKernItem), 1, f);
                        m_realKerning[std::make_pair(rki.m_prev, rki.m_cur)] = rki.m_corr / 2048.0;
                    }
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
    m_texSuffix = CID_RUSLAT;
    switch (left) {
        default:
            break;
        case LOC_JAPAN:
            m_texSuffix = CID_JAPAN;
            break;
        case LOC_KOREAN:
            m_texSuffix = CID_KOREAN;
            break;
        case LOC_CHINESE:
            m_texSuffix = CID_CHINESE;
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
    m_info.m_charsetId = m_texSuffix;
    if (m_texSuffix)
        m_info.m_fileHdrV3.m_tex[1] = GFX_CreateTextureFromTGAFile(tname, -1, true);
    m_lineHeight = 0;
    for (int i = 0; i < m_info.m_fileHdrV3.m_charsCnt; i++) {
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
            auto cb_3 = sizeof(tViewport<uint16_t>) * m_info.m_fileHdrV1.m_charsCnt;
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
                    auto gsz = sizeof(CFont2D_glyph) * m_info.m_fileHdrV3.m_charsCnt;
                    m_glyphs = (CFont2D_glyph *)malloc(gsz);
                    memmove(m_glyphs, pFontHeaderV3 + 1, gsz);
                    for (int g = 0; g < m_info.m_fileHdrV3.m_charsCnt; ++g) {
                        auto codePoint = m_glyphs[g].m_codePoint;
                        if (codePoint == 0xFFFF) {
                            Log("Loading font with codepoint beyond max: %d\n", codePoint);
                        } else {
                            m_info.m_glyphIndexes[codePoint] = g;
                            if (uint8_t(m_glyphs[g].m_lid - 1) <= 2) { //i.e. [1,2,3]
                                bool found = false;
                                auto &vect = m_lidKernIdx[m_glyphs[g].m_lid];
                                for (auto x : vect) {
                                    if (x == m_glyphs[g].m_kernIdx)
                                        found = true;
                                }
                                if (!found)
                                    vect.push_back(m_glyphs[g].m_kernIdx);
                            }
                            //if (codePoint == 33)
                            //    m_glyphs[g].m_view.m_width *= 0.7f;
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
                    static_assert(sizeof(RealKernItem) == 5);
                    RealKernItem *rki = (RealKernItem *)((const char *)(pFontHeaderV3 + 1) + gsz);
                    for (int k = 0; k < m_info.m_fileHdrV3.m_realKerns; k++) {
                        m_realKerning[std::make_pair(rki->m_prev, rki->m_cur)] = rki->m_corr / 2048.0;
                        rki++;
                    }
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
bool CFont2D::LoadDirect(const char *name) { return LoadFont(name) || LoadFontFromWad(name); }
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
    m_info.m_charsetId = CID_CNT;
    m_texSuffix = CID_RUSLAT;
    m_kern[0] = 1.0f;
    m_kern[1] = 1.0f; //1.08935f;
    m_kern[2] = 1.0f;
    m_kern[3] = 1.0f; //1.08435f;
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
void CFont2D::SetLanguageKerningScalar(CHARSET_IDS cid, float kern) {
    if (cid < CID_CNT)
        m_kern[cid] = kern;
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
struct UrsoftKerner {
    const CFont2D &m_font;
    std::pair<UChar, UChar> m_realKernKey;
    float operator ()(UChar ch) {
#ifdef URSOFT_KERNING
        m_realKernKey.first = m_realKernKey.second;
        m_realKernKey.second = ch;
        if (ch != 32 && m_realKernKey.first && !m_font.m_realKerning.empty()) {
            auto fnd = m_font.m_realKerning.find(m_realKernKey);
            if (fnd != m_font.m_realKerning.end())
                return fnd->second;
        }
#endif
        return 0;
    }
};
float CFont2D::StringWidthW_ulen(const UChar *uText, uint32_t textLen) {
    static_assert(sizeof(CFont2D_info) == 0x20990);
    static_assert(sizeof(CFont2D_glyph) == 20);
    if (!m_loadedV3 || !uText || !*uText || !textLen)
        return 0.0f;
    float       ret = 0.0f;
    const UChar *uEnd = uText + textLen;
    UrsoftKerner uk{ .m_font = *this };
    do {
        auto ch = *uText;
        auto gidx = m_info.m_glyphIndexes[ch];
        if (gidx != 0xFFFF) {
            float mult = (ch == 32) ? this->m_spaceScale : 1.0f;
            auto  kidx = m_glyphs[gidx].m_kernIdx;
            ret += m_kern[kidx] * m_kerning * m_info.m_fileHdrV3.m_kern[kidx] * (m_glyphs[gidx].m_view.m_width + uk(ch)) * mult;
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
    UrsoftKerner uk{ .m_font = *this };
    while (true) {
        auto ch = str[i];
        auto v11 = m_info.m_glyphIndexes[ch];
        if (v11 != 0xFFFF) {
            auto kernIdx = m_glyphs[v11].m_kernIdx;
            v8 += m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * (m_glyphs[v11].m_view.m_width + uk(ch)) * m_scale * (ch == 32 ? m_spaceScale : 1.0);
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
        UrsoftKerner uk0{ .m_font = *this };
        for (int i = 0; i < v11; i++) {
            auto ch = str[i];
            auto v21 = m_info.m_glyphIndexes[ch];
            if (v21 != 0xFFFF) {
                auto kernIdx = m_glyphs[v21].m_kernIdx;
                v16 += m_kern[kernIdx] * m_kerning * m_info.m_fileHdrV3.m_kern[kernIdx] * (m_glyphs[v21].m_view.m_width + uk0(ch)) * m_scale * (ch == 32 ? m_spaceScale : 1.0f);
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
                    UrsoftKerner uk{ .m_font = *this };
                    while (1) {
                        auto ch = *v29;
                        auto v30 = m_info.m_glyphIndexes[ch];
                        if (v30 != 0xFFFF) {
                            auto v33 = m_glyphs[v30].m_kernIdx;
                            v16 += m_kern[v33] * m_kerning * m_info.m_fileHdrV3.m_kern[v33] * (m_glyphs[v30].m_view.m_width + uk(ch)) * m_scale * (ch == 32 ? m_spaceScale : 1.0f);
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
        UrsoftKerner uk{ .m_font = *this };
        for (int v35 = 0; v35 < v11; ++v35) {
            auto v36 = str[v35];
            auto v37 = m_info.m_glyphIndexes[v36];
            if (v37 != 0xFFFF) {
                auto v40 = m_glyphs[v37].m_kernIdx;
                v15 += m_kern[v40] * m_kerning * m_info.m_fileHdrV3.m_kern[v40] * (m_glyphs[v37].m_view.m_width + uk(v36)) * m_scale * ((v36 == 32) ? m_spaceScale : 1.0f);
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
            UrsoftKerner uk{ .m_font = *this };
            for (int i = 0; i < textLen; ++i, ++text) {
                auto v35 = v31;
                auto ch = *text;
                auto v36 = m_info.m_glyphIndexes[ch];
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
                    if (ch == 32) {
                        dx = g.m_view.m_width * v42 * v17 * m_kerning * m_kern[g.m_kernIdx] * m_spaceScale;
                        if (m_cache && !forceDrawMalloc && m_cacheCntUsed) {
                            ++m_curCache;
                            m_cacheCntUsed--;
                        }
                    } else {
                        auto pint = (uint32_t *)pflt;
                        auto corr = uk(ch);
                        cx += v15 * corr * v42;
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
            if (ch >= m_info.m_fileHdrV1.m_charsCnt)
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
        if (ch < m_info.m_fileHdrV1.m_charsCnt) {
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
                UrsoftKerner uk{ .m_font = *this };
                for (int v20 = 0; v20 < eatChars; v20++) {
                    auto ch = _str[v20];
                    auto v24 = m_info.m_glyphIndexes[ch];
                    if (v24 != 0xFFFF) {
                        auto v25 = ox_;
                        auto v27 = m_glyphs[v24].m_kernIdx;
                        ox_ += m_kern[v27] * m_kerning * (m_glyphs[v24].m_view.m_width + uk(ch)) * m_info.m_fileHdrV3.m_kern[v27] * scaleW * m_scale * ((ch == 32) ? m_spaceScale : 1.0f);
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
        UrsoftKerner uk{ .m_font = *this };
        while (i < index && *_str) {
            ++i;
            auto v26 = *_str++;
            auto v27 = m_info.m_glyphIndexes[v26];
            if (v27 != 0xFFFF) {
                if (v26 == 10) {
                    ret.m_data[0] = 0.0f;
                    y += lh;
                } else {
                    auto v29 = m_glyphs[v27].m_kernIdx;
                    ret.m_data[0] += m_kern[v29] * (m_glyphs[v27].m_view.m_width + uk(v26)) * m_info.m_fileHdrV3.m_kern[v29] * v24 * ((v26 == 32) ? m_spaceScale : 1.0f);
                }
            }
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
enum FontBitmapPixel : uint8_t { FBP_NOP, FBP_LIGHT, FBP_MEDIUM, FBP_SOLID, FBP_NOP1 /*занято пустое место*/, FBP_NOP2 /*занято пустое место, несколько глифов уже */ };
const int FONT_BITMAP_SIZE = 2048, GLYPH_MAX_SIZE = 170;
using FontBitmapPixelGrid = std::array<std::array<FontBitmapPixel, FONT_BITMAP_SIZE>, FONT_BITMAP_SIZE>;
FontBitmapPixelGrid *LoadFontBitmap(FILE *ftxt) {
    auto ret = new FontBitmapPixelGrid;
    char srow[FONT_BITMAP_SIZE + 3];
    for (int row = 0; row < FONT_BITMAP_SIZE; ++row) {
        fgets(srow, sizeof(srow), ftxt);
        for (int col = 0; col < FONT_BITMAP_SIZE; col++) {
            switch (srow[col]) {
            case ' ':
                (*ret)[row][col] = FBP_NOP;
                break;
            case '\xdb': //full block (100%)
                (*ret)[row][col] = FBP_SOLID;
                break;
            case '\xb0': //light block (25%)
                (*ret)[row][col] = FBP_LIGHT;
                break;
            case '\xb1': //med block (50%)
                (*ret)[row][col] = FBP_MEDIUM;
                break;
            default:
                assert(0);
            }
        }
    }
    return ret;
}
#if 1
using GlyphBitmapPixelGrid = std::array<std::array<FontBitmapPixel, GLYPH_MAX_SIZE>, GLYPH_MAX_SIZE>;
struct FinderCache {
    std::multimap<uint64_t, POINT> m_sameGlyphs;
    POINT m_trialPoint{};
};
struct CFont2D_glyphBMP : public CFont2D_glyph {
    SIZE m_iSize{};
    RECT m_paddings{};
    GlyphBitmapPixelGrid m_grid;
    uint64_t m_sum{};
    int m_freeAtRight[GLYPH_MAX_SIZE]{};
    void initFromGrid(const FontBitmapPixelGrid *grid) {
        memset(m_freeAtRight, 0, sizeof(m_freeAtRight));
        m_sum = 0;
        m_iSize = { LONG(m_view.m_width * 2048), LONG(m_view.m_height * 2048) };
        if (m_iSize.cx > GLYPH_MAX_SIZE || m_iSize.cy > GLYPH_MAX_SIZE) {
            printf("overflow\n");
        }
        m_paddings.right = m_paddings.left = m_paddings.top = m_paddings.bottom = -1;
        for (int row = m_view.m_top * 2048, irow = 0; irow < m_iSize.cy; ++row, ++irow) {
            bool pixFound = false;
            m_freeAtRight[irow] = m_iSize.cx;
            for (int col = m_view.m_left * 2048, icol = 0; icol < m_iSize.cx; ++col, ++icol) {
                FontBitmapPixel p = col < 0 ? FBP_NOP : grid->at(row).at(col);
                m_grid[irow][icol] = p;
                if (p != FBP_NOP) {
                    m_sum += p;
                    if (pixFound == false) {
                        pixFound = true;
                        if (m_paddings.left == -1 || m_paddings.left > icol)
                            m_paddings.left = icol;
                    }
                    m_freeAtRight[irow] = m_iSize.cx - icol - 1;
                }
            }
            if (m_paddings.right == -1 || m_paddings.right > m_freeAtRight[irow])
                m_paddings.right = m_freeAtRight[irow];
            if (pixFound) { //not empty row -> set top/bottom paddings
                if (m_paddings.top == -1) //if not set yet
                    m_paddings.top = irow;
                m_paddings.bottom = m_iSize.cy - irow - 1;
            }
        }
        if (m_paddings.top == -1) //if not set yet
            m_paddings.top = m_iSize.cy;
        if (m_paddings.bottom == -1) //if not set yet
            m_paddings.bottom = m_iSize.cy;
        if (m_paddings.left == -1) //if not set yet
            m_paddings.left = m_iSize.cx;
        if (m_paddings.right == -1) //if not set yet
            m_paddings.right = m_iSize.cx;
        if (m_paddings.right == 0 || /*m_paddings.left == 0 || m_paddings.top == 0 || */ m_paddings.bottom == 0) {
            printf("why aligned to border");
        }
    }
    bool initFromLetterTxt(const char *file);
    bool appendTo(FontBitmapPixelGrid *grid, FinderCache *cache);
    bool findEmptySpace(FontBitmapPixelGrid *grid, POINT *result, FinderCache *cache);
    bool conflictAt(FontBitmapPixelGrid *grid, POINT tryPoint);
};
constexpr char glbGlyphLegend[]{ '.', '\xb0', '\xb1', '\xdb', '\'', '"'};
void dumpGlyphBmp(CFont2D_glyphBMP &g, const FontBitmapPixelGrid *grid, FILE *fAlphabet) {
    g.initFromGrid(grid);
    fprintf(fAlphabet, "Glyph codePoint:%04x, w=%d, h=%d, pad.ltrb=[%d, %d, %d, %d]\n", g.m_codePoint, g.m_iSize.cx, g.m_iSize.cy,
        g.m_paddings.left, g.m_paddings.top, g.m_paddings.right, g.m_paddings.bottom);
    for (int irow = 0; irow < g.m_iSize.cy; irow++) {
        for (int icol = 0; icol < g.m_iSize.cx; icol++) {
            fprintf(fAlphabet, "%c", glbGlyphLegend[g.m_grid[irow][icol]]);
        }
        fprintf(fAlphabet, "\n");
    }
    fprintf(fAlphabet, "\n");
}
int dumpKernPairBmp(const CFont2D_glyphBMP &g1, const CFont2D_glyphBMP &g2, const char *fmt, int kern, bool file) {
    char fileName[MAX_PATH];
    sprintf(fileName, fmt, g1.m_codePoint, g2.m_codePoint);
    FILE *f = file ? fopen(fileName, "wt") : nullptr;
    if (f) {
        fprintf(f, "Glyph1 codePoint:%04x, w=%d, h=%d, pad.ltrb=[%d, %d, %d, %d]\n", g1.m_codePoint, g1.m_iSize.cx, g1.m_iSize.cy,
            g1.m_paddings.left, g1.m_paddings.top, g1.m_paddings.right, g1.m_paddings.bottom);
        fprintf(f, "Glyph2 codePoint:%04x, w=%d, h=%d, pad.ltrb=[%d, %d, %d, %d]\n", g2.m_codePoint, g2.m_iSize.cx, g2.m_iSize.cy,
            g2.m_paddings.left, g2.m_paddings.top, g2.m_paddings.right, g2.m_paddings.bottom);
    }
    const char legend1[]{ '.', '\xb0', '\xb1', '\xdb' };
    const char legend2[]{ ':', '\xfe', '\xfe', '\xfe' };
    const int colMax = GLYPH_MAX_SIZE * 2 + 32;
    char buf[GLYPH_MAX_SIZE][colMax];
    memset(buf, 32, sizeof(buf));
    for (int irow = 0; irow < g1.m_iSize.cy; irow++) {
        int icol = 0;
        for (; icol < g1.m_iSize.cx; icol++) {
            buf[irow][icol] = legend1[g1.m_grid[irow][icol]];
        }
    }
    bool bConflict = false;
    for (int irow = 0; irow < g2.m_iSize.cy; irow++) {
        int col = g1.m_iSize.cx + kern;
        for (int icol = 0; icol < g2.m_iSize.cx; icol++) {
            if (irow >= g1.m_iSize.cy || col >= g1.m_iSize.cx || g1.m_grid[irow][col] == FBP_NOP || g2.m_grid[irow][icol] == FBP_NOP) {
                if (irow < g1.m_iSize.cy && col < g1.m_iSize.cx && g1.m_grid[irow][col] != FBP_NOP && g2.m_grid[irow][icol] == FBP_NOP)
                    col++; //do not overwrite g1 with g2.nop
                else
                    buf[irow][col++] = legend2[g2.m_grid[irow][icol]];
            } else {
                bConflict = true;
                auto ch1 = g1.m_grid[irow][col]; (void)ch1;
                auto ch2 = g2.m_grid[irow][icol]; (void)ch2;
                buf[irow][col++] = '#';
            }
        }
    }
    int rowMax = 0;
    for (int irow = 0; irow < GLYPH_MAX_SIZE; irow++) {
        for (int icol = colMax - 2; icol >= 0; icol--) {
            if (buf[irow][icol] == ' ') {
                buf[irow][icol] = '\n';
                buf[irow][icol+1] = 0;
            } else {
                break;
            }
        }
        if (buf[irow][0] == '\n') {
            rowMax = irow;
            break;
        }
    }
    int shortestRay = 2000, shortestDir{}, shortestX{}, shortestY{};
    for (int irow = 0; irow < g1.m_iSize.cy; irow++) {
        if (g1.m_freeAtRight[irow] != g1.m_iSize.cx) {
            for (int dir = -1; dir < 2; dir++) { //2 diagonal and one straight
                int x = g1.m_iSize.cx - g1.m_freeAtRight[irow], y = irow;
                int curLen = 0;
                do {
                    switch (buf[y][x]) {
                    case ' ': case '.': case ':': //empty: go next pixel
                        curLen++;
                        x++;
                        y += dir;
                        if (y < 0 || y >= rowMax) //out of field
                            curLen = 0;
                        break;
                    case '\xfe': //g2 - stop count, accept curLen
                        if (curLen * (dir ? 1.4 : 1.0) < shortestRay * (shortestDir ? 1.4 : 1.0)) {
                            shortestRay = curLen;
                            shortestY = y - dir;
                            shortestX = x - 1;
                            shortestDir = dir;
                        }
                        curLen = 0;
                        break;
                    case '\n': //end of buffer - stop count, reject curLen
                        curLen = 0;
                        break;
                    default: //myself - stop count, reject curLen
                        curLen = 0;
                        break;
                    }
                } while (curLen);
            }
        }
    }
    if (shortestRay == 2000) {
        if (f) fprintf(f, "Shortest ray not found!!!\n");
    } else {
        for (int i = 0; i < shortestRay; i++) {
            buf[shortestY][shortestX--] = '*';
            shortestY -= shortestDir;
        }
    }
    for (int irow = 0; irow < GLYPH_MAX_SIZE; irow++) {
        if (buf[irow][0] == '\n') {
            break;
        }
        if (f) fputs(buf[irow], f);
    }
    if (bConflict)
        if (f) fprintf(f, "Conflicts!!!\n");
    if (shortestDir)
        shortestRay = shortestRay * 14 / 10;
    if (f) {
        fprintf(f, "Kern=%d -> shortestRay=%d\n", kern, shortestRay);
        fclose(f);
    }
    return shortestRay;
}
TEST(SmokeTestFont, PatchToCyr_105_1) {
    //instruction:
    //1. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoBlack105ptW_EFIGS_K.bin to g:\Fonts\105\in\
    //2. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoBlack105ptW0.dds to g:\Fonts\105\in\
    //3. convert dds to txt (4198400 bytes) with BMP-DDS util
    //4. create g:\Fonts\105\in\letters directory
    //5. run this test
    // 5.1 letters dir filled with *.txt files
    // 5.2 bin simplified (no kerning for digits) & converted to txt also
    //6. edit letters, and correct kerning pairs ZwiftFondoBlack105ptW_EFIGS_K.txt as you wish then run PatchToCyr_105_2 test
    FILE *fBMP{ fopen("g:\\fonts\\105\\in\\ZwiftFondoBlack105ptW0.txt", "rt") };
    FILE *fBIN{ fopen("g:\\fonts\\105\\in\\ZwiftFondoBlack105ptW_EFIGS_K.bin", "rb") };
    FILE *fTXT{ fopen("g:\\fonts\\105\\in\\ZwiftFondoBlack105ptW_EFIGS_K.txt", "wt") };
    CFont2D_fileHdrV3 h;
    fread(&h, sizeof(h), 1, fBIN);
    CFont2D_glyphBMP tmp;
    std::map<wchar_t, CFont2D_glyphBMP> map;
    const int sh105[]{ -1,-2,0,-4,-1,-2,-2,-4,-2,0 }, width = 103;
    fprintf(fTXT, "points:%u\n", h.m_charsCnt);
    for (int i = 0; i < h.m_charsCnt; i++) {
        fread(&tmp, sizeof(CFont2D_glyph), 1, fBIN);
        if (tmp.m_lid != 0)
            continue;
        //patch digits
        if (tmp.m_codePoint <= '9' && tmp.m_codePoint >= '0') {
            tmp.m_view.m_width = width / 2048.0f;
            tmp.m_view.m_left += sh105[tmp.m_codePoint - L'0'] / 2048.0f;
        }
        fprintf(fTXT, "%u left=%d width=%d top=%d height=%d\n",
            tmp.m_codePoint, int(tmp.m_view.m_left * 2048.0f + 0.5), int(tmp.m_view.m_width * 2048.0f + 0.5),
            int(tmp.m_view.m_top * 2048.0f + 0.5), int(tmp.m_view.m_height * 2048.0f + 0.5));
        map[(wchar_t)tmp.m_codePoint] = tmp;
    }
    std::unique_ptr<RealKernItem[]> prki{ new RealKernItem[h.m_realKerns] };
    fprintf(fTXT, "kerns:%u\n", h.m_realKerns);
    if (h.m_realKerns != fread(prki.get(), sizeof(RealKernItem), h.m_realKerns, fBIN))
        FAIL();
    //patch kerns (no digit kerning)
    for (int k = 0; k < h.m_realKerns; k++) {
        bool prevNotDigit = prki[k].m_prev < '0' || prki[k].m_prev > '9';
        bool curNotDigit = prki[k].m_cur < '0' || prki[k].m_cur > '9';
        if (prevNotDigit && curNotDigit) {
            fprintf(fTXT, "%u>%u corr=%d\n", prki[k].m_prev, prki[k].m_cur, prki[k].m_corr);
        } else if (!prevNotDigit && (prki[k].m_cur == '.' || prki[k].m_cur == ':')) {
            fprintf(fTXT, "%u>%u corr=4\n", prki[k].m_prev, prki[k].m_cur);
        }
    }
    fclose(fTXT);
    auto fullBMP = LoadFontBitmap(fBMP);
    for (auto &g : map) {
        std::wstring fileName{ L"g:\\fonts\\105\\in\\letters\\" + std::to_wstring(g.first) };
        fileName.push_back(L'_');
        if (g.first > L' ' && !wcschr(L"<>:\"/\\|?*", (wchar_t)g.first))
            fileName.push_back(g.first);
        FILE *fAlphabet{ _wfopen((fileName + L".txt").c_str(), L"wt")};
        if (nullptr == fAlphabet)
            FAIL();
        if (g.second.m_lid == 0)
            dumpGlyphBmp(g.second, fullBMP, fAlphabet);
        else
            FAIL();
        fclose(fAlphabet);
    }
    delete fullBMP;
    fclose(fBMP);
    fclose(fBIN);
}
TEST(SmokeTestFont, PatchToCyr_54_1) {
    //instruction:
    //1. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoMedium54ptW_EFIGS_K.bin to g:\Fonts\54\in\
    //2. run this test -> bin simplified (no kerning for digits) & converted to txt also
    //3. correct kerning pairs ZwiftFondoMedium54ptW_EFIGS_K.txt as you wish then run PatchToCyr_54_2 test
    FILE *fBIN{ fopen("g:\\fonts\\54\\in\\ZwiftFondoMedium54ptW_EFIGS_K.bin", "rb") };
    FILE *fTXT{ fopen("g:\\fonts\\54\\in\\ZwiftFondoMedium54ptW_EFIGS_K.txt", "wt") };
    CFont2D_fileHdrV3 h;
    fread(&h, sizeof(h), 1, fBIN);
    CFont2D_glyphBMP tmp;
    std::map<wchar_t, CFont2D_glyphBMP> map;
    const int sh54[]{ 1,0,1,-1,-1,0,0,-1,0,0 }, width = 71;
    fprintf(fTXT, "points:%u\n", h.m_charsCnt);
    for (int i = 0; i < h.m_charsCnt; i++) {
        fread(&tmp, sizeof(CFont2D_glyph), 1, fBIN);
        if (tmp.m_lid != 0)
            continue;
        //patch digits
        if (tmp.m_codePoint <= '9' && tmp.m_codePoint >= '0') {
            tmp.m_view.m_width = width / 2048.0f;
            tmp.m_view.m_left += sh54[tmp.m_codePoint - L'0'] / 2048.0f;
        }
        fprintf(fTXT, "%u left=%d width=%d top=%d height=%d\n",
            tmp.m_codePoint, int(tmp.m_view.m_left * 2048.0f + 0.5), int(tmp.m_view.m_width * 2048.0f + 0.5),
            int(tmp.m_view.m_top * 2048.0f + 0.5), int(tmp.m_view.m_height * 2048.0f + 0.5));
        map[(wchar_t)tmp.m_codePoint] = tmp;
    }
    std::unique_ptr<RealKernItem[]> prki{ new RealKernItem[h.m_realKerns] };
    fprintf(fTXT, "kerns:%u\n", h.m_realKerns);
    if (h.m_realKerns != fread(prki.get(), sizeof(RealKernItem), h.m_realKerns, fBIN))
        FAIL();
    //patch kerns (no digit kerning)
    for (int k = 0; k < h.m_realKerns; k++) {
        bool prevNotDigit = prki[k].m_prev < '0' || prki[k].m_prev > '9';
        bool curNotDigit = prki[k].m_cur < '0' || prki[k].m_cur > '9';
        if (prevNotDigit && curNotDigit) {
            fprintf(fTXT, "%u>%u corr=%d\n", prki[k].m_prev, prki[k].m_cur, prki[k].m_corr);
        } else if (!prevNotDigit && (prki[k].m_cur == '.' || prki[k].m_cur == ':')) {
            fprintf(fTXT, "%u>%u corr=4\n", prki[k].m_prev, prki[k].m_cur);
        }
    }
    fclose(fTXT);
    fclose(fBIN);
}
bool CFont2D_glyphBMP::initFromLetterTxt(const char *file) {
    bool ret = false;
    memset(m_freeAtRight, 0, sizeof(m_freeAtRight));
    m_sum = 0;
    FILE *letter = fopen(file, "rt");
    if (letter) {
        char line[4096]{};
        fgets(line, sizeof(line), letter);
        int cp, w, h, l, t, r, b;
        if (sscanf_s(line, "Glyph codePoint:%x, w=%d, h=%d, pad.ltrb=[%d, %d, %d, %d]", &cp, &w, &h, &l, &t, &r, &b) == 7) {
            m_codePoint = (uint16_t)cp;
            m_iSize.cx = w; m_iSize.cy = h;
            m_paddings.left = l; m_paddings.right = r; m_paddings.top = t; m_paddings.bottom = b;
            int row{}, col{}, minFreeAtRight = FONT_BITMAP_SIZE + 1, minFreeAtLeft = FONT_BITMAP_SIZE + 1, topFree{}, bottomFree{};
            ret = true;
            bool wasDirtyLine{};
            while (ret && fgets(line, sizeof(line), letter)) {
                if (line[0] == '\n')
                    break;
                char *ptr = line;
                int freeAtRight{}, freeAtLeft{};
                bool dirty{};
                col = 0;
                while (*ptr) {
                    switch (*ptr++) {
                    case glbGlyphLegend[FBP_NOP]:
                        ++freeAtRight;
                        if (!dirty)
                            ++freeAtLeft;
                        m_grid[row][col++] = FBP_NOP;
                        break;
                    case glbGlyphLegend[FBP_LIGHT]:
                        m_grid[row][col++] = FBP_LIGHT;
                        m_sum += FBP_LIGHT;
                        freeAtRight = 0;
                        dirty = true;
                        break;
                    case glbGlyphLegend[FBP_MEDIUM]:
                        m_grid[row][col++] = FBP_MEDIUM;
                        m_sum += FBP_MEDIUM;
                        freeAtRight = 0;
                        dirty = true;
                        break;
                    case glbGlyphLegend[FBP_SOLID]:
                        m_grid[row][col++] = FBP_SOLID;
                        m_sum += FBP_SOLID;
                        freeAtRight = 0;
                        dirty = true;
                        break;
                    case '\n':
                        break;
                    default:
                        ret = false;
                        break;
                    }
                }
                m_freeAtRight[row++] = freeAtRight;
                minFreeAtRight = std::min(minFreeAtRight, freeAtRight);
                minFreeAtLeft = std::min(minFreeAtLeft, freeAtLeft);
                if (freeAtRight == freeAtLeft) { //empty line
                    if(!wasDirtyLine) ++topFree;
                    ++bottomFree;
                } else {
                    wasDirtyLine = true;
                    bottomFree = 0;
                }
            }
            if (topFree != m_paddings.top)
                ret = false;
            if (bottomFree != m_paddings.bottom)
                ret = false;
            if (minFreeAtRight != m_paddings.right)
                ret = false;
            if (minFreeAtLeft != m_paddings.left)
                ret = false;
            if (col != m_iSize.cx)
                ret = false;
            if (row != m_iSize.cy)
                ret = false;
        }
        fclose(letter);
    }
    return ret;
}
bool CFont2D_glyphBMP::conflictAt(FontBitmapPixelGrid *grid, POINT p) {
    for (int y = 0; y < m_iSize.cy; y++) {
        for (int x = 0; x < m_iSize.cx; x++) {
            auto prev = (*grid)[p.y + y][p.x + x];
            auto pix = m_grid[y][x];
            switch (prev) {
            case FBP_NOP:
                //FBP_NOP примет любого
                break;
            case FBP_NOP1: case FBP_NOP2:
                if (pix != FBP_NOP)
                    return true;
                break;
            default:
                if (pix != prev)
                    return true;
            }
        }
    }
    return false;
}
bool CFont2D_glyphBMP::findEmptySpace(FontBitmapPixelGrid *grid, POINT *result, FinderCache *cache) {
    for (auto [itr, rangeEnd] = cache->m_sameGlyphs.equal_range(m_sum); itr != rangeEnd; ++itr) {
        if (!conflictAt(grid, itr->second)) {
            *result = itr->second;
            return true;
        }
    }
    int checksRemaining = (FONT_BITMAP_SIZE - m_iSize.cy) * (FONT_BITMAP_SIZE - m_iSize.cx);
    for (; ; cache->m_trialPoint.y = (cache->m_trialPoint.y + 1) % (FONT_BITMAP_SIZE - m_iSize.cy), cache->m_trialPoint.x = 0) {
        for (; cache->m_trialPoint.x < FONT_BITMAP_SIZE - m_iSize.cx; ++cache->m_trialPoint.x) {
            if (!conflictAt(grid, cache->m_trialPoint)) {
                cache->m_sameGlyphs.insert({ m_sum, cache->m_trialPoint });
                *result = cache->m_trialPoint;
                return true;
            }
            if (checksRemaining-- < 0)
                return false;
        }
    }
}
bool CFont2D_glyphBMP::appendTo(FontBitmapPixelGrid *grid, FinderCache *cache) {
    //найти свободное место, занять его и обновить себе m_view[l, t, w, h]
    POINT pt;
    if (!findEmptySpace(grid, &pt, cache))
        return false; //увы, место закончилось
    for (int y = 0; y < m_iSize.cy; y++) {
        for (int x = 0; x < m_iSize.cx; x++) {
            auto &prev = (*grid)[pt.y + y][pt.x + x];
            auto pix = m_grid[y][x];
            switch (prev) {
            case FBP_NOP:
                prev = (pix == FBP_NOP) ? FBP_NOP1 : pix;
                break;
            case FBP_NOP1: case FBP_NOP2:
                if (pix != FBP_NOP)
                    return false;
                prev = FBP_NOP2;
                break;
            default:
                if (pix != prev)
                    return false;
            }
        }
    }
    m_view.m_top = pt.y / 2048.0f;
    m_view.m_left = pt.x / 2048.0f;
    m_view.m_height = m_iSize.cy / 2048.0f;
    m_view.m_width = m_iSize.cx / 2048.0f;
    return true;
}
void dumpFontBitmapPixelGrid(const FontBitmapPixelGrid &outDds, FILE *fBMP) {
    for (int irow = 0; irow < FONT_BITMAP_SIZE; irow++) {
        for (int icol = 0; icol < FONT_BITMAP_SIZE; icol++) {
            fprintf(fBMP, "%c", glbGlyphLegend[outDds[irow][icol]]);
        }
        fprintf(fBMP, "\n");
    }
}
TEST(SmokeTestFont, PatchToCyr_105_2) {
    //instruction:
    //1. create g:\Fonts\105\out directory
    //2. run this test:
    // 2.1 new g:\Fonts\105\out\ZwiftFondoBlack105ptW0.txt is created with condensed and filtered letters
    // 2.2 new g:\Fonts\105\out\ZwiftFondoBlack105ptW_EFIGS_K.bin is created from old bin and corrected txt kerns
    //3. convert txt to dds with BMP-DDS util then edit *.dds in VS editor then execute "Generate Mips" command in VS
    //4. convert *.dds back to ztx with tgax util then copy out bin and tgax back to assets\fonts\font.wad\Fonts\ and data\Fonts\ 
    FILE *fBMP{ fopen("g:\\fonts\\105\\out\\ZwiftFondoBlack105ptW0.txt", "wt") }; //resulting dds 2048*2048
    FILE *fBIN[]{ fopen("g:\\fonts\\105\\in\\ZwiftFondoBlack105ptW_EFIGS_K.bin", "rb"), fopen("g:\\fonts\\105\\out\\ZwiftFondoBlack105ptW_EFIGS_K.bin", "wb") }; //i&o fonts
    FILE *fTXT{ fopen("g:\\fonts\\105\\in\\ZwiftFondoBlack105ptW_EFIGS_K.txt", "rt") }; //you can correct kerns in this file only; todo: 0->autocorrect
    //read our corrections
    char line[4096];
    CFont2D_glyphBMP tmp;
    std::map<wchar_t, CFont2D_glyphBMP> map;
    //read our new glyphs
    auto outDds{ std::make_unique<FontBitmapPixelGrid>() };
    auto path{ std::filesystem::path("g:\\fonts\\105\\in\\letters") };
    FinderCache cache;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
        EXPECT_TRUE(tmp.initFromLetterTxt(entry.path().generic_string().c_str()));
        EXPECT_TRUE(tmp.appendTo(outDds.get(), &cache));
        map[(wchar_t)tmp.m_codePoint] = tmp;
        printf("%03d/271\r", (int)map.size());
    }
    dumpFontBitmapPixelGrid(*outDds.get(), fBMP);
    fclose(fBMP);
    std::vector<RealKernItem> newKerns;
    newKerns.reserve(70000);
    //read our new kerns
    while (fgets(line, sizeof(line), fTXT)) {
        int prev, cur, corr;
        if (sscanf_s(line, "%d>%d corr=%d", &prev, &cur, &corr) == 3) {
            if (map.contains(prev) && map.contains(cur))
                newKerns.push_back(RealKernItem{ .m_prev = (UChar)prev, .m_cur = (UChar)cur, .m_corr = (char)corr });
        }
    }
    //write back to bin
    CFont2D_fileHdrV3 hIn, hOut;
    fread(&hIn, sizeof(hIn), 1, fBIN[0]);
    hOut = hIn;
    hOut.m_realKerns = (int)newKerns.size();
    hOut.m_charsCnt = 0;
    fwrite(&hOut, sizeof(hOut), 1, fBIN[1]);
    for (int i = 0; i < hIn.m_charsCnt; i++) {
        fread(&tmp, sizeof(CFont2D_glyph), 1, fBIN[0]);
        if (tmp.m_lid == 0) {
            //correct if any
            auto f = map.find(tmp.m_codePoint);
            if (f == map.end())
                continue; //deleted
            tmp = f->second;
        }
        hOut.m_charsCnt++;
        fwrite(&tmp, sizeof(CFont2D_glyph), 1, fBIN[1]);
        map.erase(tmp.m_codePoint);
    }
    //write our new glyphs
    for (auto newg : map) {
        hOut.m_charsCnt++;
        fwrite(&newg.second, sizeof(CFont2D_glyph), 1, fBIN[1]);
    }
    //write our new kerns
    fwrite(newKerns.data(), sizeof(RealKernItem), newKerns.size(), fBIN[1]);
    fseek(fBIN[1], 0, SEEK_SET);
    fwrite(&hOut, sizeof(hOut), 1, fBIN[1]);
    for (auto f : fBIN) fclose(f);
    fclose(fTXT);
}
TEST(SmokeTestFont, PatchToCyr_54_2) {
    //new g:\Fonts\54\ZwiftFondoMedium54ptW_EFIGS_K.bin is created from old bin and corrected txt kerns
    FILE *fBIN[]{ fopen("g:\\fonts\\54\\in\\ZwiftFondoMedium54ptW_EFIGS_K.bin", "rb"), fopen("g:\\fonts\\54\\ZwiftFondoMedium54ptW_EFIGS_K.bin", "wb") }; //i&o fonts
    FILE *fTXT{ fopen("g:\\fonts\\54\\in\\ZwiftFondoMedium54ptW_EFIGS_K.txt", "rt") }; //you can correct kerns in this file only; todo: 0->autocorrect
    //read our corrections
    char line[4096];
    std::vector<RealKernItem> newKerns;
    newKerns.reserve(70000);
    //read our new kerns
    while (fgets(line, sizeof(line), fTXT)) {
        int prev, cur, corr;
        if (sscanf_s(line, "%d>%d corr=%d", &prev, &cur, &corr) == 3) {
            newKerns.push_back(RealKernItem{ .m_prev = (UChar)prev, .m_cur = (UChar)cur, .m_corr = (char)corr });
        }
    }
    //write back to bin
    CFont2D_fileHdrV3 hIn, hOut;
    fread(&hIn, sizeof(hIn), 1, fBIN[0]);
    hOut = hIn;
    hOut.m_realKerns = (int)newKerns.size();
    CFont2D_glyph tmp;
    fwrite(&hOut, sizeof(hOut), 1, fBIN[1]);
    const int sh54[]{ 1,0,1,-1,-1,0,0,-1,0,0 }, width = 71;
    for (int i = 0; i < hIn.m_charsCnt; i++) {
        fread(&tmp, sizeof(CFont2D_glyph), 1, fBIN[0]);
        if (tmp.m_lid == 0) {
            //patch digits
            if (tmp.m_codePoint <= '9' && tmp.m_codePoint >= '0') {
                tmp.m_view.m_width = width / 2048.0f;
                tmp.m_view.m_left += sh54[tmp.m_codePoint - L'0'] / 2048.0f;
            }
        }
        fwrite(&tmp, sizeof(CFont2D_glyph), 1, fBIN[1]);
    }
    //write our new kerns
    fwrite(newKerns.data(), sizeof(RealKernItem), newKerns.size(), fBIN[1]);
    for (auto f : fBIN) fclose(f);
    fclose(fTXT);
}
TEST(SmokeTestFont, PatchTo_Cyr_old) {
    //prepare:
    // 1. unpack zwift-offline\scripts\ZwiftApp\1.32.1_106405\assets\fonts\font.wad\Fonts\*.* to zwift-offline\scripts\ZwiftApp\1.32.1_106405\data\Fonts
    // 2. convert zwift-offline\scripts\ZwiftApp\1.32.1_106405-Debug\data\Fonts\*.ztx to *.dds (zwift-offline\scripts\textures\tgax) and edit *.dds in VS editor then execute "Generate Mips" command in VS
    // 3. convert *.dds back to ztx (the same utility)
    // 4. fill coordinates below:
    struct ddsCoordinates { wchar_t codePoint, stoleFrom; RECT coords[2]; /*54, 105*/ }; //l t r->w b->h
    const int h1 = 120, h2 = 168, row1_54 = 1041, row2_54 = row1_54 + 130, row3_54 = row2_54 + 130,
        row1_105 = 1 + 168 * 10, row2_105 = row1_105 + 168;
    ddsCoordinates patch[]{
        //                  x             w             x              w
        { L'А', L'A', {{    0, row1_54,  76, h1 }, {                         }}},
        { L'Б',    0, {{   72, row1_54,  68, h1 }, {    0, row1_105, 102, h2 }}},
        { L'В', L'B', {{  136, row1_54,  66, h1 }, {                         }}},
        { L'Г',    0, {{  198, row1_54,  58, h1 }, {  101, row1_105,  91, h2 }}},
        { L'Д',    0, {{  252, row1_54,  85, h1 }, {  727, row1_105, 139, h2 }}},
        { L'Е', L'E', {{  333, row1_54,  63, h1 }, {                         }}},
        { L'Ё', L'Ë', {{  390, row1_54,  64, h1 }, {                         }}},
        { L'Ж',    0, {{  452, row1_54, 110, h1 }, {  301, row1_105, 165, h2 }}},
        { L'З', L'3', {{  560, row1_54,  61, h1 }, {                         }}},
        { L'И',    0, {{  617, row1_54,  75, h1 }, {  188, row1_105, 116, h2 }}},
        { L'Й',    0, {{  688, row1_54,  76, h1 }, {    0, row2_105, 116, h2 }}},
        { L'К', L'K', {{  760, row1_54,  72, h1 }, {                         }}},
        { L'Л',    0, {{  828, row1_54,  76, h1 }, {  463, row1_105, 116, h2 }}},
        { L'М', L'M', {{  902, row1_54,  95, h1 }, {                         }}},
        { L'Н', L'H', {{  994, row1_54,  71, h1 }, {                         }}},
        { L'О', L'O', {{ 1061, row1_54,  89, h1 }, {                         }}},
        { L'П',    0, {{ 1146, row1_54,  75, h1 }, {  964, row1_105, 116, h2 }}},
        { L'Р', L'P', {{ 1217, row1_54,  64, h1 }, {                         }}},
        { L'С', L'C', {{ 1277, row1_54,  69, h1 }, {                         }}},
        { L'Т', L'T', {{ 1344, row1_54,  66, h1 }, {                         }}},
        { L'У',    0, {{ 1408, row1_54,  73, h1 }, { 1077, row1_105, 103, h2 }}},
        { L'Ф', L'Ø', {{ 1479, row1_54,  96, h1 }, {                         }}},
        { L'Х', L'X', {{ 1573, row1_54,  74, h1 }, {                         }}},
        { L'Ц',    0, {{ 1643, row1_54,  80, h1 }, { 1455, row1_105, 120, h2 }}},
        { L'Ч',    0, {{ 1720, row1_54,  70, h1 }, {  516, row2_105, 101, h2 }}},
        { L'Ш',    0, {{ 1786, row1_54, 110, h1 }, { 1177, row1_105, 140, h2 }}},
        { L'Щ',    0, {{ 1892, row1_54, 122, h1 }, { 1314, row1_105, 145, h2 }}},
        { L'Ь',    0, {{    0, row3_54,  62, h1 }, { 1572, row1_105,  93, h2 }}},
        { L'Ы',    0, {{    0, row3_54,  90, h1 }, { 1572, row1_105, 140, h2 }}},
        { L'Ъ',    0, {{   85, row3_54,  81, h1 }, { 1709, row1_105, 117, h2 }}},
        { L'Э',    0, {{  164, row3_54,  69, h1 }, { 1958, row1_105,  90, h2 }}},
        { L'Ю',    0, {{  229, row3_54, 107, h1 }, { 1823, row1_105, 138, h2 }}},
        { L'Я',    0, {{  334, row3_54,  68, h1 }, {  865, row1_105, 104, h2 }}},
        { L'а', L'a', {{    0, row2_54,  59, h1 }, {                         }}},
        { L'б',    0, {{   55, row2_54,  68, h1 }, {  113, row2_105,  90, h2 }}},
        { L'в',    0, {{  120, row2_54,  58, h1 }, {  200, row2_105,  77, h2 }}},
        { L'г',    0, {{  175, row2_54,  50, h1 }, {  274, row2_105,  64, h2 }}},
        { L'д',    0, {{  220, row2_54,  76, h1 }, {  333, row2_105,  96, h2 }}},
        { L'е', L'e', {{  292, row2_54,  64, h1 }, {                         }}},
        { L'ё', L'ë', {{  353, row2_54,  63, h1 }, {                         }}},
        { L'ж',    0, {{  413, row2_54,  98, h1 }, {  576, row1_105, 134, h2 }}},
        { L'з',    0, {{  506, row2_54,  55, h1 }, {  614, row2_105,  69, h2 }}},
        { L'и', L'u', {{  558, row2_54,  64, h1 }, {                         }}},
        { L'й', L'ú', {{  619, row2_54,  61, h1 }, {                         }}},
        { L'к',    0, {{  677, row2_54,  61, h1 }, {  757, row2_105,  81, h2 }}},
        { L'л',    0, {{  735, row2_54,  65, h1 }, {  835, row2_105,  82, h2 }}},
        { L'м',    0, {{  795, row2_54,  82, h1 }, {  913, row2_105, 109, h2 }}},
        { L'н',    0, {{  874, row2_54,  60, h1 }, { 1019, row2_105,  79, h2 }}},
        { L'о', L'o', {{  931, row2_54,  70, h1 }, {                         }}},
        { L'п', L'n', {{  998, row2_54,  64, h1 }, {                         }}},
        { L'р', L'p', {{ 1058, row2_54,  69, h1 }, {                         }}},
        { L'с', L'c', {{ 1123, row2_54,  56, h1 }, {                         }}},
        { L'т', L'm', {{ 1176, row2_54,  60, h1 }, {                         }}},
        { L'у', L'y', {{ 1233, row2_54,  64, h1 }, {                         }}},
        { L'ф',    0, {{ 1293, row2_54,  93, h1 }, { 1095, row2_105, 122, h2 }}},
        { L'х', L'x', {{ 1382, row2_54,  62, h1 }, {                         }}},
        { L'ц',    0, {{ 1443, row2_54,  66, h1 }, { 1214, row2_105,  89, h2 }}},
        { L'ч',    0, {{ 1505, row2_54,  62, h1 }, { 1300, row2_105,  79, h2 }}},
        { L'ш',    0, {{ 1563, row2_54,  96, h1 }, { 1375, row2_105, 115, h2 }}},
        { L'щ',    0, {{ 1655, row2_54, 102, h1 }, { 1485, row2_105, 128, h2 }}},
        { L'ь',    0, {{ 1747, row2_54,  60, h1 }, {  428, row2_105,  71, h2 }}},
        { L'ы',    0, {{ 1747, row2_54,  84, h1 }, { 1610, row2_105, 101, h2 }}},
        { L'ъ',    0, {{  400, row3_54,  73, h1 }, { 1708, row2_105,  91, h2 }}},
        { L'э',    0, {{  468, row3_54,  60, h1 }, { 1796, row2_105,  75, h2 }}},
        { L'ю',    0, {{  523, row3_54,  91, h1 }, { 1868, row2_105, 105, h2 }}},
        { L'я',    0, {{  609, row3_54,  64, h1 }, { 1970, row2_105,  78, h2 }}},
    };
    FILE *fOrg[]{ fopen("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_K.bin.org", "rb"), fopen("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_K.bin.org", "rb") };
    FILE *fKernCache[]{ fopen("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_K.bin.cache", "rb"), fopen("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_K.bin.cache", "rb") };
    FILE *fPatched[]{ fopen("data/Fonts/ZwiftFondoMedium54ptW_EFIGS_K.bin", "wb"), fopen("data/Fonts/ZwiftFondoBlack105ptW_EFIGS_K.bin", "wb") };
    FILE *fBMP[]{ fopen("data/Fonts/ZwiftFondoMedium54ptW0.txt", "rt"), fopen("data/Fonts/ZwiftFondoBlack105ptW0.txt", "rt") };
    FILE *fAlphabet[]{ fopen("data/Fonts/ZwiftFondoMedium54ptW0-a.txt", "wt"), fopen("data/Fonts/ZwiftFondoBlack105ptW0-a.txt", "wt") };
    const char *kernDumpFmt[] = { "data/Fonts/ZwiftFondoMedium54ptW0/%04x-%04x.txt", "data/Fonts/ZwiftFondoBlack105ptW0/%04x-%04x.txt" };
    const int goalKernDistance[] = { 19, 22 };
    for (int i = 0; i < 2; i++) {
        CFont2D_fileHdrV3 h, hk;
        fread(&h, sizeof(h), 1, fOrg[i]);
        fread(&hk, sizeof(hk), 1, fKernCache[i]);
        std::unique_ptr<RealKernItem[]> prki{ new RealKernItem[hk.m_realKerns] };
        fseek(fKernCache[i], -(int)sizeof(RealKernItem) * hk.m_realKerns, SEEK_END);
        if (hk.m_realKerns != fread(prki.get(), sizeof(RealKernItem), hk.m_realKerns, fKernCache[i]))
            FAIL();
        h.m_charsCnt += _countof(patch);
        fwrite(&h, sizeof(h), 1, fPatched[i]);
        CFont2D_glyphBMP tmp;
        std::map<wchar_t, CFont2D_glyphBMP> map;
        std::set<std::string> line_height;
        while (fread(&tmp, sizeof(CFont2D_glyph), 1, fOrg[i])) {
            if (tmp.m_codePoint == L'!') {
                tmp.m_view.m_width += (i ? 5 : 4) / 2048.0;
            }
            if (i == 1 && tmp.m_lid == 0) { //compact 105
                int row = (int(tmp.m_view.m_top * 2048.0f + 0.5) - 1) / 168;
                if (row > 0)
                    tmp.m_view.m_top = (row * 168 + 1) / 2048.0f;
            }
            fwrite(&tmp, sizeof(CFont2D_glyph), 1, fPatched[i]);
            map[(wchar_t)tmp.m_codePoint] = tmp;
            if (tmp.m_lid == 0)
                line_height.insert(std::to_string(int(tmp.m_view.m_top * 2048.0f + 0.5)) + "x" + std::to_string(int(tmp.m_view.m_height * 2048.0f + 0.5)));
        }
        for (const auto &g : patch) {
            CFont2D_glyphBMP newg{};
            if (g.coords[i].right == 0 && g.stoleFrom != 0) {
                assert(map.contains(g.stoleFrom));
                newg = map[g.stoleFrom];
                newg.m_codePoint = g.codePoint;
            } else {
                newg.m_codePoint = g.codePoint;
                newg.m_view.m_left = g.coords[i].left / 2048.0f;
                newg.m_view.m_top = g.coords[i].top / 2048.0f;
                newg.m_view.m_width = g.coords[i].right / 2048.0f;
                newg.m_view.m_height = g.coords[i].bottom / 2048.0f;
            }
            fwrite(&newg, sizeof(CFont2D_glyph), 1, fPatched[i]);
            map[(wchar_t)newg.m_codePoint] = newg;
        }
        auto fullBMP = LoadFontBitmap(fBMP[i]);
        for (auto &g : map) {
            if (g.second.m_lid == 0)
                dumpGlyphBmp(g.second, fullBMP, fAlphabet[i]);
        }
        delete fullBMP;
        std::map<std::pair<UChar, UChar>, char> autoKern;
        std::vector<RealKernItem> autoKernArray;
        for (int k = 0; k < hk.m_realKerns; k++) {
            auto key = std::make_pair(prki[k].m_prev, prki[k].m_cur);
            if (autoKern.contains(key))
                printf("autoKern.contains\n");
            autoKern[key] = prki[k].m_corr;
        }
        for (auto g1 : map) {
            if (g1.second.m_codePoint == ' ' || g1.second.m_codePoint == 10 || g1.second.m_codePoint == 160 || g1.second.m_codePoint == 0x200b || g1.second.m_lid != 0) continue;
            for (auto g2 : map) {
                if (g2.second.m_codePoint == ' ' || g2.second.m_codePoint == 10 || g2.second.m_codePoint == 160 || g2.second.m_codePoint == 0x200b || g2.second.m_lid != 0) continue;
                auto key = std::make_pair(g1.second.m_codePoint, g2.second.m_codePoint);
                char &corr = autoKern[key];
                std::map<char, int> tries;
                do {
                    auto dist = dumpKernPairBmp(g1.second, g2.second, kernDumpFmt[i], corr, false);
                    tries[corr] = dist;
                    if (dist == 2000) {
                        break;
                    }
                    if (corr == -16)
                        break;
                    auto deltaDist = dist - goalKernDistance[i];
                    if (deltaDist == 0) break;
                    corr = std::clamp(corr - deltaDist / abs(deltaDist), -16, 16);
                    if (tries.contains(corr))
                        break;
                } while (true);
                int bestDelta = 2000;
                if (tries.size() > 1) {
                    for (auto &t : tries) {
                        auto dt = abs(t.second - goalKernDistance[i]);
                        if (dt < bestDelta) {
                            bestDelta = dt;
                            corr = t.first;
                        }
                    }
                    if (corr && tries[0] == tries[corr])
                        corr = 0;
                }
                //dumpKernPairBmp(g1.second, g2.second, kernDumpFmt[i], corr, true);
            }
        }
        for (auto &mi : autoKern)
            if (mi.second)
                autoKernArray.push_back(RealKernItem{ .m_prev = mi.first.first, .m_cur = mi.first.second, .m_corr = mi.second });
        h.m_realKerns = (int)autoKernArray.size();
        fwrite(autoKernArray.data(), h.m_realKerns * sizeof(RealKernItem), 1, fPatched[i]);
        fseek(fPatched[i], 0, SEEK_SET);
        fwrite(&h, sizeof(h), 1, fPatched[i]);
    }
    for (auto f : fPatched) fclose(f);
    for (auto f : fOrg) fclose(f);
    for (auto f : fBMP) fclose(f);
    for (auto f : fAlphabet) fclose(f);
    for (auto f : fKernCache) fclose(f);
}
#endif