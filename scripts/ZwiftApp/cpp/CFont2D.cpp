#include "ZwiftApp.h"
bool CFont2D::LoadFont(const char *name) {
    //TODO
    return true;
}
bool CFont2D::LoadFontFromWad(const char *name) {
    //TODO
    return true;
}
bool CFont2D::LoadFontWFromWadV3(const char *name) {
    //TODO
    return true;
}
bool CFont2D::LoadFontWV3(const char *name) {
    //TODO
    return true;
}
bool CFont2D::LoadDirect(const char *name) { return LoadFontFromWad(name) || LoadFont(name); }
void CFont2D::LoadDirectEast(const char *name1, const char *name2) {
    LoadDirect(name1);
    if (!m_field_20A59)
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
            m_field_20A54 = 1.98;
            if (!m_field_20A59)
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
            m_field_20A54 = 1.5;
            if (!m_field_20A59)
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
    m_field_20A54 = 1.0;
    m_glyphs = nullptr;
    m_allocPtr = nullptr;
    m_field_20A3C = 1.0;
    m_scale = 1.0;
    m_kerning = 1.0;
    m_cache = nullptr;
    m_cacheCnt = 0;
    m_cacheCntUsed = 0;
    m_field_20A58 = 0; //QUEST: WORD 8-9?
    m_field_20A59 = 0;
    m_info.m_someCnt = LID_CNT; //QUEST: probably not
    m_field_20A34 = 0;
    m_kern[0] = 1.0;
    m_kern[1] = 1.08935;
    m_kern[2] = 1.0;
    m_kern[3] = 1.08435;
    m_headLine = 22.0;
    m_baseLine = 10.0;
}
CFont2D::~CFont2D() {
    free(m_allocPtr);
    m_allocPtr = nullptr;
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
        auto v4 = GFX_DrawMalloc(sizeof(CFont2D_cache) * cnt, LID_CNT /* ??? or not */);
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
#if 0 //TODO
        CurrentShaderHandle = GFX_GetCurrentShaderHandle();
        if (m_field_20A59) {
            GFX_SetShader(fontWShader);
            GFX_ActivateTexture(*(_DWORD *)&field_94[160], 0, 0i64, 1u);
            v5 = g_WhiteHandle;
            if (*(_DWORD *)&field_94[164] != -1)
                v5 = *(_DWORD *)&field_94[164];
            GFX_ActivateTexture(v5, 2u, 0i64, 1u);
            GFX_SetTextureFilter(2u, 2u);
            sub_7FF713E60D80(2, -0.5);
            GFX_SetTextureFilter(0, 2u);
            sub_7FF713E60D80(0, -0.5);
        } else {
            GFX_SetShader(dword_7FF7147C67D0);
            GFX_ActivateTexture(*(_DWORD *)&field_20A38, 0xFFFFFFFF, 0i64, 1u);
        }
        GFX_SetAlphaBlendEnable(1u);
        GFX_SetBlendFunc(0, 4u, 5u);
        GFX_MatrixMode(GMT_2);
        GFX_PushMatrix();
        GFX_LoadIdentity();
        CurrentRT = VRAM_GetCurrentRT();
        if (uiProjection)
            GFX_SetupUIProjection();
        else
            GFX_Ortho(0.0, (float)*(int *)(CurrentRT + 8), (float)*(int *)(CurrentRT + 12), 0.0, -1.0, 1.0);
        GFX_MatrixMode(GMT_1);
        GFX_PushMatrix();
        GFX_LoadIdentity();
        GFX_MatrixMode(GMT_0);
        GFX_PushMatrix();
        GFX_LoadIdentity();
        GFX_UpdateMatrices(0);
        GFX_DrawPrimitive(3, (__int64)m_cache->field_0, 6 * m_cacheCntUsed);
        sub_7FF713E60D80(2, 0.0);
        GFX_MatrixMode(GMT_2);
        GFX_PopMatrix();
        GFX_MatrixMode(GMT_1);
        GFX_PopMatrix();
        GFX_MatrixMode(GMT_0);
        GFX_PopMatrix();
        sub_7FF713E60D80(0, 0.0);
        GFX_SetShader(CurrentShaderHandle);
#endif
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
    m_hbStack.emplace_back(VEC2{ mult * m_headLine, mult * m_baseLine });
    m_headLine = head;
    m_baseLine = base;
}
float CFont2D::GetBaseline(float mult) { return m_baseLine * m_field_20A3C * m_scale * mult; }
float CFont2D::GetHeadline(float mult) { return m_headLine * m_field_20A3C * m_scale * mult; }
float CFont2D::StringWidthW(const char *text) {
    BufSafeToUTF8 buf;
    return StringWidthW(SafeToUTF8(text, &buf));
}
float CFont2D::StringWidthW(const UChar *uText) { return CFont2D::StringWidthW(uText, u_strlen(uText)); }
float CFont2D::StringWidthW(const UChar *uText, uint32_t textLen) {
    //static_assert(sizeof(CFont2D) == 0x20AA0);
    static_assert(sizeof(CFont2D_info) == 0x20990);
    static_assert(sizeof(CFont2D_glyph) == 20);
    
    if (!m_field_20A59 || !uText || !*uText || !textLen)
        return 0.0f;
    float ret = 0.0f;
    const UChar *uEnd = uText + textLen;
    do {
        auto gidx = m_info.m_glyphIndexes[*uText];
        if (gidx != 0xFFFF) {
            float mult = (*uText == 32) ? this->m_field_20A54 : 1.0f;
            auto kidx = m_glyphs[gidx].m_kernIdx;
            ret += m_kern[kidx] * m_kerning * m_info.m_kern[kidx] * m_glyphs[gidx].m_width * mult;
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
    auto ret = m_field_20A30 * m_field_20A3C * m_scale;
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
