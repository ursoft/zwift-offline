#pragma once
struct RenderTarget { //0xB0 (176) bytes
    int m_dw_width, m_dw_height;
};
inline RenderTarget g_RTPreviewWindow, gRT_BackBuffer, gRT_ReflectionMap;
void VRAM_CreateAllRenderTargets();
void VRAM_ReleaseRenderTargetVRAM(RenderTarget *ptr);
void GFX_AddPerformanceFlags(uint64_t f);
struct ArtConfig {
    static void LoadArtConfig() { /*TODO*/ }
    static void ReloadIfChanged() { /*TODO*/ }
};