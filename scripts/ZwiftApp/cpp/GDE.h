#pragma once
struct GDE_Tex { //32 bytes
    char *m_name;
    uint32_t m_assetCrc;
    char gap[16];
};
struct GDE_Usage {
  int8_t *m_pConstantParams, *m_somePtr, *m_pAnumBindings;
  int64_t m_gap8;
};
struct GDE_Animators_0 {
    int field_0;
    uint16_t m_count, field_6;
    int *m_arr1;
    int *m_arr2;
};
struct GDE_Animators { //0x20 bytes
    GDE_Animators_0 *m_field_0;
    int **m_pArr1, **m_pArr2;
    uint8_t *m_field_18;
};
struct GDE_Anim { //0x170 (368) bytes
    char field_0[116];
    int m_field_74;
    GDE_Animators *m_pAnimators;
    GDE_Usage m_pUsage[4];
    bool m_hasAnimators;
    char field_101[15];
    uint8_t m_texIdx[6];
    char field_116[58];
    void *m_field_150;
    int m_texGlid[6];
};
struct GDE_RuntimeItem { //0x98 (152) bytes
    char gap[152];
};
struct VEC3B { 
    int8_t m_data[3]; 
    VEC3B &operator=(const VEC3 &src) { m_data[0] = 127.0 * src.m_data[0]; m_data[1] = 127.0 * src.m_data[1]; m_data[2] = 127.0 * src.m_data[2]; return *this; }
};
enum GdeStat { GS_LEAN = 4 };
struct GDE_MeshItemData0 { //36 bytes
    VEC3 m_point;
    uint32_t m_color;
    char field_10[11];
    char m_field_1B;
    char field_1C;
    char field_1D;
    char field_1E;
    char m_field_1F;
    char field_20;
    char field_21;
    char field_22;
    char m_field_23;
};
struct GDE_MeshItemData1 { //44 bytes
    VEC3 m_point;
    uint32_t m_color;
    char field_10[19];
    char m_field_23;
    char field_24;
    char field_25;
    char field_26;
    char m_field_27;
    char field_28;
    char field_29;
    char field_2A;
    char m_field_2B;
};
struct GDE_MeshItemData2 { //40 bytes
    VEC3 m_point;
    uint32_t m_color1;
    uint32_t m_color2;
    char field_14[11];
    char m_field_1F;
    char field_20;
    char field_21;
    char field_22;
    char m_field_23;
    char field_24;
    char field_25;
    char field_26;
    char m_field_27;
};
struct GDE_MeshItemData3 { //48 bytes
    VEC3 m_point;
    uint32_t m_color1;
    uint32_t m_color2;
    char field_14[19];
    char m_field_27;
    char field_28;
    char field_29;
    char field_2A;
    char m_field_2B;
    char field_2C;
    char field_2D;
    char field_2E;
    char m_field_2F;
};
struct GDE_MeshItemData4 { //48 bytes
    VEC3 m_point;
    uint32_t m_color1;
    uint32_t m_color2;
    char m_raw[16];
    VEC3B m_pointN;
    char m_field_27;
    VEC3B m_point1;
    char m_field_2B;
    VEC3B m_point2;
    char m_field_2F;
};
struct GDE_MeshItemData4_file { //72 bytes
    VEC3 m_point;
    char m_raw[16];
    VEC3 m_pointN;
    VEC3 m_point1;
    VEC3 m_point2;
    int m_color1;
    int m_color2;
};
struct GDE_MeshSubItem { //24 bytes
    void *m_pIndices;
    int m_ibHandle;
    int m_numIndices;
    int m_vertexHandle;
    char field_14;
    char field_15;
    char field_16;
    char field_17;
};
struct GDE_MeshItem { //80 bytes
    int m_vbHandle;
    int m_field_4;
    GDE_MeshSubItem *m_subItemBegPtr;
    void *field_10;
    void *field_18;
    void *field_20;
    GDE_MeshSubItem *m_subItemEndPtr;
    void *m_pVerts;
    void *m_pUnused;
    uint8_t gap40[4];
    int m_numVerts;
    int m_itemKind;
    int m_instancesCount;
};
struct GDE_Mesh { //0x1F8 (504) bytes
    uint32_t m_version, m_meshItemsCnt;
    GDE_MeshItem m_meshItems[6];
    VEC4 m_bounds;
};
struct GDE_Runtime { //0x28 bytes
    uint32_t m_sign1, field_4, m_sign2, field_C;
    GDE_Runtime *m_next;
    uint64_t m_runtimeItemsCnt;
    GDE_RuntimeItem *m_runtimeItems;
};
const uint32_t GDE_SKIN_MESH = '\x10\0\0\x01', GDE_MESH = '\x10\0\0\x01';
struct GDE_Header_360 { //0x70 bytes
    uint32_t m_ver, m_meshSign;
    int m_animCnt;
    int m_texturesCnt;
    int m_field_10;
    int field_14;
    GDE_Anim *m_anims;
    GDE_Tex *m_textures;
    uint8_t *m_field_28; // = malloc(16 * file->m_field_10); //sizeof smth
    GDE_Runtime *m_runtime;
    GDE_Mesh *m_mesh;
    char field_40[48];
};
struct GDE_360_TIE {};
struct GDE_MaterialUsage {};
enum InstanceResourceState { IRS_UNLOADED = 0, IRS_NEED_LOAD = 1, IRS_LOAD_FAILED = 2, IRS_3 = 3, IRS_4 = 4, IRS_INITIAL = 5 };
struct InstanceResource { //72 bytes
    GDE_Header_360 *m_gdeFile;
    VEC4 m_bounds;
    bool m_manyInstances, m_isSkin, m_field_1A_1, field_1B;
    int field_1C;
    char *m_gdeName;
    int m_gdeNameCRC;
    char field_2C;
    char field_2D;
    char field_2E;
    char field_2F;
    int m_creationTime;
    int m_frameCnt;
    InstanceResourceState m_state;
    AssetCategory m_assetCategory;
    int m_vramUsed;
    char field_44;
    char field_45;
    char field_46;
    char field_47;
};

inline InstanceResource g_ENG_InstanceResources[6500];
inline int g_ScreenMeshHandle, g_TrainerMeshHandle, g_HandCycleTrainerMeshHandle, g_TreadmillMeshHandle, g_PaperMeshHandle, g_ENG_nResources;
inline int g_hGDE_TRI_VERT, g_hTRI_VERT_COLOR1_UV1, g_hTRI_VERT_COLOR1_UV2, g_hTRI_VERT_COLOR2_UV1, g_hTRI_VERT_COLOR2_UV2, g_hDEFORM_SKIN_VERT_GL, g_hGDE_RIGID_SKIN_VERT;
inline int64_t g_VRAMBytes_GDERAM;

void GDEMESH_Initialize();
int GDEMESH_Internal_FindLoadedMesh(const char *name, bool manyInstances);
int GDEMESH_GetFreeMeshResourceHandle();
GDE_Header_360 *GDEMESH_GetMesh(int handle);
VEC4 *GDEMESH_GetMeshBounds(VEC4 *ret, int handle);
void GDE_FreeRuntime(GDE_Header_360 *h);
void GDE_NormalizeVector(VEC3 *dest, VEC3 *src);
