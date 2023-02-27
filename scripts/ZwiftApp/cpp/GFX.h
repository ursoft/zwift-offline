#pragma once
enum GFX_RenderPass { GRP_CNT };
enum AssetCategory : uint32_t { AC_UNK, AC_1, AC_2, AC_CNT };
enum GFX_FILL_MODE { GFM_POINT, GFM_LINE, GFM_FILL, GFM_FALSE };
enum GFX_COMPARE_FUNC { GCF_NEVER, GCF_LESS, GCF_EQUAL, GCF_LEQUAL, GCF_GREATER, GCF_NOTEQUAL, GCF_GEQUAL, GCF_ALWAYS };
enum GFX_StencilOp { GSO_FALSE_0, GSO_KEEP, GSO_REPLACE, GSO_INCR, GSO_INCR_WRAP, GSO_DECR, GSO_DECR_WRAP, GSO_INVERT, GSO_FALSE_8, GSO_FALSE_9 };
struct GFX_InitializeParams {
    int WINWIDTH, WINHEIGHT;
    bool field_8, FullScreen, GlCoreProfile, HasPickingBuf;
    uint32_t GlContextVer, VSYNC, GPU, GFX_TIER;
    int PREFERRED_MONITOR;
    char *m_vendorName;
    char *m_renderer;
    int64_t field_38, field_3C;
    int16_t field_44;
};
enum PerformanceGroup { GPG_BASIC, GPG_MEDIUM, GPG_HIGH, GPG_ULTRA, GPG_CNT };
enum GFX_PerformanceFlags { GPF_SMALL_PERF_PENALTY = 1, GPF_BIG_PERF_PENALTY = 2, GPF_NO_AUTO_BRIGHT = 0x4'000'000, GPF_NO_COLOR_CLAMP = 0x8'000'000 };
struct GraphicsCardProfile {
    const char *m_vendorName, *m_renderer;
    PerformanceGroup m_pg;
    uint64_t m_perfFlags; //GFX_PerformanceFlags
};
struct GFX_RegisterRef {
    enum class Ty : uint8_t { Scene, Object, Draw, User, CNT } m_ty;
    uint16_t m_offset, m_cnt;
};
enum GFX_SHADER_REGISTERS { GSR_0 = 0, GSR_CNT = 29 };
enum GFX_SHADER_MATRICES { GSM_0 = 0, GSM_CNT = 9 };
struct GFX_BlendIdxs {
    bool operator == (const GFX_BlendIdxs &peer) { return m_modeIdx == peer.m_modeIdx && m_sFactorIdx == peer.m_sFactorIdx && m_dFactorIdx == peer.m_dFactorIdx; }
    bool operator != (const GFX_BlendIdxs &peer) { return !(*this == peer); }
    uint8_t m_modeIdx, m_sFactorIdx, m_dFactorIdx;
};
struct GFX_StateBlock {
    enum Bits { GSB_PEND_ALTB = 1, GSB_PEND_EAB = 16, GSB_PEND_CULL = 32, GSB_PEND_BLEND = 64 };
    uint64_t m_bits;
    int m_depthTest;            // push[0] as bool
    int m_depthFuncIdx;         // push[1]
    int m_depthMask;            // push[2] as bool
    GFX_FILL_MODE m_fillMode;   // push[3]
    int m_newCullIdx;             // push[4]
    int m_field_1C;             // push[5] as bool
    //char field20[24];
    int m_scissorTest;          // push[12] as bool
    float m_polyOffset;         // push[13]
    //int field3d
    GFX_BlendIdxs m_prBlendIdxs; // push[15]

