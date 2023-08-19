#pragma once
struct WorldProp { //92 bytes
    MATRIX44 m_mx;
    Sphere m_bounds;
    int m_packedCoords;
    uint16_t m_cellId;
    char m_tileID;
    char gap[5];
};
struct PropType { //0x18f0 bytes
    std::vector<float> m_fltConts[64];
    std::vector<Sphere> m_sphereConts[64];
    std::vector<WorldProp> m_worldProps[64];
    VEC3 m_mins[64], m_maxs[64];
    std::vector<float> m_ovfFltCont;
    std::vector<Sphere> m_ovfSphereCont;
    std::vector<WorldProp> m_ovfWpCont;
    //TODO std::vector<???> field_1878;
    uint64_t m_flags;
    int m_gdeHandle, m_cnt;
    void clear();
};
struct InstancedObjects { //0x28 bytes
    std::list<PropType> m_plist;
    std::map<int, PropType *> m_pmap;
    bool m_detached = true;
    InstancedObjects();
    ~InstancedObjects();
};
inline InstancedObjects g_BikeInstancedObjects, g_WorldProps;
//inlined void INSTANCING_AddPreculledInstance(InstancedObjects *, int, MATRIX44 *, VEC4, int);
void INSTANCING_EndFrame(bool);
//inlined void INSTANCING_GetDistanceToGridCellBounds(int, VEC3*, VEC3*, VEC3);
struct BillBoardedSpriteInfo;
void INSTANCING_AddInstance(InstancedObjects *iob, int a2, const MATRIX44 &a3, const VEC4 &a4);
uint32_t INSTANCING_AddMesh(InstancedObjects *, const char *, uint64_t, float, BillBoardedSpriteInfo *, const VEC4 &);
void INSTANCING_ClearInstances();
void INSTANCING_CreateDitherTex();
//inlined void INSTANCING_GetGridIDForPosition(VEC3);
void INSTANCING_OptimizeGrids(InstancedObjects *);
//inlined void INSTANCING_ReleaseMutex();
void INSTANCING_RenderAll(InstancedObjects *, GFX_RenderPass, int);
void INSTANCING_TakeMutex();
void INSTANCING_UnloadAll();