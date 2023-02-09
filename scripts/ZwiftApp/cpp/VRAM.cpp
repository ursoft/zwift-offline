#include "ZwiftApp.h"
void VRAM_CreateRenderTarget(RenderTarget *rt, uint32_t, uint32_t, bool, bool, bool) {
    //TODO
}
void VRAM_EndRenderTo(uint64_t) {
    //TODO
}
void VRAM_Initialize(bool aHasPickingBuffer) {
    g_HasPickingBuffer = aHasPickingBuffer;
    CONSOLE_AddCommand("res", CMD_ChangeRes);
    CONSOLE_AddCommand("sres", CMD_ChangeShadowRes);
    VRAM_CreateAllRenderTargets();
}