    int m_cullIdx;              // @push[4]
    bool m_blend;               // @push[5]
    GFX_BlendIdxs m_blendIdxs;  // @push[15]
    int m_vaIdx, m_field_A4;
    uint64_t m_field_A8, m_field_B8, m_hasRegTypes;
    uint8_t *m_attrData;
    int m_actTex, m_arrBuf, m_altArrBuf, m_shader, m_field_C8, m_elArrBuf;
    uint8_t m_colorMask[4], m_stensilRef, m_stensilFuncMask, m_stensilFunc, m_stensilMask;
    GFX_StencilOp m_sopsFail, m_sopdpFail, m_sopdpPass;
    bool m_enableStensilTest;
    void UnbindBuffer(int);
    void SetUniform(const GFX_RegisterRef &, const VEC4 *, uint16_t, uint64_t);
    void SetUniform(const GFX_RegisterRef &, const VEC4 &, uint64_t);
    void SetUniform(const GFX_RegisterRef &ref, const VEC2 *vec, uint16_t sz, uint64_t skipTag);
    void SetUniform(const GFX_RegisterRef &, const MATRIX44 *, uint16_t, uint64_t);
    void SetUniform(const GFX_RegisterRef &, const MATRIX44 &, uint64_t);
    void Reset();
    bool Realize();
    static const VEC4 *GetUniform(GFX_SHADER_REGISTERS);
    static const VEC4 *GetUniform(GFX_SHADER_MATRICES);
    static const VEC4 *GetUniform(const GFX_RegisterRef &);
    void BindVertexBuffer(int);
    enum tagCounts { TCN_SCENE = 46, TCN_OBJECT = 12, TCN_DRAW = 197, TCN_USER = 256 };
    static inline VEC4 bufferRegs_Scene[TCN_SCENE], bufferRegs_Object[TCN_OBJECT], bufferRegs_Draw[TCN_DRAW], bufferRegs_User[TCN_USER];
    static inline uint64_t bufferTags_Scene[TCN_SCENE], bufferTags_Object[TCN_OBJECT], bufferTags_Draw[TCN_DRAW], bufferTags_User[TCN_USER];
    struct Uniform { VEC4 *m_pRegs; uint64_t *m_pTags; uint16_t m_offset; uint16_t m_size; uint32_t m_unk4dw; };
    static inline Uniform uniformRegs[(int)GFX_RegisterRef::Ty::CNT] = { { bufferRegs_Scene, bufferTags_Scene }, { bufferRegs_Object, bufferTags_Object }, { bufferRegs_Draw, bufferTags_Draw }, { bufferRegs_User, bufferTags_User } };
    static const GFX_RegisterRef s_registerRefs[GSR_CNT], s_matrixRefs[GSM_CNT], s_matrixArrayRefs[2];
};
struct GfxCaps {
    int64_t field_48, field_50, max_uniform_block_sz, max_uniform_bo_align, max_sh_sbs;
    int max_texture_size, max_3d_texture_size, max_arr_tex_layers, max_tex_buf, max_compute_wg_cnt0, max_compute_wg_cnt1, max_compute_wg_cnt2,
        max_compute_wg_sz0, max_compute_wg_sz1, max_compute_wg_sz2, max_compute_wg_inv, max_compute_sm_sz;
    bool draw_elements_base_vertex, field_2, ARB_base_instance, instanced_arrays, bglGenVertexArrays, field_6, field_7, field_18, field_19, texture_compression_s3tc,
        shader_group_vote, shader_ballot, clip_control, texture_array, texture_buffer_object, texture_view, texture_swizzle, field_B, field_14, uniform_buffer_object, 
        conservative_depth, shader_storage_bo, compute_shader, tex_cube_map_array, texture_storage;
    uint8_t max_anisotropy, max_v_attribs, max_color_atchs, max_tex_iu, max_v_tex_iu, max_ctex_iu, max_clip_planes, dual_src_drb, max_v_img_uniforms, max_f_img_uniforms,
        max_v_uniform_blocks, max_f_uniform_blocks, max_v_shb, max_f_shb, max_comp_tex_iu, max_comp_uf_b, max_comp_s_b, max_comp_img_uf;
};
namespace GfxConfig {
    inline int gLODBias, gFXAA;
}
namespace GameShaders {
    void LoadAll();
    inline int shGNLinearizeDepth, shGNDownsampleLinearizeDepth, shGNRoadSSR;
}
struct GFX_ShaderPair { //440 bytes
    int m_vshId, m_fshId, m_program;
    int m_attribLocations[12], m_locations[GSR_CNT], m_matLocations[GSM_CNT], m_matArrLocations[2], m_samplers[16], m_field_16C[18];
    uint16_t m_vertIdx, m_fragIdx;
    uint8_t m_field_12C[64], m_modelIndex;
};
struct GFX_VertexAttr { //4 byte
    uint8_t m_idx, m_fmtIdx, m_dataOffset, m_strideIdx;
};
struct GFX_Vertex { //152 bytes
    int64_t m_attrCnt, m_field8;
    GFX_VertexAttr m_attrs[29 /*less maybe*/];
    uint8_t gap[4];
    uint8_t m_strides[16 /*less maybe*/];
};
struct GFX_CreateVertexParams {

};
struct GFX_VertexArray {
    GFX_Vertex *fast64[64];
    uint64_t size;
    GFX_Vertex **extra;
};
struct GFX_CreateShaderParams {
    const char *m_name;
    uint16_t m_vertIdx = 0, m_fragIdx = 0;
};
#pragma pack(push)
#pragma pack(1)
struct TGAX_HEADER {
    uint16_t        wDummy0;
    uint16_t        wDummy2;
    DWORD           dwDummy0;
    DWORD           dwDummy0a;
    uint16_t        wWidth;
    uint16_t        wHeight;
    uint16_t        wType;
};
#pragma pack(pop)
struct GFX_TextureStruct { //64 bytes
    const char *m_name;
    uint32_t m_glid, m_align, m_nameSCRC, m_field_20_5, m_texTime, m_totalBytes;
    uint16_t m_bestWidth, m_bestHeight;
    AssetCategory m_assetCategory;
    uint8_t m_loaded; //bool or bit field?
    uint8_t m_field_36_3, m_fromLevel, m_field_39_0, m_toLevel;
    bool InHardware() const { return m_glid != -1; }
};

