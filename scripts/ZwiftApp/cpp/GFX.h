#pragma once
//__m128 _mm_shuffle_ps(__m128 lo, __m128 hi, _MM_SHUFFLE(hi3, hi2, lo1, lo0)) 
// Interleave inputs into low 2 floats and high 2 floats of output. Basically
// out[0] = lo[lo0];
// out[1] = lo[lo1];
// out[2] = hi[hi2];
// out[3] = hi[hi3];
// For example, _mm_shuffle_ps(a, a, _MM_SHUFFLE(i, i, i, i)) copies the float a[i] into all 4 output floats.
//_MM_SHUFFLE(dd, cc, bb, aa) just packs the low 2 bits of each arg into a 0bddccbbaa.
// 
//_mm_shuffle_ps(v2, v2, 225=0b11100001) -> v2[1] v2[0] v2[2] v2[3]
//_mm_shuffle_ps(v2, v2, 255=0b11111111) -> v2[3] in all
//_mm_shuffle_ps(v2, v2, 85 =0b01010101) -> v2[1] in all
//_mm_shuffle_ps(v2, v2, 170=0b10101010) -> v2[2] in all
static inline const int INVALID_TEXTURE_HANDLE = -1;
enum GFX_RenderPass { GRP_CNT };
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
struct DRAW_VERT_POS_COLOR_UV { //32 bytes
    enum { MULT = 32, VAO = 1 }; 
    VEC3 m_pos;
    uint32_t m_color;
    float m_field_10;
    float m_field_14;
    float field_18;
    float field_1C;
};
struct DRAW_VERT_POS_COLOR { //16 bytes 
    enum { MULT = 16, VAO = 0 }; 
    VEC2 m_point;
    int m_dummy;
    uint32_t m_color;
};
struct DRAW_VERT_POS_COLOR_UV_NORM  { enum { MULT = 44, VAO = 3 }; };
struct DRAW_VERT_POS_COLOR_UV_NORM_TAN {};
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
    MATRIX44 g_HACK_CachedWorldViewProj;
    MATRIX44 g_HACK_CachedView;
    MATRIX44 g_HACK_CachedWorld;
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
enum GFX_SHADER_REGISTERS { GSR_0 = 0, GSR_5 = 5, GSR_7 = 7, GSR_11 = 11, GSR_21 = 21, GSR_22 = 22, GSR_23 = 23, GSR_24 = 24, GSR_27 = 27, GSR_28 = 28, GSR_CNT = 29 };
enum GFX_SHADER_MATRICES { GSM_0 = 0, GSM_1, GSM_2, GSM_3, GSM_4, GSM_CNT = 9 };
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
    int m_attribLocations[12], m_locations[GSR_CNT], m_matLocations[GSM_CNT], m_matArrLocations[2], m_samplers[16], m_texHandles[16], m_uniformLocations[18];
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
    bool m_field_2, m_field_3;
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
inline int g_BlurShaderHandle = -1, g_CurrentShaderHandle = -1;
enum DetailedRender { DR_NO, DR_MIDDLE, DR_VERBOSE };
inline DetailedRender g_renderDetailed = DR_VERBOSE;
inline float g_kwidth, g_kheight, g_UI_WindowOffsetX, g_UI_WindowOffsetY, g_UI_WindowWidth, g_UI_WindowHeight, g_Aniso = 1.0f, g_instantaniousFPS, g_smoothedFPS, g_SecondsUnplugged, g_TargetBatteryFPS, g_TotalRenderTime;
inline int g_WIN32_WindowWidth, g_WIN32_WindowHeight, g_bFullScreen, g_nShadersLoaded, g_TotalShaderCreationTime;
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
inline int g_SimpleShaderHandle = -1, g_WorldNoLightingHandle = -1, g_ShadowmapShaderHandle = -1, g_ShadowmapInstancedShaderHandle = -1, g_ShadowmapHairShaderHandle = -1, g_TestShaderHandle = -1, 
    g_RoadShader = -1, g_RoadAccessoryShader = -1, g_RoadAccessoryShaderSSR = -1, g_RoadWetShader = -1, g_RoadAccessoryWetShader = -1, g_HeatHazeShader = -1, g_CausticShader = -1, g_CrepuscularHandle = -1,
    g_WorldShaderHandle = -1, g_WorldAOShaderHandle = -1, g_WorldShaderBillboardedHandle = -1, g_WorldShaderShinyHandle = -1, g_WorldShaderSimpleHandle = -1, g_HologramShader = -1,
    g_LockedItemShader = -1, g_LondonTerrainShader = -1, g_LondonTerrainHeightMapShader = -1, g_FranceTerrainShader = -1, g_FranceTerrainHeightMapShader = -1, g_BasicTerrainShader = -1,
    g_BasicTerrainHeightMapShader = -1, g_BasicTerrainNoSnowShader = -1, g_BasicTerrainNoSnowHeightMapShader = -1, g_InnsbruckTerrainHeightMapShader = -1, g_InnsbruckTerrainHandle = -1,
    g_BolognaTerrainHeightMapShader = -1, g_BolognaTerrainShader = -1, g_YorkshireTerrainShader = -1, g_YorkshireTerrainHeightMapShader = -1, g_RichmondTerrainShader = -1, g_RichmondTerrainHeightMapShader = -1,
    g_WorkoutHologramShader = -1, g_WorkoutHologramPrShader = -1, g_FinalCopyShader = -1, g_WorldShaderTerrainHandle = -1, g_WorldShaderTerrainHeightMapHandle = -1, g_ShadowShaderTerrainHeightMap = -1,
    g_WatopiaSpecialTileShaderHeightmap = -1, g_WatopiaSpecialTileShader = -1, g_WorldShaderInstancedHandle = -1, g_WorldShaderInstancedTerrainConformingHandle = -1, g_World2LayerShaderHandle = -1,
    g_VegetationShaderHandle = -1, g_VegetationShaderInstancedTerrainConformHandle = -1, g_VegetationShaderInstancedHandle = -1, g_VegetationShadowmapShaderHandle = -1, g_VegetationShadowmapInstancedShaderHandle = -1,
    g_WireShaderHandle = -1, g_WireShadowShaderHandle = -1, g_BikeShaderInstancedHandle = -1, g_HairShaderHandle = -1, g_SkinShader = -1, g_ShadowmapSkinShader = -1, g_SkinShaderHologram = -1, g_grayScaleShader = -1;
