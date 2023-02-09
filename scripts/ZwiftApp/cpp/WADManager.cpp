#include "ZwiftApp.h"
const char *AssetType(WAD_ASSET_TYPE at) {
    const char *assetNames[] = { "gde", "sky", "coll", "bog", "snd", "entity", "moby",
        "tie", "shrub", "texture", "shader", "particle", "ui", "global", "nav", "pvar_include",
        "tuning_include", "???" };
    return assetNames[(int)at];
}
void WADManager::LoadWADFile(const char *name) {
    //TODO
}
void *WAD_FindAssetsByName(const char *pItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader) {
    if (!pItemPathName)
        return nullptr;
    return WAD_FindAssetsByCrcName(SIG_CalcCaseInsensitiveSignature(pItemPathName), type, pHeader);
}
void *WAD_FindAssetsByCrcName(uint32_t crcItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader) {
    if (!crcItemPathName)
        return nullptr;
    //TODO
    return nullptr;
}
WAD_FILE_HEADER *WADManager::GetWadFileHeaderByItemName(const char *pItemPathName, WAD_ASSET_TYPE type, uint64_t *ptr, WAD_FILE_HEADER **ptrFh) {
    //TODO
    return nullptr;
}
