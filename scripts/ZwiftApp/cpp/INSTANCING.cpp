#include "ZwiftApp.h"
std::vector<InstancedObjects *> g_InstancedObjectsLists;
InstancedObjects::InstancedObjects() {
    g_InstancedObjectsLists.push_back(this);
}
InstancedObjects::~InstancedObjects() {
    if (!m_detached) {
        for (auto i = g_InstancedObjectsLists.begin(); i != g_InstancedObjectsLists.end(); i++) {
            if (*i == this) {
                g_InstancedObjectsLists.erase(i);
                break;
            }
        }
    }
}
int g_InstancingMutex = -1;
void vec_x_mx(VEC3 *ret, const VEC3 &vec, const MATRIX44 &mx) {
    auto x = vec.m_data[0], y = vec.m_data[1], z = vec.m_data[2];
    ret->m_data[0] = mx.m_data[0].m_data[0] * x + mx.m_data[1].m_data[0] * y + mx.m_data[2].m_data[0] * z + mx.m_data[3].m_data[0];
    ret->m_data[1] = mx.m_data[0].m_data[1] * x + mx.m_data[1].m_data[1] * y + mx.m_data[2].m_data[1] * z + mx.m_data[3].m_data[1];
    ret->m_data[2] = mx.m_data[0].m_data[2] * x + mx.m_data[1].m_data[2] * y + mx.m_data[2].m_data[2] * z + mx.m_data[3].m_data[2];
}
void INSTANCING_AddInstance(InstancedObjects * /*always g_WorldProps*/, int key, const MATRIX44 &mx, const VEC4 &vec) {
    static_assert(sizeof(WorldProp) == 92);
    static_assert(sizeof(Sphere) == 16);
    switch (GFX_GetPerformanceGroup()) {
    case GPG_BASIC:
        g_maxBillboardPixelSize = 20.0f;
        g_billboardFadeBandPixels = 10.0f;
        break;
    case GPG_MEDIUM:
        g_maxBillboardPixelSize = 15.0f;
        g_billboardFadeBandPixels = 8.0f;
        break;
    case GPG_HIGH:
        g_maxBillboardPixelSize = 10.0f;
        g_billboardFadeBandPixels = 5.0f;
        break;
    default: //GPG_ULTRA
        g_maxBillboardPixelSize = 6.0f;
        g_billboardFadeBandPixels = 3.0f;
        break;
    }
    auto existProp = g_WorldProps.m_pmap[key];
    if (existProp && existProp->m_gdeHandle != -1) {
        Sphere meshBounds;
        GDEMESH_GetMeshBounds(&meshBounds, existProp->m_gdeHandle);
        WorldProp wp{ .m_mx = mx };
        vec_x_mx(&wp.m_bounds.m_center, meshBounds.m_center, mx);
        auto v61 = mx.m_data[0].m_data[0], v59 = mx.m_data[1].m_data[0], v56 = mx.m_data[2].m_data[0];
        auto v19 = mx.m_data[0].m_data[1], v15 = mx.m_data[1].m_data[1], v17 = mx.m_data[2].m_data[1];
        auto v62 = mx.m_data[0].m_data[2], v60 = mx.m_data[1].m_data[2], v57 = mx.m_data[2].m_data[2];
        auto scale2 = sqrtf(v17 * v17 + v56 * v56 + v57 * v57);
        auto scale = fmaxf(sqrtf(v15 * v15 + v59 * v59 + v60 * v60), sqrtf(v19 * v19 + v61 * v61 + v62 * v62));
        if (scale <= scale2)
            scale = scale2;
        wp.m_bounds.m_radius = scale * meshBounds.m_radius;
        vec_x_mx(&wp.m_bounds.m_center, meshBounds.m_center, mx);
        VEC2 loc{ wp.m_bounds.m_center.m_data[0], wp.m_bounds.m_center.m_data[2] };
        if (existProp->m_flags & 0x10) {
            auto tileID = HeightMapManager::GetInst()->GetTerrainTileIDAtLocation(loc);
            zassert(tileID < 256);
            wp.m_tileID = (uint8_t)tileID;
        } else {
            wp.m_tileID = -1;
        }
        auto cenx = wp.m_bounds.m_center.m_data[0], ceny = wp.m_bounds.m_center.m_data[1], cenz = wp.m_bounds.m_center.m_data[2];
        auto ao = 1.0f;
        if (g_pGameWorld->WorldID() != WID_LONDON && fabs(HeightMapManager::GetInst()->GetHeightAtLocation(loc) - ceny) < 500.0f)
            ao = HeightMapManager::GetInst()->GetAOAtLocation(loc);
        // OCCLUSION_GetCellIDForPosition inlined
        wp.m_packedCoords = int(vec.m_data[0] * 255.0f) | ((int(vec.m_data[1] * 255.0) | (( (int(ao * 255.0f) << 8) | int(vec.m_data[2] * 255.0f)) << 8)) << 8);
        auto v36 = (cenx - g_worldXMin) / (g_worldXMax - g_worldXMin) * 32.0f;
        auto v37 = (cenz - g_worldZMin) / (g_worldZMax - g_worldZMin) * 16.0f;
        if (v36 < 0.0f || v36 >= 32.0f || v37 < 0.0f || v37 >= 16.0f) {
            wp.m_cellId = 0xFFFF;
        } else {
            int cellID = (int)v36 + 32 * (int)v37;
            zassert(cellID < 65535);
            wp.m_cellId = cellID;
        }
        auto v39 = v36 / 4.0f, v40 = v37 / 2.0f;
        auto v41 = (int)v39 + 8 * (int)v40;
        if (v39 < 0.0f || v39 >= 8.0f || v40 < 0.0f || v40 >= 8.0f || (unsigned int)v41 > 0x3F) {
            existProp->m_ovfWpCont.push_back(wp);
            existProp->m_ovfSphereCont.push_back(wp.m_bounds);
            existProp->m_ovfFltCont.push_back(0.0f);
        } else {
            existProp->m_mins[v41].m_data[0] = fminf(cenx - wp.m_bounds.m_radius, existProp->m_mins[v41].m_data[0]);
            existProp->m_maxs[v41].m_data[0] = fmaxf(cenx + wp.m_bounds.m_radius, existProp->m_maxs[v41].m_data[0]);
            existProp->m_maxs[v41].m_data[1] = fmaxf(ceny + wp.m_bounds.m_radius, existProp->m_maxs[v41].m_data[1]);
            existProp->m_mins[v41].m_data[1] = fminf(ceny - wp.m_bounds.m_radius, existProp->m_mins[v41].m_data[1]);
            existProp->m_maxs[v41].m_data[2] = fmaxf(cenz + wp.m_bounds.m_radius, existProp->m_maxs[v41].m_data[2]);
            existProp->m_mins[v41].m_data[2] = fminf(cenz - wp.m_bounds.m_radius, existProp->m_mins[v41].m_data[2]);
            existProp->m_worldProps[v41].push_back(wp);
            existProp->m_sphereConts[v41].push_back(wp.m_bounds);
            existProp->m_fltConts[v41].push_back(0.0f);
        }
    }
}
uint32_t INSTANCING_AddMesh(InstancedObjects *a1, const char *a2, uint64_t a3, float a4, BillBoardedSpriteInfo *a5, const VEC4 &a6) {
    //TODO
    return 0;
}
void PropType::clear() {
    m_ovfFltCont.clear();
    m_ovfSphereCont.clear();
    m_ovfWpCont.clear();
    //TODO: clear field_1878 container
    for (auto &f : m_fltConts)
        f.clear();
    for (auto &s : m_sphereConts)
        s.clear();
    for (auto &w : m_worldProps)
        w.clear();
    for (auto &f : m_mins)
        f.m_data[0] = f.m_data[1] = f.m_data[2] = 9.9999998e10f;
    for (auto &f : m_maxs)
        f.m_data[0] = f.m_data[1] = f.m_data[2] = -9.9999998e10f;
}
void INSTANCING_ClearInstances() {
    for (auto &i : g_BikeInstancedObjects.m_plist)
        i.clear();
}
int g_DithTex = -1;
uint8_t g_bytesDitherTex[64] = {
  255, 127,  31, 159,   7, 135,  39, 167, 191,  63, 223,  95, 199,  71, 231, 103,  47, 175,  15, 143, 55, 183,  23, 151, 239, 111, 207,  79, 247, 119,
  215,  87,  11, 139,  43, 171,   3, 131,  35, 163, 203,  75, 235, 107, 195,  67, 227,  99,  59, 187, 27, 155,  51, 179,  19, 147, 251, 123, 219,  91, 243, 115, 211,  83
};
void GFXAPI_CreateTextureFromR(int hnd, GLsizei w, GLsizei h, const GLvoid *d) {
    auto pGlid = &g_Textures[hnd].m_glid;
    glGenTextures(1, pGlid);
    glBindTexture(GL_TEXTURE_2D, *pGlid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGetError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, d);
    auto sq = h * w;
    if (glGetError()) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, d);
        g_VRAMBytes_Textures += 4 * sq;
    } else {
        g_VRAMBytes_Textures += sq;
    }
}
int GFX_CreateTextureFromLuminanceR8(GLsizei w, GLsizei h, const GLvoid *d) {
    auto v3 = g_nTexturesLoaded;
    if (g_nTexturesLoaded >= _countof(g_Textures))
        return -1;
    ++g_nTexturesLoaded;
    if (v3 == -1)
        return -1;
    g_Textures[v3].m_loaded &= ~1u;
    g_Textures[v3].m_bestWidth = w;
    g_Textures[v3].m_bestHeight = h;
    g_Textures[v3].m_align = 2;
    g_Textures[v3].m_field_36_3 = 1;
    g_Textures[v3].m_texState = TS_LOADED;
    GFXAPI_CreateTextureFromR(v3, w, h, d);
    return v3;
}
void INSTANCING_CreateDitherTex() {
    if (g_DithTex == -1) {
        g_DithTex = GFX_CreateTextureFromLuminanceR8(8, 8, g_bytesDitherTex);
        GFX_SetTextureFilter(0, GFF_NEAFEST);
    }
}
void INSTANCING_EndFrame(bool) {
    //empty in Android
}
void INSTANCING_OptimizeGrids() {
    INSTANCING_TakeMutex();
    for (auto &i : g_WorldProps.m_plist) {
        int cnt = 0, idx = 0;
        for (auto &w : i.m_worldProps) {
            if (w.size()) {
                if (idx != cnt) {
                    i.m_fltConts[cnt].swap(i.m_fltConts[idx]); i.m_fltConts[idx].clear();
                    i.m_sphereConts[cnt].swap(i.m_sphereConts[idx]); i.m_sphereConts[idx].clear();
                    i.m_worldProps[cnt].swap(i.m_worldProps[idx]); i.m_worldProps[idx].clear();
                    i.m_mins[cnt] = i.m_mins[idx];
                    i.m_maxs[cnt] = i.m_maxs[idx];
                }
                cnt++;
            }
            idx++;
        }
        i.m_cnt = cnt;
    }
    ZwiftLeaveCriticalSection(g_InstancingMutex);
}
void INSTANCING_RenderAll(InstancedObjects *, GFX_RenderPass, int) {
    //TODO
}
bool INSTANCING_TakeMutex() {
    if (g_InstancingMutex == -1)
        g_InstancingMutex = ZMUTEX_Create("INSTANCINGMUTEX");
    return g_InstancingMutex != -1 && ZwiftEnterCriticalSection(g_InstancingMutex);
}
void INSTANCING_UnloadAll() {
    for (auto pIO : g_InstancedObjectsLists) {
        for (auto &prop : pIO->m_plist)
            prop.clear();
        if (pIO != &g_BikeInstancedObjects) {
            pIO->m_plist.clear();
            pIO->m_pmap.clear();
        }
    }
}