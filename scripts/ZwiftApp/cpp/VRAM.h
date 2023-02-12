#pragma once
void VRAM_CreateRenderTarget(RenderTarget *rt, uint32_t, uint32_t, bool, bool, bool);
void VRAM_EndRenderTo(uint64_t);
void VRAM_Initialize(bool aHasPickingBuffer);
float VRAM_GetUIAspectRatio();
inline bool g_HasPickingBuffer;
const float g_UI_AspectRatio = 1.7777778f; //16x9
