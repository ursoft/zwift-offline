#include "ZwiftApp.h"

bool g_HasPickingBuffer;
RenderTarget gRT_BackBuffer;

uint64_t g_GFX_PerformanceFlags;
enum ZwiftPerfFlags { ZPF_DISABLE_AUTO_BRIGHT = 0x4000000 };

int64_t GFX_GetPerformanceFlags() {
	return g_GFX_PerformanceFlags;
}
void GFX_AddPerformanceFlags(uint64_t f) {
	g_GFX_PerformanceFlags |= f;
}
void VRAM_CreateAllRenderTargets() {
}
void VRAM_ReleaseRenderTargetVRAM(RenderTarget *ptr) {}

void VRAM_Initialize(bool aHasPickingBuffer) {
	g_HasPickingBuffer = aHasPickingBuffer;
	CONSOLE_AddCommand("res", CMD_ChangeRes);
	CONSOLE_AddCommand("sres", CMD_ChangeShadowRes);
	VRAM_CreateAllRenderTargets();
}
