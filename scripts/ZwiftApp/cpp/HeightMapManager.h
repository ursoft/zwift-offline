#pragma once //READY for testing
struct HeightMapTile { //0x418 bytes
    static inline int s_GroundVBOHandles[]{ -1,-1,-1,-1 }, s_GroundIBOHandles[]{ -1,-1,-1,-1 }, s_GroundVAOHandle{ -1 }, s_nGroundVertices[4], s_nGroundSubtileIndices[4], s_nGroundIndices[4], s_GroundSubtileIndexOffset[256],
        s_GroundSubtileMinIndex[256], s_GroundSubtileMaxIndex[256];
    static inline const uint32_t s_TerrainLODRes[]{ 512,256,128,64 }, s_TerrainTileRes[]{ 64,32,16,8 }, s_TerrainTileVertDimension[]{ 65,33,17,9 }, s_TerrainLODVertDimension[]{ 520,264,136,72 };
    static inline const float s_TerrainScale = 468.75f;
    static inline bool g_useTerrainConfigData;
    HeightMapTile *m_siblingXplus = nullptr, *m_siblingZplus = nullptr, *m_siblingXminus = nullptr, *m_siblingZminus = nullptr;
    uint64_t m_time = 0;
    uint8_t *m_AOData = nullptr;
    float m_dyNorm[8][8]{}; //нормированный перепад высот
    int m_buffers[4], m_texs2[2], m_texs6[6], m_idx /*TODO: enum?*/, m_field_0 = 0, m_texOverrideKey = 0, m_field_3DC = 0;
    float *m_heights = nullptr, m_columns, m_rows, m_dx, m_dz, m_centerXscaled, m_centerZscaled, m_minX, m_minZ, m_maxX, m_maxZ,
        m_minY = 1'000'000'000.0f, m_maxY = -1'000'000'000.0f;
    VEC2 m_rangesY[8][8], m_centerXZ;
    VEC3 m_worldCenter, m_boundMin3, m_boundMax3;
    VEC4 m_terrainVec1, m_terrainVec2, m_terrainVec3;
    Sphere m_spheres[8][8];
    bool m_field_3D8 = false;
    static void InitStaticVertexData();
    float GetAOAtLocation(VEC2 loc);
    float GetTexelHeight(int, int);
    float GetTexelAO(int, int);
    float GetHeightAtLocation(VEC2);
    bool LoadHeightMap(int, const char *, int, const VEC3 &);
    HeightMapTile();
    void ExportOBJ(const char *fileName);
    bool LoadAOData(const char *fileName);
    void CalcBounds();
    void GenerateTerrainTextures();
    void GenerateTerrainVerts();
    void Render(int);
    void SetupVertexArrays(int);
    ~HeightMapTile();
    void DestroyTerrainTextures();
    bool IsNearTileByWorldPos(float, float, float);
    void LoadOverridenTextures(const TextureOverrides &);
    void UpdateTerrainVerts();
/* inlined:
GetTexelNormalIncludingRoad(int,int,bool)
ApplyTerrainConfigParms(int)
DestroyStaticVertexData()
DestroyTerrainVerts(void)
DisableVertexArrays(void) - empty in android
ExportAllTiles(void)
GetTexelNormal(int,int,bool)
IsInTileByWorldPos(float,float)
IsInTileByTexelPos(int,int)
IsTileIntersectingRect(RECT2)
LoadLodClampValues(std::unordered_map<int,int> &)
SetTerrainConfig(bool,VEC4 const&,VEC4 const&,VEC4 const&,uint)*/
};
struct HeightMapManager { //120 bytes (0x78)
    enum SIDE { SIDE_0 };
    static inline HeightMapManager *g_pHeightMapManager;
    std::vector<HeightMapTile *> m_heightMapTiles;
    VEC4 *m_bounds = nullptr; //xz min and max pairs
    VEC4i *m_scaledTileBounds = nullptr;
    uint8_t /*not sure, but 1 byte*/ *m_field_70 = nullptr;
    VEC3 m_center{}, m_min{ 1'000'000'000.0f, 1'000'000'000.0f, 1'000'000'000.0f }, m_max{ -1'000'000'000.0f, -1'000'000'000.0f, -1'000'000'000.0f };
    float m_dx = 0.0f, m_dz = 0.0f, m_scaledDX = 0.0f, m_scaledDZ = 0.0f;
    VEC4i m_scaledBounds{ 1'000'000'000, 1'000'000'000, -1'000'000'000, -1'000'000'000 };
    static HeightMapManager *GetInst();
    HeightMapManager();
    bool LoadHeightMap(const char *, const char *, const VEC3 &, int);
    int GetTerrainTileIDAtLocation(VEC2);
    float GetHeightAtLocation(VEC2);
    float GetAOAtLocation(VEC2 loc);
    float GetAOAtLocation(const VEC3 &loc);
    void AutoStitchTilesHeightmap();
    void DoneLoading();
    void Render(int);
    static void Destroy();
    ~HeightMapManager();
    /*inlined: 
    void DestroyTerrainGeometry(int);
    void GenerateTerrainGeometry(int);
    void GetHeightTextureForTileID(int);
    void StitchSideHeightmap(HeightMapTile *, HeightMapTile *, HeightMapManager::SIDE);
    void IsInHeightMapByTexelPos(int, int, bool **);
    void IsInHeightMapByTexelPosFast(int, int);
    void IsInHeightMapByTexelRect(int, int, int, int, bool **);
    void IsInHeightMapByWorldPos(float, float, bool **);
    void IsInHeightMapByWorldPosFast(float, float);
    void IsInHeightMapByWorldRect(float, float, float, float, bool **);
    void IsTileSideConnected(HeightMapTile *, HeightMapTile *);*/
};
inline float g_maxBillboardPixelSize, g_billboardFadeBandPixels, g_worldXMin = 1.0e9, g_worldZMin = 1.0e9, g_worldXMax = -1.0e9, g_worldZMax = -1.0e9;
//inlined void OCCLUSION_DoFrame();
//empty void OCCLUSION_GenerateFrame(const VEC3 &, const VEC3 &);
int OCCLUSION_GetCellIDForPosition(const VEC3 &);
//not used void OCCLUSION_GetHeightRnage(int);
void OCCLUSION_Initialize();
bool OCCLUSION_IsBoundsOccluded(const VEC3 &, float);
bool OCCLUSION_IsCellOccluded(int);
bool OCCLUSION_IsPointOccluded(const VEC3 &);
//inlined void OCCLUSION_SetEnabled(bool);
//empty void OCCLUSION_ShutDown();
