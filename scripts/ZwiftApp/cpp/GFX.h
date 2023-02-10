#pragma once
enum AssetCategory { AC_UNK, AC_1, AC_2, AC_CNT };
struct GFX_InitializeParams {
    int WINWIDTH;
    int WINHEIGHT;
    bool FullScreen;
    int VSYNC;
    int GPU;
    int GFX_TIER;
    bool HasPickingBuf;
    int PREFERRED_MONITOR;
    char *m_vendorName;
    char *m_renderer;
    int64_t field_3C;
    int16_t field_44;
};
struct GfxVendorParams {
    const char *m_vendorName, *m_renderer, *m_version;
};
enum ZwiftPerfFlags { ZPF_DISABLE_AUTO_BRIGHT = 0x4000000 };
enum PerformanceGroup { GPG_BASIC, GPG_MEDIUM, GPG_HIGH, GPG_ULTRA, GPG_CNT };
namespace GfxConfig {
    inline int gLODBias, gFXAA;
}
inline float g_TargetBatteryFPS;
inline size_t g_TotalMemoryInKilobytes;
inline AssetCategory g_CurrentAssetCategory = AC_UNK;
inline uint64_t g_GFX_PerformanceFlags;
inline PerformanceGroup g_GFX_Performance = GPG_ULTRA;
inline int g_nSkipMipCount;
inline GfxVendorParams g_GL{"", "", ""};
inline bool g_openglDebug, g_openglCore, g_bGFXINITIALIZED;
inline char g_strCPU[0x40];
inline uint32_t g_BlurShaderHandle;
enum DetailedRender { DR_NO, DR_MIDDLE, DR_VERBOSE };
inline DetailedRender g_renderDetailed = DR_VERBOSE;

void GFX_SetLoadedAssetMode(bool);
bool GFX_Initialize();
bool GFXAPI_Initialize(const GFX_InitializeParams &);
bool GFX_Initialize(const GFX_InitializeParams &);
bool GFX_Initialize(uint32_t, uint32_t, bool, bool, uint32_t, const char *, const char *);
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
uint32_t GFX_CreateShaderFromFile(const char *fileName, int);