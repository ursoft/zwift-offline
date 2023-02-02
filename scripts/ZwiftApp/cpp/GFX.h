#pragma once
enum AssetCategory { AC_UNK, AC_1, AC_2, AC_CNT };
extern AssetCategory g_CurrentAssetCategory;
void GFX_SetLoadedAssetMode(bool);