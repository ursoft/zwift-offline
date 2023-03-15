#include "ZwiftApp.h"
void VRAM_CreateRenderTarget(RenderTarget *rt, uint32_t w, uint32_t h, bool a4, bool try16, bool a6) { //VRAM_CreateRenderTarget_0
    VRAM_CreateRenderTarget(rt, w, h, (1 * a4) | (16 * a6) | (VF_BUF16 * try16));
}
bool g_bFAILEDF16BUFFER;
void VRAM_CreateTexs(RenderTarget *rt, int a2, GLint clr, GLsizei w, GLsizei h, GLenum format, GLenum type, int cnt, int flags_ex, int bpp) {
    for (int i = 0; i < cnt; ++i) {
        int height = h;
        int width = w;
        for (int j = 0; ; ++j) {
            glTexImage2D((GLenum)(i + a2), j, clr, width, height, 0, format, type, 0);
            g_VRAMBytes_RenderTargets += bpp * width * height;
            if (width == 1 && height == 1)
                break;
            if ((flags_ex & VF_MIPMAP) == 0)
                break;
            auto v17 = width >> 1;
            width = 1;
            if (v17 > 1)
                width = v17;
            auto v18 = height >> 1;
            height = 1;
            if (v18 > 1)
                height = v18;
            ++rt->m_mmTexsCnt;
        }
    }
}
const GLenum g_MRT[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
void VRAM_CreateRenderTarget(RenderTarget *rt, int w, int h, int flags_ex) {
    VRAM_ReleaseRenderTargetVRAM(rt);
    rt->m_width1 = w;
    rt->m_width2 = w;
    rt->m_dw_width = w;
    rt->m_height1 = h;
    rt->m_height2 = h;
    rt->m_dw_height = h;
    rt->m_field_14 = 0;
    rt->m_mmTexsCnt = 1;
    rt->m_field_18 = 0;
    rt->m_field_90 = 0;
    rt->m_field_92 = 0;
    rt->m_field_93 = 0;
    rt->m_field_96 = 4;
    glGenFramebuffersEXT(1, &rt->m_fb_30);
    glBindFramebufferEXT(GL_FRAMEBUFFER, rt->m_fb_30);
    if (flags_ex & 1) {
        g_VRAMBytes_RenderTargets += 4 * w * h;
        glGenRenderbuffersEXT(1, &rt->m_rb_50);
        glBindRenderbufferEXT(GL_RENDERBUFFER, rt->m_rb_50);
        glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->m_rb_50);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rt->m_rb_50);
        rt->m_field_9B = 8;
    } else if (flags_ex & 2) {
        g_VRAMBytes_RenderTargets += 4 * w * h;
        glGenTextures(1, &rt->m_tex_60);
        glBindTexture(GL_TEXTURE_2D, rt->m_tex_60);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rt->m_tex_60, 0);
        rt->m_field_9B = 8;
    } else {
        rt->m_rb_50 = 0;
    }
    rt->m_isCubeMap = (flags_ex & VF_CUBEMAP) != 0;
    auto v9 = rt->m_isCubeMap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;
    auto v8 = rt->m_isCubeMap ? 6 : 1;
    auto v11 = rt->m_isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glGenTextures(1, &rt->m_tex_38);
    glBindTexture(v11, rt->m_tex_38);
    glTexParameteri(v11, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    auto v12 = (flags_ex & VF_MIPMAP) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(v11, GL_TEXTURE_MIN_FILTER, v12);
    if (rt == &g_RT_EntityPickingBuffer || rt == &g_RT_TerrainPickingBuffer) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, w, h, 0, GL_RGBA, GL_FLOAT, 0);
        rt->m_field_96 = 0x8;
        rt->m_field_97 = 0x5;
    } else {
        glGetError();
        if (flags_ex & VF_BUF16) {
            VRAM_CreateTexs(rt, v9, GL_RGBA16F, w, h, GL_RGBA, GL_FLOAT, v8, flags_ex, 8);
            if (glGetError()) {
                g_bFAILEDF16BUFFER = true; //was used in analytics
            } else {
                rt->m_field_92 = 1;
                rt->m_field_96 = 0x8;
                rt->m_field_97 = 0x5;
            }
        }
        if (!rt->m_field_92) {
            VRAM_CreateTexs(rt, v9, GL_RGBA8, w, h, GL_RGBA, GL_UNSIGNED_BYTE, v8, flags_ex, 4);
            rt->m_field_97 = 4;
        }
    }
    g_VRAMBytes_RenderTargets += w * h * rt->m_field_96;
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, v9, rt->m_tex_38, 0);
    if (rt == &g_RT_BackBuffer || rt == &g_RT_EntityPickingBuffer || rt == &g_RT_TerrainPickingBuffer) {
        if (GFX_GetPerformanceGroup()) {
            glGenTextures(1, &rt->m_tex_40);
            glBindTexture(GL_TEXTURE_2D, rt->m_tex_40);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            auto v15 = 4 * w * h;
            g_VRAMBytes_RenderTargets += v15;
            rt->m_field_98 = 4;
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, rt->m_tex_40, 0);
            glGenTextures(1, &rt->m_tex_48);
            glBindTexture(GL_TEXTURE_2D, rt->m_tex_48);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            g_VRAMBytes_RenderTargets += v15;
            rt->m_field_99 = 2;
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, rt->m_tex_48, 0);
            glDrawBuffers(3, g_MRT);
            if (glDisableIndexedEXT) {
                glDisableIndexedEXT(GL_BLEND, 1);
                glDisableIndexedEXT(GL_BLEND, 2);
            }
            rt->m_field_94 = 1;
        }
    } else if (flags_ex & 4) {
        glGenTextures(1, &rt->m_tex_48);
        glBindTexture(GL_TEXTURE_2D, rt->m_tex_48);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        g_VRAMBytes_RenderTargets += 4 * w * h;
        rt->m_field_99 = 2;
    }
    auto v17 = glCheckFramebufferStatusEXT(GL_DRAW_FRAMEBUFFER);
    if (v17 != GL_FRAMEBUFFER_COMPLETE) {
        Log("[GFX]: framebuffer status %x", v17);
    } else if (g_nMultiSamples && GLEW_EXT_framebuffer_multisample) {
        if (rt == &g_RT_BackBuffer) {
            GLint params = 0;
            glGetIntegerv(GL_MAX_SAMPLES, &params);
            glGenFramebuffersEXT(1, &rt->m_field_7C);
            glBindFramebufferEXT(GL_FRAMEBUFFER, rt->m_field_7C);
            glGenRenderbuffersEXT(1, &rt->m_field_80);
            glBindRenderbufferEXT(GL_RENDERBUFFER, rt->m_field_80);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, params, GL_DEPTH_STENCIL, w, h);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->m_field_80);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rt->m_field_80);
            rt->m_field_97 = 8;
            glGenRenderbuffersEXT(1, &rt->m_field_84);
            glBindRenderbufferEXT(GL_RENDERBUFFER, rt->m_field_84);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, params, GL_RGBA8, w, h);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rt->m_field_84);
            rt->m_field_97 = 4;
            glGenRenderbuffersEXT(1, &rt->m_field_88);
            glBindRenderbufferEXT(GL_RENDERBUFFER, rt->m_field_88);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, params, GL_RGBA8, w, h);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rt->m_field_88);
            rt->m_field_98 = 4;
            glGenRenderbuffersEXT(1, &rt->m_field_8C);
            glBindRenderbufferEXT(GL_RENDERBUFFER, rt->m_field_8C);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, params, GL_LUMINANCE_FLOAT32_APPLE, w, h);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, rt->m_field_8C);
            rt->m_field_97 = 2;
            glDrawBuffers(3, g_MRT);
            v17 = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
            if (v17 != GL_FRAMEBUFFER_COMPLETE)
                Log("[GFX]: framebuffer status %x", v17);
        }
    } else {
        rt->m_field_7C = 0;
    }
}
void GFX_Clear(char a1) {
    GLuint v2 = ((a1 & 0x3C) ? 0x4000 : 0) | 0x100;
    if ((a1 & 1) == 0)
        v2 = (a1 & 0x3C) ? 0x4000 : 0;
    GLuint v3 = v2 | 0x400;
    if ((a1 & 2) == 0)
        v3 = v2;
    glClear(v3);
}
void VRAM_Initialize(bool aHasPickingBuffer) {
    g_HasPickingBuffer = aHasPickingBuffer;
    CONSOLE_AddCommand("res", CMD_ChangeRes);
    CONSOLE_AddCommand("sres", CMD_ChangeShadowRes);
    VRAM_CreateAllRenderTargets();
}
float VRAM_GetUIAspectRatio() {
    return g_UI_AspectRatio;
}
void VRAM_EndRenderTo(uint8_t flags) {
    g_RT_BackBufferVRAM.m_dw_width = 1280;
    g_pCurrentRenderTarget = &g_RT_BackBufferVRAM;
    g_RT_BackBufferVRAM.m_fb_30 = 0;
    g_RT_BackBufferVRAM.m_dw_height = (int)(1280.0f / g_UI_AspectRatio);
    glViewport(0, 0, g_RT_BackBufferVRAM.m_dw_width, g_RT_BackBufferVRAM.m_dw_height);
    glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
    if (flags & 0x3F) {
        if (flags & 2) {
            glStencilMask(0xFFu);
            glClearStencil(0);
        }
        GFX_Clear(flags);
    }
}