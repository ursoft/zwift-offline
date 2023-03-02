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
struct GDE_Header_360 { //0x70 bytes
    uint32_t m_ver;
    int field_4;
    int m_animCnt;
    int m_texturesCnt;
    int m_field_10;
    int field_14;
    GDE_Anim *m_anims;
    GDE_Tex *m_textures;
    uint8_t *m_field_28; // = malloc(16 * file->m_field_10); //sizeof smth
    char field_30[64];
};
struct GDE_360_TIE {};
struct GDE_MaterialUsage {};
void GDEMESH_Initialize();
int GDEMESH_Internal_FindLoadedMesh(const char *name, bool temporary);
int GDEMESH_GetFreeMeshResourceHandle();
struct InstanceResource_ { //72 bytes
    void *m_ptr;
    bool m_temp;
    uint32_t m_nameCRC;
};
struct InstanceResource : InstanceResource_ {
    char gap[72 - sizeof(InstanceResource_)];
};
inline InstanceResource g_ENG_InstanceResources[6500];
inline int g_ScreenMeshHandle, g_TrainerMeshHandle, g_HandCycleTrainerMeshHandle, g_TreadmillMeshHandle, g_PaperMeshHandle, g_ENG_nResources;
inline int64_t g_VRAMBytes_GDERAM;