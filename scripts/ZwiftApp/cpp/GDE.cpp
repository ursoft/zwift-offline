#include "ZwiftApp.h"
void GDEMESH_Initialize() {
    /*enum GFX_VertexFormat : uint8_t { GVF_FALSE=0, GVF_UNSIGNED_BYTE1, GVF_BYTE=2, GVF_UNSIGNED_BYTE3, GVF_UNSIGNED_SHORT=4, GVF_FLOAT5, GVF_FLOAT6, GVF_FLOAT7, GVF_CNT };
    parms.m_attrCnt = 7i64;
    parms.m_stridesCnt = 1i64;
    parms.m_attrs[0] = (GFX_VertexAttr)0x70000;
    parms.m_attrs[1] = (GFX_VertexAttr)0xC060600;
    parms.m_attrs[2] = (GFX_VertexAttr)0x14060700;
    parms.m_attrs[3] = (GFX_VertexAttr)0x1C070100;
    parms.m_attrs[4] = (GFX_VertexAttr)0x28070200;
    parms.m_attrs[5] = (GFX_VertexAttr)0x34070300;
    parms.m_attrs[6] = (GFX_VertexAttr)0x40010400;
    parms.m_strides[0] = (GFX_Stride)0x14800;*/
    g_hGDE_TRI_VERT = GFX_CreateVertex(GFX_CreateVertexParams{7, 1, { // m_attrCnt, m_stridesCnt, m_attrs
        //m_strideIdx, m_atrIdx, m_fmtIdx, m_dataOffset
            { 0, 0, GVF_FLOAT7 },       //0x7 00 00
            { 0, 6, GVF_FLOAT6, 0xC },  //0xC 06 06 00
            { 0, 7, GVF_FLOAT6, 0x14 }, //0x14 06 07 00
            { 0, 1, GVF_FLOAT7, 0x1C },
            { 0, 2, GVF_FLOAT7, 0x28 },
            { 0, 3, GVF_FLOAT7, 0x34 },
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x40 }
        }, { //m_strides
            {0, 0x48, 1} //each stride: 0x14800 m_strideIdx, m_strideCnt, field_2
        } });
    g_hTRI_VERT_COLOR1_UV1 = GFX_CreateVertex(GFX_CreateVertexParams{6, 1, {
            { 0, 0, GVF_FLOAT7 },
            { 0, 6, GVF_FLOAT6, 0x10 },
            { 0, 1, GVF_BYTE, 0x10 },
            { 0, 2, GVF_BYTE, 0x1C },
            { 0, 3, GVF_BYTE, 0x20 },
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C }
        }, { {0, 0x24, 1} } });
    g_hTRI_VERT_COLOR1_UV2 = GFX_CreateVertex(GFX_CreateVertexParams{7, 1, {
            { 0, 0, GVF_FLOAT7 }, 
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C },
            { 0, 6, GVF_FLOAT6, 0x10 },
            { 0, 7, GVF_FLOAT6, 0x18 },
            { 0, 1, GVF_BYTE, 0x20 },
            { 0, 2, GVF_BYTE, 0x24 },
            { 0, 3, GVF_BYTE, 0x28 }
        }, { {0, 0x2C, 1} } });
    g_hTRI_VERT_COLOR2_UV1 = GFX_CreateVertex(GFX_CreateVertexParams{7, 1, {
            { 0, 0, GVF_FLOAT7 },
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C },
            { 0, 5, GVF_UNSIGNED_BYTE1, 0x10 },
            { 0, 6, GVF_FLOAT6, 0x14 },
            { 0, 1, GVF_BYTE, 0x1C },
            { 0, 2, GVF_BYTE, 0x20 },
            { 0, 3, GVF_BYTE, 0x24 }
        }, { {0, 0x28, 1} } });
    g_hTRI_VERT_COLOR2_UV2 = GFX_CreateVertex(GFX_CreateVertexParams{8, 1, {
            { 0, 0, GVF_FLOAT7 },
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C },
            { 0, 5, GVF_UNSIGNED_BYTE1, 0x10 },
            { 0, 6, GVF_FLOAT6, 0x14 },
            { 0, 7, GVF_FLOAT6, 0x1C },
            { 0, 1, GVF_BYTE, 0x24 },
            { 0, 2, GVF_BYTE, 0x28 },
            { 0, 3, GVF_BYTE, 0x2C }
        }, { {0, 0x30, 1} } });
    g_hDEFORM_SKIN_VERT_GL = GFX_CreateVertex(GFX_CreateVertexParams{8, 1, {
            { 0, 0, GVF_FLOAT7 },
            { 0, 1, GVF_BYTE, 0x14 },
            { 0, 2, GVF_BYTE, 0x18 },
            { 0, 3, GVF_BYTE, 0x1C },
            { 0, 6, GVF_FLOAT6, 0x0C },
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x20 },
            { 0, 9, GVF_UNSIGNED_BYTE1, 0x28 },
            { 0, 8, GVF_UNSIGNED_BYTE3, 0x24 }
        }, { {0, 0x2C, 1} } });
    g_hGDE_RIGID_SKIN_VERT = GFX_CreateVertex(GFX_CreateVertexParams{ 6, 1, {
            { 0, 0, GVF_FLOAT7 },
            { 0, 1, GVF_FLOAT7, 0x14 },
            { 0, 2, GVF_FLOAT7, 0x20 },
            { 0, 3, GVF_FLOAT7, 0x2C },
            { 0, 6, GVF_FLOAT6, 0x0C },
            { 0, 4, GVF_UNSIGNED_BYTE1, 0x38 }
        }, { {0, 0x40, 1} } });
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
        if (!GFX_Internal_LoadOnDemandMeshHandle(handle, m.m_isSkin ? GMK_SKIN : GMK_VERT_BUF))
            m.m_state = IRS_LOAD_FAILED;
    } else if (m.m_state == IRS_UNLOADED) {
        Log("trying to activate unloaded mesh %s", m.m_gdeName);
    }
    m.m_frameCnt = GFX_GetFrameCount();
    m.m_creationTime = g_MeshTimeThisFrame;
    return m.m_gdeFile;
}
Sphere g_defMeshBounds{ {0.0f, 0.0f, 0.0f}, 1.0f };
Sphere *GDEMESH_GetMeshBounds(Sphere *ret, int handle) {
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
GDE_MeshItemData2_ext &GDE_MeshItemData2_ext::operator=(const GDE_MeshItemData2 &src) {
    m_point = src.m_point;
    m_color1 = src.m_color1;
    m_color2 = src.m_color2;
    field_14 = src.field_14;
    field_1C = src.field_1C;
    //two floats not initialized yet
    field_1D = src.field_1D;
    field_1E = src.field_1E;
    m_field_1F = src.m_field_1F;
    field_20 = src.field_20;
    field_21 = src.field_21;
    field_22 = src.field_22;
    m_field_23 = src.m_field_23;
    field_24 = src.field_24;
    field_25 = src.field_25;
    field_26 = src.field_26;
    m_field_27 = src.m_field_27;
    return *this;
};
GDE_MeshItemData0_ext &GDE_MeshItemData0_ext::operator =(const GDE_MeshItemData0 &src) {
    m_point = src.m_point;
    m_color = src.m_color;
    field_10 = src.field_10;
    field_14 = src.field_14;
    field_18 = src.field_18;
    field_19 = src.field_19;
    field_1A = src.field_1A;
    m_field_1B = src.m_field_1B;
    field_1C = src.field_1C;
    field_1D = src.field_1D;
    field_1E = src.field_1E;
    m_field_1F = src.m_field_1F;
    field_20 = src.field_20;
    field_21 = src.field_21;
    field_22 = src.field_22;
    m_field_23 = src.m_field_23;
    return *this;
}
void GDE_UpgradeMesh(GDE_Mesh_VERT_BUF *pMesh) {
    if (pMesh->m_version == 9) {
        for (int i = 0; i < pMesh->m_lodMax; i++)
            pMesh->m_data[i].m_itemKind = 4;
    }
}
