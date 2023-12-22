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
const int FONT_BITMAP_SIZE = 2048, GLYPH_MAX_SIZE = 180;
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
    const int GridSize = 8; //mipmap 128*128 (где ячейки 16*16) нечитабелен, зато 256*256 уже что-то
};
constexpr char glbGlyphLegend[]{ '.', '\xb0', '\xb1', '\xdb', '\'', '"'};
struct CFont2D_glyphBMP : public CFont2D_glyph {
    SIZE m_iSize{};
    POINT m_iPos{};
    RECT m_paddings{};
    GlyphBitmapPixelGrid m_grid;
    uint64_t m_sum{};
    int m_freeAtRight[GLYPH_MAX_SIZE]{};
    void Clear() {
        memset(m_freeAtRight, 0, sizeof(m_freeAtRight));
        memset(&m_grid, 0, sizeof(m_grid));
        m_sum = 0;
        m_paddings.right = m_paddings.left = m_paddings.top = m_paddings.bottom = -1;
    }
    void initFromGrid(const FontBitmapPixelGrid *grid) {
        Clear();
        if (m_iSize.cx > GLYPH_MAX_SIZE || m_iSize.cy > GLYPH_MAX_SIZE) {
            printf("overflow\n");
        }
        for (int row = m_iPos.y, irow = 0; irow < m_iSize.cy; ++row, ++irow) {
            bool pixFound = false;
            m_freeAtRight[irow] = m_iSize.cx;
            for (int col = m_iPos.x, icol = 0; icol < m_iSize.cx; ++col, ++icol) {
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
            //printf("why aligned to border");
        }
    }
    bool initFromLetterTxt(const wchar_t *file);
    bool appendTo(FontBitmapPixelGrid *grid, FinderCache *cache);
    bool findEmptySpace(FontBitmapPixelGrid *grid, POINT *result, FinderCache *cache);
    bool conflictAt(FontBitmapPixelGrid *grid, POINT tryPoint, FinderCache *cache);
    void dumpTo(FILE *fAlphabet) {
        fprintf(fAlphabet, "Glyph codePoint:%04x, w=%d, h=%d, pad.ltrb=[%d, %d, %d, %d]\n", m_codePoint, m_iSize.cx, m_iSize.cy,
            m_paddings.left, m_paddings.top, m_paddings.right, m_paddings.bottom);
        for (int irow = 0; irow < m_iSize.cy; irow++) {
            for (int icol = 0; icol < m_iSize.cx; icol++) {
                fprintf(fAlphabet, "%c", glbGlyphLegend[m_grid[irow][icol]]);
            }
            fprintf(fAlphabet, "\n");
        }
        fprintf(fAlphabet, "\n");
    }
    FontBitmapPixel AvgFor(int y, int x, int psz) const {
        int sum{}, maxSum = psz * psz * FBP_SOLID;
        for (int row = y; row < y + psz; row++) {
            for (int col = x; col < x + psz; col++) {
                sum += m_grid[row][col];
            }
        }
        if (sum > maxSum * 9 / 12) return FBP_SOLID;
        if (sum > maxSum * 8 / 12) return FBP_MEDIUM;
        if (sum > maxSum * 7 / 12) return FBP_LIGHT;
        return FBP_NOP;
    }
    CFont2D_glyphBMP asPixelSize(int psz) const { //2048 to required
        CFont2D_glyphBMP ret;
        ret.m_codePoint = m_codePoint;
        ret.m_iSize.cx = m_iSize.cx / psz;
        ret.m_iSize.cy = m_iSize.cy / psz;
        ret.m_iPos.x = m_iPos.x / psz;
        ret.m_iPos.y = m_iPos.y / psz;
        for (int y = 0; y < ret.m_iSize.cy; y++) {
            for (int x = 0; x < ret.m_iSize.cx; x++) {
                auto avgColor = AvgFor(y * psz, x * psz, psz);
                ret.m_grid[y][x] = avgColor;
            }
        }
        if (ret.m_iSize.cx > 2 && ret.m_iSize.cy > 2) {
            //крайние - внутрь, чтобы символы не сливались
            for (int x = 0; x < ret.m_iSize.cx; x++) {
                if (ret.m_grid[0][x] && ret.m_grid[1][x] < FBP_SOLID)
                    ret.m_grid[1][x] = FontBitmapPixel(ret.m_grid[1][x] + 1);
                ret.m_grid[0][x] = FBP_NOP;
                if (ret.m_grid[ret.m_iSize.cy - 1][x] && ret.m_grid[ret.m_iSize.cy - 2][x] < FBP_SOLID)
                    ret.m_grid[ret.m_iSize.cy - 2][x] = FontBitmapPixel(ret.m_grid[ret.m_iSize.cy - 2][x] + 1);
                ret.m_grid[ret.m_iSize.cy - 1][x] = FBP_NOP;
            }
            for (int y = 0; y < ret.m_iSize.cy; y++) {
                if (ret.m_grid[y][0] && ret.m_grid[y][1] < FBP_SOLID)
                    ret.m_grid[y][1] = FontBitmapPixel(ret.m_grid[y][1] + 1);
                ret.m_grid[y][0] = FBP_NOP;
                if (ret.m_grid[y][ret.m_iSize.cx - 1] && ret.m_grid[y][ret.m_iSize.cx - 2] < FBP_SOLID)
                    ret.m_grid[y][ret.m_iSize.cx - 2] = FontBitmapPixel(ret.m_grid[y][ret.m_iSize.cx - 2] + 1);
                ret.m_grid[y][ret.m_iSize.cx - 1] = FBP_NOP;
            }
        }
        ret.m_paddings.bottom = ret.m_iSize.cy;
        ret.m_paddings.left = ret.m_paddings.right = ret.m_iSize.cx;
        for (int y = 0; y < ret.m_iSize.cy; y++) {
            LONG rowSum{}, rowLeftPad{};
            ret.m_freeAtRight[y] = ret.m_iSize.cx;
            for (int x = 0; x < ret.m_iSize.cx; x++) {
                auto val = ret.m_grid[y][x];
                rowSum += val;
                if (rowSum == 0) rowLeftPad++;
                if (val) ret.m_freeAtRight[y] = ret.m_iSize.cx - x - 1;
            }
            if (ret.m_paddings.left > rowLeftPad)
                ret.m_paddings.left = rowLeftPad;
            if (ret.m_paddings.right > ret.m_freeAtRight[y])
                ret.m_paddings.right = ret.m_freeAtRight[y];

            ret.m_sum += rowSum;
            if (ret.m_sum == 0) ret.m_paddings.top++;
            if (rowSum) ret.m_paddings.bottom = ret.m_iSize.cy - y - 1;
        }
        //плавающая позиция/размеры не посчитаны, т.к. для mipmap они не сохраняются никуда и не имеют смысла
        return ret;
    }
    bool placeExactlyTo(FontBitmapPixelGrid *grid, POINT pt) const;
};
void dumpGlyphBmp(CFont2D_glyphBMP &g, const FontBitmapPixelGrid *grid, FILE *fAlphabet) {
    g.initFromGrid(grid);
    g.dumpTo(fAlphabet);
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
bool CFont2D_glyphBMP::initFromLetterTxt(const wchar_t *file) {
    bool ret = false;
    Clear();
    FILE *letter = _wfopen(file, L"rt");
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
                if (freeAtRight == m_iSize.cx) { //empty line
                    if(!wasDirtyLine) ++topFree;
                    ++bottomFree;
                } else {
                    wasDirtyLine = true;
                    bottomFree = 0;
                }
            }
            if (topFree != m_paddings.top && w && h)
                ret = false;
            if (bottomFree != m_paddings.bottom && w && h)
                ret = false;
            if (minFreeAtRight != m_paddings.right && w && h)
                ret = false;
            if (minFreeAtLeft != m_paddings.left && w && h)
                ret = false;
            if (col != m_iSize.cx && w && h)
                ret = false;
            if (row != m_iSize.cy && w && h)
                ret = false;
        } else {
            printf("sscanf"); //break here
        }
        fclose(letter);
    }
    return ret;
}
bool CFont2D_glyphBMP::conflictAt(FontBitmapPixelGrid *grid, POINT p, FinderCache *cache) {
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
        if (!conflictAt(grid, itr->second, cache)) {
            *result = itr->second;
            return true;
        }
    }
    int checksRemaining = (FONT_BITMAP_SIZE - m_iSize.cy) * (FONT_BITMAP_SIZE - m_iSize.cx) / cache->GridSize / cache->GridSize;
    while(true) {
        for (; cache->m_trialPoint.x < FONT_BITMAP_SIZE - m_iSize.cx - cache->GridSize; cache->m_trialPoint.x += cache->GridSize) {
            if (!conflictAt(grid, cache->m_trialPoint, cache)) {
                cache->m_sameGlyphs.insert({ m_sum, cache->m_trialPoint });
                *result = cache->m_trialPoint;
                return true;
            }
            if (checksRemaining-- < 0)
                return false;
        }
        cache->m_trialPoint.y += cache->GridSize;
        cache->m_trialPoint.x = 0;
        if (cache->m_trialPoint.y + m_iSize.cy >= FONT_BITMAP_SIZE)
            cache->m_trialPoint.y = 0;
    }
}
bool CFont2D_glyphBMP::placeExactlyTo(FontBitmapPixelGrid *grid, POINT pt) const {
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
    return true;
}
bool CFont2D_glyphBMP::appendTo(FontBitmapPixelGrid *grid, FinderCache *cache) {
    //найти свободное место, занять его и обновить себе m_view[l, t, w, h]
    POINT pt;
    if (!findEmptySpace(grid, &pt, cache))
        return false; //увы, место закончилось
    auto ret = placeExactlyTo(grid, pt);
    if (ret) {
        m_view.m_top = pt.y / 2048.0f;
        m_view.m_left = pt.x / 2048.0f;
        m_view.m_height = m_iSize.cy / 2048.0f;
        m_view.m_width = m_iSize.cx / 2048.0f;
    }
    return ret;
}
void dumpFontBitmapPixelGrid(const FontBitmapPixelGrid &outDds, FILE *fBMP, int size = FONT_BITMAP_SIZE) {
    for (int irow = 0; irow < size; irow++) {
        for (int icol = 0; icol < size; icol++) {
            fprintf(fBMP, "%c", glbGlyphLegend[outDds[irow][icol]]);
        }
        fprintf(fBMP, "\n");
    }
}
#if 1
typedef struct {			/**** Surface pixel format ****/
    unsigned int dwsize;			//Structure size
    unsigned int dwflags;			//Values which indicate what type of data is in the surface
    unsigned int dwfourCC;			//Four-character codes for specifying compressed or custom formats
    unsigned int dwRGBBitCount;		//Number of bits in an RGB (possibly including alpha) format
    unsigned int dwRBitMask;		//Red (or lumiannce or Y) mask for reading color data
    unsigned int dwGBitMask;		//Green (or U) mask for reading color data
    unsigned int dwBBitMask;		//Blue (or V) mask for reading color data
    unsigned int dwABitMask;		//Alpha mask for reading alpha data
} DDS_PIXELFORMAT;
#define DDPF_FOURCC 0x4									//Flag to show that texture contains compressed RGB data; dwFourCC contains valid data.
#define DXT1_FOURCC (MAKEFOURCC('D', 'X', 'T', '1'))	//DXT1 value for dwfourCC
#define FLAGS 0x81007									//DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE
#define HEADER_SIZE 124									//Constant in all DDS files
#define INFO_SIZE 32									//Constant in all DDS files
#define DDSCAPS_TEXTURE 0x1000							//Required in DDS header
struct DDS_HEADER {			/**** DDS file header ****/
    char magic[4];
    unsigned int dwSize;				//Size of structure
    unsigned int dwFlags;				//Flags to indicate which members contain valid data
    unsigned int dwHeight;				//Surface height (in pixels)
    unsigned int dwWidth;				//Surface width (in pixels)
    unsigned int dwPitchOrLinearSize;	//The pitch or number of bytes per scan line in an uncompressed texture
    unsigned int dwDepth;				//Depth of a volume texture (in pixels), otherwise unused
    unsigned int dwMipMapCount;			//Number of mipmap levels, otherwise unused
    unsigned int dwReserved1[11];		//Unused
    DDS_PIXELFORMAT ddspf;				//The pixel format
    unsigned int dwCaps;				//Specifies the complexity of the surfaces stored
    unsigned int dwCaps2;				//Additional detail about the surfaces stored
    unsigned int dwCaps3;				//Unused
    unsigned int dwCaps4;				//Unused
    unsigned int dwReserved2;			//Unused
    bool Good() {
        if (magic[0] != 'D' || magic[1] != 'D' || magic[2] != 'S' || magic[3] != ' ')
            return false;
        if (dwSize != 124 || ddspf.dwsize != 32)
            return false;
        if (dwHeight != FONT_BITMAP_SIZE || dwWidth != FONT_BITMAP_SIZE)
            return false;
        if (ddspf.dwfourCC != DXT1_FOURCC)
            return false;
        return true;
    }
    uint8_t *decodeRGB(uint16_t color) const {
        uint8_t *rgb = new uint8_t[3];

        rgb[0] = (color >> 11) & 31;
        rgb[1] = (color >> 5) & 63;
        rgb[2] = (color) & 31;

        rgb[0] = (rgb[0] << 3) | (rgb[0] >> 2);
        rgb[1] = (rgb[1] << 2) | (rgb[1] >> 4);
        rgb[2] = (rgb[2] << 3) | (rgb[2] >> 2);

        return rgb;
    }
    uint8_t *GetUncompressedImageData(uint8_t *compressed) const {
        if (compressed == nullptr)
            return nullptr;

        const unsigned int width = dwWidth;
        const unsigned int height = dwHeight;

        //Allocate memory for uncompressed image data, width * height * 3 color bytes per pixel
        uint8_t *bytesBuffer = new uint8_t[width * height * 3];

        unsigned int index = 0;
        //uses inverted order in y so that image is represented the correct way, otherwise it would end up upside down
        for (int invertY = height / 4 - 1; invertY >= 0; --invertY) {
            for (unsigned int x = 0; x < width / 4; ++x) {

                //Read block reference colors
                const uint8_t c0_lo = compressed[index++];
                const uint8_t c0_hi = compressed[index++];
                const uint8_t c1_lo = compressed[index++];
                const uint8_t c1_hi = compressed[index++];

                //Read block pixel codes
                const uint8_t bits_0 = compressed[index++];
                const uint8_t bits_1 = compressed[index++];
                const uint8_t bits_2 = compressed[index++];
                const uint8_t bits_3 = compressed[index++];

                //Form 32 bit instance of block bits
                const uint32_t bits = bits_0 + 256 * (bits_1 + 256 * (bits_2 + 256 * bits_3));

                //Calculate the four colors used
                uint16_t co0 = c0_lo + c0_hi * 256;
                uint16_t co1 = c1_lo + c1_hi * 256;

                //Decode rgb values
                uint8_t *color0Rgb = decodeRGB(co0);
                uint8_t *color1Rgb = decodeRGB(co1);
                uint8_t *color2Rgb = new uint8_t[3];
                uint8_t *color3Rgb = new uint8_t[3];

                //Interpolate mid colors
                if (co0 > co1) {
                    color2Rgb[0] = (2 * color0Rgb[0] + color1Rgb[0]) / 3;
                    color2Rgb[1] = (2 * color0Rgb[1] + color1Rgb[1]) / 3;
                    color2Rgb[2] = (2 * color0Rgb[2] + color1Rgb[2]) / 3;

                    color3Rgb[0] = (color0Rgb[0] + 2 * color1Rgb[0]) / 3;
                    color3Rgb[1] = (color0Rgb[1] + 2 * color1Rgb[1]) / 3;
                    color3Rgb[2] = (color0Rgb[2] + 2 * color1Rgb[2]) / 3;
                } else { //Uses third color as alpha value
                    color2Rgb[0] = (color0Rgb[0] + color1Rgb[0]) / 2;
                    color2Rgb[1] = (color0Rgb[1] + color1Rgb[1]) / 2;
                    color2Rgb[2] = (color0Rgb[2] + color1Rgb[2]) / 2;

                    color3Rgb[0] = 0;
                    color3Rgb[1] = 0;
                    color3Rgb[2] = 0;
                }

                for (int invertYi = 3; invertYi >= 0; --invertYi) {
                    for (int invertXi = 3, xi = 0; invertXi >= 0; --invertXi, ++xi) {
                        int bitPos = 31 - (2 * (invertYi * 4 + invertXi) + 1);
                        int code = ((bits >> bitPos) & 3);
                        uint8_t rgb[3];

                        switch (code) {
                        case 0:
                            rgb[0] = color0Rgb[0];
                            rgb[1] = color0Rgb[1];
                            rgb[2] = color0Rgb[2];
                            break;
                        case 1:
                            rgb[0] = color1Rgb[0];
                            rgb[1] = color1Rgb[1];
                            rgb[2] = color1Rgb[2];
                            break;
                        case 2:
                            rgb[0] = color2Rgb[0];
                            rgb[1] = color2Rgb[1];
                            rgb[2] = color2Rgb[2];
                            break;
                        case 3:
                            rgb[0] = color3Rgb[0];
                            rgb[1] = color3Rgb[1];
                            rgb[2] = color3Rgb[2];
                            break;
                        default:
                            std::cerr << "Error decoding image, texel had code value " << code << std::endl;
                            break;
                        }

                        //Find position pixel position in buffer so that we dont have to iterate
                        unsigned int byteBufferPos = 3 * (width * (4 * invertY + invertYi) + 4 * x + xi);
                        bytesBuffer[byteBufferPos] = rgb[0];		//red
                        bytesBuffer[byteBufferPos + 1] = rgb[1];	//green
                        bytesBuffer[byteBufferPos + 2] = rgb[2];	//blue
                    }
                }
                delete[] color0Rgb;
                delete[] color1Rgb;
                delete[] color2Rgb;
                delete[] color3Rgb;
            }
        }

        //linear rgb buffer
        return bytesBuffer;
    }
    uint16_t compressRGBBytes(const uint8_t *const rgb) const {
        uint8_t colorLo = ((rgb[1] & 28) << 3) | (rgb[2] >> 3); //red 5 bits green 3 bits
        uint8_t colorHi = (rgb[0] & 248) | (rgb[1] >> 5);
        return colorLo + colorHi * 256;
    }
    uint8_t *DXT1Compress(const uint8_t *uncompressedData, int compressedImageSize, int imageSize, int width, int height) {
        //compressed image data to be returned
        uint8_t *imageData = new uint8_t[compressedImageSize];

        //Values used in calculations
        const unsigned int RGBBytes = 3;
        const unsigned int bytesInCompressedBlock = 8;
        const unsigned int pixelsInBlock = 16;
        const unsigned int pixelsInBlockRow = 4;

        //Loop through all blocks
        int numberOfBlocks = imageSize / (pixelsInBlock * RGBBytes);
        int blockX = 0;
        int blockY = 0;
        for (int blockNum = 0; blockNum < numberOfBlocks; ++blockNum, ++blockX) {

            //Keep track of current block
            if (blockNum != 0 && blockNum % (width / pixelsInBlockRow) == 0) {
                blockX = 0;
                ++blockY;
            }

            //Loop through pixels in block to find min and max color values
            uint16_t min16 = 65535; //initialized with max value
            uint16_t max16 = 0;		//initialized with min value
            uint8_t maxRgb[RGBBytes] = { 0, 0, 0 };		//color0
            uint8_t minRgb[RGBBytes] = { 255, 255, 255 };	//color1
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {

                    //Calculate from where to read color bytes
                    unsigned int bytePos = RGBBytes * (blockY * pixelsInBlockRow * width + blockX * pixelsInBlockRow + y * width + x);
                    uint8_t rgb[RGBBytes] = { uncompressedData[bytePos], uncompressedData[bytePos + 1], uncompressedData[bytePos + 2] };
                    uint16_t temp = compressRGBBytes(rgb);

                    //save the value if it is the biggest so far
                    if (temp > max16) {
                        max16 = temp;
                        maxRgb[0] = rgb[0];
                        maxRgb[1] = rgb[1];
                        maxRgb[2] = rgb[2];
                    }
                    //save the value if it is the smallest so far
                    if (temp < min16) {
                        min16 = temp;
                        minRgb[0] = rgb[0];
                        minRgb[1] = rgb[1];
                        minRgb[2] = rgb[2];
                    }
                }
            }

            //Place blocks in reverse order (invert both x and y)
            unsigned int totalYBlocks = height / pixelsInBlockRow; //Row = column in this case
            unsigned int totalXBlocks = width / pixelsInBlockRow;
            unsigned int compressBytePos = bytesInCompressedBlock * (totalYBlocks * totalXBlocks - blockY * totalXBlocks - totalXBlocks + blockX);

            //form the 4 first bytes from reference colors 
            //								green 3 bits			blue 5 bits
            imageData[compressBytePos++] = ((maxRgb[1] & 28) << 3) | (maxRgb[2] >> 3);		//c0_lo
            //								red 5 bits			green 3 bits
            imageData[compressBytePos++] = (maxRgb[0] & 248) | (maxRgb[1] >> 5);			//c0_hi
            imageData[compressBytePos++] = ((minRgb[1] & 28) << 3) | (minRgb[2] >> 3);		//c1_lo
            imageData[compressBytePos++] = ((minRgb[0] & 248) | minRgb[1] >> 5);			//c1_hi

            uint8_t color2[RGBBytes] = { 0,0,0 };
            uint8_t color3[RGBBytes] = { 0,0,0 };
            if (max16 > min16) {
                color2[0] = (2 * maxRgb[0] + minRgb[0]) / 3;
                color2[1] = (2 * maxRgb[1] + minRgb[1]) / 3;
                color2[2] = (2 * maxRgb[2] + minRgb[2]) / 3;

                color3[0] = (maxRgb[0] + minRgb[0] * 2) / 3;
                color3[1] = (maxRgb[1] + minRgb[1] * 2) / 3;
                color3[2] = (maxRgb[2] + minRgb[2] * 2) / 3;
            } else {
                color2[0] = (maxRgb[0] + minRgb[0]) / 2;
                color2[1] = (maxRgb[1] + minRgb[1]) / 2;
                color2[2] = (maxRgb[2] + minRgb[2]) / 2;
            }

            //Loop through pixel colors of block to map colors to reference colors
            for (int y = 3; y >= 0; --y) {
                uint8_t codeByte = 0;
                for (int x = 3; x >= 0; --x) {

                    unsigned int bytePos = RGBBytes * (blockY * pixelsInBlockRow * width + blockX * pixelsInBlockRow + y * width + x);
                    uint8_t rgb[RGBBytes] = { uncompressedData[bytePos], uncompressedData[bytePos + 1], uncompressedData[bytePos + 2] };
                    int code = 0;

                    int distanceCo0 = std::abs((maxRgb[2] + 256 * (maxRgb[1] + 256 * maxRgb[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));
                    int distanceCo2 = std::abs((color2[2] + 256 * (color2[1] + 256 * color2[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));

                    //if color is closer to the second color there is no need to compare color0 again
                    if (distanceCo0 > distanceCo2) {
                        code = 2;
                        int distanceCo3 = std::abs((color3[2] + 256 * (color3[1] + 256 * color3[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));

                        if (distanceCo2 > distanceCo3) {
                            code = 3;
                            int distanceCo1 = std::abs((minRgb[2] + 256 * (minRgb[1] + 256 * minRgb[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));

                            if (distanceCo3 > distanceCo1) {
                                code = 1;
                            }
                        }
                    }

                    //Form byte of 2 bit codes, pixels a to e are in order MSB e -> a LSB
                    codeByte = (codeByte | (uint8_t)((code & 3) << 2 * x));
                }
                imageData[compressBytePos++] = codeByte;
            }
        }
        return imageData;
    }
    void WriteHeaderTo(FILE *fDDS) {
        magic[0] = 'D';
        magic[1] = 'D';
        magic[2] = 'S';
        magic[3] = ' ';
        dwSize = HEADER_SIZE;
        dwFlags = 0x000A1007; //1:DDSD_CAPS req; 2,4:h&w, req; 0x1000 pixelformat req; 0x20000=dwMipMapCount; 0x80000=dwPitchOrLinearSize=LinearSize (no volume flag here)
        dwHeight = FONT_BITMAP_SIZE;
        dwWidth = FONT_BITMAP_SIZE;
        dwPitchOrLinearSize = FONT_BITMAP_SIZE * FONT_BITMAP_SIZE / 2; // 0x00200000
        dwMipMapCount = 12;
        dwDepth = 0; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
        dwCaps = 0x00401008; //DDSCAPS_COMPLEX(8), DDSCAPS_TEXTURE (0x1000) and DDSCAPS_MIPMAP (0x400000)
        ddspf.dwsize = INFO_SIZE;
        ddspf.dwflags = 4; //DDPF_FOURCC
        ddspf.dwfourCC = DXT1_FOURCC;
        fwrite(this, sizeof(DDS_HEADER), 1, fDDS);
    }
    void WriteFromTxtMipmapTo(FILE *fDDS, FontBitmapPixelGrid *grid, int size) {
        int imgSz = 3 * size * size;
        int comprSz = ((size + 3) / 4) * ((size + 3) / 4) * 8;
        if (comprSz < 8) comprSz = 8;
        std::unique_ptr<uint8_t[]> rgbData{new uint8_t[imgSz]};
        for (int y = size - 1; y >= 0; y--) {
            uint8_t *dest = rgbData.get() + (size * 3) * y;
            for (auto x = 0; x < size; x++) {
                switch ((*grid)[-y + size - 1][x]) {
                case FBP_SOLID:
                    *dest++ = 255;
                    *dest++ = 255;
                    *dest++ = 255;
                    break;
                case FBP_MEDIUM:
                    *dest++ = 0x90;
                    *dest++ = 0x90;
                    *dest++ = 0x90;
                    break;
                case FBP_LIGHT:
                    *dest++ = 0x50;
                    *dest++ = 0x50;
                    *dest++ = 0x50;
                    break;
                default:
                    *dest++ = 0;
                    *dest++ = 0;
                    *dest++ = 0;
                    break;
                }
            }
        }
        std::unique_ptr<uint8_t> compr{ DXT1Compress(rgbData.get(), comprSz, imgSz, size, size) };
        fwrite(compr.get(), comprSz, 1, fDDS);
    }
};
struct Dds2Txt { //here bmp in textual form
    std::wstring m_dds{ L"in\\" }, m_bin{ L"in\\" }, m_error;
    Dds2Txt(const wchar_t *filePrefix, int points) {
        m_dds += filePrefix;
        m_dds += L"0.dds";
        FILE *fDDS{ _wfopen(m_dds.c_str(), L"rb") };
        if (!fDDS) {
            m_error = L"can't open dds";
            return;
        }
        DDS_HEADER ddsh;
        if (1 != fread(&ddsh, sizeof(ddsh), 1, fDDS) || !ddsh.Good()) {
            m_error = L"can't read dds header";
            fclose(fDDS);
            return;
        }
        m_bin += filePrefix;
        m_bin += L"_EFIGS_K.bin";
        uint32_t mult = 2048, div = 1;
        for (int mipMap = 1; mipMap < 12; ++mipMap, mult /= 2, div *= 2) {
            std::wstring txtPath = std::to_wstring(points) + L"\\" + std::to_wstring(mult) + L"\\";
            CreateDirectoryW(txtPath.c_str(), nullptr);
            FILE *fBMP{ _wfopen((txtPath + filePrefix + L"0.txt").c_str(), L"wt")}; //text-form bitmap of full font in this mipmap
            auto fullBMP = new FontBitmapPixelGrid;
            //read our dds mipmap, dump it to fBMP and store to fullBMP
            uint32_t mipSize = mult * mult / 2;
            std::unique_ptr<uint8_t[]> ddsData{ new uint8_t[mipSize] };
            if (1 != fread((char *)ddsData.get(), mipSize, 1, fDDS)) {
                m_error = L"can't read dds mipmap";
                fclose(fDDS);
                fclose(fBMP);
                return;
            }
            std::unique_ptr<uint8_t[]> rgbData{ ddsh.GetUncompressedImageData(ddsData.get()) };
            for (int y = mult - 1; y >= 0; y--) {
                uint8_t *ptr = rgbData.get() + (mult * 3) * y;
                for (auto x = 0u; x < mult; x++) {
                    char sym = ' ';
                    auto sym2 = FBP_NOP;
                    //int u24 = *ptr + ptr[1] * 256 + ptr[2] * 65536;
                    if (ptr[2] >= 0xF0) {
                        sym = '\xdb'; //full block (100%)
                        sym2 = FBP_SOLID;
                    } else if (ptr[2] >= 0x90) {
                        sym = '\xb1'; //medium shade (50%)
                        sym2 = FBP_MEDIUM;
                    } else if (ptr[2] >= 0x50) {
                        sym = '\xb0'; //light shade (25%)
                        sym2 = FBP_LIGHT;
                    }
                    putc(sym, fBMP);
                    (*fullBMP)[-y + mult - 1][x] = sym2;
                    ptr += 3;
                }
                putc('\n', fBMP);
            }
            fclose(fBMP);
            FILE *fBIN{ _wfopen(m_bin.c_str(), L"rb") };
            std::wstring bin_txt = txtPath + filePrefix + L"_EFIGS_K.txt";
            FILE *fTXT{ _wfopen(bin_txt.c_str(), L"wt") };
            CFont2D_fileHdrV3 h;
            fread(&h, sizeof(h), 1, fBIN);
            CFont2D_glyphBMP tmp;
            std::map<wchar_t, CFont2D_glyphBMP> map;
            fprintf(fTXT, "points:%u\n", h.m_charsCnt);
            for (int i = 0; i < h.m_charsCnt; i++) {
                fread(&tmp, sizeof(CFont2D_glyph), 1, fBIN);
                if (tmp.m_lid != 0)
                    continue;
                tmp.m_iSize = { LONG(tmp.m_view.m_width * float(mult) + 0.05f), LONG(tmp.m_view.m_height * float(mult) + 0.05f) };
                tmp.m_iPos = { LONG(tmp.m_view.m_left * float(mult) + 0.05f), LONG(tmp.m_view.m_top * float(mult) + 0.05f) };
                fprintf(fTXT, "%u left=%d width=%d top=%d height=%d\n", tmp.m_codePoint, tmp.m_iPos.x, tmp.m_iSize.cx, tmp.m_iPos.y, tmp.m_iSize.cy);
                map[(wchar_t)tmp.m_codePoint] = tmp;
            }
            std::unique_ptr<RealKernItem[]> prki{ new RealKernItem[h.m_realKerns] };
            fprintf(fTXT, "kerns:%u\n", h.m_realKerns);
            if (h.m_realKerns != fread(prki.get(), sizeof(RealKernItem), h.m_realKerns, fBIN)) {
                m_error = L"problem with kerns count";
            } else {
                //remember - no digit kerning!
                for (int k = 0; k < h.m_realKerns; k++) {
                    fprintf(fTXT, "%u>%u corr=%d\n", prki[k].m_prev, prki[k].m_cur, int(prki[k].m_corr) * int(mult) / 2048);
                }
                for (auto &g : map) {
                    std::wstring fileName{ txtPath + std::to_wstring(g.first) };
                    fileName.push_back(L'_');
                    if (g.first > L' ' && !wcschr(L"<>:\"/\\|?*", (wchar_t)g.first))
                        fileName.push_back(g.first);
                    FILE *fAlphabet{ _wfopen((fileName + L".txt").c_str(), L"wt") };
                    if (nullptr == fAlphabet) {
                        m_error = L"open alpha failed";
                        break;
                    } else {
                        if (g.second.m_lid == 0) {
                            dumpGlyphBmp(g.second, fullBMP, fAlphabet);
                        } else {
                            m_error = L"write alpha failed";
                        }
                        fclose(fAlphabet);
                    }
                }
            }
            delete fullBMP;
            fclose(fBIN);
            fclose(fTXT);
            ddsh.dwHeight /= 2;
            ddsh.dwWidth /= 2;
        }
        fclose(fDDS);
    }
};
TEST(SmokeTestFont, DumpDDSnBIN) {
    //instruction:
    //1.1. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoBlack105ptW_EFIGS_K.bin to g:\Fonts\in\
    //1.2. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoMedium54ptW_EFIGS_K.bin to g:\Fonts\in\
    //2.1. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoBlack105ptW0.dds to g:\Fonts\in\
    //2.1. copy zwift-offline\scripts\ZwiftApp\patch\data\Fonts\ZwiftFondoMedium54ptW0.dds to g:\Fonts\in\
    //3. this test converts both dds to txt (4198400 bytes)
    //4. creates g:\Fonts\105 and g:\Fonts\54 directories for letters
    // 4.1 letters dir filled with *.txt files
    // 4.2 bin converted to txt also
    //5. then edit letters and run AutoPlace test (pack and kern)
    SetCurrentDirectoryA("g:\\Fonts\\");
    Dds2Txt d2t[]{ Dds2Txt(L"ZwiftFondoBlack105ptW", 105), Dds2Txt(L"ZwiftFondoMedium54ptW", 54) };
    for (auto &dconv : d2t) {
        EXPECT_EQ(std::wstring(), dconv.m_error);
    }
    ::CopyFileA("in\\ZwiftFondoBlack105ptW_EFIGS_K.bin", "ZwiftFondoBlack105ptW_EFIGS_K.bin", FALSE); //копия на случай, если не будут править в AutoPlace/AutoKern
    ::CopyFileA("in\\ZwiftFondoMedium54ptW_EFIGS_K.bin", "ZwiftFondoMedium54ptW_EFIGS_K.bin", FALSE);
}
#endif
namespace fs = std::filesystem;
TEST(SmokeTestFont, AutoPlace) {
    SetCurrentDirectoryA("g:\\Fonts\\");
    //instruction: 
    // input: letters from g:\Fonts\54\2048\ and g:\Fonts\204\2048\ folders
    // input: g:\Fonts\in *.bin - original font data
    // output g:\Fonts\ZwiftFondoBlack105ptW_EFIGS_K.bin and g:\Fonts\ZwiftFondoMedium54ptW_EFIGS_K.bin with new placements
    // output g:\Fonts\105\2048\ZwiftFondoBlack105ptW0.txt and g:\Fonts\54\2048\ZwiftFondoMedium54ptW0.txt (overwritten whole font txt representation)
    FILE *fBINin[]{ fopen("in\\ZwiftFondoBlack105ptW_EFIGS_K.bin", "rb"), fopen("in\\ZwiftFondoMedium54ptW_EFIGS_K.bin", "rb") }; //input fonts
    FILE *fBINout[]{ fopen("ZwiftFondoBlack105ptW_EFIGS_K.bin", "wb"), fopen("ZwiftFondoMedium54ptW_EFIGS_K.bin", "wb") }; //out fonts
    FILE *fBMP[]{ fopen("105\\2048\\ZwiftFondoBlack105ptW0.txt", "wt"), fopen("ZwiftFondoMedium54ptW0.txt", "wt") }; //resulting dds 2048*2048
    const int p[]{ 105, 54 };
    const int goalKernDistance[] = { 16, 12 };
    for (int task = 0; task < _countof(fBMP); task++) {
        CFont2D_glyphBMP tmp;
        std::map<wchar_t, CFont2D_glyphBMP> map;
        std::vector<CFont2D_glyphBMP> vecKern;
        auto outDds{ std::make_unique<FontBitmapPixelGrid>() };
        fs::path path{ std::to_string(p[task]) };
        FinderCache cache;
        for (const auto &entry : fs::directory_iterator(path / "2048")) {
            if (!std::isdigit(entry.path().filename().c_str()[0]))
                continue;
            EXPECT_TRUE(tmp.initFromLetterTxt(entry.path().generic_wstring().c_str()));
            EXPECT_TRUE(tmp.appendTo(outDds.get(), &cache));
            map[(wchar_t)tmp.m_codePoint] = tmp;
            printf("T%d: %03d/~271           \r", task, (int)map.size());
        }
        dumpFontBitmapPixelGrid(*outDds.get(), fBMP[task]);
        //copy bin with transform
        CFont2D_fileHdrV3 hIn, hOut;
        fread(&hIn, sizeof(hIn), 1, fBINin[task]);
        hOut = hIn;
        hOut.m_charsCnt = 0;
        fwrite(&hOut, sizeof(hOut), 1, fBINout[task]);
        for (int i = 0; i < hIn.m_charsCnt; i++) {
            fread(&tmp, sizeof(CFont2D_glyph), 1, fBINin[task]);
            if (tmp.m_lid == 0) {
                //correct if any
                auto f = map.find(tmp.m_codePoint);
                if (f == map.end())
                    continue; //deleted
                tmp = f->second;
            }
            hOut.m_charsCnt++;
            fwrite(&tmp, sizeof(CFont2D_glyph), 1, fBINout[task]);
            if (tmp.m_lid == 0 && !std::isdigit(tmp.m_codePoint) //цифры не должны прыгать - поэтому у них одинаковая ширина, они выровнены + константный кернинг
                && tmp.m_codePoint != 32  && tmp.m_codePoint != 10  && tmp.m_codePoint != 160  && tmp.m_codePoint != 0x200b ) {
                vecKern.push_back(tmp);
            }
            map.erase(tmp.m_codePoint);
        }
        //write our new glyphs
        for (auto newg : map) {
            hOut.m_charsCnt++;
            fwrite(&newg.second, sizeof(CFont2D_glyph), 1, fBINout[task]);
            if (newg.second.m_lid == 0)
                vecKern.push_back(newg.second);
        }
        //kerns calc
        std::map<std::pair<UChar, UChar>, char> autoKern;
        char kernDumpFmt[260];
        sprintf(kernDumpFmt, "%d\\kern\\%%04x-%%04x.txt", p[task]);
        //warm autokern {
        for (auto g1 : vecKern) {
            for (auto g2 : vecKern) {
                auto key = std::make_pair(g1.m_codePoint, g2.m_codePoint);
                char &corr = autoKern[key];
                corr = 0;
            }
        }
        //warm autokern }
#pragma omp parallel
        {
#pragma omp for
            for (int g1i = 0; g1i < (int)vecKern.size(); g1i++) { //auto g1 : vecKern
                const auto &g1 = vecKern[g1i];
                for (auto g2 : vecKern) {
                    auto key = std::make_pair(g1.m_codePoint, g2.m_codePoint);
                    char &corr = autoKern[key]; //no data race here, warmed already
                    //здесь патч для значений кернинга
                    //..если нужен
                    std::map<char, int> tries;
                    do {
                        auto dist = dumpKernPairBmp(g1, g2, kernDumpFmt, corr, false);
                        tries[corr] = dist;
                        if (dist == 2000) {
                            break;
                        }
                        if (corr == -16)
                            break;
                        auto deltaDist = dist - goalKernDistance[task];
                        if (deltaDist == 0) break;
                        corr = std::clamp(corr - deltaDist / abs(deltaDist), -16, 16);
                        if (tries.contains(corr))
                            break;
                    } while (true);
                    int bestDelta = 2000;
                    if (tries.size() > 1) {
                        for (auto &t : tries) {
                            auto dt = abs(t.second - goalKernDistance[task]);
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
        }
        //kerns write
        std::vector<RealKernItem> autoKernArray;
        for (auto &mi : autoKern)
            if (mi.second)
                autoKernArray.push_back(RealKernItem{ .m_prev = mi.first.first, .m_cur = mi.first.second, .m_corr = mi.second });
        fwrite(autoKernArray.data(), sizeof(RealKernItem), (int)autoKernArray.size(), fBINout[task]);
        fseek(fBINout[task], 0, SEEK_SET);
        hOut.m_realKerns = (int)autoKernArray.size();
        fwrite(&hOut, sizeof(hOut), 1, fBINout[task]);
    }
    for (auto i : fBMP) fclose(i);
    for (auto i : fBINin) fclose(i);
    for (auto i : fBINout) fclose(i);
}
TEST(SmokeTestFont, CorrectDumpedDDS_mipmaps) {
    SetCurrentDirectoryA("g:\\Fonts\\");
    //instruction: do this after [DumpDDSnBIN, kerning and placement algos] to create txt mipmaps
    for (int pointsSize = 54; pointsSize <= 105; pointsSize += 51) {
        auto path{ fs::path(std::to_string(pointsSize)) / "2048" };
        for (const auto &src : fs::directory_iterator(path)) {
            auto fn { src.path().filename().wstring() };
            if (std::isdigit(fn[0])) {
                CFont2D_glyphBMP srcg;
                srcg.initFromLetterTxt(src.path().generic_wstring().c_str());
                for (int destSize = 1024; destSize > 1; destSize /= 2) {
                    CFont2D_glyphBMP dstg{ srcg.asPixelSize(2048 / destSize) };
                    auto destName{ fs::path{ std::to_wstring(pointsSize) } / std::to_wstring(destSize) / fn };
                    FILE *fBMP = _wfopen(destName.generic_wstring().c_str(), L"wt");
                    dstg.dumpTo(fBMP);
                    fclose(fBMP);
                }
            }
        }
    }
}
TEST(SmokeTestFont, DDS_from_TXT) {
    SetCurrentDirectoryA("g:\\Fonts\\");
    //instruction: do this after [CorrectDumpedDDS_mipmaps, manual mipmapped letter txt cleanup] to get DDS mipmapped
    FILE *fBMPin[]{ fopen("105\\2048\\ZwiftFondoBlack105ptW0.txt", "rt"), fopen("54\\2048\\ZwiftFondoMedium54ptW0.txt", "rt") }; //input dds 2048*2048
    FILE *fBINin[]{ fopen("ZwiftFondoBlack105ptW_EFIGS_K.bin", "rb"), fopen("ZwiftFondoMedium54ptW_EFIGS_K.bin", "rb") }; //prepared new fonts
    FILE *fDDSout[]{ fopen("ZwiftFondoBlack105ptW0.dds", "wb"), fopen("ZwiftFondoMedium54ptW0.dds", "wb") };
    const int p[]{ 105, 54 };
    for (int task = 0; task < _countof(fBMPin); task++) {
        CFont2D_glyphBMP tmp;
        std::map<wchar_t, CFont2D_glyphBMP> map;
        CFont2D_fileHdrV3 hIn;
        fread(&hIn, sizeof(hIn), 1, fBINin[task]);
        for (int i = 0; i < hIn.m_charsCnt; i++) {
            fread(&tmp, sizeof(CFont2D_glyph), 1, fBINin[task]);
            if (tmp.m_lid == 0) {
                map[(wchar_t)tmp.m_codePoint] = tmp;
            }
        }
        std::unique_ptr<FontBitmapPixelGrid> outMainDds{ LoadFontBitmap(fBMPin[task]) };
        DDS_HEADER ddsh{};
        ddsh.WriteHeaderTo(fDDSout[task]); //write dds header
        ddsh.WriteFromTxtMipmapTo(fDDSout[task], outMainDds.get(), FONT_BITMAP_SIZE);
        for (int destSize = 1024; destSize > 0; destSize /= 2) {
            std::unique_ptr outDds{ std::make_unique<FontBitmapPixelGrid>() };
            for (auto &g : map) {
                std::wstring letterTxtName{ std::to_wstring(p[task]) + L"\\" + std::to_wstring(destSize) + L"\\" + std::to_wstring(g.first)};
                letterTxtName.push_back(L'_');
                if (g.first > L' ' && !wcschr(L"<>:\"/\\|?*", (wchar_t)g.first))
                    letterTxtName.push_back(g.first);
                letterTxtName += L".txt";
                g.second.initFromLetterTxt(letterTxtName.c_str());
                bool res = g.second.placeExactlyTo(outDds.get(), { LONG(g.second.m_view.m_left * destSize + 0.05f), LONG(g.second.m_view.m_top * destSize + 0.05f) });
                if (destSize > 64)
                    EXPECT_TRUE(res);
            }
            ddsh.WriteFromTxtMipmapTo(fDDSout[task], outDds.get(), destSize);
            if (destSize > 1) {
                char dumpMipMapName[256];
                sprintf(dumpMipMapName, "%d\\%d\\W0.txt", p[task], destSize);
                FILE *fBMP = fopen(dumpMipMapName, "wt");
                dumpFontBitmapPixelGrid(*outDds.get(), fBMP, destSize);
                fclose(fBMP);
            }
        }
    }
    for (auto i : fBMPin) fclose(i);
    for (auto i : fBINin) fclose(i);
    for (auto i : fDDSout) fclose(i);
}
#endif