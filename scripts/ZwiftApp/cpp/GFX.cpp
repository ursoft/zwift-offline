#include "ZwiftApp.h"
AssetCategory g_CurrentAssetCategory = AC_UNK;
void GFX_SetLoadedAssetMode(bool mode) {
    g_CurrentAssetCategory = mode ? AC_1 : AC_2;
}