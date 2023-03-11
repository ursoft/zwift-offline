#include "ZwiftApp.h"
void MATERIAL_Activate(Material_360 *, GDE_MaterialUsage *, void *) {
    //TODO
}
void MATERIAL_ApplyAnimators(Material_360 *, GDE_MaterialUsage *) {
    //TODO
}
void MATERIAL_GetDefaultNormalMap() {
    //TODO
}
void MATERIAL_GetFlipbookTimeOffset() {
    //TODO
}
void MATERIAL_Init() {
    g_DefaultNormalMap = GFX_CreateTextureFromTGAFile("defaultNormalMap.tga", -1, true);
}
void MATERIAL_Reset() {
    //TODO
}
void MATERIAL_SetFlipbookTimeOffset(float) {
    //TODO
}
void MATERIAL_SetGlobalLighting(const VEC4 &, const VEC4 &) {
    //TODO
}
void MATERIAL_SetUVOffsetOverride(bool, float, float, bool) {
    //TODO
}
void MATERIAL_Update(float) {
    //TODO
}