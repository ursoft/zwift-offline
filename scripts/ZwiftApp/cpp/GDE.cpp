#include "ZwiftApp.h" //READY for testing
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
    g_hGDE_TRI_VERT = GFX_CreateVertex(GFX_CreateVertexParams{ 7, 1, { // m_attrCnt, m_stridesCnt, m_attrs
                                                                   //m_strideIdx, m_atrIdx, m_fmtIdx, m_dataOffset
                                                                   { 0, 0, GVF_FLOAT7 },       //0x7 00 00
                                                                   { 0, 6, GVF_FLOAT6, 0xC },  //0xC 06 06 00
                                                                   { 0, 7, GVF_FLOAT6, 0x14 }, //0x14 06 07 00
                                                                   { 0, 1, GVF_FLOAT7, 0x1C },
                                                                   { 0, 2, GVF_FLOAT7, 0x28 },
                                                                   { 0, 3, GVF_FLOAT7, 0x34 },
                                                                   { 0, 4, GVF_UNSIGNED_BYTE1, 0x40 }
                                                               }, { //m_strides
                                                                   { 0, 0x48, 1 } //each stride: 0x14800 m_strideIdx, m_strideCnt, field_2
                                                               } });
    g_hTRI_VERT_COLOR1_UV1 = GFX_CreateVertex(GFX_CreateVertexParams{ 6, 1, {
                                                                          { 0, 0, GVF_FLOAT7 },
                                                                          { 0, 6, GVF_FLOAT6, 0x10 },
                                                                          { 0, 1, GVF_BYTE, 0x10 },
                                                                          { 0, 2, GVF_BYTE, 0x1C },
                                                                          { 0, 3, GVF_BYTE, 0x20 },
                                                                          { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C }
                                                                      }, { { 0, 0x24, 1 } } });
    g_hTRI_VERT_COLOR1_UV2 = GFX_CreateVertex(GFX_CreateVertexParams{ 7, 1, {
                                                                          { 0, 0, GVF_FLOAT7 },
                                                                          { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C },
                                                                          { 0, 6, GVF_FLOAT6, 0x10 },
                                                                          { 0, 7, GVF_FLOAT6, 0x18 },
                                                                          { 0, 1, GVF_BYTE, 0x20 },
                                                                          { 0, 2, GVF_BYTE, 0x24 },
                                                                          { 0, 3, GVF_BYTE, 0x28 }
                                                                      }, { { 0, 0x2C, 1 } } });
    g_hTRI_VERT_COLOR2_UV1 = GFX_CreateVertex(GFX_CreateVertexParams{ 7, 1, {
                                                                          { 0, 0, GVF_FLOAT7 },
                                                                          { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C },
                                                                          { 0, 5, GVF_UNSIGNED_BYTE1, 0x10 },
                                                                          { 0, 6, GVF_FLOAT6, 0x14 },
                                                                          { 0, 1, GVF_BYTE, 0x1C },
                                                                          { 0, 2, GVF_BYTE, 0x20 },
                                                                          { 0, 3, GVF_BYTE, 0x24 }
                                                                      }, { { 0, 0x28, 1 } } });
    g_hTRI_VERT_COLOR2_UV2 = GFX_CreateVertex(GFX_CreateVertexParams{ 8, 1, {
                                                                          { 0, 0, GVF_FLOAT7 },
                                                                          { 0, 4, GVF_UNSIGNED_BYTE1, 0x0C },
                                                                          { 0, 5, GVF_UNSIGNED_BYTE1, 0x10 },
                                                                          { 0, 6, GVF_FLOAT6, 0x14 },
                                                                          { 0, 7, GVF_FLOAT6, 0x1C },
                                                                          { 0, 1, GVF_BYTE, 0x24 },
                                                                          { 0, 2, GVF_BYTE, 0x28 },
                                                                          { 0, 3, GVF_BYTE, 0x2C }
                                                                      }, { { 0, 0x30, 1 } } });
    g_hDEFORM_SKIN_VERT_GL = GFX_CreateVertex(GFX_CreateVertexParams{ 8, 1, {
                                                                          { 0, 0, GVF_FLOAT7 },
                                                                          { 0, 1, GVF_BYTE, 0x14 },
                                                                          { 0, 2, GVF_BYTE, 0x18 },
                                                                          { 0, 3, GVF_BYTE, 0x1C },
                                                                          { 0, 6, GVF_FLOAT6, 0x0C },
                                                                          { 0, 4, GVF_UNSIGNED_BYTE1, 0x20 },
                                                                          { 0, 9, GVF_UNSIGNED_BYTE1, 0x28 },
                                                                          { 0, 8, GVF_UNSIGNED_BYTE3, 0x24 }
                                                                      }, { { 0, 0x2C, 1 } } });
    g_hGDE_RIGID_SKIN_VERT = GFX_CreateVertex(GFX_CreateVertexParams{ 6, 1, {
                                                                          { 0, 0, GVF_FLOAT7 },
                                                                          { 0, 1, GVF_FLOAT7, 0x14 },
                                                                          { 0, 2, GVF_FLOAT7, 0x20 },
                                                                          { 0, 3, GVF_FLOAT7, 0x2C },
                                                                          { 0, 6, GVF_FLOAT6, 0x0C },
                                                                          { 0, 4, GVF_UNSIGNED_BYTE1, 0x38 }
                                                                      }, { { 0, 0x40, 1 } } });
}
int GDEMESH_Internal_FindLoadedMesh(const char *name, bool manyInstances /*multiframe???*/) {
    auto crc = SIG_CalcCaseSensitiveSignature(name);
    int  result = 0;
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
Sphere g_defMeshBounds{ { 0.0f, 0.0f, 0.0f }, 1.0f };
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
}
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
void LOADER_SetFatBitVertexStreamIfNecessary(GDE_360_TIE *pMesh) {
    if (pMesh->m_version == 9) {
        for (int i = 0; i < pMesh->m_lodMax; i++)
            pMesh->m_data[i].m_itemKind = 4;
    }
}
void GDEMESH_Destroy() { //linked out
    GFX_DestroyVertex(&g_hGDE_RIGID_SKIN_VERT);
    GFX_DestroyVertex(&g_hDEFORM_SKIN_VERT_GL);
    GFX_DestroyVertex(&g_hTRI_VERT_COLOR1_UV1);
    GFX_DestroyVertex(&g_hTRI_VERT_COLOR1_UV2);
    GFX_DestroyVertex(&g_hTRI_VERT_COLOR2_UV1);
    GFX_DestroyVertex(&g_hTRI_VERT_COLOR2_UV2);
    GFX_DestroyVertex(&g_hGDE_TRI_VERT);
}
char *GDEMESH_GetFilename(int handle) { //linked out
    if (handle >= _countof(g_ENG_InstanceResources))
        return nullptr;
    else
        return g_ENG_InstanceResources[handle].m_gdeName;
}
bool GDEMESH_MeshNeedsAlphaPass(const GDE_Header_360 *hdr) {
    if (!hdr)
        return false;
    switch (hdr->m_meshKind) {
        default:
            return false;
        case GMK_VERT_BUF: {
            auto vb = hdr->m_mesh.VERT_BUF;
            auto v3 = GDE_OptimizeLod(0, vb->m_lodMax);
            for (auto i = vb->m_data[v3].m_subItemBegPtr; i < vb->m_data[v3].m_subItemEndPtr; i++)
                if (hdr->m_materials[i->m_matIdx].m_mbmAlpha)
                    return true;
            return false; }
        case GMK_SKIN: {
            auto vs = hdr->m_mesh.SKIN;
            auto v8 = GDE_OptimizeLod(0, vs->m_lodMax);
            for (auto i = vs->m_vbs[v8].m_pItems; i < vs->m_vbs[v8].m_pEndItem; i++)
                if (hdr->m_materials[i->m_materIdx].m_mbmAlpha)
                    return true;
            return false; }
    }
}
int g_texDefNormalMap = -1;
void GDE_RenderMesh(const GDE_Header_360 *hdr, uint32_t a2, int64_t bits, bool useMaterial, const VEC4 &shVec) {
    auto v18 = g_GFX_rp != 2 && g_GFX_rp != 3;
    if (hdr) {
        auto v8 = hdr->m_mesh.VERT_BUF;
        auto v9 = v8->m_data + GDE_OptimizeLod(a2, v8->m_lodMax);
        if (g_texDefNormalMap == -1)
            g_texDefNormalMap = GFX_CreateTextureFromTGAFile("defaultNormalMap.tga", -1, true);
        for (auto i = v9->m_subItemBegPtr; i < v9->m_subItemEndPtr; i++) {
            if (_bittest64(&bits, i->m_type)) {
                auto v12 = &hdr->m_materials[i->m_matIdx];
                if ((v18 || (i->m_bits & 1) != 0 && (v12->m_bits & 1) != 0)
                    && (!g_bSkinMeshMaterialCullingHack
                        || (((g_GFX_rp != 1 && g_GFX_rp != 5) || v12->m_mbmAlpha)
                            && ((g_GFX_rp != 4 && g_GFX_rp != 0) || !v12->m_mbmAlpha)))) {
                    auto shader = -1;
                    if (useMaterial) {
                        if ((v12->m_bits & 0x100) != 0 && (i->m_bits & 0x20) != 0 && !g_UVShaderChange) {
                            shader = GFX_GetCurrentShaderHandle();
                            GFX_SetShader(g_World2LayerShaderHandle);
                        }
                        auto hasAlpha = v12->m_mbmAlpha;
                        if (g_meshForcedTransparent)
                            v12->m_mbmAlpha = g_materialBlendMode;
                        MATERIAL_Activate(v12, v12->m_pUsage, nullptr);
                        if (g_meshForcedTransparent)
                            v12->m_mbmAlpha = hasAlpha;
                    }
                    auto v15 = (uint64_t)-1;
                    if (shVec.m_data[0] != 1.0f || shVec.m_data[1] != 1.0f || shVec.m_data[2] != 1.0f || shVec.m_data[3] != 1.0f)
                        v15 = 0;
                    GFX_UploadShaderVEC4(GSR_24, shVec, v15);
                    GFX_CULL cm = GFC_NONE;
                    if (v12->m_bits & 0x200) {
                        cm = (GFX_CULL)GFX_GetStateU32(GFS_CULLMODE);
                        GFX_SetCullMode(GFC_NONE);
                    }
                    GDEMESH_RenderTieMesh_Group(v9, i);
                    if (v12->m_bits & 0x200)
                        GFX_SetCullMode(cm);
                    if (shader != -1)
                        GFX_SetShader(shader);
                }
            }
        }
    }
}
bool g_GDEMESH_UploadBoneMatrices = true;
void GDEMESH_UploadBoneMatrices(const GDE_360_SKIN_STRIP *a1, const uint32_t *a2) {
    g_bonesUploaded += a1->m_countMx;
    if (g_GDEMESH_UploadBoneMatrices) {
        static MATRIX44 g_boneMxs[48]; //QUEST - why make a copy, a2 is not good enough?
        memmove(g_boneMxs, a2, a1->m_countMx * sizeof(MATRIX44));
        GFX_UploadShaderMAT4ARRAY(GSMA_1, a1->m_countMx, g_boneMxs, 0);
    } else {
        for (int v2 = 0; v2 < a1->m_countMx; ++v2)
            GFX_UploadShaderMAT4ARRAY(GSMA_1, 1, ((MATRIX44 *)a2) + a1->m_indexesMx[v2], v2);
    }
}
struct VEC4def : public VEC4 {
    VEC4def() : VEC4(g_cident0) {}
};
VEC4def g_GDE_RenderMeshInstances[16];
void GDE_RenderMeshInstances(const GDE_Header_360 *a1, uint8_t a2, int64_t bits, bool useMaterial, uint32_t a5, MATRIX44 *a6, VEC4 *a7, VEC2 *a8) {
    if (a1) {
        if (a1->m_meshKind == GMK_VERT_BUF) {
            auto v10 = a1->m_mesh.VERT_BUF;
            auto v11 = GDE_OptimizeLod(a2, v10->m_lodMax);
            for (auto i = v10->m_data[v11].m_subItemBegPtr; i < v10->m_data[v11].m_subItemEndPtr; ++i) {
                if (_bittest64(&bits, i->m_type)) {
                    if (useMaterial) {
                        auto &v15 = a1->m_materials[i->m_matIdx];
                        auto hasAlpha = v15.m_mbmAlpha;
                        if (g_meshForcedTransparent)
                            v15.m_mbmAlpha = g_materialBlendMode;
                        MATERIAL_Activate(&v15, v15.m_pUsage, nullptr);
                        if (g_meshForcedTransparent)
                            v15.m_mbmAlpha = hasAlpha;
                    }
                    GFX_SetVertexBuffer(v10->m_data[v11].m_vbHandle, 0, 0);
                    GFX_SetIndexBuffer(i->m_ibHandle);
                    GFX_SetVertex(i->m_vertexHandle);
                    if (a8)
                        GFX_UploadShaderVEC2ARRAY(GSR_28, a5, a8);
                    else
                        GFX_UploadShaderVEC4ARRAY(GSR_28, a5, g_GDE_RenderMeshInstances);
                    GFX_UploadShaderMAT4ARRAY(GSMA_0, a5, a6, 0);
                    GFX_UploadShaderVEC4ARRAY(GSR_27, a5, a7);
                    if (v10->m_data[v11].m_instancesCount <= 1)
                        GFX_DrawIndexedInstancedPrimitive(GPT_TRIANGLE_STRIP, 0, i->m_numIndices, (GFX_IndexFormat)(i->m_numIndices > 0xFFFF), a5, 0);
                    else
                        GFX_DrawIndexedPrimitive(GPT_TRIANGLE_STRIP, 0, a5 * (((i->m_numIndices & 1) == 0) + 3 + i->m_numIndices), GIF_INT, 0);
                }
            }
        } else {
            LogTyped(LOG_ERROR, "Non-tie mesh passed into GDEMESH_RenderTieMeshImmediate()\n");
        }
    }
}
void GDEMESH_RenderTieMesh_Group(const GDE_360_TIE_LOD *a1, const GDE_360_TIE_STRIPGROUP *a2) {
    GFX_SetVertexBuffer(a1->m_vbHandle, 0, 0);
    GFX_SetIndexBuffer(a2->m_ibHandle);
    GFX_SetVertex(a2->m_vertexHandle);
    auto v5 = GIF_INT;
    if (a2->m_numIndices <= 0xFFFF && a1->m_instancesCount <= 1)
        v5 = GIF_SHORT;
    GFX_DrawIndexedPrimitive(GPT_TRIANGLE_STRIP, 0, a2->m_numIndices, v5, nullptr);
}
void GDEMESH_RenderSkinMesh_Group(const GDE_360_SKINDATA_LOD *sdl, const GDE_360_SKIN_STRIP *str) {
    GFX_SetVertexBuffer(str->m_vbHandle, 0, 0);
    GFX_SetIndexBuffer(str->m_ibHandle);
    auto v3 = g_hDEFORM_SKIN_VERT_GL;
    if (str->m_flags3 & 0x10)
        v3 = g_hGDE_RIGID_SKIN_VERT;
    GFX_SetVertex(v3);
    GFX_DrawIndexedPrimitive(GPT_TRIANGLE_STRIP, 0, str->m_numIndices, GIF_SHORT, 2 * str->m_runIndices);
}
int g_skinShader_locked = -1;
void SKIN_DrawMesh(GDE_Header_360 *hdr, const GDE_360_SKINDATA *skin, void *a3, uint32_t *a4, uint32_t a5, int64_t bits, bool a7, VEC4 *a8) {
    if (a4 && a5 < 6) {
        auto cm = (GFX_CULL)GFX_GetStateU32(GFS_CULLMODE);
        if (!a8)
            GFX_SetCullMode(GFC_BACK);
        GFX_MatrixMode(GMT_0);
        GFX_PushMatrix();
        GFX_Scale(VEC3{ 100.0f,100.0f,100.0f });
        GFX_UpdateMatrices(true);
        auto v10 = hdr->m_mesh.SKIN;
        auto v11 = &v10->m_vbs[GDE_OptimizeLod(a5, v10->m_lodMax)];
        GFX_SetVertexBuffer(v11->m_pItems->m_vbHandle, 0, 0);
        GFX_SetIndexBuffer(v11->m_pItems->m_ibHandle);
        auto v49 = PC_GetWorldViewProjMatrix(), v48 = PC_GetWorldMatrix();
        auto v23 = 0;
        for (auto i = v11->m_pItems; i < v11->m_pEndItem; ++i) {
            if (!_bittest64(&bits, i->m_type)) {
                auto v26 = &hdr->m_materials[i->m_materIdx];
                if (!g_bSkinMeshMaterialCullingHack || 
                    ((((g_GFX_rp - 1) & 0xFFFFFFFB) != 0 || v26->m_mbmAlpha) && ((g_GFX_rp & 0xFFFFFFFB) != 0 || v26->m_mbmAlpha != MBM_1))) {
                    if (!a8) {
                        auto v27 = GFC_FRONT;
                        if (v26->m_bits & 0x200) {
                            v27 = GFC_NONE;
                        } else {
                            if (i->m_flags3 & 8)
                                v27 = GFC_BACK;
                        }
                        GFX_SetCullMode(v27);
                    }
                    if (i->m_flags3 & 0x10) {
                        if (v23 != 1) {
                            v23 = 1;
                            GFX_SetShader(g_WorldShaderHandle);
                            GFX_UpdateMatrices(1);
                            GFX_SetVertex(g_hGDE_RIGID_SKIN_VERT);
                        }
                        continue;
                    }
                    if (v23 != 2) {
                        v23 = 2;
                        GFX_SetVertex(g_hDEFORM_SKIN_VERT_GL);
                        if (a7) {
                            GFX_SetShader(g_SkinShader);
                            GFX_UpdateMatrices(true);
                        } else {
                            GFX_SetShader(g_ShadowmapSkinShader);
                        }
                        GFX_UploadMatrix(GMS_0, *v48);
                        GFX_UploadMatrix(GMS_4, *v49);
                    }
                    if (g_LockedMaterial == i->m_materIdx) {
                        auto ShaderFromFile = g_skinShader_locked;
                        if (g_skinShader_locked == -1) {
                            ShaderFromFile = GFX_CreateShaderFromFile("skinShader_locked", g_skinShader_locked);
                            g_skinShader_locked = ShaderFromFile;
                        }
                        GFX_SetShader(ShaderFromFile);
                    } else {
                        if (a7) {
                            if ((g_HologramTest || g_HologramMaterial != -1 || g_PowerupTest != -1) && !a8)
                                GFX_SetCullMode(GFC_NONE);
                            if (i->m_materIdx != g_HologramMaterial && (!g_HologramTest || g_HologramMaterial != -1)) {
                                if (g_PowerupTest == -1) {
                                    auto SkinShader = g_SkinShader;
                                    if (v26->m_ovrSkinShader && v26->m_ovrSkinShader->m_handle != -1)
                                        SkinShader = v26->m_ovrSkinShader->m_handle;
                                    if (GFX_GetCurrentShaderHandle() != SkinShader) {
                                        GFX_SetShader(SkinShader);
                                        GFX_UpdateMatrices(true);
                                        GFX_UploadShaderVEC4(GSR_24, g_SkinColor, 0);
                                        if (v26->m_ovrSkinShader && v26->m_ovrSkinShader->m_handle != -1) {
                                            for (int64_t vi = 0; vi < v26->m_ovrSkinShader->m_vecArrCnt; vi++) {
                                                auto &it = v26->m_ovrSkinShader->m_vecArr[vi];
                                                if (it.m_vecCnt)
                                                    GFX_UploadShaderVEC4ARRAY(it, it.m_vecCnt, it.m_vec, 0);
                                                else
                                                    GFX_UploadShaderVEC4(it, *it.m_vec, 0);
                                            }
                                        }
                                    }
                                } else if (g_PowerupBlinkCycle) {
                                    if (GFX_GetCurrentShaderHandle() != g_SkinShader) {
                                        GFX_SetShader(g_SkinShader);
                                        GFX_UpdateMatrices(true);
                                    }
                                } else {
                                    GFX_SetShader(g_PowerupInvisShaderHandle);
                                    GFX_UpdateMatrices(1);
                                    if (!a8)
                                        GFX_SetCullMode(GFC_BACK);
                                }
                            } else {
                                GFX_SetShader(g_SkinShaderHologram);
                                GFX_UpdateMatrices(1);
                                if (!a8)
                                    GFX_SetCullMode(GFC_BACK);
                                GFX_UploadShaderVEC4(GSR_24, g_SkinHologramOverrideColor, 0);
                            }
                        } else if ((unsigned int)(g_GFX_rp - 2) > 1) {
                            if (GFX_GetCurrentShaderHandle() != g_SkinShader) {
                                GFX_SetShader(g_SkinShader);
                                GFX_UpdateMatrices(true);
                            }
                            GFX_UploadShaderVEC4(GSR_24, g_SkinColor, 0);
                        }
                    }
                    GDEMESH_UploadBoneMatrices(i, a4);
                    if (a7) {
                        bool v41 = false;
                        if (g_FacialHairMaterialIndex >= 0 && g_FacialHairMaterialIndex == i->m_materIdx) {
                            v41 = true;
                            MATERIAL_SetUVOffsetOverride(true, g_FacialHairUVOffset, g_FacialHairUVOffset1, false);
                        }
                        MATERIAL_Activate(v26, v26->m_pUsage, nullptr);
                        if (v41)
                            MATERIAL_SetUVOffsetOverride(false, 0.0f, 0.0f, false);
                    }
                    if (g_HologramTest || g_PowerupTest != -1)
                        GFX_SetAlphaBlendEnable(true);
                    GFX_SetBlendFunc(GBO_FUNC_ADD, GB_SRC_ALPHA, GB_ONE_MINUS_SRC_ALPHA);
                    GDEMESH_RenderSkinMesh_Group(v11, i);
                }
            }
        }
        GFX_MatrixMode(GMT_0);
        GFX_PopMatrix();
        GFX_SetCullMode(cm);
    }
}