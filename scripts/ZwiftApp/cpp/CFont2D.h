#pragma once //READY for testing
enum FONT_STYLE { FS_SMALL, FS_SANSERIF, FS_FONDO_MED, FS_FONDO_BLACK, FS_CNT };
enum LANGUAGE_IDS : char { LID_LAT = 0, LID_JAPAN = 1, LID_KOREAN = 2, LID_CHINESE = 3, LID_CNT = 4 };
struct CFont2D_struc24 {
    std::vector<char> m_cont;
};
struct CFont2D_glyph { // 20 bytes
    uint16_t         m_codePoint;
    char             m_kernIdx;
    uint8_t          m_cnt;
    tViewport<float> m_view;
};
struct CFont2D_fileHdrV1 { //0x9C bytes
    uint16_t m_version;
    uint8_t  field_2;
    uint8_t  m_chars;
    uint8_t  m_from;
    uint8_t  m_to;
    char     m_family[16], field16, field17;
    uint16_t m_width;
    uint16_t m_height;
    char     field_1C[128];
};
struct CFont2D_fileHdrV3 { //0xF0 bytes
    uint16_t m_version;
    char     field_2;
    char     field_3;
    int      m_tex[10];
    float    m_kern[LID_CNT];
    char     field_3C[26];
    uint16_t m_usedGlyphs;
    char     field_58[152];
};
struct CFont2D_info {
    LANGUAGE_IDS        m_langId;
    CFont2D_fileHdrV1   m_fileHdrV1;
    CFont2D_fileHdrV3   m_fileHdrV3;
    tViewport<uint16_t> m_v1gls[256];
    uint16_t            m_glyphIndexes[65536];
};
struct CFont2D_cache { //192 bytes
    char gap[192];
};
enum RenderFlags { RF_CX_ISCENTER = 1, RF_CY_ISCENTER = 2, RF_CX_ISRIGHT = 4, RF_CY_ISBOTTOM = 8 };
struct CFont2D {
    std::string      m_fileName;
    CFont2D_struc24  m_struc24x4[4];
    float            m_kern[LID_CNT];
    CFont2D_info     m_info;
    CFont2D_glyph    *m_glyphs;
    void             *m_RGBAv1;
    int m_lineHeight;
    LANGUAGE_IDS     m_texSuffix;
    char             field_20A35;
    char             field_20A36;
    char             field_20A37;
    int m_tex;
    float            m_field_20A3C;
    float            m_scale;
    float            m_kerning;
    float            m_baseLine;
    float            m_headLine;
    float            m_verticalOffset;
    float            m_spaceScale;
    char             m_loadedV1;
    bool             m_loadedV3;
    char             field_20A5A;
    char             field_20A5B;
    char             field_20A5C;
    char             field_20A5D;
    char             field_20A5E;
    char             field_20A5F;
    CFont2D_cache    *m_cache;
    uint32_t         m_cacheCnt, m_cacheCntUsed;
    int m_curCache;
    char             field_20A74;
    char             field_20A75;
    char             field_20A76;
    char             field_20A77;
    std::deque<VEC2> m_hbStack;
    CFont2D();
    void Load(FONT_STYLE s);
    void SetScaleAndKerning(float scale, float kerning);
    void LoadDirectEast(const char *name1, const char *name2);
    bool LoadDirect(const char *name);
    bool PopHeadBase();
    void PushHeadBase(float head, float base);
    float GetBaseline(float mult);
    float GetHeadline(float mult);
    float GetHeadToBase(float mult);
    float StringWidth(const char *text);
    float StringWidthW_c(const char *text);
    float StringWidthW_u(const UChar *uText);
    float StringWidthW_ulen(const UChar *uText, uint32_t len);
    float GetBoundedScaleW(float w, float h, const UChar *uText);
    float GetHeight();
    CFont2D_cache *CacheMallocCharacters(uint32_t, bool);
    void StartCaching(uint32_t);
    int EndCachingAndRender(bool uiProjection);
    void RenderAllCachedContent(bool uiProjection);
    bool LoadFont(const char *name);
    bool LoadFontFromWad(const char *name);
    void LoadFontV1(const uint8_t *data);
    void LoadLanguageTextures(LOC_LANGS l);
    void RenderWString_c(float cx, float cy, const char *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc);
    void RenderWString_u(float cx, float cy, const UChar *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc, bool uiProjection);
    int GetParagraphLineCount(float, const char *, float, float, bool);
    int GetParagraphLineCountW(float, const UChar *, float, float, bool);
    int RenderParagraph(float, float, float, float, const char *, uint32_t, uint32_t, float, bool, float, float, float cropTop = 0.0f);
    int RenderParagraphW(float, float, float, float, const UChar *, uint32_t, uint32_t, float, bool, float, float, float cropTop = 0.0f);
    int FitWordsToWidthW(const UChar *, int, float);
    int FitCharsToWidthW(const UChar *, int);
    ~CFont2D();
    float GetHeadOffset(float, float);
    int GetTexture(uint32_t);
    void GetGlyphUVView(uint32_t, tViewport<float> *, uint32_t *);
    bool GetGlyphView(uint32_t ch, tViewport<int> *);
    void SetHeadAndBaseLines(float, float);
    void SetLanguageKerningScalar(LANGUAGE_IDS, float);
    float GetTopToBase(float);
    float GetBoundedScale(float totalW, float totalH, const char *txt);
    //in GUI_ToolTip used only, which is absent here void GetParagraphBoxSize(const char *text, float, float, float);
    void RenderWStringRT(float cx, float cy, const UChar *text, uint32_t color, int flags, float scale, bool needShadow, bool forceDrawMalloc) {
        RenderWString_u(cx, cy, text, color, flags, scale, needShadow, forceDrawMalloc, false);
    }
    float GetBaseOffset(float mult, float h) {
        return h - mult * (GetHeight() - m_baseLine * m_field_20A3C * m_scale);
    }
    float GetCenteredOffset(float mult, float h) {
        return (h - GetHeadToBase(mult)) * 0.5f - m_headLine * m_field_20A3C * m_scale * mult;
    }
    float CharWidthW(UChar ch);
    void RenderWStringBounded(float cx, float cy, float w, float h, UChar *txt, uint32_t color, int flags, bool needShadow, bool forceDrawMalloc) {
        RenderWString_u(cx, cy, txt, color, flags, GetBoundedScaleW(w, h, txt), needShadow, forceDrawMalloc, true);
    }
    void RenderBorderedString(float cx, float cy, const char *txt, uint32_t color, int flags, float scale, bool needShadow, bool fdm, uint32_t clr2);
    const char *Ellipsize(float scale, const char *txt, float maxW);
    const UChar *EllipsizeW(float scale, const UChar *txt, float maxW);
    void RenderStringRT(float cx, float cy, const char *txt, uint32_t color, int flags, float scale, bool needShadow);
    float FitParagraphInBox(float w, float h, const UChar *txt);
    float RenderWStringMultisized(float, float, int, const UChar **lines, uint32_t *, float *, RenderFlags hflg, RenderFlags vflg, bool needShadow, int, bool forceDrawMalloc);
    float RenderStringMultisized(float cx, float cy, int nlines, const char **lines, uint32_t *clrs, float *scales, RenderFlags hflg, RenderFlags vflg, bool needShadow, int cntLines, bool forceDrawMalloc);
    void RenderBorderedWString(float cx, float cy, const UChar *text, uint32_t clr, int flags, float scale, bool needShadow, bool forceDrawMalloc, uint32_t clr2);
    int GetParagraphIndexByPosition(float cx, float cy, float w, float h, const UChar *str, RenderFlags flags, float scaleW, float scaleH, float *ox, float *oy);
    VEC2 GetParagraphPositionByIndexW(int index, float w, UChar *str, float mult, float scale);
    std::string ImbueCommas(uint32_t val);
    float RenderBorderedWStringMultisized(float cx, float cy, int nlines, const char **lines, uint32_t *clrs, float *scales, RenderFlags hflg, RenderFlags vflg, bool needShadow, int cntLines, bool fdm);
    void RenderString(float, float, const char *, uint32_t, uint32_t, float, bool, bool, bool);
    void RenderWStringBounded(float, float, float, float, const UChar *, uint32_t, int, bool, bool);
    //not used void BreakString(float, const char *, float);
    //inlined void CreateFontHelper(void *);
    //inlined void CreateFontHelperFilename(const char *);
    //inlined void FitCharsToWidth(const char *, int);
    //inlined void FitWordsToWidth(const char *, int, float);
    //inlined void GetHeadToBottom(float);
    //Android only void GetScaleAndKerning(float *, float *);
    //inlined void IsFontTextureLoaded(uint16_t);
    //inlined void SetSpaceScale(float);
    //inlined void SetVerticalOffset(float);
};
struct BufSafeToUTF8 { UChar m_buf[512]; int m_usageCounter = 0; };
UChar *SafeToUTF8(const char *ansi, BufSafeToUTF8 *buf);

inline CFont2D    *g_ChatFontGW, *g_ChatFontLW, g_GiantFontW, g_LargeFontW, g_debugFont, g_SmallFont, g_MediumFont;
inline const bool g_bSupportFontCaching = true;
inline int        g_fontWShader = -1, g_fontShader = -1;