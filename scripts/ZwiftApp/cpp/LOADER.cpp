#include "ZwiftApp.h"
void LOADER_COLInit(uint32_t) {
    //TODO
}
void LOADER_CompressToFile(const char *, const char *, const char *) {
    //TODO
}
void LOADER_ContainsGamepath(const char *) {
    //TODO
}
void LOADER_DebugGetName(uint32_t, WAD_ASSET_TYPE) {
    //TODO
}
void LOADER_DecompressFileToMemory(std::string, void **, int &, uint32_t) {
    //TODO
}
void LOADER_DecompressToFile(void *, int, const char *) {
    //TODO
}
void LOADER_DecompressToMemory(void *, int, void **, int &, uint32_t) {
    //TODO
}
void LOADER_DumpAllGameWorldGDEs() {
    //TODO
}
void LOADER_FileExists(std::string) {
    //TODO
}
void LOADER_FindAssetsBySignature(uint32_t, WAD_ASSET_TYPE) {
    //TODO
}
void LOADER_FindAssetsByType(WAD_ASSET_TYPE) {
    //TODO
}
void LOADER_GetFileModifiedTime(std::string) {
    //TODO
}
void LOADER_GetHashedFileName(std::string, std::string) {
    //TODO
}
void LOADER_HashAndCompressXML(std::string) {
    //TODO
}
//void LOADER_IsValidCompAssetHeader(ZCompAssetHeader *);
void LOADER_ListMeshResourceSizes() {
    //TODO
}
int LOADER_LoadGdeFile(GDE_Header_360 *, const char *, uint32_t, uint32_t, int, bool) {
    //TODO
    return 0;
}
int LOADER_LoadGdeFile(const char *, bool) {
    //TODO
    return 0;
}
int LOADER_LoadGdeFileWFH(WAD_FILE_HEADER *, bool) {
    //TODO
    return 0;
}
int LOADER_LoadGdeFile_LEAN(GDE_Header_360 *, const char *, uint32_t, uint32_t, int, bool) {
    //TODO
    return 0;
}
void LOADER_LoadNavMesh() {
    //TODO
}
void LOADER_LoadTexture(uint32_t, WAD_ASSET_TYPE) {
    //TODO
}
void LOADER_SetFatBitVertexStreamIfNecessary(GDE_360_TIE *) {
    //TODO
}
void LOADER_UnloadGdeFile(int) {
    //TODO
}
void LOADER_UpdateFileList(std::string, std::string, std::string &) {
    //TODO
}
void LOADER_UseHWInstancing(bool en) {
    g_UseHWInstancing = en;
    Log("[LOADER]: Hardware instancing %s", en ? "enabled" : "disabled");
}