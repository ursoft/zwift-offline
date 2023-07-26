#pragma once
enum VRAM_FLAGS { VF_BUF16 = 8, VF_CUBEMAP = 32, VF_MIPMAP = 64 };

inline bool g_HasPickingBuffer;
const float g_UI_AspectRatio = 1.777777777f; //16x9
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
void VRAM_SetViewport(int, int, int, int);
void VRAM_SetScreenAspectRatio(float);
void VRAM_Resolve(RenderTarget *);
void VRAM_RenderToStoreFlags(RenderTarget *, uint64_t);
//TODO void VRAM_RenderTo(RenderTarget *, ulong long, VIEW_MODE, bool, RENDER_BUFFER, uint32_t, VRAM_CubeMapFace);
void VRAM_RenderFromDepth(RenderTarget *, int, const char *, bool);
//void VRAM_RenderFrom(RenderTarget *, RENDER_BUFFER, int, char const *, uint32_t);
void VRAM_ReleaseRenderTargetVRAM(RenderTarget *);
void VRAM_ReadbackPBO(RenderTarget *, uint32_t, uint32_t);
//void VRAM_ReadPixels(RENDER_BUFFER, uint32_t, uint32_t, uint32_t, uint32_t, GFX_PixelFormat, void *, uint32_t);
void VRAM_IsWideScreen(void);
void VRAM_GetViewport(int *, int *, int *, int *);
void VRAM_GetRealAspectRatio();
//void VRAM_CreateRenderTargetEx(RenderTarget *, uint32_t, uint32_t, GFX_PixelFormat, uint32_t, bool, bool);
void VRAM_CreatePBO(RenderTarget *);
void VRAM_CreateDepthRenderTarget(RenderTarget *, uint32_t, uint32_t);
void VRAM_CreateAllRenderTargets();
void VRAM_CopyPixelData(RenderTarget *, void *, uint32_t, uint32_t, uint32_t);