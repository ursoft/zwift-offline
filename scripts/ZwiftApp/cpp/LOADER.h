#pragma once
inline bool g_UseHWInstancing;

void LOADER_COLInit(uint32_t);
void LOADER_CompressToFile(const char *, const char *, const char *);
void LOADER_ContainsGamepath(const char *);
void LOADER_DebugGetName(uint32_t, WAD_ASSET_TYPE);
void LOADER_DecompressFileToMemory(std::string, void **, int &, uint32_t);
void LOADER_DecompressToFile(void *, int, const char *);
void LOADER_DecompressToMemory(void *, int, void **, int &, uint32_t);
void LOADER_DumpAllGameWorldGDEs();
void LOADER_FileExists(std::string);
void LOADER_FindAssetsBySignature(uint32_t, WAD_ASSET_TYPE);
void LOADER_FindAssetsByType(WAD_ASSET_TYPE);
void LOADER_GetFileModifiedTime(std::string);
void LOADER_GetHashedFileName(std::string, std::string);
void LOADER_HashAndCompressXML(std::string);
//void LOADER_IsValidCompAssetHeader(ZCompAssetHeader *);
void LOADER_ListMeshResourceSizes();
int LOADER_LoadGdeFile(GDE_Header_360 *, const char *, uint32_t, uint32_t, int, bool);
int LOADER_LoadGdeFile(const char *, bool);
int LOADER_LoadGdeFileWFH(WAD_FILE_HEADER *, bool);
int LOADER_LoadGdeFile_LEAN(GDE_Header_360 *, const char *, uint32_t, uint32_t, int, bool);
void LOADER_LoadNavMesh();
void LOADER_LoadTexture(uint32_t, WAD_ASSET_TYPE);
void LOADER_SetFatBitVertexStreamIfNecessary(GDE_360_TIE *);
void LOADER_UnloadGdeFile(int);
void LOADER_UpdateFileList(std::string, std::string, std::string &);
void LOADER_UseHWInstancing(bool);