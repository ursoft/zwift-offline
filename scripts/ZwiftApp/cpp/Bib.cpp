#include "ZwiftApp.h" //READY for testing
void Bib::InitOnce(/*int, int, const CFont2D &*/) {
    zassert(g_DefaultNormalMapNoGloss != INVALID_TEXTURE_HANDLE);
    zassert(g_WhiteHandle != INVALID_TEXTURE_HANDLE);
    if (g_GNSceneSystem.m_tier < 1)
        Bib::s_BibShader = GFX_CreateShaderFromFile("bib", -1);
    else
        Bib::s_BibShader = GFX_CreateShader({ "GNBib" });
    auto hNumberTag = GFX_CreateTextureFromTGAFile("Humans/Male/Textures/NumberTag.tga", -1, true);
    if (hNumberTag == -1)
        hNumberTag = g_WhiteHandle;
    s_DefaultBibInfo.m_texNumberTag = hNumberTag;
    s_DefaultBibInfo.m_pFont = &g_GiantFontW;
    s_DefaultBibInfo.m_texNormal = g_DefaultNormalMapNoGloss;
}
void Bib::Set(uint32_t raceBibNumber, BibInfo *info, ZSPORT sport) {
    if (Bib::s_BibShader != -1 && raceBibNumber < 100'000 && (!info || info->m_pFont)) {
        memset(this, 0, sizeof(Bib));
        m_raceBibNumber = raceBibNumber;
        m_pInfo = info;
        if (info) {
            char buf[8];
            auto nchars = sprintf_s(buf, "%d", raceBibNumber);
            if (nchars >= 0) {
                for (int c = 0; c < nchars; c++)
                    info->m_pFont->GetGlyphUVView(buf[c], m_viewPorts + c, &m_kernIdx);
                tViewport<int> viewPortI;
                int digMaxW = 0;
                for (int i = '0'; i <= '9'; ++i) {
                    if (info->m_pFont->GetGlyphView(i, &viewPortI) && digMaxW < viewPortI.m_width)
                        digMaxW = viewPortI.m_width;
                }
                VEC4 v28{
                    info->m_w1 / info->m_cv4i[sport].m_data[2],
                    info->m_w2 / info->m_cv4i[sport].m_data[3],
                    -info->m_cv4i[sport].m_data[0] / info->m_cv4i[sport].m_data[2],
                    -info->m_cv4i[sport].m_data[1] / info->m_cv4i[sport].m_data[3] };
                int maxW = 5 * digMaxW, totalW = 0;
                float v33 = 1.0f / maxW;
                if (nchars > 0) {
                    for (int j = 0; j < nchars; ++j) {
                        if (info->m_pFont->GetGlyphView(buf[j], &viewPortI))
                            totalW += viewPortI.m_width;
                    }
                    int startX = (maxW - totalW) / 2;
                    for (int j = 0; j < nchars; ++j)
                        if (info->m_pFont->GetGlyphView(buf[j], &viewPortI)) {
                            auto v35 = 1.0f / (viewPortI.m_width * v33);
                            m_positions[j].m_data[0] = v35 * v28.m_data[0];
                            m_positions[j].m_data[1] = v28.m_data[1];
                            m_positions[j].m_data[2] = float(v28.m_data[2] - startX * v33) * v35;
                            m_positions[j].m_data[3] = v28.m_data[3];
                            startX += viewPortI.m_width;
                        }
                }
                auto t0 = m_pInfo->m_pFont->GetTexture(0);
                if (m_pInfo->m_texNumberTag == -1 || t0 == -1) {
                    m_kHeight = m_kWidth = 1.0f;
                } else {
                    m_kWidth = (float)GFX_GetTextureWidth(m_pInfo->m_texNumberTag) / (float)GFX_GetTextureWidth(t0);
                    m_kHeight = (float)GFX_GetTextureHeight(m_pInfo->m_texNumberTag) / (float)GFX_GetTextureHeight(t0);
                }
                m_vec4.m_data[3] = 1.0f;
            }
        }
    }
}
