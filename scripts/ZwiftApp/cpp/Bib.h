#pragma once //READY for testing
struct BibInfo {
    void *m_field_0;
    int m_texNumberTag;
    void *m_field_10;
    int m_texNormal;
    CFont2D *m_pFont;
    uint16_t m_w1 = 0x0100, m_w2 = 0x0100;
    VEC4 m_cv4i[2] = { {8, 0x96, 0xF0, 0x55 }, {4, 0x14, 0x91, 0x58} };
};
struct Bib {
    tViewport<float> m_viewPorts[5] = {};
    VEC4 m_positions[5]{}, m_vec4{};
    BibInfo *m_pInfo = nullptr;
    uint32_t m_raceBibNumber = 0, m_kernIdx = 0;
    float m_kHeight = 0.0f, m_kWidth = 0.0f;
    //inlined void CalcScaleAndBias(VEC4 *, const VEC2 &, const VEC2 &);
    //inlined int GetBibShader();
    static BibInfo *GetDefaultInfo() { return s_DefaultBibInfo.m_pFont ? &s_DefaultBibInfo : nullptr; }
    inline static int s_BibShader = -1;
    inline static BibInfo s_DefaultBibInfo;
    static void InitOnce(/*int, int, const CFont2D &*/);
    void Set(uint32_t, BibInfo *, ZSPORT);
};
