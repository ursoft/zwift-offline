#pragma once
struct InstancedObjects {};
struct BillBoardedSpriteInfo;
void INSTANCING_AddInstance(InstancedObjects *, int, MATRIX44, VEC4);
void INSTANCING_AddMesh(InstancedObjects *, const char *, uint64_t, float, BillBoardedSpriteInfo *, VEC4);
void INSTANCING_AddPreculledInstance(InstancedObjects *, int, MATRIX44 *, VEC4, int);
void INSTANCING_ClearInstances(InstancedObjects *);
void INSTANCING_CreateDitherTex();
void INSTANCING_EndFrame(bool);
void INSTANCING_GetDistanceToGridCellBounds(int, VEC3*, VEC3*, VEC3);
void INSTANCING_GetGridIDForPosition(VEC3);
void INSTANCING_OptimizeGrids(InstancedObjects *);
void INSTANCING_ReleaseMutex();
void INSTANCING_RenderAll(InstancedObjects *, GFX_RenderPass, int);
void INSTANCING_TakeMutex();
void INSTANCING_UnloadAll();