#pragma once
enum FONT_STYLE { FS_GIANTW };
class CFont2D {
public:
    float m_scale, m_kerning;

    void Load(FONT_STYLE s);
    void SetScaleAndKerning(float scale, float kerning);
};
inline CFont2D *g_ChatFontGW, *g_ChatFontLW, g_GiantFontW, g_LargeFontW;
