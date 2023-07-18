#pragma once
struct InstancedObjects {
    bool m_field_20 = true;
    InstancedObjects();
    ~InstancedObjects();
};
inline InstancedObjects g_BikeInstancedObjects;
//inlined void INSTANCING_AddPreculledInstance(InstancedObjects *, int, MATRIX44 *, VEC4, int);
void INSTANCING_EndFrame(bool);
//inlined void INSTANCING_GetDistanceToGridCellBounds(int, VEC3*, VEC3*, VEC3);
struct BillBoardedSpriteInfo;
void INSTANCING_AddInstance(InstancedObjects *, int, MATRIX44, VEC4);
uint32_t INSTANCING_AddMesh(InstancedObjects *, const char *, uint64_t, float, BillBoardedSpriteInfo *, const VEC4 &);
void INSTANCING_ClearInstances();
void INSTANCING_CreateDitherTex();
//inlined void INSTANCING_GetGridIDForPosition(VEC3);
void INSTANCING_OptimizeGrids(InstancedObjects *);
//inlined void INSTANCING_ReleaseMutex();
void INSTANCING_RenderAll(InstancedObjects *, GFX_RenderPass, int);
bool INSTANCING_TakeMutex();
void INSTANCING_UnloadAll();