#include "ZwiftApp.h"
void GDEMESH_Initialize() {
    //TODO
}
int GDEMESH_Internal_FindLoadedMesh(const char *name, bool temporary) {
    auto crc = SIG_CalcCaseSensitiveSignature(name);
    int result = 0;
    if (g_ENG_nResources <= 0)
        return -1;
    for (auto &i : g_ENG_InstanceResources) {
        if (crc == i.m_nameCRC && temporary == i.m_temp)
            break;
        if (++result >= g_ENG_nResources)
            return -1;
    }
    return result;
}
int GDEMESH_GetFreeMeshResourceHandle() {
    int result = g_ENG_nResources;
    if (g_ENG_nResources >= _countof(g_ENG_InstanceResources)) {
        zassert(!"GFX_GDEMesh.cpp/GDEMESH_GetFreeMeshResourceHandle() => Cannot create new MeshHandle: MAX_MESH_RESOURCES reached");
        LogTyped(LOG_ERROR, "Cannot create new MeshHandle: MAX_MESH_RESOURCES [%d] reached!", g_ENG_nResources);
        return -1;
    } else {
        ++g_ENG_nResources;
    }
    return result;
}
