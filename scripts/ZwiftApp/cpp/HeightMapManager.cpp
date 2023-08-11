#include "ZwiftApp.h" //READY for testing
void HeightMapManager::AutoStitchTilesHeightmap() {
    for (int i = 0; i < (int)m_heightMapTiles.size(); ++i) {
        for (int v8 = 0; v8 < (int)m_heightMapTiles.size(); ++v8) {
            if (i != v8) {
                auto v10 = m_heightMapTiles[v8];
                auto v11 = m_heightMapTiles[i];
                if (v11->m_minX != v10->m_minX || v10->m_maxX != v11->m_maxX || (v10->m_maxZ != v11->m_minZ && v10->m_minZ != v11->m_maxZ)) {
                    if (v10->m_minZ == v11->m_minZ && v10->m_maxZ == v11->m_maxZ) {
                        if (v11->m_minX == v10->m_maxX) {
                            v11->m_siblingXminus = v10;
                            v10->m_siblingXplus = v11;
                        } else if (v10->m_minX == v11->m_maxX) {
                            v11->m_siblingXplus = v10;
                            v10->m_siblingXminus = v11;
                            auto h = (int)v11->m_rows, w = (int)v11->m_columns;
                            for (int v20 = 0; v20 < h; v20++)
                                v11->m_heights[w * (v20 + 1) - 1] = v10->m_heights[v20 * w];
                        }
                    }
                } else {
                    if (v10->m_maxZ == v11->m_minZ) {
                        v11->m_siblingZminus = v10;
                        v10->m_siblingZplus = v11;
                    } else {
                        v11->m_siblingZplus = v10;
                        v10->m_siblingZminus = v11;
                        auto w = (int)v11->m_columns;
                        for (int v14 = 0; v14 < w; v14++)
                            v11->m_heights[v14 + w * ((int)v11->m_rows - 1)] = v10->m_heights[v14];
                    }
                }
            }
        }
    }
}
void HeightMapManager::Destroy() {
    if (g_pHeightMapManager) // HeightMapManager::~HeightMapManager inlined
        FreeAndNil(g_pHeightMapManager);
}
void HeightMapManager::DoneLoading() {
    delete m_bounds;
    delete m_scaledTileBounds;
    delete m_field_70;
    auto v20 = m_heightMapTiles.size();
    m_bounds = new VEC4[v20];
    m_scaledTileBounds = new VEC4i[v20];
    m_field_70 = new uint8_t[v20];
    m_center.m_data[0] = (m_min.m_data[0] + m_max.m_data[0]) * 0.5f;
    m_center.m_data[1] = (m_min.m_data[1] + m_max.m_data[1]) * 0.5f;
    m_center.m_data[2] = (m_min.m_data[2] + m_max.m_data[2]) * 0.5f;
    AutoStitchTilesHeightmap();
    int i = 0;
    for (auto v15 : m_heightMapTiles) {
        m_bounds[i].m_data[0] = v15->m_centerXZ.m_data[0] - v15->m_dx * 0.5f;
        m_bounds[i].m_data[1] = v15->m_centerXZ.m_data[1] - v15->m_dz * 0.5f;
        m_bounds[i].m_data[2] = v15->m_dz * 0.5f + v15->m_centerXZ.m_data[1];
        m_bounds[i].m_data[3] = v15->m_dx * 0.5f + v15->m_centerXZ.m_data[0];
        m_scaledTileBounds[i].m_data[0] = int(v15->m_centerXscaled - v15->m_columns * 0.5f);
        m_scaledTileBounds[i].m_data[1] = int(v15->m_centerZscaled - v15->m_rows * 0.5f);
        m_scaledTileBounds[i].m_data[2] = int(v15->m_rows * 0.5f + v15->m_centerZscaled);
        m_scaledTileBounds[i].m_data[3] = int(v15->m_columns * 0.5f + v15->m_centerXscaled);
        v15->CalcBounds();
        i++;
    }
    for (auto v18 : m_heightMapTiles) {
        if (g_bUseTextureHeightmaps) {
            v18->GenerateTerrainTextures();
        } else {
            v18->GenerateTerrainVerts();
            v18->UpdateTerrainVerts();
        }
    }
}
float HeightMapManager::GetAOAtLocation(VEC2 loc) {
    zassert(m_heightMapTiles.size() > 0 && "HeightMapManager Asserted; No height map tile initialized");
    if (m_heightMapTiles.empty())
        return -1'000'000.0f;
    for (int i = 0; i < m_heightMapTiles.size(); i++) {
        auto v8 = m_bounds + i;
        if (loc.m_data[0] >= v8->m_data[0] && loc.m_data[0] <= v8->m_data[3] && loc.m_data[1] >= v8->m_data[1] && loc.m_data[1] <= v8->m_data[2])
            return m_heightMapTiles[i]->GetAOAtLocation(loc);
    }
    return 0.0f;
}
float HeightMapManager::GetAOAtLocation(const VEC3 &loc) {
    VEC2 v4{ loc.m_data[0] , loc.m_data[2] };
    auto v6 = fminf(1.0 - fabs(loc.m_data[1] - GetHeightAtLocation(v4)) * 0.0004f, 1.0f);
    if (v6 < 0.0f)
        v6 = 0.0f;
    return v6 * GetAOAtLocation(v4) + (1.0f - v6) * 0.5f;
}
float HeightMapManager::GetHeightAtLocation(VEC2 loc) {
    zassert(m_heightMapTiles.size() > 0 && "HeightMapManager Asserted; No height map tile initialized");
    if (m_heightMapTiles.empty())
        return -1'000'000.0f;
    for (int i = 0; i < m_heightMapTiles.size(); i++) {
        auto v8 = m_bounds + i;
        if (loc.m_data[0] >= v8->m_data[0] && loc.m_data[0] <= v8->m_data[3] && loc.m_data[1] >= v8->m_data[1] && loc.m_data[1] <= v8->m_data[2])
            return m_heightMapTiles[i]->GetHeightAtLocation(loc);
    }
    return 0.0f;
}
HeightMapManager *HeightMapManager::GetInst() {
    if (g_pHeightMapManager)
        g_pHeightMapManager = new HeightMapManager();
    return g_pHeightMapManager;
}
int HeightMapManager::GetTerrainTileIDAtLocation(VEC2 loc) {
    for (int ret = 0; ret < m_heightMapTiles.size(); ret++) {
        auto v5 = m_bounds + ret;
        if (loc.m_data[0] >= v5->m_data[0] && loc.m_data[0] <= v5->m_data[3] && loc.m_data[1] >= v5->m_data[1] && loc.m_data[1] <= v5->m_data[2])
            return ret; //QUEST: strange indexing
    }
    return -1;
}
HeightMapManager::HeightMapManager() {
    HeightMapTile::InitStaticVertexData();
}
bool HeightMapManager::LoadHeightMap(const char *hmFile, const char *aoFile, const VEC3 &wc, int sz) {
    auto tile = new HeightMapTile();
    if (tile->LoadHeightMap((int)m_heightMapTiles.size(), hmFile, sz, wc)) {
        m_min.m_data[0] = fminf(wc.m_data[0] - 120000.0f, m_min.m_data[0]);
        m_min.m_data[2] = fminf(wc.m_data[2] - 120000.0f, m_min.m_data[2]);
        m_min.m_data[1] = fminf(m_min.m_data[1], wc.m_data[1]);
        m_max.m_data[0] = fmaxf(wc.m_data[0] + 120000.0f, m_max.m_data[0]);
        m_max.m_data[2] = fmaxf(wc.m_data[2] + 120000.0f, m_max.m_data[2]);
        m_max.m_data[1] = fmaxf(wc.m_data[1], m_max.m_data[1]);
        m_dx = m_max.m_data[0] - m_min.m_data[0];
        m_dz = m_max.m_data[2] - m_min.m_data[2];
        auto tmp = int(m_min.m_data[0] * 0.0021333334f);
        if (m_scaledBounds.m_data[0] >= tmp)
            m_scaledBounds.m_data[0] = tmp;
        tmp = int(m_min.m_data[2] * 0.0021333334f);
        if (m_scaledBounds.m_data[1] >= tmp)
            m_scaledBounds.m_data[1] = tmp;
        tmp = int(m_max.m_data[0] * 0.0021333334f);
        if (m_scaledBounds.m_data[2] <= tmp)
            m_scaledBounds.m_data[2] = tmp;
        tmp = int(m_max.m_data[2] * 0.0021333334f);
        if (m_scaledBounds.m_data[3] <= tmp)
            m_scaledBounds.m_data[3] = tmp;
        m_scaledDX = m_scaledBounds.m_data[2] - m_scaledBounds.m_data[0];
        m_scaledDZ = m_scaledBounds.m_data[3] - m_scaledBounds.m_data[1];
        if (tile->LoadAOData(aoFile)) {
            m_heightMapTiles.push_back(tile);
            return true;
        }
    }
    delete tile;
    return false;
}
struct HeightMapTileSorter : VEC2 {
    bool operator()(HeightMapTile *a, HeightMapTile *b) const {
        //return a < b:
        auto da = a->m_centerXZ - *this;
        auto db = b->m_centerXZ - *this;
        return da.lenSquared() - db.lenSquared();
    }
};
void HeightMapManager::Render(int a2) {
    zassert(m_heightMapTiles.size() > 0 && "HeightMapManager Asserted; No height map tile initialized");
    if (m_heightMapTiles.size() > 0) {
        auto cam = g_CameraManager.GetSelectedCamera();
        HeightMapTileSorter v20{ cam->m_pos.m_data[0], cam->m_pos.m_data[2] };
        std::vector<HeightMapTile *> heightMapTiles(m_heightMapTiles);
        std::sort(heightMapTiles.begin(), heightMapTiles.end(), v20);
        bool rp = g_GFX_rp > 3 || g_GFX_rp < 2;
        if (rp)
            GFX_SetShadowParameters(240'000.0f);
        if (a2 < 0)
            a2 = -1;
        for (auto v13 : heightMapTiles) {
            if (g_pGameWorld->m_WorldID == WID_WATOPIA && rp) {
                int v17;
                switch (v13->m_idx) {
                case 3: case 2: case 13: case 14: case 15: case 16:
                    v17 = g_bUseTextureHeightmaps ? g_WatopiaSpecialTileShaderHeightmap : g_WatopiaSpecialTileShader;
                    break;
                default:
                    v17 = g_bUseTextureHeightmaps ? g_WorldShaderTerrainHeightMapHandle : g_WorldShaderTerrainHandle;
                    break;
                }
                GFX_SetShader(v17);
            }
            v13->Render(a2);
        }
    }
}
HeightMapManager::~HeightMapManager() {
    for (auto i : m_heightMapTiles)
        delete i;
    delete m_bounds;
    delete m_scaledTileBounds;
    delete m_field_70;
}
void HeightMapTile::InitStaticVertexData() {
    for (int i = 0; i < _countof(s_GroundVBOHandles); ++i) { // HeightMapTile::DestroyStaticVertexData(HeightMapTile *this)
        GFX_DestroyBuffer(s_GroundVBOHandles + i);
        GFX_DestroyBuffer(s_GroundIBOHandles + i);
    }
    GFX_DestroyVertex(&s_GroundVAOHandle);
    int v3 = 1;
    GFX_CreateBufferParams cbp1;
    GFX_CreateBufferParams cbp2;
    for (int i = 0; i < _countof(s_TerrainLODVertDimension); ++i) {
        s_nGroundVertices[i] = s_TerrainLODVertDimension[i] * s_TerrainLODVertDimension[i];
        cbp1.m_size = (8 * s_nGroundVertices[i]);
        cbp1.m_pData = malloc(cbp1.m_size);
        auto v5 = (float *)cbp1.m_pData;
        auto v6 = s_TerrainTileVertDimension[i];
        v6 = (v6 + 1) * (v6 - 1);
        auto v25 = (0x200 / v3) >> 3;
        auto v7 = v25 + 1;
        s_nGroundSubtileIndices[i] = 2 * v6;
        s_nGroundIndices[i] = v6 << 7;
        cbp2.m_size = v6 << 9;
        cbp2.m_pData = malloc(cbp2.m_size);
        auto v8 = (int *)cbp2.m_pData;
        int v9 = 0, v10 = 0, v11 = 0, v12 = 0, v13 = 0;
        auto v14_max = (v7 * v7) << 6;
        for(int v14 = 0; v14 < v14_max; v14++) {
            auto v17 = i * 64 + v10 + 8 * v11;
            if (!v12 && !v13) {
                s_GroundSubtileIndexOffset[v17] = v9;
                s_GroundSubtileMinIndex[v17] = v14;
            }
            if (v13 >= v25) {
                if (v12 >= v25)
                    s_GroundSubtileMaxIndex[v17] = v14;
            } else {
                auto v18 = v14 + v7;
                if (!v12)
                    v8[v9++] = v14;
                v8[v9++] = v14;
                v8[v9++] = v18;
                if (v12 >= v25)
                    v8[v9++] = v18;
            }
            int v20 = 512, v21 = 512;
            if (v3 * (v12 + v25 * v10) < 0x1FF)
                v20 = v3 * (v12 + v25 * v10);
            if (v3 * (v13 + v25 * v11) < 0x1FF)
                v21 = v3 * (v13 + v25 * v11);
            ++v12;
            v5[2 * v14] = (float)(v20 - 256) * s_TerrainScale;
            v5[2 * v14 + 1] = (float)(v21 - 256) * s_TerrainScale;
            if (v12 >= v7) {
                ++v13;
                v12 = 0;
                if (v13 >= v7) {
                    ++v10;
                    v13 = 0;
                    if (v10 >= 8) {
                        ++v11;
                        v10 = 0;
                    }
                }
            }
        }
        g_VRAMBytes_VBO += cbp1.m_size;
        s_GroundVBOHandles[i] = GFX_CreateBuffer(cbp1);
        g_VRAMBytes_VBO += cbp2.m_size;
        s_GroundIBOHandles[i] = GFX_CreateBuffer(cbp2);
        free(cbp1.m_pData);
        free(cbp2.m_pData);
        v3 *= 2;
    }
    GFX_CreateVertexParams cvp{};
    cvp.m_stridesCnt = 1;
    if (g_bUseTextureHeightmaps) {
        cvp.m_attrCnt = 1;
        cvp.m_attrs[0].m_fmtIdx = GVF_FLOAT6;
        cvp.m_strides[0].m_strideCnt = 8;
        cvp.m_strides[0].m_field_2 = true;
    } else {
        cvp.m_attrCnt = 3; 
        cvp.m_attrs[0].m_fmtIdx = GVF_FLOAT7;
        cvp.m_attrs[1].m_atrIdx = 4;
        cvp.m_attrs[1].m_fmtIdx = GVF_UNSIGNED_BYTE1;
        cvp.m_attrs[1].m_dataOffset = 12;
        cvp.m_attrs[2].m_atrIdx = 1;
        cvp.m_attrs[2].m_fmtIdx = GVF_UNSIGNED_BYTE1;
        cvp.m_attrs[2].m_dataOffset = 16;
        cvp.m_strides[0].m_strideCnt = 0x14;
        cvp.m_strides[0].m_field_2 = true;
    }
    if (s_GroundVAOHandle == -1)
        s_GroundVAOHandle = GFX_CreateVertex(cvp);
}
float HeightMapTile::GetTexelAO(int localX, int localY) {
    zassert(localX >= 0 && localY >= 0 && "HeightMapTile::GetTexelHeight Asserted; texel index cannot be negative, it is"
        " global texel position instead of local?");
    if (localX < 0 || localY < 0 || localX >= (int)this->m_columns || localY >= (int)this->m_rows)
        return 0.0f;
    auto v7 = (float)m_AOData[localX + localY * (int)this->m_columns];
    return v7 * v7 / 65025.0f;
}
float HeightMapTile::GetAOAtLocation(VEC2 loc) {
    auto v3 = (loc.m_data[0] - m_worldCenter.m_data[0] + m_dx * 0.5f) * m_columns / m_dx;
    auto v4 = (int)v3;
    auto v5 = (loc.m_data[1] - m_worldCenter.m_data[2] + m_dz * 0.5f) * m_rows / m_dz;
    auto v6 = v3 - v4;
    auto v7 = (int)v5;
    auto v8 = v5 - (int)v5;
    if (v6 == 0.0f && v8 == 0.0f)
        return GetTexelAO(v4, v7);
    auto v10 = int(v3 + 1.0f);
    auto v11 = int(v5 + 1.0f);
    auto v12 = fmaxf(GetTexelAO(v4, v7), 0.0f);
    auto v13 = fmaxf(GetTexelAO(v10, v7), 0.0f);
    auto v14 = fmaxf(GetTexelAO(v4, v11), 0.0f);
    auto TexelHeight = GetTexelAO(v10, v11);
    auto v16 = 1.0f - v8;
    auto v17 = 1.0f - v6;
    auto v18 = fmaxf(TexelHeight, 0.0f);
    auto v19 = v18 * v8;
    auto v20 = (1.0f - v8) * v12;
    auto v21 = (v18 + v12) * 0.5f;
    if (1.0f - v8 + v6 <= 1.0f)
        return ((v21 - v14 + v21) * v16 + v19) * v6 + (v14 * v8 + v20) * v17;
    else
        return ((v21 - v13 + v21) * v8 + v20) * v17 + (v16 * v13 + v19) * v6;
}
float HeightMapTile::GetTexelHeight(int localX, int localY) {
    auto this_ = this;
    int w = 0, h = 0;
    HeightMapTile *fnd = nullptr;
    while (1) {
        while (1) {
            while (1) {
                while (1) {
                    fnd = this_;
                    if (localX >= 0)
                        break;
                    this_ = this_->m_siblingXminus;
                    if (!this_)
                        break;
                    localX += (int)this_->m_columns;
                }
                if (localY >= 0)
                    break;
                this_ = fnd->m_siblingZminus;
                if (!this_)
                    break;
                localY += (int)this_->m_rows;
            }
            w = (int)fnd->m_columns;
            if (localX < w)
                break;
            this_ = fnd->m_siblingXplus;
            if (!this_)
                break;
            localX -= (int)this_->m_columns;
        }
        h = (int)fnd->m_rows;
        if (localY < h)
            break;
        this_ = fnd->m_siblingZplus;
        if (!this_)
            break;
        localY -= (int)this_->m_rows;
    }
    if (localX < 0 || localY < 0 || localX >= w || localY >= h)
        return 0.0f;
    else
        return fnd->m_heights[localX + localY * w];
}
float HeightMapTile::GetHeightAtLocation(VEC2 loc) {
    auto v3 = (loc.m_data[0] - m_worldCenter.m_data[0] + m_dx * 0.5f) * m_columns / m_dx;
    auto v4 = (int)v3;
    auto v5 = (loc.m_data[1] - m_worldCenter.m_data[2] + m_dz * 0.5f) * m_rows / m_dz;
    auto v6 = v3 - v4;
    auto v7 = (int)v5;
    auto v8 = v5 - (int)v5;
    if (v6 == 0.0f && v8 == 0.0f)
        return GetTexelHeight(v4, v7);
    auto v10 = int(v3 + 1.0f);
    auto v11 = int(v5 + 1.0f);
    auto v12 = fmaxf(GetTexelHeight(v4, v7), 0.0f);
    auto v13 = fmaxf(GetTexelHeight(v10, v7), 0.0f);
    auto v14 = fmaxf(GetTexelHeight(v4, v11), 0.0f);
    auto TexelHeight = GetTexelHeight(v10, v11);
    auto v16 = 1.0f - v8;
    auto v17 = 1.0f - v6;
    auto v18 = fmaxf(TexelHeight, 0.0f);
    auto v19 = v18 * v8;
    auto v20 = (1.0f - v8) * v12;
    auto v21 = (v18 + v12) * 0.5f;
    if (1.0f - v8 + v6 <= 1.0f)
        return ((v21 - v14 + v21) * v16 + v19) * v6 + (v14 * v8 + v20) * v17;
    else
        return ((v21 - v13 + v21) * v8 + v20) * v17 + (v16 * v13 + v19) * v6;
}
bool HeightMapTile::LoadHeightMap(int idx, const char *fileName, int sz, const VEC3 &worldCenter) {
    bool ret = false;
    if (fileName) {
        fileName = GAMEPATH(fileName);
        struct _stat64i32 st;
        _stat64i32(fileName, &st);
        m_time = st.st_mtime;
        if (m_heights)
            delete[] m_heights;
        m_rows = m_columns = sz;
        auto fileLength = sizeof(float) * sz * sz;
        m_heights = new float[sz * sz];
        memset(m_heights, 0, fileLength);
        int nameOffset = 0;
        if (strstr(fileName, "data/") == fileName)
            nameOffset = 5;
        auto wh = g_WADManager.GetWadFileHeaderByItemName(fileName + nameOffset, WAD_ASSET_TYPE::GLOBAL, nullptr);
        if (wh) {
            if (fileLength >= wh->m_fileLength)
                fileLength = wh->m_fileLength;
            memmove(m_heights, wh->FirstChar(), fileLength);
            ret = true;
        }
        m_worldCenter = worldCenter;
        zassert(ret && "HeightMapManager::LoadHeightMap Asserted; possible data corruption in WAD or loose heightmap file");
        if (ret) {
            m_centerXZ.m_data[0] = worldCenter.m_data[0];
            m_centerXZ.m_data[1] = worldCenter.m_data[2];
            m_dx = 240'000.0f;
            m_dz = 240'000.0f;
            m_centerXscaled = worldCenter.m_data[0] * 0.0021333334f;
            m_centerZscaled = worldCenter.m_data[2] * 0.0021333334f;
            m_minX = worldCenter.m_data[0] - 120'000.0f;
            m_minZ = m_centerXZ.m_data[1] - 120'000.0f;
            m_boundMin3.m_data[1] = m_boundMax3.m_data[0] = m_maxX = worldCenter.m_data[0] + 120'000.0;
            m_boundMax3.m_data[2] = m_maxZ = m_centerXZ.m_data[1] + 120'000.0f;
            m_boundMin3.m_data[0] = worldCenter.m_data[0] - 120'000.0f;
            m_boundMin3.m_data[2] = m_centerXZ.m_data[1] - 120'000.0f;
            m_boundMax3.m_data[1] = 100'000.0f;
            m_idx = idx;
        }
    }
    return ret;
}
bool CMD_ExportTerrain(const char *) {
    char Buffer[32];
    int v0 = 0;
    for (auto &i : HeightMapManager::GetInst()->m_heightMapTiles) { // HeightMapTile::ExportAllTiles inlined
        sprintf_s(Buffer, "exported_tile_%d.obj", v0++);
        i->ExportOBJ(Buffer);
        ++v0;
    }
    return true;
}
HeightMapTile::HeightMapTile() {
    for (auto &i : m_texs2)
        i = -1;
    for (auto &i : m_texs6)
        i = -1;
    for (auto &i : m_rangesY)
        for(auto &j : i)
            j = VEC2{ 1'000'000'000.0f, -1'000'000'000.0f };
    for (auto &i : m_buffers)
        i = -1;
    CONSOLE_AddCommand("export_terrain", CMD_ExportTerrain, nullptr, nullptr, nullptr);
    //static m_spheres = malloc(0x400ui64);
}
void HeightMapTile::ExportOBJ(const char *fileName) {
    FILE *f;
    fopen_s(&f, fileName, "w+");
    if (f) {
        fprintf(f, "g default \n");
        int v4 = 0;
        auto v5 = -m_dx * 0.5f;
        auto v7 = -m_dz * 0.5f;
        for (auto v8 = int(v5 / s_TerrainScale); v8 < 256; ++v8) {
            for (auto v11 = int(v7 / s_TerrainScale); v11 < 256; ++v11) {
                auto v14 = v8 * s_TerrainScale + m_worldCenter.m_data[0];
                auto v15 = (v8 + 1) * s_TerrainScale + m_worldCenter.m_data[0];
                auto v16 = v11 * s_TerrainScale + m_worldCenter.m_data[2];
                if (v8 == int(1.0f / s_TerrainScale * v5) || v8 == 255 || v11 == int(1.0f / s_TerrainScale * v7) || v11 == 255) {
                    fprintf(f, "v %f %f %f \n", v14, 0.0, v16);
                    fprintf(f, "v %f %f %f \n", v15, 0.0, v16);
                } else {
                    fprintf(f, "v %f %f %f \n", v14, GetHeightAtLocation({ v14, v16 }), v16);
                    fprintf(f, "v %f %f %f \n", v15, GetHeightAtLocation({ v15, v16 }), v16);
                }
                ++v4;
            }
        }
        if (v4 > 3) {
            int v20 = 4, v21 = v4 - 3;
            do {
                fprintf(f, "f %d %d %d\n", v20 - 2, v20, v20 - 1);
                fprintf(f, "f %d %d %d\n", v20 - 1, v20, v20 + 1);
                v20 += 2;
            } while (--v21);
        }
        fclose(f);
    }
}
bool HeightMapTile::LoadAOData(const char *fileName) {
    bool ret = false;
    if (fileName) {
        if (m_AOData)
            delete[] m_AOData;
        auto fileLength = uint32_t(m_rows * m_columns);
        m_AOData = new uint8_t[fileLength];
        memset(m_AOData, 0, fileLength);
        int nameOffset = 0;
        if (strstr(fileName, "data/") == fileName)
            nameOffset = 5;
        auto wh = g_WADManager.GetWadFileHeaderByItemName(fileName + nameOffset, WAD_ASSET_TYPE::GLOBAL, nullptr);
        if (wh) {
            if (fileLength >= wh->m_fileLength)
                fileLength = wh->m_fileLength;
            memmove(m_AOData, wh->FirstChar(), fileLength);
            ret = true;
        }
    }
    return ret;
}
void HeightMapTile::CalcBounds() {
    for (int i = 0; i < 128; ++i) {
        for (int iz = 0; iz < 8; ++iz) {
            for (int ix = 0; ix < 8; ++ix) {
                for (int h = 0; h < 64; h++) {
                    auto y = m_heights[(i << 12) + (iz << 9) + (ix << 6) + h] + m_worldCenter.m_data[1];
                    m_rangesY[iz][ix].m_data[0] = fminf(y, m_rangesY[iz][ix].m_data[0]);
                    m_rangesY[iz][ix].m_data[1] = fmaxf(y, m_rangesY[iz][ix].m_data[1]);
                    m_minY = fminf(m_minY, y);
                    m_maxY = fmaxf(m_maxY, y);
                }
            }
        }
    }
    auto dx = (m_maxX - m_minX) * 0.125f;
    auto dz = (m_maxZ - m_minZ) * 0.125f;
    auto v44 = fmaxf(dz, dx) * 0.7071f;
    for (int iz = 0; iz < 8; iz++) {
        for (int ix = 0; ix < 8; ix++) {
            auto &r = m_rangesY[iz][ix];
            m_dyNorm[iz][ix] = fmaxf(1.0f - (r.m_data[1] - r.m_data[0]) * 0.00006f, -2.0f);
            m_spheres[iz][ix].m_center.m_data[0] = (ix + 0.5f) * dx + m_minX;
            m_spheres[iz][ix].m_center.m_data[1] = (r.m_data[1] + r.m_data[0]) * 0.5f;
            m_spheres[iz][ix].m_center.m_data[2] = (iz + 0.5f) * dz + m_minZ;
            m_spheres[iz][ix].m_radius = fmaxf((r.m_data[1] - r.m_data[0]) * 0.70710677f, v44) * 1.1f;
        }
    }
    m_boundMin3.m_data[0] = m_minX;
    m_boundMin3.m_data[1] = m_minY;
    m_boundMin3.m_data[2] = m_minZ;
    m_boundMax3.m_data[0] = m_maxX;
    m_boundMax3.m_data[1] = m_maxY;
    m_boundMax3.m_data[2] = m_maxZ;
}
void SmoothNormals(uint32_t *tex, int w, int h) {
    if (h > 2 && w > 2) {
        int cnt = w * h * sizeof(uint32_t);
        uint8_t *texCopy = new uint8_t[cnt];
        memmove(texCopy, tex, cnt);
        for (int row = 1; row < h - 1; row++) {
            for (int col = 1; col < w - 1; col++) {
                uint8_t *left = &texCopy[(row * w + col - 1) * 4];
                uint8_t *right = &texCopy[(row * w + col + 1) * 4];
                uint8_t *self = &texCopy[(row * w + col) * 4];
                uint8_t *up = &texCopy[(row * w + col - w) * 4];
                uint8_t *dn = &texCopy[(row * w + col + w) * 4];
                tex[row * w + col] = ((left[0] + right[0] + up[0] + dn[0] + 2 * self[0]) / 6) |
                    (((left[1] + right[1] + up[1] + dn[1] + 2 * self[1]) / 6) << 8) |
                    (self[3] << 16) |
                    (((left[2] + right[2] + up[2] + dn[2] + 2 * self[2]) / 6) << 8);
            }
        }
        delete[] texCopy;
    }
}
void HeightMapTile::GenerateTerrainTextures() {
    GFX_TextureBytes v40;
    int w = (int)this->m_columns, h = (int)this->m_rows, cnt = w * h;
    if (m_texs2[0] == -1) {
        if (g_GNSceneSystem.m_tier) {
            auto words = new int16_t[cnt];
            for (int i = 0; i < cnt; ++i)
                words[i] = int16_t(0.1f * m_heights[i]);
            v40.m_def.m_field_44 = 65537;
            v40.m_def.m_field_48 = 0x10000;
            v40.m_def.m_field_4C = 4;
            v40.m_def.m_field_50 = 0;
            v40.m_def.m_field_52 = 0;
            v40.m_data = words;
            v40.m_def.m_w2 = w;
            v40.m_def.m_w1 = w;
            v40.m_def.m_field_C = 0;
            v40.m_def.m_field_14 = 1;
            v40.m_def.m_h2 = h;
            v40.m_def.m_h1 = h;
            v40.m_def.m_2w = w + w;
            m_texs2[0] = GFX_CreateTexture(v40.m_def, v40, 1);
            GFX_ActivateTexture(m_texs2[0], 14, nullptr, TWM_REPEAT);
            GFX_SetTextureFilter(14, GFF_NEAFEST);
            delete[] words;
        } else {
            auto floats = new float[cnt];
            for (int i = 0; i < cnt; i++)
                floats[i] = 0.1f * m_heights[i];
            m_texs2[0] = GFX_CreateTextureFromLuminanceF32(w, h, floats);
            delete[] floats;
        }
    }
    if (m_texs2[1] == -1) {
        auto v26 = new uint32_t[cnt];
        auto ptr = v26;
        for (int v27 = 0; v27 < h; v27++) {
            for (int i = 0; i < w; ++i) {
                auto v34 = s_TerrainScale * 2.0f;
                auto v35 = GetTexelHeight(i - 1, v27) - GetTexelHeight(i + 1, v27);
                auto v38 = GetTexelHeight(i, v27 - 1) - GetTexelHeight(i, v27 + 1);
                auto v39 = 1.0f / sqrtf(v35 * v35 + v34 * v34 + v38 * v38);
                *ptr++ = uint32_t((v39 * v35 + 1.0) * 127.5f) |
                    (uint32_t((v34 * v39 + 1.0) * 127.5f) << 8) |
                    (uint32_t(GetTexelAO(i, v27) * 255.0f) << 16) |
                    (uint32_t((v39 * v38 + 1.0) * 127.5f) << 24);
            }
        }
        SmoothNormals(v26, w, h);
        m_texs2[1] = GFXAPI_CreateTextureFromRGBA(w, h, v26, false);
        delete[] v26;
    }
}
void HeightMapTile::GenerateTerrainVerts() {
    for (int i = 0; i < _countof(m_buffers); i++) {
        if (m_buffers[i] == -1) {
            auto d = s_TerrainLODVertDimension[i];
            m_buffers[i] = GFX_CreateBuffer({ 20 * d * d, nullptr });
        }
    }
}
void HeightMapTile::Render(int a2) {
    if (!GFX_AABBInCurrentFrustum(m_boundMin3, m_boundMax3, nullptr))
        return;
    auto isInnsbruck = false;
    if (g_pGameWorld->WorldID() == WID_INNSBRUCK) {
        GFX_SetDepthBias(2.0f);
        isInnsbruck = true;
    }
    GFX_MatrixMode(GMT_0);
    GFX_PushMatrix();
    GFX_LoadIdentity();
    GFX_Translate(m_worldCenter);
    GFX_UpdateMatrices(false);
    GFX_GetCurrentShaderHandle();
    if (HeightMapTile::g_useTerrainConfigData) {   // HeightMapTile::ApplyTerrainConfigParms inlined
        int shader = g_BasicTerrainNoSnowShader;
        if (g_bUseTextureHeightmaps)
            shader = (m_field_0 == 1) ? g_BasicTerrainNoSnowHeightMapShader : g_BasicTerrainHeightMapShader;
        else if (m_field_0 != 1)
            shader = g_BasicTerrainShader;
        if (shader != -1) {
            GFX_SetShader(shader);
            SetupLightmaps(1);
            GFX_UploadShaderVEC4({2, "terrainParm1" }, m_terrainVec1, 0);
            GFX_UploadShaderVEC4({3, "terrainParm2" }, m_terrainVec2, 0);
            GFX_UploadShaderVEC4({4, "terrainParm3" }, m_terrainVec3, 0);
        }
    }
    a2 = std::max(3, a2);
    auto wd = g_pGameWorld->GetWorldDef();
    auto toi = wd->m_texOverrides.find(m_texOverrideKey);
    const int offsets[] = {0, 2, 4, 1, 3, 5};
    if (toi != wd->m_texOverrides.end()) {
        for (int i = 0; i < _countof(m_texs6); i++) {
            if (toi->second.m_texName[i].size())
                GFX_ActivateTexture(m_texs6[i], offsets[i], nullptr, TWM_REPEAT);
        }
    }
    if (wd->m_allowWaterCaustics && g_CameraManager.GetCameraPos().m_data[1] < wd->m_waterLevel) {
        GFX_ActivateTexture(g_CausticTexture, -1, nullptr, TWM_REPEAT);
    } else if (g_pGameWorld->WorldID() == WID_CRIT_CITY) {
        GFX_ActivateTexture(g_InnsbruckConcreteTexture, -1, nullptr, TWM_REPEAT);
    }
    if (g_bUseTextureHeightmaps) {
        if (m_texs2[0] == -1 || m_texs2[1] == -1)
            GenerateTerrainTextures();
        GFX_ActivateTexture(m_texs2[0], 0xE, nullptr, TWM_CLAMP_TO_EDGE);
        GFX_ActivateTexture(m_texs2[1], 0xF, nullptr, TWM_CLAMP_TO_EDGE);
    } else if (m_buffers[0] == -1) {
        GenerateTerrainVerts();
        UpdateTerrainVerts();
    }
    GFX_UploadShaderVEC4({1, "tileTint"}, g_Vec4White, 0);
    if (a2 == -1) {
        bool v4 = false;
        static std::map<float, uint32_t> s_heightMapRenderMap[4];
        for (auto &i : s_heightMapRenderMap)
            i.clear();
        //VEC4 v66{ 60'000.0f, 100'000.0f, 140'000.0f, 180'000.0f }; //can't find usage
        auto sc = g_CameraManager.GetSelectedCamera();
        VEC2 campos{ sc->m_pos.m_data[0], sc->m_pos.m_data[2] };
        auto occ = fmaxf((m_maxX - m_minX) * 0.125f, (m_maxZ - m_minZ) * 0.125f) * 0.70709997f;
        float v67[64];
        GFX_SpheresInCurrentFrustumDist(&m_spheres[0][0], 64, v67);
        auto v26 = v67;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (*v26 > 0.0f) {
                    auto cpos = m_spheres[row][col].m_center;
                    auto v34 = (campos - VEC2{cpos.m_data[0], cpos.m_data[2]}).len();
                    for (int i = 0; i < 4; ++i) {
                        if (m_field_3D8) {
                            if (i < m_field_3DC)
                                i = m_field_3DC;
                        }
                        if (v34 < v67[i - 4] - m_dyNorm[row][col] * 40'000.0f) {
                            if (!OCCLUSION_IsBoundsOccluded(cpos, occ)) {
                                s_heightMapRenderMap[i][v34] = col + row * 8;
                                v4 = true;
                            }
                            break;
                        }
                    }
                }
                ++v26;
            }
        }
        if (v4) {
            for (int i = 0; i < 4; i++) {
                GFX_SetVertexBuffer(g_bUseTextureHeightmaps ? s_GroundVBOHandles[i] : m_buffers[i], 0, 0);
                GFX_SetIndexBuffer(s_GroundIBOHandles[i]);
                GFX_SetVertex(s_GroundVAOHandle);
                for(auto &mi : s_heightMapRenderMap[i]) {
                    GFX_DrawIndexedPrimitive(GPT_TRIANGLE_STRIP, 0, s_nGroundSubtileIndices[i], GIF_INT,
                        4 * s_GroundSubtileIndexOffset[64 * i + mi.second]);
                }
            }
        }
    } else {
        SetupVertexArrays(a2);
        GFX_DrawIndexedPrimitive(GPT_TRIANGLE_STRIP, 0, s_nGroundIndices[a2], GIF_INT, 0);
    }
    if (isInnsbruck)
        GFX_SetDepthBias(0.0f);
    GFX_MatrixMode(GMT_0);
    GFX_PopMatrix();
}
void HeightMapTile::SetupVertexArrays(int idx) {
    GFX_SetVertexBuffer(g_bUseTextureHeightmaps ? s_GroundVBOHandles[idx] : m_buffers[idx], 0, 0);
    GFX_SetIndexBuffer(s_GroundIBOHandles[idx]);
    GFX_SetVertex(s_GroundVAOHandle);
}
HeightMapTile::~HeightMapTile() {
    delete[] m_heights;
    delete[] m_AOData;
    //static delete m_spheres;
    DestroyTerrainTextures();
    for (auto &b : m_buffers)
        GFX_DestroyBuffer(&b);
}
void HeightMapTile::DestroyTerrainTextures() {
    for (auto &t : m_texs2) {
        GFX_UnloadTexture(t);
        t = -1;
    }
    for (auto &t : m_texs6) {
        GFX_UnloadTexture(t);
        t = -1;
    }
}
bool HeightMapTile::IsNearTileByWorldPos(float x, float z, float d) {
    auto halfX = m_dx / 2;
    auto halfZ = m_dz / 2;
    return (x + d) >= (m_centerXZ.m_data[0] - halfX) && (x - d) <= (m_centerXZ.m_data[0] + halfX) && (z + d) >= (m_centerXZ.m_data[1] - halfZ) && (z - d) <= (m_centerXZ.m_data[1] + halfZ);
}
void HeightMapTile::LoadOverridenTextures(const TextureOverrides &overr) {
    static_assert(_countof(overr.m_texName) == _countof(m_texs6));
    for (int i = 0; i < _countof(overr.m_texName); i++)
        m_texs6[i] = GFX_CreateTextureFromTGAFile(overr.m_texName->c_str(), -1, true);
}
void HeightMapTile::UpdateTerrainVerts() {
    int v3 = 1;
    for(int v1 = 0; v1 < 4; v1++) {
        if (m_buffers[v1] != -1) {
            auto v63 = GFX_MapBuffer(m_buffers[v1], GFX_MapBufferParams{ uint64_t(20 * s_nGroundVertices[v1]), 0, GFX_MapBufferParams::BUF_WRITE });
            if (v63) {
                auto v5 = ((int)m_rows - 1) / v3;
                int v10 = (0x200 / v3) >> 3, v11 = v10 + 1, v12 = (v11 * v11) << 6, cols = (int)m_columns;
                int v13 = cols / v3 % v10 + v11 * (v5 % v10 + v11 * (cols / v3 / v10 + 8 * (v5 / v10)));
                if (v13) {
                    if (v12 > v13)
                        v12 = v13;
                    int v16 = 0, v17 = 0, v18 = 0, v19 = 0;
                    for (int v20 = 0; v20 < v12; ++v20) {
                        int v21 = 5 * v20;
                        int v22 = 511;
                        *(int *)(v63 + 20 * v20 + 12) = -1;
                        if (v3 * (v16 + v10 * v17) < 0x1FF)
                            v22 = v3 * (v16 + v10 * v17);
                        int v23 = v3 * (v19 + v10 * v18);
                        int v24 = 511;
                        if (v23 < 0x1FF)
                            v24 = v23;
                        int v25 = 512;
                        if (v22 < 511)
                            v25 = v22;
                        int v27 = 512;
                        if (v24 < 511)
                            v27 = v24;
                        auto v28 = (v25 - 256) * s_TerrainScale;
                        auto v35 = (v27 - 256) * s_TerrainScale;
                        *(float *)(v63 + 4 * v21) = v28;
                        *(float *)(v63 + 4 * v21 + 8) = v35;
                        v28 += m_centerXZ.m_data[0];
                        v35 += m_centerXZ.m_data[1];
                        *(float *)(v63 + 4 * v21 + 4) = GetHeightAtLocation(VEC2{ v28, v35 });
                        // GetTexelNormalIncludingRoad inlined
                        int v36 = 512;
                        if (v22 + 1 < 511)
                            v36 = v22 + 1;
                        int v39 = 512;
                        if (v22 - 1 < 511)
                            v39 = v22 - 1;
                        int v42 = 512;
                        if (v24 + 1 < 511)
                            v42 = v24 + 1;
                        auto v44 = (v39 - 256) * s_TerrainScale + m_centerXZ.m_data[0];
                        int v46 = 512;
                        if (v24 - 1 < 511)
                            v46 = v24 - 1;
                        auto v40 = (v36 - 256) * s_TerrainScale + m_centerXZ.m_data[0];
                        auto v49 = m_centerXZ.m_data[1] + (v42 - 256) * s_TerrainScale;
                        auto v50 = (v46 - 256) * s_TerrainScale + m_centerXZ.m_data[1];
                        v35 = GetHeightAtLocation(VEC2{ v44, v35 }) - GetHeightAtLocation(VEC2{ v40, v35 });
                        v50 = GetHeightAtLocation(VEC2{ v28, v50 }) - GetHeightAtLocation(VEC2{ v28, v49 });
                        auto v53 = 1.0f / sqrtf(s_TerrainScale * s_TerrainScale * 4.0f + v35 * v35 + v50 * v50);
                        *(uint32_t *)(v63 + 4 * v21 + 16) = uint32_t((v53 * v35 + 1.0f) * 127.5f) |
                            (uint32_t((v53 * s_TerrainScale * 2.0f + 1.0f) * 127.5f) << 8) |
                            (uint32_t(GetTexelAO(v22, v24) * 255.0f) << 16) |
                            (uint32_t((v53 * v50 + 1.0f) * 127.5f) << 24);
                        v16++;
                        if (v16 >= v11) {
                            v16 = 0;
                            ++v19;
                            if (v19 >= v11) {
                                ++v17;
                                v19 = 0;
                                if (v17 >= 8) {
                                    ++v18;
                                    v17 = 0;
                                }
                            }
                        }
                    }
                }
                GFX_UnmapBuffer(m_buffers[v1]);
            }
        }
        v3 *= 2;
    }
}
VEC3 g_camPos;
bool g_bBypassOcclusion, g_bCurrentOcclusionIsValid;
uint32_t *g_pOcculusionDB, *g_pCurrentOcclusion;
bool OCCLUSION_IsCellOccluded(int a1) {
    if (!g_bBypassOcclusion)
        if (g_bCurrentOcclusionIsValid)
            if (a1 <= 0x1FF)
                if (g_pCurrentOcclusion)
                    return (g_pCurrentOcclusion[a1 >> 5] & (1 << a1)) == 0;
    return false;
}
float g_FOVX = 40.0f;
bool OCCLUSION_IsBoundsOccluded(const VEC3 &a1, float a2) {
    if (!g_bBypassOcclusion && g_FOVX >= 6.0f && a2 <= 20000.0f) {
        if (OCCLUSION_IsPointOccluded(a1)) {
            if (a2 <= 2500.0f)
                return true;
            VEC3 pos{ a1.m_data[0] + a2, a1.m_data[1], a1.m_data[2] };
            if (OCCLUSION_IsPointOccluded(pos)) {
                pos.m_data[0] = a1.m_data[0] - a2;
                if (OCCLUSION_IsPointOccluded(pos)) {
                    pos.m_data[0] = a1.m_data[0];
                    pos.m_data[2] += a2;
                    if (OCCLUSION_IsPointOccluded(pos)) {
                        pos.m_data[2] = a1.m_data[2] - a2;
                        return OCCLUSION_IsPointOccluded(pos);
                    }
                }
            }
        }
    }
    return false;
}
uint32_t *Internal_ODB_GetCellDataForPosition(const VEC3 &pos) {
    if (g_pOcculusionDB) {
        auto v2 = int((pos.m_data[0] - g_worldXMin) / (g_worldXMax - g_worldXMin) * 128.0f);
        auto v3 = int((pos.m_data[2] - g_worldZMin) / (g_worldZMax - g_worldZMin) * 128.0f);
        if (v2 <= 0x7F && v3 <= 0x7F) {
            auto v4 = v2 + (v3 << 7);
            if (v4 >= 0)
                return g_pOcculusionDB + 16 * v4;
        }
    }
    return nullptr;
}
bool OCCLUSION_IsPointOccluded(const VEC3 &pos) {
    if (!g_bBypassOcclusion && g_bCurrentOcclusionIsValid) {
        auto cd = Internal_ODB_GetCellDataForPosition(g_camPos);
        if (cd) {
            auto v3 = int((pos.m_data[0] - g_worldXMin) / (g_worldXMax - g_worldXMin) * 32.0f);
            auto v4 = int((pos.m_data[2] - g_worldZMin) / (g_worldZMax - g_worldZMin) * 16.0f);
            return v3 <= 0x1F && v4 <= 0xF && (((cd[v4] >> v3) & 1) == 0);
        }
    }
    return false;
}
int OCCLUSION_GetCellIDForPosition(const VEC3 &pos) {
    int result = -1;
    auto v4 = (pos.m_data[2] - g_worldZMin) / (g_worldZMax - g_worldZMin) * 16.0f;
    if (v4 < 16.0f) {
        auto v5 = (pos.m_data[0] - g_worldXMin) / (g_worldXMax - g_worldXMin) * 32.0f;
        if (v5 >= 0.0f && v5 < 32.0f && v4 >= 0.0f)
            return (int)v5 + 32 * (int)v4;
    }
    return result;
}
float g_OCCL_HeightRange[16][32][2]; //min-max
void OCCLUSION_Initialize() {
    g_worldXMin = 1'000'000'000.0f;
    g_worldXMax = -1'000'000'000.0f;
    g_worldZMin = 1'000'000'000.0f;
    g_worldZMax = -1'000'000'000.0f;
    for (int v0 = 0; v0 < g_pRoadManager->GetRoadCount(); ++v0) {
        auto v2 = g_pRoadManager->GetRoadSegment(v0);
        if (v2 && !v2->IsPlaceholder()) {
            auto mine = v2->GetMinExtents();
            g_worldXMin = std::min(g_worldXMin, mine.m_data[0]);
            g_worldZMin = std::min(g_worldZMin, mine.m_data[2]);
            auto maxe = v2->GetMaxExtents();
            g_worldXMax = std::max(g_worldXMax, maxe.m_data[0]);
            g_worldZMax = std::max(g_worldZMax, maxe.m_data[2]);
        }
    }
    g_worldXMax += 10'000.0f;
    g_worldZMax += 10'000.0f;
    g_worldZMin -= 10'000.0f;
    g_worldXMin -= 10'000.0f;
    auto dx = (g_worldXMax - g_worldXMin) * 0.03125f;
    auto dz = (g_worldZMax - g_worldZMin) * 0.0625f;
    for (int row = 0; row < 16; ++row) {
        auto curZo = row * dz;
        for (int col = 0; col < 32; col++) {
            g_OCCL_HeightRange[row][col][0] = 9.9999998e10f;
            g_OCCL_HeightRange[row][col][1] = -9.9999998e10f;
            auto curZ = curZo + g_worldZMin;
            auto curX = col * dx + g_worldXMin;
            for (int v13 = 0; v13 < 11; v13++) {
                auto v17 = v13 * 0.1f * dz + curZ;
                for (int v16 = 0; v16 < 11; v16++) {
                    auto h = HeightMapManager::GetInst()->GetHeightAtLocation(VEC2{v16 * 0.1f * dx + curX, v17});
                    if (g_pGameWorld->m_WorldID == WID_WATOPIA && curX < 70'000.0f && curZ > -245'000.0f) {
                        if ((curX > -34'000.0f && curZ < -120'000.0f) || (curX > -282'000.0f && curX < -217'000.0f && curZ > 150'000.0f && curZ < 230'000.0f)) {
                            h = 36'000.0;
                            g_OCCL_HeightRange[row][col][0] = 0.0f;
                            g_OCCL_HeightRange[row][col][1] = 36'000.0f;
                            continue;
                        }
                    }
                    if (h < g_OCCL_HeightRange[row][col][0])
                        g_OCCL_HeightRange[row][col][0] = h;
                    if (h > g_OCCL_HeightRange[row][col][1])
                        g_OCCL_HeightRange[row][col][1] = h;
                }
            }
        }
    }
    if (g_pOcculusionDB) {
        free(g_pOcculusionDB);
        g_pOcculusionDB = nullptr;
        g_pCurrentOcclusion = nullptr;
    }
    g_pOcculusionDB = (uint32_t *)calloc(0x100'000, 1); // _Internal_ODB_ReadFromDisk inlined
    char buf[260];
    sprintf_s(buf, "data/Worlds/world%d/visdb.dat", g_pGameWorld->WorldID());
    auto v23 = fopen(GAMEPATH(buf), "rb");
    if (v23) {
        fread(g_pOcculusionDB, 1i64, 0x100'000, v23);
        fclose(v23);
    }
}
