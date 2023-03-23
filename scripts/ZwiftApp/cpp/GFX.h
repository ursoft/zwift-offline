#pragma once
//_mm_shuffle_ps(v66, v66, 255) -> v66[3] in all
//_mm_shuffle_ps(v2, v2, 85) -> v2[1], v2[0], v2[3], v2[2]
//_mm_shuffle_ps(v2, v2, 170) -> v2[2], v2[3], v2[0], v2[1]
enum GFX_RenderPass { GRP_CNT };
enum AssetCategory : uint32_t { AC_UNK, AC_1, AC_2, AC_CNT };
enum GFX_FILL_MODE { GFM_POINT, GFM_LINE, GFM_FILL, GFM_FALSE, GFM_CNT };
enum GFX_COMPARE_FUNC : uint8_t { GCF_NEVER, GCF_LESS, GCF_EQUAL, GCF_LEQUAL, GCF_GREATER, GCF_NOTEQUAL, GCF_GEQUAL, GCF_ALWAYS, GCF_CNT };
enum GFX_StencilOp : uint8_t { GSO_FALSE_0, GSO_KEEP, GSO_REPLACE, GSO_INCR, GSO_INCR_WRAP, GSO_DECR, GSO_DECR_WRAP, GSO_INVERT, GSO_FALSE_8, GSO_FALSE_9, GSO_FALSE_10, GSO_CNT };
enum GFX_MatrixType { GMT_0, GMT_1, GMT_2, GMT_CNT };
enum GFX_PRIM_TYPE { GPT_0, GPT_1, GPT_LINE_STRIP, GPT_TRIANGLES, GPT_TRIANGLE_STRIP, GPT_QUADS, GPT_POLYGON_BIT, GPT_7, GPT_CNT };
enum GFX_CULL { GFC_NONE, GFC_FRONT, GFC_BACK, GFC_CNT };
enum GFX_TEXTURE_WRAP_MODE : uint8_t { TWM_REPEAT, TWM_CLAMP_TO_EDGE, TWM_MIRRORED_REPEAT, TWM_CLAMP_TO_BORDER, TWM_CNT };
enum GFX_FILTER : uint8_t { GFF_NEAFEST, GFF_LINEAR, GFF_LINEAR_MIPMAP_NEAREST, GFF_LINEAR_MIPMAP_LINEAR, GFF_CNT };
enum GFX_BLEND : uint8_t { GB_FALSE, GB_TRUE, GB_SRC_COLOR, GB_ONE_MINUS_SRC_COLOR, GB_SRC_ALPHA, GB_ONE_MINUS_SRC_ALPHA, GB_DST_COLOR, GB_ONE_MINUS_DST_COLOR, GB_DST_ALPHA, 
    GB_ONE_MINUS_DST_ALPHA, GB_CONSTANT_COLOR, GB_ONE_MINUS_CONSTANT_COLOR, GB_CONSTANT_ALPHA, GB_ONE_MINUS_CONSTANT_ALPHA, GB_CNT };
