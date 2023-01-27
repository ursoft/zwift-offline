#pragma once

struct RenderTarget { //176 byts
};

void VRAM_CreateAllRenderTargets();
void VRAM_ReleaseRenderTargetVRAM(RenderTarget *ptr);
void VRAM_Initialize(bool aHasPickingBuffer);
void GFX_AddPerformanceFlags(uint64_t f);