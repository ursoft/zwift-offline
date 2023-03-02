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
WAD_FILE_HEADER *LOADER_FindAssetsBySignature(uint32_t crc, WAD_ASSET_TYPE wat) { //QUEST: how these pointers are filled???
    WAD_FILE_HEADER *result = WAD_FindAssetsBySignature(crc, wat, g_levelWadGroup);
    if (!result)
    {
        result = WAD_FindAssetsBySignature(crc, wat, g_levelWadGroupP1);
        if (!result)
        {
            result = WAD_FindAssetsBySignature(crc, wat, g_globalWadGroup);
            if (!result)
            {
                result = WAD_FindAssetsBySignature(crc, wat, g_globalWadGroupP1);
                if (!result)
                {
                    result = WAD_FindAssetsBySignature(crc, wat, g_languageWadGroup);
                    if (!result)
                    {
                        result = WAD_FindAssetsBySignature(crc, wat, g_languageWadGroupP1);
                        if (!result)
                        {
                            result = WAD_FindAssetsBySignature(crc, wat, g_homeMenuGroup);
                            if (!result)
                            {
                                result = WAD_FindAssetsBySignature(crc, wat, g_homeMenuGroupP1);
                                if (!result)
                                {
                                    result = WAD_FindAssetsBySignature(crc, wat, g_bootWadGroup);
                                    if (!result)
                                    {
                                        result = WAD_FindAssetsBySignature(crc, wat, g_bootWadGroupP1);
                                        if (!result)
                                        {
                                            result = WAD_FindAssetsBySignature(crc, wat, g_legalWadGroup);
                                            if (!result)
                                                return WAD_FindAssetsBySignature(crc, wat, g_legalWadGroupP1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
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
void LOADER_LoadGdeFileAnim(GDE_Header_360 *file, GDE_Anim *pAnim, int *texs) {
    static_assert(sizeof(GDE_Tex) == 32);
    static_assert(sizeof(GDE_Anim) == 0x170);
    static_assert(sizeof(GDE_Header_360) == 0x70);
    static_assert(sizeof(InstanceResource) == 72);
    static_assert(sizeof(GDE_Animators) == 0x20);
    static_assert(sizeof(GDE_Animators_0) == 0x18);
    static_assert(_countof(pAnim->m_texIdx) == _countof(pAnim->m_texGlid));
    if (pAnim->m_hasAnimators) {
        GDE_Animators *pAnimators = pAnim->m_pAnimators;
        ShiftPointer(&pAnimators, file);
        GDE_Animators_0 *pAnimators_0 = pAnimators->m_field_0;
        ShiftPointer(&pAnimators_0, file);
        auto count = pAnimators_0->m_count;
        int **pArr1_src = pAnimators->m_pArr1;
        ShiftPointer(&pArr1_src, file);
        int *arr1_src = *pArr1_src;
        ShiftPointer(&arr1_src, file);
        int **pArr2_src = pAnimators->m_pArr2;
        ShiftPointer(&pArr2_src, file);
        int *arr2_src = *pArr2_src;
        ShiftPointer(&arr2_src, file);
        auto pNewAnimators = (GDE_Animators *)malloc(sizeof(GDE_Animators));
        auto pNewAnimators_0 = (GDE_Animators_0 *)malloc(sizeof(GDE_Animators_0));
        auto size = count * sizeof(int);
        auto arr1 = (int *)malloc(size);
        auto arr2 = (int *)malloc(size);
        if (!pNewAnimators || !pNewAnimators_0 || !arr1 || !arr2) {
            pAnim->m_hasAnimators = false;
            if (pNewAnimators)
                free(pNewAnimators);
            if (pNewAnimators_0)
                free(pNewAnimators_0);
            if (arr1)
                free(arr1);
            if (arr2)
                free(arr2);
            Log("Failed to allocated pAnimators because of allocation error");
        } else {
            memcpy(pNewAnimators_0, pAnimators_0, 16); //QUEST: why 16, not 8?
            memmove(arr1, arr1_src, size);
            memmove(arr2, arr2_src, size);
            pNewAnimators->m_field_0 = pNewAnimators_0;
            pNewAnimators->m_pArr1 = nullptr;
            pNewAnimators->m_pArr2 = nullptr;
            pNewAnimators->m_field_18 = nullptr;
            pNewAnimators_0->m_arr1 = arr1;
            pNewAnimators_0->m_arr2 = arr2;
            pAnim->m_pAnimators = pNewAnimators;
        }
        pAnim->m_field_74 |= 0x10u;
    } else {
        pAnim->m_pAnimators = nullptr;
    }
    pAnim->m_field_150 = nullptr;
    for (int i = 0; i < _countof(pAnim->m_texIdx); i++)
        pAnim->m_texGlid[0] = ((unsigned)pAnim->m_texIdx[i] >= file->m_texturesCnt) ? -1 : texs[pAnim->m_texIdx[i]];
    for (int i = 0; i < _countof(pAnim->m_pUsage); i++) {
        ShiftPointer(&pAnim->m_pUsage->m_pConstantParams, file);
        ShiftPointer(&pAnim->m_pUsage->m_somePtr, file);
        ShiftPointer(&pAnim->m_pUsage->m_pAnumBindings, file);
    }
}
int g_textureToHandleTable[1024]; //TODO
int LOADER_LoadGdeFile_LEAN(GDE_Header_360 *file, const char *name, uint32_t size, uint32_t nInstances, int handle) {
    zassert(nInstances == 1);
    auto headerCopy = (GDE_Header_360 *)malloc(sizeof(GDE_Header_360));
    *headerCopy = *file;
    if (handle == -1) {
        handle = GDEMESH_GetFreeMeshResourceHandle();
        if (handle == -1)
            return -1;
    }
    const uint32_t expGdeVer = 0x10012;
    if (file->m_ver != expGdeVer) {
        LogLev(LL_WARNING, "Game expects GDE file version 0x%08X, but GDE file \"%s\" is actually version 0x%08X.", expGdeVer, name, file->m_ver);
        return -1;
    }
    ShiftPointer(&file->m_anims, file);
    ShiftPointer(&file->m_textures, file);
    ShiftPointer(&file->m_field_28, file);
    headerCopy->m_anims = (GDE_Anim *)malloc(sizeof(GDE_Anim) * file->m_animCnt);
    headerCopy->m_textures = (GDE_Tex *)malloc(sizeof(GDE_Tex) * file->m_texturesCnt);
    headerCopy->m_field_28 = (uint8_t *)malloc(16 * file->m_field_10); //sizeof smth
    zassert(file->m_texturesCnt < _countof(g_textureToHandleTable));
    auto destTextures = headerCopy->m_textures;
    auto srcTextures = file->m_textures;
    int texHandles[_countof(g_textureToHandleTable)];
    for (int t = 0; t < file->m_texturesCnt; t++) {
        if (srcTextures[t].m_name) {
            memcpy(destTextures + t, srcTextures + t, sizeof(GDE_Tex));
            srcTextures[t].m_name += (uint64_t)file;
            destTextures[t].m_name = strdup(srcTextures[t].m_name);
            auto v38 = strstr(destTextures[t].m_name, "YorkShire\\Textures");
            if (v38)
                v38[4] = 's';
            auto v39 = strstr(destTextures[t].m_name, ".htx");
            if (v39)
                strcpy(v39, ".tga");
        }
        if (destTextures[t].m_name == nullptr || strlen(destTextures[t].m_name)) {
            if (LOADER_FindAssetsBySignature(destTextures[t].m_assetCrc, WAD_ASSET_TYPE::TEXTURE)) {
                texHandles[t] = 0;
                zassert(0);
            } else {
                auto tgaHandle = GFX_CreateTextureFromTGAFile(destTextures[t].m_name, -1, true);
                if (tgaHandle == -1) {
                    const char *tgaAltName = nullptr;
                    int destNameLen = strlen(destTextures[t].m_name);
                    if (destNameLen > 0) {
                        auto destName = destTextures[t].m_name;
                        const char *dch = destName + destNameLen;
                        do {
                            if (*dch == '\\')
                                break;
                            --dch;
                            tgaAltName = destName + destNameLen;
                            destNameLen--;
                        } while (destNameLen > 0);
                        if (tgaAltName)
                            tgaHandle = GFX_CreateTextureFromTGAFile(tgaAltName, -1, true);
                    }
                }
                texHandles[t] = tgaHandle;
                if (tgaHandle == -1) {
                    LogLev(LL_DEBUG, "LOADER_LoadGdeFile_LEAN() Unable to load texture file %s, from %s", name, destTextures[t].m_name);
                    texHandles[t] = 0;
                }
            }
        }
    }
    for (int uIdx = 0; uIdx < file->m_animCnt; ++uIdx) {
        auto uSrc = file->m_anims + uIdx, uDest = headerCopy->m_anims + uIdx;
        LOADER_LoadGdeFileAnim(file, uSrc, texHandles);
        *uDest = *uSrc;
    }
    //TODO
    //v11 = -1i64;
    //v155 = qword_7FF7FBFD6238;
    return 0;
}
int LOADER_LoadGdeFile(GDE_Header_360 *file, const char *name, uint32_t fileSize, uint32_t nInstances, int handle) { //LOADER_LoadGdeFile_0
    if (nInstances == 1)
        return LOADER_LoadGdeFile_LEAN(file, name, fileSize, 1, handle);
    //TODO
    return -1;
}
int LOADER_LoadGdeFile(const char *name, bool temporary) {
    if (g_UseHWInstancing)
        temporary = false;
    int handle = -1;
    if (strstr(name, ".gde")) {
        handle = GDEMESH_Internal_FindLoadedMesh(name, temporary);
        if ((unsigned)handle >= _countof(g_ENG_InstanceResources) || nullptr == g_ENG_InstanceResources[handle].m_ptr) {
            name = GAMEPATH(name);
            auto f = fopen(name, "rb");
            if (!f)
                return -1;
            fseek(f, 0, SEEK_END);
            auto fileSize = ftell(f);
            if (fileSize < 0) {
                fclose(f);
                return -1;
            }
            fseek(f, 0, SEEK_SET);
            auto fileBuf = malloc(fileSize);
            g_VRAMBytes_GDERAM += fileSize;
            fread(fileBuf, 1, fileSize, f);
            fclose(f);
            handle = LOADER_LoadGdeFile((GDE_Header_360 *)fileBuf, name, fileSize, temporary ? 16 : 1, handle);
            if (!temporary) {
                //what for ??? memset(fileBuf_, 0, fileSize_);
                free(fileBuf);
                g_VRAMBytes_GDERAM -= fileSize;
            }
        }
    }
    return handle;
}
int LOADER_LoadGdeFileWFH(WAD_FILE_HEADER *, bool) {
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