enum GFX_BLEND_OP : uint8_t { GBO_FUNC_ADD, GBO_FUNC_SUBTRACT, GBO_MIN, GBO_MAX, GBO_FUNC_REVERSE_SUBTRACT, GBO_FALSE, GBO_CNT };
enum GFX_IndexFormat { GIF_SHORT, GIF_INT, GIF_CNT };
enum GFX_VertexFormat : uint8_t { GVF_FALSE, GVF_UNSIGNED_BYTE1, GVF_BYTE, GVF_UNSIGNED_BYTE3, GVF_UNSIGNED_SHORT, GVF_FLOAT5, GVF_FLOAT6, GVF_FLOAT7, GVF_CNT };
inline const GLenum g_PRIM_TO_GLPRIM[GPT_CNT] = { GL_FALSE, GL_TRUE, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_QUADS, GL_POLYGON_BIT, GL_FALSE };
inline const GLenum g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[TWM_CNT] = { GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_CLAMP_TO_BORDER };
inline const GLenum g_IF_TO_GLIF[GIF_CNT] = { GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
struct DRAW_VERT_POS_COLOR_1UV { enum { MULT = 24, VAO = 2 }; };
struct DRAW_VERT_POS_COLOR_UV { enum { MULT = 32, VAO = 1 }; };
struct DRAW_VERT_POS_COLOR { //16 bytes 
    enum { MULT = 16, VAO = 0 }; 
    VEC2 m_point;
    int m_dummy;
    uint32_t m_color;
};
struct DRAW_VERT_POS_COLOR_UV_NORM  { enum { MULT = 44, VAO = 3 }; };
struct DRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED { enum { MULT = 32, VAO = 5 };};
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
struct GFX_MatrixContextItem {
    MATRIX44 *m_pStack;
    MATRIX44 *m_matrix;
    int m_mxCount;
    char gap[4];
};
struct GFX_MatrixContext { //0x170 (368) bytes
    GFX_MatrixType m_curMode;
    char field_4;
    bool m_field_5;
    char field_6;
    char field_7;
    MATRIX44 *m_matrix;
    GFX_MatrixContextItem m_stacks[GMT_CNT];
    uint64_t m_modesUpdCnt[GMT_CNT];
    uint64_t field_70;
    uint64_t m_applUpdatesCnt;
    MATRIX44 m_field_80;
    MATRIX44 m_field_C0;
    MATRIX44 m_field_100;
    VEC4 m_field_140;
    uint64_t m_modesUpdCntCache[GMT_CNT];
    char gap[8];
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
enum GFX_SHADER_REGISTERS { GSR_0 = 0, GSR_24 = 24, GSR_CNT = 29 };
enum GFX_SHADER_MATRICES { GSM_0 = 0, GSM_1, GSM_2, GSM_3, GSM_CNT = 9 };
struct GFX_BlendFunc {
    bool operator == (const GFX_BlendFunc &peer) { return m_mode == peer.m_mode && m_srcFactor == peer.m_srcFactor && m_dstFactor == peer.m_dstFactor; }
    bool operator != (const GFX_BlendFunc &peer) { return !(*this == peer); }
    GFX_BLEND_OP m_mode;
    GFX_BLEND m_srcFactor, m_dstFactor, gap;
};
struct GFX_StencilFunc {
    bool m_testEnabled;
    GFX_COMPARE_FUNC m_compFunc;
    uint8_t m_ref, m_funcMask, m_mask;
    GFX_StencilOp m_sfail, m_dpfail, m_dppass;
};
struct GFX_StateBlock { // 0xD0 (208) bytes
    enum Bits { GSB_PEND_VBO = 1, GSB_PEND_ATTRDATA = 2, GSB_PEND_VAO = 4, GSB_PEND_VAIDX = 8, GSB_PEND_IB = 16, GSB_PEND_CULL = 32, GSB_PEND_BLEND = 64 };
    uint64_t m_bits;
    float m_depthTest;
    GFX_COMPARE_FUNC m_depthFunc;
    int m_depthMask;
    GFX_FILL_MODE m_fillMode;
    GFX_CULL m_cullIdx1;
    int m_alphaBlend1;
    char field_20[24];
    int m_scissorTest;
    float m_polyOffset;
    int f_40;
    GFX_BlendFunc m_blendFunc1;
    GFX_FILTER m_filters[8];
    GFX_TEXTURE_WRAP_MODE m_wrapS[8];
    GFX_TEXTURE_WRAP_MODE m_wrapT[8];
    char field_60[16];
    GFX_CULL m_cullIdx2;
    int m_alphaBlend2;
    GFX_BlendFunc m_blendFunc2;
    GFX_StencilFunc m_stencil;
    char m_colorMask[4];
    int m_vertexBuffer;
    int field_8C;
    uint64_t m_attrData;
    int m_indexBuffer;
    int m_vertex;
    uint32_t m_shader;
    uint32_t m_vaIdx;
    uint64_t m_field_A8;
    int m_actTex;
    int m_arrBuf;
    uint64_t m_VAO;
    uint64_t m_hasRegTypes;
    uint32_t m_shaderMAT4ARRAY;
    uint32_t m_has2;

    void UnbindBuffer(int);
    void SetUniform(const GFX_RegisterRef &, const VEC4 &, uint16_t, uint64_t);
    void SetUniform(const GFX_RegisterRef &, const VEC4 &, uint64_t);
    void SetUniform(const GFX_RegisterRef &ref, const VEC2 &vec, uint16_t sz, uint64_t skipTag);
    void SetUniform(const GFX_RegisterRef &, const MATRIX44 &, uint16_t, uint64_t);
    void SetUniform(const GFX_RegisterRef &, const MATRIX44 &, uint64_t);
    void Reset();
    bool Realize();
    static const VEC4 &GetUniform(GFX_SHADER_REGISTERS);
    static const MATRIX44 &GetUniform(GFX_SHADER_MATRICES);
    static const VEC4 &GetUniform(const GFX_RegisterRef &);
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
    inline int dualAlphaShader, shGaussianBlur;
    void LoadAll();
    inline int shGNLinearizeDepth, shGNDownsampleLinearizeDepth, shGNRoadSSR;
}
struct GFX_ShaderPair { //440 bytes
    int m_vshId, m_fshId, m_program;
    int m_attribLocations[12], m_locations[GSR_CNT], m_matLocations[GSM_CNT], m_matArrLocations[2], m_samplers[16], m_texHandles[16], m_field_16C[18];
    uint16_t m_vertIdx, m_fragIdx;
    uint8_t m_modelIndex;
};
struct GFX_VertexAttr { //4 byte
    uint8_t m_strideIdx, m_atrIdx;
    GFX_VertexFormat m_fmtIdx;
    uint8_t m_dataOffset;
};
struct GFX_Stride {
    char m_strideIdx;
    char m_strideCnt;
    char field_2;
    char field_3;
};
struct GFX_CreateVertexParams { //136 (0x88) bytes
    uint64_t m_attrCnt;
    uint64_t m_stridesCnt;
    GFX_VertexAttr m_attrs[16];
    GFX_Stride m_strides[14];
};
struct GFX_Vertex { //152 bytes
    GFX_CreateVertexParams m_creParams;
    uint8_t m_strides[16 /*less maybe*/];
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
    uint32_t        dwDummy0;
    uint32_t        dwDummy0a;
    uint16_t        wWidth;
    uint16_t        wHeight;
    uint16_t        wType;
};
#pragma pack(pop)
enum TEX_STATE { TS_UNLOADED = 0, TS_1, TS_2, TS_LOADED = 5, TS_INVALID = -1 };
struct GFX_TextureStruct { //64 bytes
    const char *m_name;
    uint32_t m_glid;
    int field_C;
    int m_align;
    uint16_t m_bestWidth;
    uint16_t m_bestHeight;
    int m_nameSCRC;
    AssetCategory m_assetCategory;
    TEX_STATE m_texState;
    int m_texTime;
    float m_aniso;
    int field_2C;
    int m_totalBytes;
    GFX_TEXTURE_WRAP_MODE m_wrapModeS;
    GFX_TEXTURE_WRAP_MODE m_wrapModeT;
    char m_field_36_3;
    uint8_t m_loaded; //bool or bit field?
    char m_fromLevel;
    char m_field_39_0;
    char m_toLevel;
    char field_3B;
    int field_3C;
    bool InHardware() const { return m_glid != -1; }
};
struct GFX_CreateBufferParams {
    size_t m_size;
    void *m_pData;
};
struct GFX_AnimatedTexture { //0x104 (260) bytes
    uint32_t m_curFrame;
    int m_framesCnt;
    float m_delay;
    char field_C;
    char field_D;
    char field_E;
    char field_F;
    float m_frameRate;
    uint32_t m_nameSCRC;
    int m_frameHandles[59];
};

inline GFX_TextureStruct g_Textures[0x3000];        // 0xC0'000 / 64
inline GFX_AnimatedTexture g_AnimatedTextures[256];
inline static uint8_t g_WhiteTexture[0x1000];
inline int g_WhiteHandle, g_nTexturesLoaded, g_nAnimatedTexturesLoaded, g_FlipbookTextureIndexOverride;
inline int64_t g_VRAMBytes_Textures;
inline const int g_DrawBufferSize = 0x800'000, MAX_SHADERS = 0x400;
inline GFX_VertexArray g_vertexArray;
inline GFX_ShaderPair g_Shaders[MAX_SHADERS], *g_pCurrentShader;
inline GFX_ShaderPair *GFX_GetCurrentShader() { return g_pCurrentShader; }
inline size_t g_TotalMemoryInKilobytes;
inline AssetCategory g_CurrentAssetCategory = AC_1;
inline uint64_t g_GFX_PerformanceFlags, g_VRAMBytes_VBO;
inline PerformanceGroup g_GFX_Performance = GPG_ULTRA;
inline int g_nSkipMipCount;
inline const char *g_GL_vendor = "", *g_GL_renderer = "", *g_GL_apiName = "";
inline bool g_openglDebug, g_glCoreContext, g_bGFXINITIALIZED, g_bUseEmptyShadowMapsHack, g_bInvertCulling;
inline char g_strCPU[0x40];
inline int g_BlurShaderHandle, g_CurrentShaderHandle = -1;
enum DetailedRender { DR_NO, DR_MIDDLE, DR_VERBOSE };
inline DetailedRender g_renderDetailed = DR_VERBOSE;
inline GLFWwindow *g_mainWindow;
inline bool g_MaintainFullscreenForBroadcast = true, g_removeFanviewHints, g_bShutdown, g_WorkoutDistortion, g_openglFail;
inline float g_kwidth, g_kheight, g_view_x, g_view_y, g_view_w, g_view_h, g_Aniso = 1.0f, g_instantaniousFPS, g_smoothedFPS, g_SecondsUnplugged, g_TargetBatteryFPS, g_TotalRenderTime;
inline int g_width, g_height, g_bFullScreen, g_nShadersLoaded, g_TotalShaderCreationTime;
inline uint32_t g_glVersion, g_CoreVA, g_UBOs[(int)GFX_RegisterRef::Ty::CNT], g_gfxTier, g_DrawPrimVBO, g_nTotalFrames;
inline uint8_t g_colorChannels, g_gfxShaderModel;
inline GfxCaps g_gfxCaps;
inline const GfxCaps &GFX_GetCaps() { return g_gfxCaps; }
enum GFX_CoordinateMap { GCM_CNT = 12 };
inline float g_coordMap[GCM_CNT] = { 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 0.0 };
inline float *GFX_GetCoordinateMap() { return g_coordMap; }
inline GFX_StateBlock g_GFX_CurrentStates[8], *g_pGFX_CurrentStates = g_GFX_CurrentStates;
inline int g_DrawNoTextureShaderHandle = -1, g_DrawTexturedShaderHandle = -1, g_DrawTexturedSimpleShaderHandle = -1, g_DrawTexturedGammaCorrectShaderHandle = -1;
inline uint8_t *g_DrawBuffers[2];
inline uint32_t g_CurrentBuffer, g_CurrentBufferOffset, g_PreviousBufferOffset;
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
    g_RockTexture, g_FranceRockTexture, g_FranceRockNTexture, g_RockNormalTexture, g_ShowroomFloorTexture, g_HeadlightTexture, g_VignetteTexture, g_FFtextureHandle;
inline uint32_t g_TextureTimeThisFrame, g_MeshTimeThisFrame;
inline GFX_MatrixContext g_MatrixContext;
inline VEC4 g_frustumPlanes[6], g_Vec4White{1.0, 1.0, 1.0, 1.0};
extern const MATRIX44 g_mxIdentity;
inline bool g_bIsAwareOfWideAspectUI, g_b2D720pRenderIsSetup;
inline float g_CurrentUISpace_Height = 720.0f, g_WideUISpace_Height = 720.0f, g_CurrentUISpace_Width = 1280.0f, g_WideUISpace_Width = 1280.0f, g_OrthoScalarW = 1.0f, g_OrthoScalarH = 1.0f;

void GFX_BEGIN_2DUISpace();
inline bool GFX_GetWideAspectAwareUI() { return g_bIsAwareOfWideAspectUI; }
void GFX_ActivateTexture(int handle, int offset, const char *name, GFX_TEXTURE_WRAP_MODE wm);
void GFX_SetBlendFunc(GFX_BLEND_OP op, GFX_BLEND src, GFX_BLEND dst);
void GFX_SetTextureFilter(uint32_t tn, GFX_FILTER f);
void GFX_SetupUIProjection();
void GFX_Ortho(float, float w, float h, float, float, float);
void GFX_UpdateMatrices(bool);
void GFX_SetAlphaBlendEnable(bool en);
void GFX_ActivateTextureEx(int tn, GLfloat lodBias);
void GFX_DestroyVertex(int *pIdx);
int GFX_CreateVertex(const GFX_CreateVertexParams &parms);
void GFXAPI_CreateVertex(int idx, const GFX_CreateVertexParams &parms);
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
void GFX_SetWideAspectAwareUI(bool val);
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
void GFX_MeshSystem_Update();
inline uint32_t GFX_GetFrameCount() { return g_nTotalFrames; }
int GFX_CreateBuffer(const GFX_CreateBufferParams &);
void GFXAPI_DestroyBuffer(GLuint handle);
void GFX_DestroyBuffer(int *pHandle);
void GFX_CreateVertexBuffer(int *pHandleDest, uint32_t size, void *data);
void GFX_CreateIndexBuffer(int *dest, uint32_t size, void *buf);
void GFX_LoadIdentity();
void GFX_RotateX(float angle);
void GFX_RotateY(float angle);
void GFX_RotateZ(float angle);
void GFX_LoadMatrix(const MATRIX44 &src);
void GFX_LookAt(VEC3 *a1, VEC3 *a2, VEC3 *a3);
void GFX_Scale(const VEC3 &mult);
VEC4 *GFX_GetFrustumPlanes();
void GFX_MulMatrix(const MATRIX44 &m);
void GFX_MatrixMode(GFX_MatrixType newMode);
bool GFX_GetFlipRenderTexture();
void GFX_SetFlipRenderTexture(bool newVal);
void GFX_PopMatrix();
void GFX_PushMatrix();
void GFX_UploadShaderMAT4(GFX_SHADER_MATRICES where, const MATRIX44 &what, uint64_t counter);
void GFX_TransposeMatrix44(MATRIX44 *dest, const MATRIX44 &src);
void GFX_UpdateFrustum(const MATRIX44 &a1, const MATRIX44 &a2);
void GFX_StoreMatrix(MATRIX44 *dest);
void GFX_UploadShaderVEC4(GFX_SHADER_REGISTERS, const VEC4 &, uint64_t);
uint32_t GFX_Align(uint32_t addr, uint32_t align);
uint8_t *GFX_DrawMalloc(int size, uint32_t align);
void GFX_DrawFlip();
inline void GFX_SetCurrentAniso(float a) { g_Aniso = fmaxf(1.0, a); }
void GFX_UnloadTexture(int handle);
void GFX_Internal_UnloadTexture(int handle, TEX_STATE s);
void GFX_UnloadTexture(int handle);
void GFX_Present();
void GFX_EndFrame();
void GFX_SetScissorTestEnable(bool en);
void GFX_SetDepthTestEnable(bool en);
void GFX_SetDepthWrite(bool en);
void GFX_SetCullMode(GFX_CULL cm);
void GFX_Internal_SetActiveTexture(int glHandle);
void GFXAPI_ActivateTexture(int handle, int glOffset, const char *uniformName, GFX_TEXTURE_WRAP_MODE wm);
int32_t GFX_GetStateU32(int idx);
void GFX_SetDepthTestFunc(GFX_COMPARE_FUNC fu);
void GFX_SetDepthBias(float b);
void GFX_SetIndexBuffer(int ib);
void GFX_SetTextureWrap(uint32_t tn, GFX_TEXTURE_WRAP_MODE t, GFX_TEXTURE_WRAP_MODE s);
void GFX_SetVertex(int handle);
void GFX_SetVertexBuffer(int vb, uint64_t notUsed, uint64_t attrData);
void GFXAPI_ReUploadShaderCache();
void GFX_DrawIndexedInstancedPrimitive(GFX_PRIM_TYPE a1, uint32_t a2, uint32_t a3, GFX_IndexFormat gif, uint32_t a5, uint32_t a6);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE ty, int baseVertex, uint32_t cnt, GFX_IndexFormat gif, const void *indices);
void GFX_internal_DrawPrimitive(GFX_PRIM_TYPE ty, int first, uint32_t count);
int GFX_Internal_LoadTextureFromTGAFile(const char *name, int handle);
void GFX_Draw2DQuad(float l, float t, float w, float h, uint32_t color, bool uiProjection);
void GFX_Draw2DQuad_720p(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, int color, float a10, int a11, int a12);
template <typename T> int GFX_GetVertexHandle();
template <typename T> void DefineVAO(uint32_t a1, const T *data, uint32_t cnt) {
    if (g_glCoreContext) {
        g_pGFX_CurrentStates->BindVertexBuffer(g_DrawPrimVBO);
        glBufferData(GL_ARRAY_BUFFER, T::MULT * cnt, data, GL_STREAM_DRAW);
        if (g_pGFX_CurrentStates->m_vertexBuffer != g_DrawPrimVBO) {
            g_pGFX_CurrentStates->m_vertexBuffer = g_DrawPrimVBO;
            g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VBO;
        }
        if (g_pGFX_CurrentStates->m_attrData != 0) {
            g_pGFX_CurrentStates->m_attrData = 0;
            g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_ATTRDATA;
        }
        if (g_DrawPrimVBO != -1) {
            if (g_pGFX_CurrentStates->m_VAO != 0) {
                g_pGFX_CurrentStates->m_VAO = 0;
                g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VAO;
            }
        }
    } else {
        if (g_pGFX_CurrentStates->m_VAO != (uint64_t)data) {
            g_pGFX_CurrentStates->m_VAO = (uint64_t)data;
            g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VAO;
        }
        if (data) {
            if (g_pGFX_CurrentStates->m_attrData != 0) {
                g_pGFX_CurrentStates->m_attrData = 0;
                g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_ATTRDATA;
            }
            if (g_pGFX_CurrentStates->m_vertexBuffer != -1) {
                g_pGFX_CurrentStates->m_vertexBuffer = -1;
                g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VBO;
            }
        }
    }
    auto v17 = GFX_GetVertexHandle<T>();
    if (g_pGFX_CurrentStates->m_vertex != v17) {
        g_pGFX_CurrentStates->m_vertex = v17;
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VAIDX;
    }
}
template <typename T> void GFX_DrawPrimitive(GFX_PRIM_TYPE t, const T *data, uint32_t cnt) {
    if (data && cnt) {
        DefineVAO<T>(T::VAO, data, cnt);
        if (g_pGFX_CurrentStates->Realize())
            glDrawArrays(g_PRIM_TO_GLPRIM[t], 0, cnt);
    }
}
inline void GFX_Draw2DQuad_UI(float l, float t, float w, float h, int color) { GFX_Draw2DQuad(l, t, w, h, color, true); }
inline float GFX_UI_GetCurrentSpaceWidth() { return g_CurrentUISpace_Width; }