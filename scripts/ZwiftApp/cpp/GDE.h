#pragma once
struct GDE_Tex { //32 bytes
    char *m_name;
    uint32_t m_assetCrc;
    char gap[16];
};
struct GDE_Header_360 { //0x70 bytes
    uint32_t m_ver, m_textureNum, m_field_8, m_field_10;
    void *m_field_18;// = malloc(368 * file->m_field_8); //sizeof smth
    GDE_Tex *m_textures;
    void *m_field_28;// = malloc(16 * file->m_field_10); //sizeof smth
};
struct GDE_360_TIE {};
struct GDE_MaterialUsage {};
void GDEMESH_Initialize();
int GDEMESH_Internal_FindLoadedMesh(const char *name, bool temporary);
int GDEMESH_GetFreeMeshResourceHandle();
struct InstanceResource { //72 bytes
    void *m_ptr;
    bool m_temp;
    uint32_t m_nameCRC;
};

inline InstanceResource g_ENG_InstanceResources[6500];
inline int g_ScreenMeshHandle, g_TrainerMeshHandle, g_HandCycleTrainerMeshHandle, g_TreadmillMeshHandle, g_PaperMeshHandle, g_ENG_nResources;
inline int64_t g_VRAMBytes_GDERAM;