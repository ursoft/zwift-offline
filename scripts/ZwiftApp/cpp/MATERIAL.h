#pragma once
inline int g_DefaultNormalMap = 1, g_LockedMaterial = -1;
inline float g_MaterialTime;

void MATERIAL_Activate(Material_360 *, GDE_MaterialUsage *, void *);
void MATERIAL_ApplyAnimators(Material_360 *, GDE_MaterialUsage *);
void MATERIAL_GetDefaultNormalMap();
void MATERIAL_GetFlipbookTimeOffset();
void MATERIAL_Init();
void MATERIAL_Reset();
void MATERIAL_SetFlipbookTimeOffset(float);
void MATERIAL_SetGlobalLighting(const VEC4 &, const VEC4 &);
void MATERIAL_SetUVOffsetOverride(bool, float, float, bool);
void MATERIAL_Update(float);