#pragma once
enum FONT_STYLE { FS_SMALL, FS_SANSERIF, FS_FONDO_MED, FS_FONDO_BLACK, FS_CNT };
enum LANGUAGE_IDS : char { LID_LAT = 0, LID_JAPAN = 1, LID_KOREAN = 2, LID_CHINESE = 3, LID_CNT = 4 };
struct CFont2D_struc24 {
    std::vector<char> m_cont;
};
struct CFont2D_glyph { // 20 bytes
    uint16_t m_codePoint;
    char m_kernIdx;
    uint8_t m_cnt;
    float m_field_4, m_field_8, m_width, m_height;
};
struct CFont2D_fileHdrV1 { //0x9C bytes
    uint16_t m_version;
    uint8_t field_2;
    uint8_t m_reserve;
    uint8_t m_from;
    uint8_t m_to;
    char m_family[16], field16, field17;
    uint16_t m_width;
    uint16_t m_height;
    char field_1C[128];
};
struct CFont2D_fileHdrV3 { //0xF0 bytes
    uint16_t m_version;
    char field_2;
    char field_3;
    int m_tex1;
    int m_tex2;
    char field_C[32];
    float m_kern[LID_CNT];
    char field_3C[26];
    uint16_t m_usedGlyphs;
    char field_58[152];
};
struct CFont2D_v1_8b { //8 bytes
    char field_0;
    char field_1;
    char field_2;
    char field_3;
    uint16_t m_field_4, m_field_6;
};
struct CFont2D_info {
    LANGUAGE_IDS m_langId;
    CFont2D_fileHdrV1 m_fileHdrV1;
    CFont2D_fileHdrV3 m_fileHdrV3;
    CFont2D_v1_8b m_field_190[256];
    uint16_t m_glyphIndexes[65536];
};
struct CFont2D_cache { //192 bytes
    char gap[192];
};
enum RenderFlags { RF_CX_ISCENTER = 1, RF_CY_ISCENTER = 2, RF_CX_ISRIGHT = 4, RF_CY_ISTOP = 8 };
class CFont2D {
public:
    std::string m_fileName;
    CFont2D_struc24 m_struc24x4[4];
    float m_kern[LID_CNT];
    CFont2D_info m_info;
    CFont2D_glyph *m_glyphs;
    void *m_RGBAv1;
    float m_lineHeight;
    LANGUAGE_IDS m_texSuffix;
    char field_20A35;
    char field_20A36;
    char field_20A37;
    int m_tex;
    float m_field_20A3C;
    float m_scale;
    float m_kerning;
    float m_baseLine;
    float m_headLine;
    char m_field_20A50;
    char field_20A51;
    char field_20A52;
    char field_20A53;
    float m_field_20A54;
    char m_loadedV1;
    bool m_loadedV3;
    char field_20A5A;
    char field_20A5B;
    char field_20A5C;
    char field_20A5D;
    char field_20A5E;
    char field_20A5F;
    CFont2D_cache *m_cache;
    uint32_t m_cacheCnt, m_cacheCntUsed;
    int m_curCache;
    char field_20A74;
    char field_20A75;
    char field_20A76;
    char field_20A77;
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
    float StringWidthW(const UChar *uText);
    float StringWidthW(const UChar *uText, uint32_t len);
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
    void RenderWString(float cx, float cy, const char *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc);
    void RenderWString(float cx, float cy, const UChar *text, uint32_t color, uint32_t flags, float scale, bool needShadow, bool forceDrawMalloc, bool uiProjection);
    int GetParagraphLineCount(float, const char *, float, float, bool);
    int GetParagraphLineCountW(float, const UChar *, float, float, bool);
    int RenderParagraph(float, float, float, float, const char *, uint32_t, int, float, bool, float, float);
    int RenderParagraphW(float, float, float, float, const UChar *, uint32_t, int, float, bool, float, float);
    ~CFont2D();

    void GetParagraphBoxSize(const char *text, float, float, float);

    void BreakString(float, const char *, float);
    void CharWidthW(uint16_t);
    void CreateFontHelper(void *);
    void CreateFontHelperFilename(const char *);
    void Ellipsize(float, const char *, float);
    void Ellipsize(float, const UChar *, float);
    void FitCharsToWidth(const char *, int);
    void FitCharsToWidthW(const UChar *, int);
    void FitParagraphInBox(float, float, const UChar *);
    void FitWordsToWidth(const char *, int, float);
    void FitWordsToWidthW(const UChar *, int, float);
    void GetBaseOffset(float, float);
    void GetBoundedScale(float, float, const char *);
    void GetCenteredOffset(float, float);
    //void GetGlyphUVView(uint32_t, tViewport<float> &, uint32_t *);
    //void GetGlyphView(uint32_t, tViewport<int> &);
    void GetHeadOffset(float, float);
    void GetHeadToBottom(float);
    void GetParagraphIndexByPosition(float, float, float, float, const UChar *, int, float, float, float *, float *);
    void GetParagraphPositionByIndex(int, float, float, const char *, int, float, float);
    void GetParagraphPositionByIndexW(int, float, float, const UChar *, int, float, float);
    void GetScaleAndKerning(float *, float *);
    void GetTexture(uint32_t);
    void GetTopToBase(float);
    void ImbueCommas(uint32_t);
    void IsFontTextureLoaded(uint16_t);
    void RenderBorderedString(float, float, const char *, uint32_t, int, float, bool, bool, float, uint32_t);
    void RenderBorderedStringMultisized(float, float, int, const char **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderBorderedString(float, float, const char *, uint32_t, int, float, bool, bool, uint32_t);
    void RenderBorderedWString(float, float, const UChar *, uint32_t, int, float, bool, bool, uint32_t);
    void RenderBorderedWStringMultisized(float, float, int, const UChar **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderString(float, float, const char *, uint32_t, int, float, bool, bool, float, bool);
    void RenderStringBounded(float, float, float, float, const char *, uint32_t, int, bool, bool);
    void RenderStringMultisized(float, float, int, const char **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderStringRT(float, float, const char *, uint32_t, int, float, bool, bool);
    void RenderWStringBounded(float, float, float, float, const UChar *, uint32_t, int, bool, bool);
    void RenderWStringMultisized(float, float, int, const UChar **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderWStringRT(float, float, const UChar *, uint32_t, int, float, bool, bool);
    void SetHeadAndBaseLines(float, float);
    void SetLanguageKerningScalar(LANGUAGE_IDS, float);
    void SetSpaceScale(float);
    void SetVerticalOffset(float);
};
struct BufSafeToUTF8 { UChar m_buf[512]; int m_usageCounter = 0; };
UChar *SafeToUTF8(const char *ansi, BufSafeToUTF8  *buf);

inline CFont2D *g_ChatFontGW, *g_ChatFontLW, g_GiantFontW, g_LargeFontW, g_debugFont, g_SmallFont, g_MediumFont;
inline const bool g_bSupportFontCaching = true;
inline int g_fontWShader = -1, g_fontShader = -1;