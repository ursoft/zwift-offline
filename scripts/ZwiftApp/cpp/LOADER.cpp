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
bool LOADER_IsValidCompAssetHeader(const char *data /*ZCompAssetHeader **/) {
    return data && data[0] == 'Z' && data[1] == 'H' && data[2] == 'R' && data[3] == 1; //*.ztx
}
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
size_t ZLIB_Decompress(uint8_t *src, int uncompr_sz, uint8_t *dest) {
    size_t ret = 0;
    z_stream s = {};
    char chunk[16384] = {};
    if (!inflateInit(&s)) {
        do {
            uInt a_in = s.avail_in = std::min(uInt(uncompr_sz), uInt(sizeof(chunk)));
            if (!a_in)
                break;
            s.next_in = (Bytef *)src;
            do {
                s.avail_out = sizeof(chunk);
                s.next_out = (Bytef *)chunk;
                int iret = inflate(&s, 0);
                if (iret != Z_OK && iret != Z_STREAM_END) {
                    inflateEnd(&s);
                    return 0;
                }
                int dec_sz = sizeof(chunk) - s.avail_out;
                memcpy(&dest[ret], chunk, dec_sz);
                ret += dec_sz;
            } while (!s.avail_out);
            uncompr_sz -= a_in;
            src += a_in;
        } while (uncompr_sz >= 0);
        inflateEnd(&s);
    }
    return ret;
}
size_t ZLIB_Compress(const void *src, size_t src_len, void *dest, size_t dest_len) {
    uLongf ret = (uLongf)dest_len;
    z_stream s = {};
    if (Z_OK == compress((Bytef *)dest, &ret, (const Bytef *)src, (uLongf)src_len)) {
        return ret;
    }
    return 0;
}