inline GFX_TextureStruct g_Textures[0x3000];// 0xC0'000 / 64
inline static uint8_t g_WhiteTexture[0x1000];
inline int g_WhiteHandle, g_nTexturesLoaded;
inline int64_t g_VRAMBytes_Textures;
inline const int g_DrawBufferSize = 0x800'000, MAX_SHADERS = 0x400;
inline GFX_VertexArray g_vertexArray;
inline GFX_ShaderPair g_Shaders[MAX_SHADERS], *g_pCurrentShader;
inline float g_TargetBatteryFPS;
inline size_t g_TotalMemoryInKilobytes;
inline AssetCategory g_CurrentAssetCategory = AC_1;
inline uint64_t g_GFX_PerformanceFlags;
inline PerformanceGroup g_GFX_Performance = GPG_ULTRA;
inline int g_nSkipMipCount;
inline const char *g_GL_vendor = "", *g_GL_renderer = "", *g_GL_apiName = "";
inline bool g_openglDebug, g_openglCore, g_bGFXINITIALIZED, g_bUseEmptyShadowMapsHack;
inline char g_strCPU[0x40];
inline int g_BlurShaderHandle, g_CurrentShaderHandle;
enum DetailedRender { DR_NO, DR_MIDDLE, DR_VERBOSE };
inline DetailedRender g_renderDetailed = DR_VERBOSE;
inline GLFWwindow *g_mainWindow;
inline bool g_MaintainFullscreenForBroadcast = true, g_removeFanviewHints, g_bShutdown, g_WorkoutDistortion, g_openglFail;
inline float g_kwidth, g_kheight, g_view_x, g_view_y, g_view_w, g_view_h;
inline int g_width, g_height, g_MinimalUI, g_bFullScreen, g_nShadersLoaded, g_TotalShaderCreationTime;
inline uint32_t g_glVersion, g_CoreVA, g_UBOs[(int)GFX_RegisterRef::Ty::CNT], g_gfxTier, g_DrawPrimVBO;
inline uint8_t g_colorChannels, g_gfxShaderModel;
inline GfxCaps g_gfxCaps;
inline const GfxCaps &GFX_GetCaps() { return g_gfxCaps; }
inline float g_floatConsts12[12] = { 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 0.0 };
inline GFX_StateBlock g_GFX_CurrentStates[8 /*TODO: maybe more*/], *g_pGFX_CurrentStates = g_GFX_CurrentStates;
inline int g_DrawNoTextureShaderHandle = -1, g_DrawTexturedShaderHandle = -1, g_DrawTexturedSimpleShaderHandle = -1, g_DrawTexturedGammaCorrectShaderHandle = -1;
inline void *g_DrawBuffers[2];
inline std::unordered_map<uint32_t, int> g_ShaderMap; //shaderId (name & params crc) -> shaderHandle (0...MAX_SHADERS)
inline int g_SimpleShaderHandle, g_WorldNoLightingHandle, g_ShadowmapShaderHandle, g_ShadowmapInstancedShaderHandle, g_ShadowmapHairShaderHandle,
    g_TestShaderHandle, g_RoadShader, g_RoadAccessoryShader, g_RoadAccessoryShaderSSR, g_RoadWetShader, g_RoadAccessoryWetShader, g_HeatHazeShader, g_CausticShader, g_CrepuscularHandle,
    g_WorldShaderHandle, g_WorldAOShaderHandle, g_WorldShaderBillboardedHandle, g_WorldShaderShinyHandle, g_WorldShaderSimpleHandle, g_HologramShader,
    g_LockedItemShader, g_LondonTerrainShader, g_LondonTerrainHeightMapShader, g_FranceTerrainShader, g_FranceTerrainHeightMapShader, g_BasicTerrainShader,
    g_BasicTerrainHeightMapShader, g_BasicTerrainNoSnowShader, g_BasicTerrainNoSnowHeightMapShader, g_InnsbruckTerrainHeightMapShader, g_InnsbruckTerrainHandle,
    g_BolognaTerrainHeightMapShader, g_BolognaTerrainShader, g_YorkshireTerrainShader, g_YorkshireTerrainHeightMapShader, g_RichmondTerrainShader, g_RichmondTerrainHeightMapShader,
    g_WorkoutHologramShader, g_WorkoutHologramPrShader, g_FinalCopyShader, g_WorldShaderTerrainHandle, g_WorldShaderTerrainHeightMapHandle, g_ShadowShaderTerrainHeightMap,
    g_WatopiaSpecialTileShaderHeightmap, g_WatopiaSpecialTileShader, g_WorldShaderInstancedHandle, g_WorldShaderInstancedTerrainConformingHandle, g_World2LayerShaderHandle,
    g_VegetationShaderHandle, g_VegetationShaderInstancedTerrainConformHandle, g_VegetationShaderInstancedHandle, g_VegetationShadowmapShaderHandle, g_VegetationShadowmapInstancedShaderHandle,
    g_WireShaderHandle, g_WireShadowShaderHandle, g_BikeShaderInstancedHandle, g_HairShaderHandle, g_SkinShader, g_ShadowmapSkinShader, g_SkinShaderHologram, g_grayScaleShader;
