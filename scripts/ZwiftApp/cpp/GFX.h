#pragma once
enum AssetCategory { AC_UNK, AC_1, AC_2, AC_CNT };
struct GFX_InitializeParams {

};
enum ZwiftPerfFlags { ZPF_DISABLE_AUTO_BRIGHT = 0x4000000 };

inline AssetCategory g_CurrentAssetCategory = AC_UNK;
inline uint64_t g_GFX_PerformanceFlags;

void GFX_SetLoadedAssetMode(bool);
bool GFX_Initialize();
bool GFXAPI_Initialize(const GFX_InitializeParams &);
bool GFX_Initialize(const GFX_InitializeParams &);
bool GFX_Initialize(uint32_t, uint32_t, bool, bool, uint32_t, const char *, const char *);
bool GFX_Initialize3DTVSpecs(float, float);
void GFX_DrawInit();
int64_t GFX_GetPerformanceFlags();
void GFX_AddPerformanceFlags(uint64_t f);