inline bool g_bUseTextureHeightmaps = true;
inline int g_ButterflyTexture = -1, g_RedButterflyTexture = -1, g_MonarchTexture = -1, g_FireflyTexture = -1, g_CausticTexture = -1, g_GrassTexture = -1, g_GravelMtnGrassTexture = -1,
    g_InnsbruckConcreteTexture = -1, g_ParisConcreteTexture = -1, g_DefaultNormalMapNoGloss, g_RoadDustTexture = -1, g_GravelDustTexture = -1, g_SandTexture = -1, g_SandNormalTexture = -1,
    g_RockTexture = -1, g_FranceRockTexture = -1, g_FranceRockNTexture = -1, g_RockNormalTexture = -1, g_ShowroomFloorTexture = -1, g_HeadlightTexture = -1, g_VignetteTexture = -1, g_FFtextureHandle = -1, g_GFX_rp;
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
using GDE_360_INDEXBUFFER = int32_t;
using GDE_360_VERTEXBUFFER = int32_t;
void GFX_CreateIndexBuffer(GDE_360_INDEXBUFFER *dest, uint32_t size, void *buf);
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
enum GFX_STATE { GFS_CULLMODE, GFS_12 };
int32_t GFX_GetStateU32(GFX_STATE idx);
//inlined void GFX_SetState(GFX_STATE, uint32_t);
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
void GFX_SetShadowParameters(float sp);
struct GFX_TextureDef {
    int m_2w;
    int m_field_C;
    uint16_t m_w1;
    uint16_t m_h1;
    uint16_t m_field_14;
    char field_16[42];
    uint16_t m_w2;
    uint16_t m_h2;
    int m_field_44;
    int m_field_48;
    int m_field_4C;
    uint16_t m_field_50;
    char m_field_52;
};
struct GFX_TextureBytes {
    int16_t *m_data;
    GFX_TextureDef m_def;
};
int GFX_CreateTexture(const GFX_TextureDef &def, const GFX_TextureBytes &ptr, uint64_t);
enum GFX_QueryType { GQT_0 };
void GFXAPI_BeginQuery(GFX_QueryType, int);
void GFXAPI_DrawIndexedPrimitive(GFX_PRIM_TYPE, const void *, uint32_t, GFX_IndexFormat, const void *);
void GFXAPI_EndQuery(GFX_QueryType);
struct GFX_MapBufferParams {
    enum Access { BUF_READ = 1, BUF_WRITE = 2, BUF_READ_WRITE = 3 };
    uint64_t m_field_0, m_offset;
    uint64_t m_access, field_18, field_20;
};
uint8_t *GFXAPI_MapBuffer(int, const GFX_MapBufferParams &);
struct GFX_ReadPixelsParams {};
void GFXAPI_ReadPixels(const GFX_ReadPixelsParams &);
struct GFX_UpdateBufferParams {};
void GFXAPI_UpdateBuffer(int, const GFX_UpdateBufferParams &);
struct GFX_UpdateSubBufferParams {};
void GFXAPI_UpdateSubBuffer(int, const GFX_UpdateSubBufferParams &);
enum GFX_PixelFormat { GPF_0 };
void GFXU_ConvertImage(uint8_t *, GFX_PixelFormat, GFX_PixelFormat, const tVEC2<uint32_t> &);
void GFXU_DownsampleImage(uint8_t *, tVEC2<uint32_t> &, GFX_PixelFormat, const tVEC2<uint32_t> &);
void GFXU_FlipImageY(uint8_t *, const tVEC2<uint32_t> &, GFX_PixelFormat);
void GFXU_SaveTGA(FILE *, tVEC2<uint32_t> const &, const uint8_t *, GFX_PixelFormat);
bool GFX_AABBInCurrentFrustum(const VEC3 &, const VEC3 &, float *);
struct Plane {};
void GFX_AABBInFrustum(VEC3, VEC3, Plane *, float *);
void GFX_ActivateAnimatedTexture_INTERNAL(int, int, const char *, GFX_TEXTURE_WRAP_MODE);
void GFX_ActivateTexture(int, int, const char *, GFX_TEXTURE_WRAP_MODE);
void GFX_AddPerformanceFlagChange(uint64_t);
void GFX_BEGIN_2DUISpace();
void GFX_Begin();
void GFX_BeginCapture();
void GFX_BeginQuery(GFX_QueryType, int);
void GFX_BitsPerPixel(GFX_PixelFormat);
void GFX_Calculate3DTVProjMatrices(float *, float *, const float *, float, float);
void GFX_CalculateOVRProjMatrices(float *, float *, const float *, float, float);
void GFX_CheckForAllRequiredExtensions();
void GFX_CheckFramebufferStatus();
void GFX_ComponentsPerPixel(GFX_PixelFormat);
void GFX_ComputeClipInfo(float, float);
void GFX_CreateAnimatedTextureFromTGAFiles(const char *, int);
struct GFX_CreateDepthStencilStateParams {};
void GFX_CreateDepthStencilState_Base(const GFX_CreateDepthStencilStateParams &);
struct GfxPipelineDesc {};
void GFX_CreatePipeline_Base(const GfxPipelineDesc &);
void GFX_CreateQuery();
void GFX_CreateTextureFromJPEGFile(const char *);
int GFX_CreateTextureFromLuminanceF32(uint32_t, uint32_t, float *);
int GFX_CreateTextureFromLuminanceR8(GLsizei w, GLsizei h, const GLvoid *d);
void GFX_CreateTextureFromRGB(uint32_t, uint32_t, void *);
void GFX_CreateTextureFromRGBA(uint32_t, uint32_t, void *, bool);
void GFX_CreateTextureFromTGA(void *, int);
void GFX_CreateTextureFromTGAFile(const char *, int, bool, bool);
void GFX_CreateTextureFromTGAX(void *, int);
void GFX_CreateTextureFromZTX(void *, int, int);
void GFX_CreateThread();
void GFX_CreateVertexBuffer(GDE_360_VERTEXBUFFER *, uint32_t, void *);
void GFX_DestroyBuffer(int &);
void GFX_DestroyIndexBuffer(GDE_360_INDEXBUFFER *);
void GFX_DestroyQuery(int &);
void GFX_DestroyShader(int &);
void GFX_DestroyThread();
void GFX_DestroyVertexBuffer(GDE_360_VERTEXBUFFER *);
void GFX_DisableShaderProgramOverride();
void GFX_DisableVideoCapture();
void GFX_Draw2DOffsetRotatedQuad(float, float, float, float, float, float, float, float, uint32_t, uint32_t, uint32_t, uint32_t, float, float, float, int, bool, float);
void GFX_Draw2DPie(float, float, float, float, uint32_t, float, int, bool, float);
void GFX_Draw2DPixeledQuadV(float, float, float, float, uint32_t, uint32_t, bool);
void GFX_Draw2DQuad(float, float, float, float, float *, uint32_t *, float, int, bool);
void GFX_Draw2DQuad(float, float, float, float, float, float, float, float, uint32_t, float, int, bool, float);
void GFX_Draw2DQuad(float, float, float, float, float, float, float, float, uint32_t, uint32_t, uint32_t, uint32_t, float, int, bool, float);
void GFX_Draw2DQuad(float, float, float, float, uint32_t, bool);
void GFX_Draw2DQuad(float, float, float, float, uint32_t, uint32_t, uint32_t, uint32_t, bool);
void GFX_Draw2DQuad_720p(float, float, float, float, float, float, float, float, uint32_t, float, int, float);
void GFX_Draw2DQuad_720p(float, float, float, float, float, float, float, float, uint32_t, uint32_t, uint32_t, uint32_t, float, int);
void GFX_Draw2DQuad_720p(float, float, float, float, uint32_t);
void GFX_Draw2DQuad_720p(float, float, float, float, uint32_t, uint32_t, uint32_t, uint32_t);
void GFX_Draw2DTrapezoid(float, float, float, float, float, uint32_t, bool);
void GFX_DrawArcedCapsule(float, float, float, float, uint32_t, float, float, float, int, bool);
void GFX_DrawBuffers(uint64_t);
void GFX_DrawCapsule(float, float, float, float, uint32_t);
void GFX_DrawCircle(float, float, float, float, uint32_t);
void GFX_DrawFlip();
void GFX_DrawIndexedInstancedPrimitive(GFX_PRIM_TYPE, uint32_t, uint32_t, GFX_IndexFormat, uint32_t, uint32_t, uint32_t, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, DRAW_VERT_POS_COLOR_1UV *, uint32_t, uint32_t *, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, DRAW_VERT_POS_COLOR_1UV const *, uint32_t, uint16_t const *, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, DRAW_VERT_POS_COLOR_UV *, uint32_t, uint32_t *, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, DRAW_VERT_POS_COLOR_UV_NORM_TAN *, uint32_t, uint32_t *, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, DRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED *, uint32_t, uint16_t *, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, uint32_t, uint32_t, GFX_IndexFormat, uint32_t);
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, const void *, uint32_t, GFX_IndexFormat, const void *);
void GFX_DrawInit();
void GFX_DrawInstancedPrimitive(GFX_PRIM_TYPE, uint32_t, uint32_t, uint32_t, uint32_t);
void GFX_DrawPrimitive_OGL(GFX_PRIM_TYPE, DRAW_VERT_POS_COLOR_UV *, uint32_t);
enum RENDER_BUFFER { RB_0 };
struct RenderTarget;
void GFX_DrawRenderTarget(const RECT2 &, const RECT2 &, RenderTarget *, RENDER_BUFFER, const VEC4 &, GFX_TEXTURE_WRAP_MODE);
void GFX_DrawRenderTarget(RenderTarget *, RenderTarget *, RENDER_BUFFER, const VEC4 &, GFX_TEXTURE_WRAP_MODE);
void GFX_DrawRenderTarget2(float, float, float, float, RenderTarget *, RENDER_BUFFER, const VEC4 &, GFX_TEXTURE_WRAP_MODE);
void GFX_DrawRenderTargetDepth(float, float, float, float, RenderTarget *, RENDER_BUFFER, GFX_TEXTURE_WRAP_MODE);
void GFX_DrawSphere(const VEC3 &, float, uint32_t, uint32_t);
void GFX_END_2DUISpace();
void GFX_EnableShaderProgramOverride(uint32_t);
//TODO void GFX_EnableVideoCapture(std::weak_ptr<GFX_VideoCaptureHandler> const &);
void GFX_EndCapture();
void GFX_EndFrame();
void GFX_EndQuery(GFX_QueryType);
void GFX_Flush();
void GFX_GLFWErrorCallback(int, const char *);
void GFX_GaussianBlur(RenderTarget *, RenderTarget *, RenderTarget *, float, GFX_TEXTURE_WRAP_MODE);
struct GaussianTap {};
void GFX_GaussianTaps(GaussianTap *, uint64_t, uint64_t &, uint64_t, float, bool);
void GFX_GetAPI();
void GFX_GetAPIVersion();
void GFX_GetCurrentMonitorHash();
void GFX_GetDepthTestFunc();
void GFX_GetDestBlend();
void GFX_GetDisplayCategory();
void GFX_GetError();
void GFX_GetInstantaniousFPS();
void GFX_GetMaxFPSOnBattery();
void GFX_GetMemory();
void GFX_GetPerformanceGroupName();
void GFX_GetPrimCount(GFX_PRIM_TYPE, uint32_t);
void GFX_GetPxlShaderConstantHandleByName(const char *);
void GFX_GetPxlShaderConstantHandleByRegister(uint32_t);
void GFX_GetQueryResult(int, uint64_t &, uint64_t);
void GFX_GetScreenDPI();
void GFX_GetShaderProgram();
void GFX_GetShaderProgramOverride();
void GFX_GetShaderVEC4(GFX_SHADER_REGISTERS);
void GFX_GetSmoothedFPS();
void GFX_GetSrcBlend();
void GFX_GetTargetSize(RenderTarget *);
void GFX_GetTexture(uint32_t);
void GFX_GetTextureDef(int);
int GFX_GetTextureHeight(int);
int GFX_GetTextureWidth(int h);
void GFX_GetThread();
void GFX_GetThreadList();
void GFX_GetTotalAppTimeSeconds();
void GFX_GetUIScissor();
void GFX_GetViewport(int &, int &, uint32_t &, uint32_t &);
enum VrtShaderConstantId { VSC_0 };
void GFX_GetVrtShaderConstantHandleById(VrtShaderConstantId);
void GFX_GetVrtShaderConstantHandleByName(const char *);
void GFX_GetVrtShaderConstantHandleByRegister(uint32_t);
void GFX_InitDrawPrimVAOs();
void GFX_Initialize(uint32_t, uint32_t, bool, bool, uint32_t, const char *, const char *);
void GFX_Internal_GetNextTextureHandle();
void GFX_Internal_LoadOnDemandMeshHandle(int, uint32_t);
void GFX_Internal_LoadOnDemandTextureHandle(int);
void GFX_Internal_LoadTextureFromTGAFile(const char *, int, bool);
void GFX_Internal_LoadTextureFromWADFile(const char *, int);
void GFX_Internal_SetActiveTexture(uint32_t);
void GFX_Internal_UnloadTexture(int, TEX_STATE);
void GFX_Internal_UpdateVertexArrayAttribEnable(uint64_t);
void GFX_Internal_fixupShaderAddresses(GFX_ShaderPair *);
void GFX_InvPerspective(MATRIX44 *, float, float, float, float);
void GFX_InvalidateCachedTextureState();
void GFX_InvalidateIndexBuffer(GDE_360_INDEXBUFFER *);
void GFX_InvalidateMatrixStack();
void GFX_InvalidateVertexBuffer(GDE_360_VERTEXBUFFER *);
void GFX_IsAnimatedTexture(int);
enum TEX_FORMATS { TF_0 };
void GFX_IsCompressed(TEX_FORMATS);
void GFX_IsLowVRAMDevice();
void GFX_IsVideoCaptureSupported();
void GFX_LineWidth(float);
void GFX_LoadIdentity();
void GFX_LoadMatrix(const MATRIX44 &);
void GFX_LookAt(const VEC3 &, const VEC3 &, const VEC3 &);
uint8_t *GFX_MapBuffer(int, const GFX_MapBufferParams &);
void GFX_MatrixMode(GFX_MatrixType);
void GFX_MatrixStackInitialize();
void GFX_MeshSystem_Update(float);
void GFX_MulMatrix(const MATRIX44 &);
void GFX_MulMatrixInternal(MATRIX44 *, const MATRIX44 &);
struct GFX_Noesis {
    /*void GFX_Noesis::BeginRender(bool);
    void GFX_Noesis::BeginTile(const Noesis::Tile &, uint32_t, uint32_t);
    void GFX_Noesis::CloneRenderTarget(const char *, Noesis::RenderTarget *);
    void GFX_Noesis::CreateRenderTarget(const char *, uint32_t, uint32_t, uint32_t);
    void GFX_Noesis::CreateTexture(const char *, uint32_t, uint32_t, uint32_t, Noesis::TextureFormat::Enum, const void **);
    void GFX_Noesis::DrawBatch(const Noesis::Batch &);
    void GFX_Noesis::EndRender();
    void GFX_Noesis::EndTile();
    void GFX_Noesis::GetCaps();
    void GFX_Noesis::MapIndices(uint32_t);
    void GFX_Noesis::MapVertices(uint32_t);
    void GFX_Noesis::OffscreenRender();
    void GFX_Noesis::OnscreenRender();
    void GFX_Noesis::ResolveRenderTarget(Noesis::RenderTarget *, Noesis::Tile const *, uint32_t);
    void GFX_Noesis::SetRenderTarget(Noesis::RenderTarget *);
    void GFX_Noesis::UnmapIndices();
    void GFX_Noesis::UnmapVertices();
    void GFX_Noesis::UpdateTexture(Noesis::Texture *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, const void *);
    void GFX_Noesis::~GFX_Noesis();*/
};
void GFX_NoesisCreateDevice(bool, int);
struct GFX_NoesisDevice {
    /*void GFX_NoesisDevice::BeginRender(bool);
void GFX_NoesisDevice::BeginTile(Noesis::Tile const &, uint32_t, uint32_t);
void GFX_NoesisDevice::CloneRenderTarget(const char *, Noesis::RenderTarget *);
void GFX_NoesisDevice::CreateRenderTarget(const char *, uint32_t, uint32_t, uint32_t);
void GFX_NoesisDevice::CreateTexture(const char *, uint32_t, uint32_t, uint32_t, Noesis::TextureFormat::Enum, const void **);
void GFX_NoesisDevice::DrawBatch(Noesis::Batch const &);
void GFX_NoesisDevice::EndRender();
void GFX_NoesisDevice::EndTile();
void GFX_NoesisDevice::GFX_NoesisDevice(bool);
void GFX_NoesisDevice::GetCaps();
void GFX_NoesisDevice::MapIndices(uint32_t);
void GFX_NoesisDevice::MapVertices(uint32_t);
void GFX_NoesisDevice::ResolveRenderTarget(Noesis::RenderTarget *, Noesis::Tile const *, uint32_t);
void GFX_NoesisDevice::SetRenderTarget(Noesis::RenderTarget *);
void GFX_NoesisDevice::UnmapIndices();
void GFX_NoesisDevice::UnmapVertices();
void GFX_NoesisDevice::UpdateTexture(Noesis::Texture *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, const void *);
void GFX_NoesisDevice::~GFX_NoesisDevice();
*/
};
struct GFX_NoesisGFX {
    GFX_NoesisGFX(bool);
    ~GFX_NoesisGFX();
};
struct GFX_NoesisOGL {
    GFX_NoesisOGL(bool);
    void OffscreenRender();
    void OnscreenRender();
    ~GFX_NoesisOGL();
};
void GFX_NoesisOffscreenRender(Noesis::RenderDevice *);
void GFX_NoesisOnscreenRender(Noesis::RenderDevice *);
struct GFX_NoesisRenderTarget {
    GFX_NoesisRenderTarget(const char *, const GFX_NoesisRenderTarget *);
    GFX_NoesisRenderTarget(const char *, uint32_t, uint32_t, uint32_t);
    void GetTexture();
    void RenderTo();
    ~GFX_NoesisRenderTarget();
};
enum GFX_SHADER_SAMPLERS { GSS_0 };
struct GFX_NoesisRenderTargetTexture {
    GFX_NoesisRenderTargetTexture(const char *, RenderTarget *);
    void Bind(GFX_SHADER_SAMPLERS, const Noesis::SamplerState &);
    void GetHeight();
    void GetWidth();
    void HasMipMaps();
    void IsInverted();
    void Update(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, const void *);
    ~GFX_NoesisRenderTargetTexture();
};
struct GFX_NoesisTexture2D {
    GFX_NoesisTexture2D(const char *, uint32_t, uint32_t, uint32_t, Noesis::TextureFormat::Enum, const void **);
    void Bind(GFX_SHADER_SAMPLERS, Noesis::SamplerState const &);
    void GetHeight();
    void GetWidth();
    void HasMipMaps();
    void IsInverted();
    void Update(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, const void *);
    ~GFX_NoesisTexture2D();
};
struct GFX_NoesisTexture {
    GFX_NoesisTexture(const char *);
    ~GFX_NoesisTexture();
};
void GFX_Ortho(float, float, float, float, float, float);
void GFX_Ortho3D(float, float, float, float, float, float);
void GFX_Perspective(float, float, float, float);
void GFX_PointSize(float);
void GFX_PopMatrix();
void GFX_PopStates();
void GFX_PopUIScissor();
void GFX_Present();
void GFX_PushMatrix();
void GFX_PushStates();
void GFX_PushUIScissor();
//OMIT enum GFX_API { GA_0 };
//struct GFX_API_Version {};
//void GFX_QueryAPI(GFX_API, GFX_API_Version *);
void GFX_ReadPixels(const GFX_ReadPixelsParams &);
void GFX_ReplaceTextureWithRGBA(int, uint32_t, uint32_t, void *);
void GFX_RotateX(float);
void GFX_RotateY(float);
void GFX_RotateZ(float);
void GFX_Scale(const VEC3 &);
void GFX_Scissor(int, int, uint32_t, uint32_t);
void GFX_SetAlphaBlendEnable(bool);
void GFX_SetAnimatedTextureFrame(int, int);
void GFX_SetAnimatedTextureFramerate(int, float);
void GFX_SetBlendColor(float, float, float, float);
void GFX_SetBlendFunc(GFX_BLEND_OP, GFX_BLEND, GFX_BLEND);
void GFX_SetClearColor(float, float, float, float);
void GFX_SetClearDepth(float);
void GFX_SetClipPlane(uint64_t, const Plane &, const MATRIX44 &);
void GFX_SetClipPlaneEnable(uint64_t, bool);
void GFX_SetColorMask(uint64_t, uint8_t);
void GFX_SetCullMode(GFX_CULL);
void GFX_SetCurrentAniso(float);
void GFX_SetDepthBias(float);
void GFX_SetDepthTestEnable(bool);
void GFX_SetDepthTestFunc(GFX_COMPARE_FUNC);
void GFX_SetDepthWrite(bool);
void GFX_SetFillMode(GFX_FILL_MODE);
void GFX_SetFlipRenderTexture(bool);
void GFX_SetIndexBuffer(int);
void GFX_SetLoadedAssetMode(bool);
void GFX_SetMaxFPSOnBattery(float);
void GFX_SetMipBias(float);
struct GFX_UserRegister {
    int64_t m_offset;
    const char *m_name;
};
void SetupLightmaps(bool a1);
void GFX_SetPxlShaderConstByName_Vec4(const GFX_UserRegister &, const VEC4 &);
void GFX_SetPxlShaderConst_Bool(uint32_t, bool);
void GFX_SetPxlShaderConst_Float(uint32_t, float);
void GFX_SetPxlShaderConst_Int(uint32_t, int);
void GFX_SetPxlShaderConst_Int2(uint32_t, int, int);
void GFX_SetPxlShaderConst_Int3(uint32_t, int, int, int);
void GFX_SetPxlShaderConst_Mat4x4(uint32_t, const MATRIX44 &);
void GFX_SetPxlShaderConst_Vec4(uint32_t, const VEC4 &);
void GFX_SetScissorTestEnable(bool);
void GFX_SetSelfIllumDayNightLerpVal(float);
void GFX_SetSelfIllumIntensityBias(float);
enum GFX_SHADER_COLORSPACE { GSC_0 };
void GFX_SetShaderColorSpace(GFX_SHADER_COLORSPACE);
void GFX_SetShadowParameters(float);
void GFX_SetSlopeScaleDepthBias(float);
void GFX_SetStencilFunc(bool, GFX_COMPARE_FUNC, uint8_t, uint8_t, GFX_StencilOp, GFX_StencilOp, GFX_StencilOp);
void GFX_SetStencilRef(uint8_t);
struct GFX_SetBufferParams {};
void GFX_SetStorageBuffer(const GFX_SetBufferParams *, uint64_t);
void GFX_SetTextureFilter(uint32_t, GFX_FILTER);
void GFX_SetTextureFilterCap(GFX_FILTER);
void GFX_SetTextureLodBias(uint32_t, float);
void GFX_SetTextureWrap(uint32_t, GFX_TEXTURE_WRAP_MODE, GFX_TEXTURE_WRAP_MODE);
void GFX_SetUIScissor(float, float, float, float, bool);
void GFX_SetVAO(int);
void GFX_SetVertex(int);
void GFX_SetVertexBuffer(int, uint64_t, uint64_t);
void GFX_SetVrtShaderConstByName_Vec4(const GFX_UserRegister &, const VEC4 &);
void GFX_SetVrtShaderConst_Bool(uint32_t, bool);
void GFX_SetVrtShaderConst_Float(uint32_t, float);
void GFX_SetVrtShaderConst_Int(uint32_t, int);
void GFX_SetVrtShaderConst_Int2(uint32_t, int, int);
void GFX_SetVrtShaderConst_Int3(uint32_t, int, int, int);
void GFX_SetVrtShaderConst_Mat4x4(uint32_t, const MATRIX44 &);
void GFX_SetVrtShaderConst_Vec4(uint32_t, const VEC4 &);
void GFX_SetWideAspectAwareUI(bool);
void GFX_SetupUIProjection();
enum GFX_ShaderModel { GSHM_0 };
void GFX_ShaderModelValue(GFX_ShaderModel);
void GFX_Shadow(MATRIX44 *);
void GFX_Shutdown();
struct GFX_SortKeyParams {};
struct GFX_SortKey {
    void SetDistance(const VEC3 &, const GFX_SortKeyParams &);
};
void GFX_SphereInCurrentFrustum(float, float, float, float);
void GFX_SphereInCurrentFrustumDist(float, float, float, float);
void GFX_SphereInFrustum(float, float, float, float, Plane *);
void GFX_SphereInFrustumDist(float, float, float, float, Plane *);
void GFX_SphereInFrustumDistFromSides(float, float, float, float);
struct Sphere;
void GFX_SpheresInCurrentFrustumDist(Sphere *, int, float *);
void GFX_SpheresInFrustumDist(Sphere *, int, float *, Plane *);
void GFX_StoreMatrix(MATRIX44 *);
void GFX_StreamMalloc(uint64_t, uint64_t);
void GFX_TestInitGL(int, int, bool);
void GFX_TexPerspective(float, float, float, float, float, float);
void GFX_TextureSys_Initialize();
void GFX_TextureSys_Update(float);
struct GFX_Thread {
    GFX_Thread();
    ~GFX_Thread();
};
void GFX_TransformPoint(const VEC3 &);
void GFX_Translate(const VEC3 &);
void GFX_UI_GetCurrentSpaceHeight();
inline float GFX_UI_GetLeftEdgePad() { return 0.0f; }
inline float GFX_UI_GetRightEdgePad() { return 0.0f; }
void GFX_UI_GetWideSpaceHeight();
void GFX_UI_GetWideSpaceWidth();
void GFX_UnloadTexture(int);
void GFX_UnmapBuffer(int bufId);
void GFX_UnsetShader();
void GFX_UpdateBuffer(int, const GFX_UpdateBufferParams &);
void GFX_UpdateMatrices(bool);
void GFX_UpdateStack_internal();
void GFX_UpdateSubBuffer(int, const GFX_UpdateSubBufferParams &);
enum GFX_MatrixSlot { GMS_0 = 0, GMS_4 = 4 };
void GFX_UploadMatrix(GFX_MatrixSlot, const MATRIX44 &);
void GFX_UploadShaderFloat(GFX_SHADER_REGISTERS, float);
void GFX_UploadShaderFloat(const GFX_UserRegister &, float);
void GFX_UploadShaderMAT4(const GFX_UserRegister &, const MATRIX44 &, uint64_t);
enum GFX_SHADER_MATRIX_ARRAYS { GSMA_0, GSMA_1 };
void GFX_UploadShaderMAT4ARRAY(GFX_SHADER_MATRIX_ARRAYS, uint32_t, const MATRIX44 *, uint64_t);
void GFX_UploadShaderMAT4ARRAY(const GFX_UserRegister &, uint32_t, const MATRIX44 *, uint64_t);
void GFX_UploadShaderVEC2(GFX_SHADER_REGISTERS, const VEC2 &);
void GFX_UploadShaderVEC2(const GFX_UserRegister &, const VEC2 &);
void GFX_UploadShaderVEC2ARRAY(GFX_SHADER_REGISTERS, uint32_t, const VEC2 *);
void GFX_UploadShaderVEC2ARRAY(const GFX_UserRegister &, uint32_t, const VEC2 *);
void GFX_UploadShaderVEC3(GFX_SHADER_REGISTERS, const VEC3 &);
void GFX_UploadShaderVEC3(const GFX_UserRegister &, const VEC3 &);
void GFX_UploadShaderVEC4(GFX_SHADER_REGISTERS, const VEC4 &, uint64_t);
void GFX_UploadShaderVEC4(const GFX_UserRegister &, const VEC4 &, uint64_t);
void GFX_UploadShaderVEC4ARRAY(GFX_SHADER_REGISTERS, uint32_t, const VEC4 *);
void GFX_UploadShaderVEC4ARRAY(const GFX_UserRegister &, uint32_t, const VEC4 *, uint64_t);
void GFX_VHSGaussianBlur(RenderTarget *, RenderTarget *, RenderTarget *, float, GFX_TEXTURE_WRAP_MODE);
//inlined void GFX_Viewport(int, int, uint32_t, uint32_t);
void GFX_internal_DrawIndexedPrimitive(GFX_PRIM_TYPE, GFX_IndexFormat, const void *, uint32_t);
void GFX_internal_DrawPrimitive(GFX_PRIM_TYPE, uint32_t, uint32_t);
enum GFX_TextureType { GTT_0 };
void GFXtoOGLTarget(GFX_TextureType);
//GFXAPI_UpdateSubTexture(int, uint, uint, uint, uint, uint, void const *, ulong long, ulong long)
//GFXAPI_UnloadTexture(int, TEX_STATE)
//GFXAPI_StreamMalloc(ulong long, ulong long)
//GFXAPI_Shutdown(void)
inline void GFXAPI_SetFlipbookTextureIndexOverride(int v) { g_FlipbookTextureIndexOverride = v; }
//GFXAPI_SetFlipRenderTexture(bool)
//GFXAPI_ReplaceTextureWithRGBA(int, uint, uint, void const *)
//GFXAPI_NoesisCreateDevice(bool, int)
//GFXAPI_GetTargetSize(RenderTarget *)
//GFXAPI_GetQueryResult(int, ulong long &, ulong long)
//GFXAPI_GenerateTextureMips(int)
//GFXAPI_DestroyQuery(int)
//GFXAPI_CreateTextureFromRGB(int, uint, uint, void const *)
//GFXAPI_CreateTextureFromLuminanceU8(int, uint, uint, uchar const *)
//GFXAPI_CreateTextureFromLuminanceF32(int, uint, uint, float const *)
//GFXAPI_CreateShaderFromFile(int, char const *)
//GFXAPI_CreateQuery(int)
//GFXAPI_Build2dMipmaps(int, uint, uint, uchar const *, ulong long)
void TEXMGR_DumpAllGameWorldTextures();
//empty void TEXMGR_DumpUsageStats();
//assert-only void TEXMGR_LoadTextureFile(WAD_FILE_HEADER *);
void TEXMGR_OverrideHWHandle(int texh, uint32_t id);
enum dialogBoxStyle { DBS_0 };
void ZwiftFrame_Draw_f(float, float, float, float, dialogBoxStyle, uint32_t, float, bool, bool);
void ZwiftFrame_Draw_u(float, float, float, float, dialogBoxStyle, uint32_t, uint32_t, float, bool);
MATRIX44 *PC_GetWorldViewProjMatrix();
MATRIX44 *PC_GetWorldMatrix();
MATRIX44 *PC_GetViewMatrix();
void PC_CacheWorldViewProjMatrix(const MATRIX44 &);
void PC_CacheWorldMatrix(const MATRIX44 &);
void PC_CacheViewMatrix(const MATRIX44 &);
inline bool g_bSkinMeshMaterialCullingHack, g_HologramTest;
inline VEC4 g_SkinColor, g_SkinHologramOverrideColor;
inline int g_PowerupTest = -1, g_FacialHairMaterialIndex = -1, g_PowerupInvisShaderHandle = -1, g_PowerupBlinkCycle, g_HologramMaterial = -1;
inline float g_FacialHairUVOffset, g_FacialHairUVOffset1;