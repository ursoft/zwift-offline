#pragma once
enum VRAM_FLAGS { VF_BUF16 = 8, VF_CUBEMAP = 32, VF_MIPMAP = 64 };

inline bool g_HasPickingBuffer;
const float g_UI_AspectRatio = 1.7777778f; //16x9
inline uint64_t g_VRAMBytes_RenderTargets;
inline int g_nMultiSamples;
inline RenderTarget *g_pCurrentRenderTarget;

void VRAM_CreateRenderTarget(RenderTarget *rt, uint32_t, uint32_t, bool, bool, bool);
void VRAM_CreateRenderTarget(RenderTarget *ptr, int w, int h, int flags_ex);
void VRAM_EndRenderTo(uint8_t flags);
void VRAM_Initialize(bool aHasPickingBuffer);
float VRAM_GetUIAspectRatio();
void GFX_Clear(char a1);
inline RenderTarget *VRAM_GetCurrentRT() { return g_pCurrentRenderTarget; }
