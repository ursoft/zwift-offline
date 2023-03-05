#include "ZwiftApp.h"
void GDEMESH_Initialize() {
#if 0 //TODO
    GFX_CreateVertexParams parms;
    *parms.m_field_10 = 0x70000;
    *&parms.m_field_10[4] = 0xC060600;
    *&parms.m_field_10[8] = 0x14060700;
    parms.m_attrCnt = 7i64;
    *&parms.m_field_10[12] = 470221056;
    *&parms.m_field_10[16] = 671547904;
    *&parms.m_field_10[20] = 872874752;
    parms.m_field_8 = 1i64;
    *parms.m_field_28 = 1073808384;
    memset(&parms.m_field_28[4], 0, 36);
    parms.m_field_50 = 83968;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hGDE_TRI_VERT = GFX_CreateVertex(&parms);
    parms.m_attrCnt = 6i64;
    parms.m_field_8 = 1i64;
    *parms.m_field_10 = 458752;
    *&parms.m_field_10[4] = 268830208;
    *&parms.m_field_10[8] = 402784512;
    *&parms.m_field_10[12] = 469893632;
    *&parms.m_field_10[16] = 537002752;
    *&parms.m_field_10[20] = 201393152;
    memset(parms.m_field_28, 0, sizeof(parms.m_field_28));
    parms.m_field_50 = 0x12400;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hTRI_VERT_COLOR1_UV1 = GFX_CreateVertex(&parms);
    parms.m_attrCnt = 7i64;
    parms.m_field_8 = 1i64;
    *parms.m_field_10 = 458752;
    *&parms.m_field_10[4] = 201393152;
    *&parms.m_field_10[8] = 268830208;
    *&parms.m_field_10[12] = 403048192;
    *&parms.m_field_10[16] = 537002240;
    *&parms.m_field_10[20] = 604111360;
    *parms.m_field_28 = 671220480;
    memset(&parms.m_field_28[4], 0, 36);
    parms.m_field_50 = 76800;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hTRI_VERT_COLOR1_UV2 = GFX_CreateVertex(&parms);
    parms.m_attrCnt = 7i64;
    parms.m_field_8 = 1i64;
    *parms.m_field_10 = 458752;
    *&parms.m_field_10[4] = 201393152;
    *&parms.m_field_10[8] = 268502272;
    *&parms.m_field_10[12] = 335939072;
    *&parms.m_field_10[16] = 469893376;
    *&parms.m_field_10[20] = 537002496;
    *parms.m_field_28 = 604111616;
    memset(&parms.m_field_28[4], 0, 36);
    parms.m_field_50 = 75776;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hTRI_VERT_COLOR2_UV1 = GFX_CreateVertex(&parms);
    parms.m_attrCnt = 8i64;
    parms.m_field_8 = 1i64;
    *parms.m_field_10 = 458752;
    *&parms.m_field_10[4] = 201393152;
    *&parms.m_field_10[8] = 268502272;
    *&parms.m_field_10[12] = 335939072;
    *&parms.m_field_10[16] = 470157056;
    *&parms.m_field_10[20] = 604111104;
    *parms.m_field_28 = 671220224;
    *&parms.m_field_28[4] = 738329344;
    memset(&parms.m_field_28[8], 0, 32);
    parms.m_field_50 = 77824;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hTRI_VERT_COLOR2_UV2 = GFX_CreateVertex(&parms);
    parms.m_attrCnt = 8i64;
    parms.m_field_8 = 1i64;
    *parms.m_field_10 = 458752;
    *&parms.m_field_10[4] = 335675648;
    *&parms.m_field_10[8] = 402784768;
    *&parms.m_field_10[12] = 469893888;
    *&parms.m_field_10[16] = 201721344;
    *&parms.m_field_10[20] = 536937472;
    *parms.m_field_28 = 671156480;
    *&parms.m_field_28[4] = 604178432;
    memset(&parms.m_field_28[8], 0, 32);
    parms.m_field_50 = 76800;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hDEFORM_SKIN_VERT_GL = GFX_CreateVertex(&parms);
    parms.m_attrCnt = 6i64;
    parms.m_field_8 = 1i64;
    *parms.m_field_10 = 458752;
    *&parms.m_field_10[4] = 336003328;
    *&parms.m_field_10[8] = 537330176;
    *&parms.m_field_10[12] = 738657024;
    *&parms.m_field_10[16] = 201721344;
    *&parms.m_field_10[20] = 939590656;
    memset(parms.m_field_28, 0, sizeof(parms.m_field_28));
    parms.m_field_50 = 81920;
    memset(parms.m_field_54, 0, sizeof(parms.m_field_54));
    g_hGDE_RIGID_SKIN_VERT = GFX_CreateVertex(&parms);
#endif
}
int GDEMESH_Internal_FindLoadedMesh(const char *name, bool manyInstances /*multiframe???*/) {
    auto crc = SIG_CalcCaseSensitiveSignature(name);
    int result = 0;
    if (g_ENG_nResources <= 0)
        return -1;
    for (auto &i : g_ENG_InstanceResources) {
        if (crc == i.m_gdeNameCRC && manyInstances == i.m_manyInstances)
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
GDE_Header_360 *GDEMESH_GetMesh(int handle) {
    if ((unsigned)handle >= _countof(g_ENG_InstanceResources))
        return nullptr;
    auto &m = g_ENG_InstanceResources[handle];
    if (m.m_state == IRS_NEED_LOAD) {
        if (!GFX_Internal_LoadOnDemandMeshHandle(handle, m.m_isSkin ? GDE_SKIN_MESH : GDE_MESH))
            m.m_state = IRS_LOAD_FAILED;
    } else if (m.m_state == IRS_UNLOADED) {
        Log("trying to activate unloaded mesh %s", m.m_gdeName);
    }
    m.m_frameCnt = GFX_GetFrameCount();
    m.m_creationTime = g_MeshTimeThisFrame;
    return m.m_gdeFile;
}
VEC4 g_defMeshBounds{0.0f, 0.0f, 0.0f, 1.0f};
VEC4 *GDEMESH_GetMeshBounds(VEC4 *ret, int handle) {
    if ((unsigned)handle >= _countof(g_ENG_InstanceResources))
        *ret = g_defMeshBounds;
    else
        *ret = g_ENG_InstanceResources[handle].m_bounds;
    return ret;
}
void GDE_FreeRuntime(GDE_Header_360 *h) {
    while (h->m_runtime) {
        if (h->m_runtime->m_sign2 == 'GATD' || h->m_runtime->m_sign2 == 'LITE') {
            free(h->m_runtime->m_runtimeItems);
            h->m_runtime->m_runtimeItems = nullptr;
        }
        auto next = h->m_runtime->m_next;
        free(h->m_runtime);
        h->m_runtime = next;
    }
    h->m_runtime = nullptr;
}
void GDE_NormalizeVector(VEC3 *dest, VEC3 *src) {
    auto leng = sqrt(src->m_data[0] * src->m_data[0] + src->m_data[1] * src->m_data[1] + src->m_data[2] * src->m_data[2]);
    if (leng == 0.0 && dest != src) {
        *dest = *src;
    } else {
        dest->m_data[0] = src->m_data[0] / leng;
        dest->m_data[1] = src->m_data[1] / leng;
        dest->m_data[2] = src->m_data[2] / leng;
    }
}