inline bool g_bUseTextureHeightmaps = true;
inline int g_ButterflyTexture, g_RedButterflyTexture, g_MonarchTexture, g_FireflyTexture, g_CausticTexture, g_GrassTexture, g_GravelMtnGrassTexture,
    g_InnsbruckConcreteTexture, g_ParisConcreteTexture, g_DefaultNormalMapNoGloss, g_RoadDustTexture, g_GravelDustTexture, g_SandTexture, g_SandNormalTexture,
    g_RockTexture, g_FranceRockTexture, g_FranceRockNTexture, g_RockNormalTexture, g_ShowroomFloorTexture, g_HeadlightTexture, g_VignetteTexture;
inline int g_TextureTimeThisFrame;

void GFX_DestroyVertex(int *pIdx);
int GFX_CreateVertex(GFX_CreateVertexParams *parms);
void GFXAPI_CreateVertex(int idx, GFX_CreateVertexParams *parms);
void GFX_SetColorMask(uint64_t idx, uint8_t mask);
void GFX_SetStencilRef(uint8_t ref);
void GFX_SetFillMode(GFX_FILL_MODE m);
void GFX_SetStencilFunc(bool, GFX_COMPARE_FUNC, uint8_t, uint8_t, GFX_StencilOp, GFX_StencilOp, GFX_StencilOp);
void GFX_SetLoadedAssetMode(bool);
bool GFX_Initialize();
bool GFXAPI_Initialize(const GFX_InitializeParams &);
bool GFX_Initialize(const GFX_InitializeParams &);
bool GFX_Initialize3DTVSpecs(float, float);
void GFX_DrawInit();
int64_t GFX_GetPerformanceFlags();
void GFX_AddPerformanceFlags(uint64_t f);
PerformanceGroup GFX_GetPerformanceGroup();
void GFX_SetMaxFPSOnBattery(float fps);
const char *GFX_GetVersion();
const char *GFX_GetAPIName();
const char *GFX_GetVendorName();
const char *GFX_GetRendererName();
struct MonitorInfo {
    uint32_t m_dpiX, m_dpiY, m_dpiPhys, m_width, m_height;
    float m_dsf;
};
void GetMonitorCaps(MonitorInfo *dest);
void GFX_MatrixStackInitialize();
void GFXAPI_CalculateGraphicsScore();
void GFX_TextureSys_Initialize();
int GFX_CreateShaderFromFile(const char *fileName, int handle);
int GFX_CreateShaderFromFile(const GFX_CreateShaderParams &s, int handle); //GFX_CreateShaderFromFile_0
int GFXAPI_CreateShaderFromFile(int handle, const GFX_CreateShaderParams &s);
int GFX_Internal_GetNextShaderHandle();
bool GFX_CheckExtensions();
void GFX_PushStates();
void GFX_PopStates();
int GFX_GetCurrentShaderHandle();
void GFX_UnsetShader();
bool GFX_SetShader(int sh);
void GFX_Begin();
uint32_t GFX_ShaderModelValue(int idx);
void GFXAPI_CreateTextureFromRGBA(int idx, uint32_t w, uint32_t h, const void *data, bool genMipMap); //GFXAPI_CreateTextureFromRGBA_idx
int GFXAPI_CreateTextureFromRGBA(uint32_t w, uint32_t h, const void *data, bool genMipMap);
int GFXAPI_CreateShaderFromBuffers(int handle, int vshLength, const char *vshd, const char *vsh, int pshLength, const char *pshd, const char *psh);
inline uint32_t GFX_GetTier() { return g_gfxTier; }
int GFX_CreateShader(const GFX_CreateShaderParams &p);
int GFX_CreateTextureFromTGAFile(const char *name, int handle, bool tryAnimated);
void GFX_SetAnimatedTextureFramerate(int tex, float r);
int GFX_CreateAnimatedTextureFromTGAFiles(const char *name);
void GFXAPI_CreateTexture(int handle, int w, int h, int mipMapLevelIdx);
bool GFX_IsCompressed(uint32_t formatIdx);
void GFXAPI_UpdateTexture(int handle, int level, int w, int h, uint32_t formatIdx, const void *data, int dataBytes);
int GFX_CreateTextureFromTGAX(uint8_t *data, int handle);
int GFX_Internal_LoadTextureFromTGAXFile(const char *name, int handle);