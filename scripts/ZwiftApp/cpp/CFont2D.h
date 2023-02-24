#pragma once
enum FONT_STYLE { FS_0, FS_1, FS_2 };
enum LANGUAGE_IDS { LID_03 = 3 };
class CFont2D {
public:
    float m_scale, m_kerning;

    CFont2D();
    void Load(FONT_STYLE s);
    void SetScaleAndKerning(float scale, float kerning);

    void BreakString(float, const char *, float);
    void CacheMallocCharacters(uint32_t, bool);
    void CharWidthW(uint16_t);
    void CreateFontHelper(void *);
    void CreateFontHelperFilename(const char *);
    void Ellipsize(float, const char *, float);
    void Ellipsize(float, const uint16_t *, float);
    void EndCachingAndRender(bool);
    void FitCharsToWidth(const char *, int);
    void FitCharsToWidthW(const uint16_t *, int);
    void FitParagraphInBox(float, float, const uint16_t *);
    void FitWordsToWidth(const char *, int, float);
    void FitWordsToWidthW(const char *, int, float);
    void FitWordsToWidthW(const uint16_t *, int, float);
    void GetBaseOffset(float, float);
    void GetBaseline(float);
    void GetBoundedScale(float, float, const char *);
    void GetBoundedScaleW(float, float, const uint16_t *);
    void GetCenteredOffset(float, float);
    //void GetGlyphUVView(uint32_t, tViewport<float> &, uint32_t *);
    //void GetGlyphView(uint32_t, tViewport<int> &);
    void GetHeadOffset(float, float);
    void GetHeadToBase(float);
    void GetHeadToBottom(float);
    void GetHeadline(float);
    void GetHeight();
    void GetParagraphBoxSize(const char *, float, float, float);
    void GetParagraphIndexByPosition(float, float, float, float, const uint16_t *, int, float, float, float *, float *);
    void GetParagraphLineCount(float, const char *, float, float, bool);
    void GetParagraphLineCountW(float, const char *, float, float, bool);
    void GetParagraphLineCountW(float, const uint16_t *, float, float, bool);
    void GetParagraphPositionByIndex(int, float, float, const char *, int, float, float);
    void GetParagraphPositionByIndex(int, float, float, const uint16_t *, int, float, float);
    void GetScaleAndKerning(float *, float *);
    void GetTexture(uint32_t);
    void GetTopToBase(float);
    void ImbueCommas(uint32_t);
    void IsFontTextureLoaded(uint16_t);
    void LoadDirect(const char *);
    void LoadFont(const char *);
    void LoadFontFromWad(const char *);
    void LoadFontWFromWadV3(const char *);
    void LoadFontWV3(const char *);
    void LoadLanguageTextures(int);
    void PopHeadBase();
    void PushHeadBase(float, float);
    void RenderAllCachedContent(bool);
    void RenderBorderedString(float, float, const char *, uint32_t, int, float, bool, bool, float, uint32_t);
    void RenderBorderedStringMultisized(float, float, int, const char **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderBorderedWString(float, float, const char *, uint32_t, int, float, bool, bool, uint32_t);
    void RenderBorderedWString(float, float, const uint16_t *, uint32_t, int, float, bool, bool, uint32_t);
    void RenderBorderedWStringMultisized(float, float, int, const char **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderBorderedWStringMultisized(float, float, int, const uint16_t **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderParagraph(float, float, float, float, const char *, uint32_t, int, float, bool, float, float);
    void RenderParagraphW(float, float, float, float, const char *, uint32_t, int, float, bool, float, float);
    void RenderParagraphW(float, float, float, float, const uint16_t *, uint32_t, int, float, bool, float, float);
    void RenderString(float, float, const char *, uint32_t, int, float, bool, bool, float, bool);
    void RenderStringBounded(float, float, float, float, const char *, uint32_t, int, bool, bool);
    void RenderStringMultisized(float, float, int, const char **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderStringRT(float, float, const char *, uint32_t, int, float, bool, bool);
    void RenderWString(float, float, const char *, uint32_t, int, float, bool, bool);
    void RenderWString(float, float, const uint16_t *, uint32_t, int, float, bool, bool, bool);
    void RenderWStringBounded(float, float, float, float, const uint16_t *, uint32_t, int, bool, bool);
    void RenderWStringMultisized(float, float, int, const char **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderWStringMultisized(float, float, int, const uint16_t **, uint32_t *, float *, int, int, bool, int, bool);
    void RenderWStringRT(float, float, const char *, uint32_t, int, float, bool, bool);
    void RenderWStringRT(float, float, const uint16_t *, uint32_t, int, float, bool, bool);
    void SetHeadAndBaseLines(float, float);
    void SetLanguageKerningScalar(LANGUAGE_IDS, float);
    void SetSpaceScale(float);
    void SetVerticalOffset(float);
    void StartCaching(uint32_t);
    void StringWidth(const char *);
    void StringWidthW(const char *);
    void StringWidthW(const uint16_t *);
    void StringWidthW(const uint16_t *, uint32_t);
    ~CFont2D();
};
inline CFont2D *g_ChatFontGW, *g_ChatFontLW, g_GiantFontW, g_LargeFontW, g_debugFont, g_SmallFont, g_MediumFont;
