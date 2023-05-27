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
struct GDE_Material { //0x170 (368) bytes
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
    VEC3B &operator=(const VEC3 &src) { m_data[0] = int8_t(127.0 * src.m_data[0]); m_data[1] = int8_t(127.0 * src.m_data[1]); m_data[2] = int8_t(127.0 * src.m_data[2]); return *this; }
};
enum GdeStat { GS_VB_CLUST = 2, GS_VERT_BUF = 4, GS_SKIN = 6 };
struct GDE_MeshItemData0 { //36 bytes
    VEC3 m_point;
    uint32_t m_color, field_10, field_14;
    char field_18, field_19, field_1A, m_field_1B;
    char field_1C;
    char field_1D;
    char field_1E;
    char m_field_1F;
    char field_20;
    char field_21;
    char field_22;
    char m_field_23;
};
struct GDE_MeshItemData0_ext { //44 bytes
    VEC3 m_point;
    uint32_t m_color, field_10, field_14;
    float m_instIdx1, m_instIdx2;
    char field_18, field_19, field_1A, m_field_1B;
    char field_1C;
    char field_1D;
    char field_1E;
    char m_field_1F;
    char field_20;
    char field_21;
    char field_22;
    char m_field_23;
    GDE_MeshItemData0_ext &operator =(const GDE_MeshItemData0 &);
};
struct GDE_MeshItemData1 { //44 bytes
    VEC3 m_point;
    uint32_t m_color;
    uint32_t field_10, field_14;
    float m_instIdx, gap;
    char field_20, field_21, field_22, m_field_23;
    char field_24;
    char field_25;
    char field_26;
    char m_field_27;
    char field_28;
    char field_29;
    char field_2A;
    char m_field_2B;
};
struct GDE_MeshItemData2_ext;
struct GDE_MeshItemData2 { //40 bytes
    VEC3 m_point;
    uint32_t m_color1;
    uint32_t m_color2;
    uint32_t field_14, field_18;
    char field_1C, field_1D, field_1E;
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
struct GDE_MeshItemData2_ext { //48 bytes
    VEC3 m_point;
    uint32_t m_color1;
    uint32_t m_color2;
    uint32_t field_14, field_18;
    float m_instIdx1, m_instIdx2;
    char field_1C, field_1D, field_1E;
    char m_field_1F;
    char field_20;
    char field_21;
    char field_22;
    char m_field_23;
    char field_24;
    char field_25;
    char field_26;
    char m_field_27;
    GDE_MeshItemData2_ext &operator=(const GDE_MeshItemData2 &);
};
struct GDE_MeshItemData3 { //48 bytes
    VEC3 m_point;
    uint32_t m_color1;
    uint32_t m_color2;
    uint32_t field_14, field_18;
    float m_instIdx, gap;
    char field_24[3];
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
    uint32_t field_14, field_18;
    float m_instIdx, gap;
    VEC3B m_pointN;
    char m_field_27;
    VEC3B m_point1;
    char m_field_2B;
    VEC3B m_point2;
    char m_field_2F;
};
struct GDE_MeshItemData4_file { //72 bytes
    VEC3 m_point;
    uint32_t field_14, field_18;
    float m_instIdx, gap;
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
struct GDE_Mesh_VERT_BUFi { //80 bytes, GMK_VERT_BUF
    int m_vbHandle;
    int m_field_4;
    GDE_MeshSubItem *m_subItemBegPtr;
    void *m_field_10;
    void *m_field_18;
    void *m_field_20;
    GDE_MeshSubItem *m_subItemEndPtr;
    void *m_pVerts;
    void *m_pUnused;
    uint8_t gap40[4];
    int m_numVerts;
    int m_itemKind;
    int m_instancesCount;
};
struct GDE_Mesh_SHRUB { //??? bytes, GMK_SHRUB
    uint32_t m_version, m_field_4;
    char gap1[32];
    void *m_field_20, *m_field_28;
};
struct GDE_SkinVB_Item { //96 bytes
    uint16_t *m_pIndices;
    uint32_t *m_pVerts;
    int m_numIndices, m_ibHandle, m_numVerts, m_vbHandle, gap2, m_runIndices, gap3[12];
    int m_flags, gap4;
};
struct GDE_SkinVB { //40 bytes
    GDE_SkinVB_Item *m_pItems, *m_field_8, *m_field_10, *m_field_18, *m_pEndItem;
};
struct GDE_Mesh_SKIN { //0x108 (264) bytes, GMK_SKIN
    uint32_t m_version, m_unk;
    GDE_SkinVB m_vbs[6];
    int m_lodMax;
    char gap[12];
};
struct GDE_Mesh_VERT_BUF { //0x1F8 (504) bytes
    uint32_t m_version;
    int32_t m_lodMax;
    GDE_Mesh_VERT_BUFi m_data[6];
    VEC4 m_bounds;
};
struct GDE_Cluster { //72 bytes
    char field_0;
    char field_1;
    uint16_t m_numIndices;
    char field_4;
    char field_5;
    char field_6;
    char field_7;
    uint16_t *m_pIndices;
    uint16_t m_numVerts;
    char field_12;
    char field_13;
    char field_14;
    char field_15;
    char field_16;
    char field_17;
    void *m_pVerts;
    int m_ibHandle, m_vbHandle;
    char field_28;
    char field_29;
    char field_2A;
    char field_2B;
    char field_2C;
    char field_2D;
    char field_2E;
    char field_2F;
    char field_30;
    char field_31;
    char field_32;
    char field_33;
    char field_34;
    char field_35;
    char field_36;
    char field_37;
    char field_38;
    char field_39;
    char field_3A;
    char field_3B;
    char field_3C;
    char field_3D;
    char field_3E;
    char field_3F;
    char field_40;
    char field_41;
    char field_42;
    char field_43;
    char field_44;
    char field_45;
    char field_46;
    char field_47;
};
struct GDE_Group { //32 bytes
    char field_0;
    char field_1;
    uint16_t m_clustersCnt;
    char field_4;
    char field_5;
    char field_6;
    char field_7;
    GDE_Cluster *m_clusters;
    char field_10;
    char field_11;
    char field_12;
    char field_13;
    char field_14;
    char field_15;
    char field_16;
    char field_17;
    char field_18;
    char field_19;
    char field_1A;
    char field_1B;
    char field_1C;
    char field_1D;
    char field_1E;
    char field_1F;
};
struct GDE_SimpleMaterial { //16 bytes
    char field_0;
    char field_1;
    uint16_t m_groupsCnt;
    char field_4;
    char field_5;
    char field_6;
    char field_7;
    GDE_Group *m_groups;
};
struct GDE_Mesh_VB_CLUST { //? bytes
    uint32_t m_version;
    int field_4;
    int field_8;
    int field_C;
    int m_materialsCnt;
    int field_14;
    GDE_SimpleMaterial *m_materials;
    void *m_field_20;
};
struct GDE_Runtime { //0x28 bytes
    uint32_t m_sign1, field_4, m_sign2, field_C;
    GDE_Runtime *m_next;
    uint64_t m_runtimeItemsCnt;
    GDE_RuntimeItem *m_runtimeItems;
};
struct GDE_Shader { //16 bytes
    uint64_t data1, data2;
};
struct GDE_DefaultComponents { //48 bytes
    char gap[48];
};
struct GDE_BoneInfo { //80 bytes
    char gap[80];
};
struct GDE_SkelInfo { //0x40 (64) bytes
    int field_0, m_arraysCnt;
    GDE_BoneInfo *m_pBoneInfoArray;
    GDE_DefaultComponents *m_pDefaultComponents;
    void *m_field_18, *m_field_20;
    char gap2[24];
};
enum GDE_MeshKind : uint32_t { GMK_VB_CLUSTER = 0x1000'0000, GMK_SKIN = 0x1000'0001, GMK_SHRUB = 0x1000'0002, GMK_VERT_BUF = 0x1000'0003 };
struct GDE_Header_360 { //0x70 bytes
    uint32_t m_ver;
    GDE_MeshKind m_meshKind;
    uint32_t m_materialsCnt;
    uint32_t m_texturesCnt;
    uint32_t m_shadersCnt;
    int field_14;
    GDE_Material *m_materials;
    GDE_Tex *m_textures;
    GDE_Shader *m_shaders;
    GDE_Runtime *m_runtime;
    union { 
        GDE_Mesh_VERT_BUF *VERT_BUF;
        GDE_Mesh_VB_CLUST *VB_CLUST;
        GDE_Mesh_SHRUB *SHRUB;
        GDE_Mesh_SKIN *SKIN;
    } m_mesh;
    GDE_SkelInfo *m_pSkelInfo[6];
};
struct GDE_360_TIE {};
struct GDE_MaterialUsage {};
enum InstanceResourceState { IRS_UNLOADED = 0, IRS_NEED_LOAD = 1, IRS_LOAD_FAILED = 2, IRS_3 = 3, IRS_4 = 4, IRS_INITIAL = 5 };
struct InstanceResource { //72 bytes
    GDE_Header_360 *m_gdeFile;
    VEC4 m_bounds;
    bool m_manyInstances, m_isSkin, m_heapUsed;
    uint8_t field_1B;
    int field_1C;
    char *m_gdeName;
    uint32_t m_gdeNameCRC;
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
