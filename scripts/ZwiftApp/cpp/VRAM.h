#pragma once
void VRAM_CreateRenderTarget(RenderTarget *rt, uint32_t, uint32_t, bool, bool, bool);
void VRAM_EndRenderTo(uint64_t);
void VRAM_Initialize(bool aHasPickingBuffer);

inline bool g_HasPickingBuffer;
