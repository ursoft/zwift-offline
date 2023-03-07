#pragma once
inline bool g_UseHWInstancing;
template <typename T, typename F> void ShiftPointer(T **dest, const F *offset) {
    *dest = (T *)((uint8_t *)offset + (intptr_t)*dest);
}
void LOADER_COLInit(uint32_t);
void LOADER_CompressToFile(const char *, const char *, const char *);
void LOADER_ContainsGamepath(const char *);
void LOADER_DebugGetName(uint32_t, WAD_ASSET_TYPE);
void LOADER_DecompressFileToMemory(std::string, void **, int &, uint32_t);
void LOADER_DecompressToFile(void *, int, const char *);
void LOADER_DecompressToMemory(void *, int, void **, int &, uint32_t);
void LOADER_DumpAllGameWorldGDEs();
void LOADER_FileExists(std::string);
WAD_FILE_HEADER *LOADER_FindAssetsBySignature(uint32_t, WAD_ASSET_TYPE);
void LOADER_FindAssetsByType(WAD_ASSET_TYPE);
void LOADER_GetFileModifiedTime(std::string);
void LOADER_GetHashedFileName(std::string, std::string);
void LOADER_HashAndCompressXML(std::string);
bool LOADER_IsValidCompAssetHeader(const char * /*ZCompAssetHeader **/);
void LOADER_ListMeshResourceSizes();
int LOADER_LoadGdeFile(GDE_Header_360 *, const char *, uint32_t, uint32_t, int);
int LOADER_LoadGdeFile(const char *, bool);
int LOADER_LoadGdeFileWFH(WAD_FILE_HEADER *, bool);
int LOADER_LoadGdeFile_LEAN(GDE_Header_360 *, const char *, uint32_t, uint32_t, int);
void LOADER_LoadNavMesh();
void LOADER_LoadTexture(uint32_t, WAD_ASSET_TYPE);
void LOADER_SetFatBitVertexStreamIfNecessary(GDE_360_TIE *);
void LOADER_UnloadGdeFile(int);
void LOADER_UpdateFileList(std::string, std::string, std::string &);
void LOADER_UseHWInstancing(bool);
size_t ZLIB_Decompress(uint8_t *src, int uncompr_sz, uint8_t *dest);
size_t ZLIB_Compress(const void *src, size_t src_len, void *dest, size_t dest_len);
int LOAD_CHARACTER_SkinGdeFile(char *name);
bool GFX_Internal_LoadOnDemandMeshHandle(int handle, GDE_MeshKind gmk);
void LOADER_DumpAllGameWorldGDEs();
int LOAD_CHARACTER_SkinGdeFile_LEAN(GDE_Header_360 *file, char *name, uint32_t fileLength, int handle);