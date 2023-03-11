#include "ZwiftApp.h"
void VRAMAPI_PostRenderTarget() {
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_RT_BackBuffer.m_rb_50);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_RT_BackBuffer.m_rb_50);
}
void VRAM_CreateDepthRenderTarget(RenderTarget *rt, GLsizei w, GLsizei h) {
    rt->m_width2 = w;
    rt->m_dw_width = w;
    rt->m_height2 = h;
    rt->m_dw_height = h;
    if (rt->m_fb_30)
        glDeleteFramebuffersEXT(1, &rt->m_fb_30);
    if (rt->m_tex_60)
        glDeleteTextures(1, &rt->m_tex_60);
    glGenFramebuffersEXT(1, &rt->m_fb_30);
    glBindFramebufferEXT(GL_FRAMEBUFFER, rt->m_fb_30);
    glGenTextures(1, &rt->m_tex_60);
    glBindTexture(GL_TEXTURE_2D, rt->m_tex_60);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 33071.0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 33071.0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    rt->m_field_9B = 8;
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rt->m_tex_60, 0);
    glDrawBuffer(GL_FALSE);
    glReadBuffer(GL_FALSE);
    auto v8 = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
    if (v8 == GL_FRAMEBUFFER_COMPLETE)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDepthMask(1);
        glClearDepth(1.0);
        glClear(0x100u);
    } else {
        Log("[GFX]: framebuffer status %x", v8);
    }
}
void VRAM_CreateAllRenderTargets() {
    const int BACKBUFFER_WIDTH = 0x500, BACKBUFFER_HEIGHT = 0x2D0, SHADOWMAP_WIDTH = 0x800, SHADOWMAP_HEIGHT = 0x800, LUMINANCE_WIDTH = 0xF0, LUMINANCE_HEIGHT = 0x87, ENVCUBEMAP_SIZE = 0x80;
    auto pf = (uint64_t)GFX_GetPerformanceFlags();
    int buf16 = (~(uint8_t)(pf >> 24)) & 8;
    bool b16 = (pf & 0x8000000) == 0;
    VRAM_CreateRenderTarget(&g_RT_BackBuffer, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, buf16 | 0x12);
    g_RT_BackBuffer.m_field_90 = 1;
    VRAM_CreateRenderTarget(&g_RT_BackBuffer_Final, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, 0, b16, 1);
    VRAMAPI_PostRenderTarget();
    VRAM_CreateRenderTarget(&g_RT_BackBuffer_Temp, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, 0, b16, 1);
    VRAMAPI_PostRenderTarget();
    if (GFX_GetTier() >= 1 && GFX_GetPerformanceGroup() >= GPG_HIGH)
        VRAM_CreateRenderTarget(&g_RT_BackBuffer_Prev, BACKBUFFER_WIDTH >> 1, BACKBUFFER_HEIGHT >> 1, buf16 | 0x44);
    if (GFX_GetTier() >= 1)
        VRAM_CreateRenderTarget(&g_RT_CubeMap, ENVCUBEMAP_SIZE, ENVCUBEMAP_SIZE, buf16 | 0x21);
    if (g_HasPickingBuffer) {
        if (!g_height || !g_width)
            glfwGetWindowSize(g_mainWindow, &g_width, &g_height);
        VRAM_CreateRenderTarget(&g_RT_EntityPickingBuffer, g_width, g_height, true, false, false);
        VRAM_CreateRenderTarget(&g_RT_TerrainPickingBuffer, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, true, false, false);
    }
    int v14 = 2; //2: 0 & 1, 4: 2 & 3, 8: 4 & 5, 16: 6 & 7, 32: 8 & 9
    for (auto &v13 : g_RT_BackBufferDownsamples) {
        v13.m_field_90 = 1;
        v13.m_name = "BackBufferDownsamples";
        VRAM_CreateRenderTarget(&v13, g_RT_BackBuffer.m_dw_width / v14, g_RT_BackBuffer.m_dw_height / v14, false, b16, true);
        v14 <<= 1;
    }
    VRAM_CreateRenderTarget(&g_RT_Luminance, LUMINANCE_WIDTH, LUMINANCE_HEIGHT, false, false, false);
    const char *names[] = { "ShadowMap[0]", "ShadowMap[1]", "ShadowMap[2]", "ShadowMap[3]" };
    for (int i = 0; i < g_nShadowMaps; i++) {
        VRAM_CreateDepthRenderTarget(g_RT_ShadowMaps + i, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
        g_RT_ShadowMaps[i].m_name = names[i];
    }
    VRAM_CreateDepthRenderTarget(&g_RT_SpotlightShadowMap, 1024, 1024);
    VRAM_CreateDepthRenderTarget(&g_RT_EmptyShadowMap, 64, 64);
    VRAM_CreateRenderTarget(&g_RT_ReflectionMap, 1024, 1024, true, false, true);
}
void VRAM_ReleaseRenderTargetVRAM(RenderTarget *rt) {
    if (rt->m_dw_width && rt->m_dw_height) {
        if (rt->m_rb_50) {
            glDeleteRenderbuffersEXT(1, &rt->m_rb_50);
            g_VRAMBytes_RenderTargets -= 4 * rt->m_dw_width * rt->m_dw_height;
        }
        if (rt->m_tex_38) {
            glDeleteTextures(1, &rt->m_tex_38);
            g_VRAMBytes_RenderTargets -= rt->m_dw_width * rt->m_dw_height * rt->m_field_96;
        }
        if (rt->m_tex_40) {
            glDeleteTextures(1, &rt->m_tex_40);
            g_VRAMBytes_RenderTargets -= 4 * rt->m_dw_width * rt->m_dw_height;
        }
        if (rt->m_tex_48) {
            glDeleteTextures(1, &rt->m_tex_48);
            g_VRAMBytes_RenderTargets -= 4 * rt->m_dw_width * rt->m_dw_height;
        }
        if (rt->m_field_7C)
            glDeleteFramebuffersEXT(1, &rt->m_field_7C);
        if (rt->m_field_80)
            glDeleteRenderbuffersEXT(1i64, &rt->m_field_80);
        if (rt->m_field_84)
            glDeleteRenderbuffersEXT(1i64, &rt->m_field_84);
        if (rt->m_fb_30)
            glDeleteFramebuffersEXT(1, &rt->m_fb_30);
        if (rt->m_field_34) {
            glDeleteBuffers(1, &rt->m_field_34);
            g_VRAMBytes_RenderTargets -= rt->m_dw_width * rt->m_dw_height * rt->m_field_96;
        }
        rt->m_width2 = rt->m_height2 = 0;
        rt->m_dw_width = rt->m_dw_height = 0;
        rt->m_fb_30 = 0;
        rt->m_field_34 = 0;
        rt->m_rb_50 = 0;
        rt->m_tex_38 = 0;
        rt->m_field_7C = 0;
        rt->m_field_80 = 0;
        rt->m_field_84 = 0;
    }
}

