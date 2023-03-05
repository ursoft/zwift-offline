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
void GDE_UpgradeMesh(GDE_Mesh *pMesh) {
    if (pMesh->m_version == 9) {
        for (int i = 0; i < pMesh->m_meshItemsCnt; i++)
            pMesh->m_meshItems[i].m_itemKind = 4;
    }
}
bool GFX_PerfPenalty() {
    return (g_GFX_PerformanceFlags & (GPF_BIG_PERF_PENALTY | GPF_SMALL_PERF_PENALTY)) != 0;
}
int __fastcall LOAD_CHARACTER_SkinGdeFile_LEAN(GDE_Header_360 *file, char *name, uint32_t fileLength, int handle) {
    //TODO
    return -1;
}
int Internal_LOAD_CHARACTER_FromGdeFileInWad(char *name) {
    struct _stat64i32 st;
    int sr;
    time_t wadTouchTime = 0;
    auto wad = g_WADManager.GetWadFileHeaderByItemName(name + 5, WAD_ASSET_TYPE::GDE, &wadTouchTime);
    if (wad && ((sr = _stat64i32(name, &st), st.st_mtime < wadTouchTime) || sr))
        return LOAD_CHARACTER_SkinGdeFile_LEAN((GDE_Header_360 *)wad->FirstChar(), name, wad->m_fileLength, -1);
    return -1;
}
int Internal_LOAD_CHARACTER_FromGdeFileOnDisk(char *name, int handle) {
    auto f = fopen(GAMEPATH(name), "rb");
    if (!f)
        return -1;
    fseek(f, 0, SEEK_END);
    auto fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    auto h = (GDE_Header_360 *)malloc(fileSize);
    fread(h, 1, fileSize, f);
    fclose(f);
    auto ret = LOAD_CHARACTER_SkinGdeFile_LEAN(h, name, fileSize, handle);
    free(h);
    return ret;
}
int LOAD_CHARACTER_SkinGdeFile(char *name) {
    auto handle = GDEMESH_Internal_FindLoadedMesh(name, false);
    if ((unsigned)handle < _countof(g_ENG_InstanceResources))
    {
        if (g_ENG_InstanceResources[handle].m_gdeFile)
            return handle;
        Log("Found previous mesh slot for %s", g_ENG_InstanceResources[handle].m_gdeName);
    }
    handle = Internal_LOAD_CHARACTER_FromGdeFileInWad(name);
    if (handle == -1)
        return Internal_LOAD_CHARACTER_FromGdeFileOnDisk(name, handle);
    return handle;
}
int g_nMeshReloads;
bool GFX_Internal_LoadOnDemandMeshHandle(int handle, int magic) {
    auto m_gdeName = g_ENG_InstanceResources[handle].m_gdeName;
    int handleReturned = (magic == GDE_MESH) ? LOADER_LoadGdeFile(m_gdeName, g_ENG_InstanceResources[handle].m_manyInstances) : LOAD_CHARACTER_SkinGdeFile(m_gdeName);
    zassert(handleReturned == handle);
    if (handleReturned == -1)
        return false;
    ++g_nMeshReloads;
    //nop("Reloaded mesh %s (instanced = %s) [nReloads = %d]");
    return true;
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
void LOADER_LoadGdeFileRuntime(GDE_Header_360 *file, GDE_Header_360 *headerCopy) {
    static_assert(sizeof(GDE_Runtime) == 0x28);
    static_assert(sizeof(GDE_RuntimeItem) == 152);
    if (!file->m_runtime || file == headerCopy || headerCopy == nullptr)
        return;
    headerCopy->m_runtime = nullptr;
    GDE_Runtime **pNextRuntimeDest = &headerCopy->m_runtime;
    bool ok = true;
    for (auto runtime = file->m_runtime; ok && runtime; runtime = runtime->m_next) {
        ShiftPointer(&runtime, file);
        if (runtime->m_sign1 != 'GDS ') { //Ursoft FIX, originally was endless while
            Log("WARNING: A GDE run time data appears corrupted. All run time data will be ignored.");
            ok = false;
            break;
        }
        if (runtime->m_sign2 == 'GATD' || runtime->m_sign2 == 'LITE') {
            ShiftPointer(&runtime->m_runtimeItems, file);
            auto newRuntime = (GDE_Runtime *)malloc(sizeof(GDE_Runtime));
            if (!newRuntime) { ok = false; break; }
            *newRuntime = *runtime;
            newRuntime->m_runtimeItems = (GDE_RuntimeItem *)malloc(sizeof(GDE_RuntimeItem) * newRuntime->m_runtimeItemsCnt);
            if (!newRuntime->m_runtimeItems) { free(newRuntime); ok = false; break; }
            memmove(newRuntime->m_runtimeItems, runtime->m_runtimeItems, sizeof(GDE_RuntimeItem) * newRuntime->m_runtimeItemsCnt);
            newRuntime->m_next = nullptr;
            *pNextRuntimeDest = newRuntime;
            pNextRuntimeDest = &newRuntime->m_next;
        } else {
            Log("WARNING: Encountered unknown data type when interpreting run time data. All run time data will be ignored.");
            ok = false; 
            break;
        }
    }
    ShiftPointer(&file->m_runtime, file);
    if (!ok) {
        if (headerCopy && headerCopy->m_runtime) {
            auto v18 = headerCopy->m_runtime;
            do {
                if (v18->m_sign2 == 'GATD' || v18->m_sign2 == 'LITE')
                    free(v18->m_runtimeItems);
                auto v19 = v18->m_next;
                free(v18);
                v18 = v19;
            } while (v18);
            headerCopy->m_runtime = nullptr;
        }
    }
}
int g_textureToHandleTable[1024]; //TODO
int GDE_OptimizeLod(uint32_t a1, uint32_t meshItemsCnt) {
    if (GFX_PerfPenalty() && meshItemsCnt > 1 && a1 == 0)
        return 1;
    return std::min(a1, meshItemsCnt - 1);
}
void GDE_CopyMeshItemData0(GDE_MeshItemData0 *dest, GDE_MeshItemData0 *src, uint32_t cnt, VEC3 *mn, VEC3 *mx) {
    for (int i = 0; i < cnt; i++) {
        *dest = *src;
        dest->m_field_1B = 0;
        dest->m_field_1F = 0;
        dest->m_field_23 = 0;
        mx->m_data[0] = fmaxf(dest->m_point.m_data[0], mx->m_data[0]);
        mx->m_data[1] = fmaxf(dest->m_point.m_data[1], mx->m_data[1]);
        mx->m_data[2] = fmaxf(dest->m_point.m_data[2], mx->m_data[2]);
        mn->m_data[0] = fminf(dest->m_point.m_data[0], mn->m_data[0]);
        mn->m_data[1] = fminf(dest->m_point.m_data[1], mn->m_data[1]);
        mn->m_data[2] = fminf(dest->m_point.m_data[2], mn->m_data[2]);
        dest++; src++;
    }
}
void GDE_CopyMeshItemData1(GDE_MeshItemData1 *dest, GDE_MeshItemData1 *src, uint32_t cnt, VEC3 *mn, VEC3 *mx) {
    for (int i = 0; i < cnt; i++) {
        *dest = *src;
        dest->m_field_23 = 0;
        dest->m_field_27 = 0;
        dest->m_field_2B = 0;
        mx->m_data[0] = fmaxf(dest->m_point.m_data[0], mx->m_data[0]);
        mx->m_data[1] = fmaxf(dest->m_point.m_data[1], mx->m_data[1]);
        mx->m_data[2] = fmaxf(dest->m_point.m_data[2], mx->m_data[2]);
        mn->m_data[0] = fminf(dest->m_point.m_data[0], mn->m_data[0]);
        mn->m_data[1] = fminf(dest->m_point.m_data[1], mn->m_data[1]);
        mn->m_data[2] = fminf(dest->m_point.m_data[2], mn->m_data[2]);
        dest++; src++;
    }
}
void GDE_CopyMeshItemData2(GDE_MeshItemData2 *dest, GDE_MeshItemData2 *src, uint32_t cnt, VEC3 *mn, VEC3 *mx) {
    for (int i = 0; i < cnt; i++) {
        *dest = *src;
        dest->m_field_23 = 0;
        dest->m_field_27 = 0;
        dest->m_field_1F = 0;
        mx->m_data[0] = fmaxf(dest->m_point.m_data[0], mx->m_data[0]);
        mx->m_data[1] = fmaxf(dest->m_point.m_data[1], mx->m_data[1]);
        mx->m_data[2] = fmaxf(dest->m_point.m_data[2], mx->m_data[2]);
        mn->m_data[0] = fminf(dest->m_point.m_data[0], mn->m_data[0]);
        mn->m_data[1] = fminf(dest->m_point.m_data[1], mn->m_data[1]);
        mn->m_data[2] = fminf(dest->m_point.m_data[2], mn->m_data[2]);
        dest++; src++;
    }
}
void GDE_CopyMeshItemData3(GDE_MeshItemData3 *dest, GDE_MeshItemData3 *src, uint32_t cnt, VEC3 *mn, VEC3 *mx) {
    for (int i = 0; i < cnt; i++) {
        *dest = *src;
        dest->m_field_2B = 0;
        dest->m_field_27 = 0;
        dest->m_field_2F = 0;
        mx->m_data[0] = fmaxf(dest->m_point.m_data[0], mx->m_data[0]);
        mx->m_data[1] = fmaxf(dest->m_point.m_data[1], mx->m_data[1]);
        mx->m_data[2] = fmaxf(dest->m_point.m_data[2], mx->m_data[2]);
        mn->m_data[0] = fminf(dest->m_point.m_data[0], mn->m_data[0]);
        mn->m_data[1] = fminf(dest->m_point.m_data[1], mn->m_data[1]);
        mn->m_data[2] = fminf(dest->m_point.m_data[2], mn->m_data[2]);
        dest++; src++;
    }
}
uint32_t GDE_Color(uint32_t src) { return (src & 0xFF00FF00) | ((uint8_t)src << 16) | (uint8_t)(src >> 16); } //QUEST: stored with bytes swapped???
int g_gdeStat[21]; //QUEST: 6219 in ZA ??? - looks like debugging stuff
GDE_Header_360 *g_gdeMinToStat;
uint64_t g_gdeOffset;
void GDE_StoreStat(GdeStat gdeStatKind, int fileSize, GDE_Header_360 *gde) { //бред
    if (gde >= g_gdeMinToStat && gde < g_gdeMinToStat + g_gdeOffset / sizeof(GDE_Header_360 *) && (int)gdeStatKind < 21)
        g_gdeStat[gdeStatKind] += fileSize;
}
int LOADER_LoadGdeFile_LEAN(GDE_Header_360 *file, const char *name, uint32_t fileSize, uint32_t nInstances, int handle) {
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
                    LogDebug("LOADER_LoadGdeFile_LEAN() Unable to load texture file %s, from %s", name, destTextures[t].m_name);
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
    LOADER_LoadGdeFileRuntime(file, headerCopy);
    auto VRAMBytes_VBO = g_VRAMBytes_VBO;
    if (file->m_meshSign == GDE_MESH) {
        ShiftPointer(&file->m_mesh, file);
        static_assert(sizeof(GDE_Mesh) == 0x1F8);
        auto newMesh = (GDE_Mesh *)malloc(sizeof(GDE_Mesh));
        headerCopy->m_mesh = newMesh;
        *newMesh = *file->m_mesh;
        auto mesh = file->m_mesh;
        if (mesh->m_version != 10) {
            LogDebug("LOADER_LoadGdeFile_LEAN() Loading old version of mesh, %s, filesize, %d", name, fileSize);
        }
        g_ENG_InstanceResources[handle].m_bounds = mesh->m_bounds;
        GDE_UpgradeMesh(mesh);
        auto optLod = GDE_OptimizeLod(0, mesh->m_meshItemsCnt);
        auto checkedInstCnt = std::min(std::min(1u, nInstances), 16u);
        newMesh->m_meshItems[0].m_instancesCount = checkedInstCnt;
        VEC3 minCoord{ +1.0e10f, +1.0e10f, +1.0e10f };
        VEC3 maxCoord{ -1.0e10f, -1.0e10f, -1.0e10f };
        for (auto mi = optLod; mi < mesh->m_meshItemsCnt; mi++) {
            static_assert(sizeof(GDE_MeshItem) == 80);
            auto &meshItem = mesh->m_meshItems[mi];
            auto &newMeshItem = newMesh->m_meshItems[mi];
            auto subItemArSize = sizeof(GDE_MeshSubItem) * ((meshItem.m_subItemEndPtr - meshItem.m_subItemBegPtr) / sizeof(GDE_MeshSubItem));
            auto newMeshSubItems = (GDE_MeshSubItem *)malloc(subItemArSize);
            memmove(newMeshSubItems, (char *)file + (uint64_t)meshItem.m_subItemBegPtr, subItemArSize);
            newMeshItem = meshItem; //QUEST: already copied?
            newMeshItem.m_vbHandle = newMeshItem.m_field_4 = -1;
            newMeshItem.m_subItemBegPtr = newMeshSubItems;
            newMeshItem.m_instancesCount = checkedInstCnt;
            auto itemKind = meshItem.m_itemKind;
            auto numVerts = meshItem.m_numVerts;
            GDE_MeshItemData0 *p0;
            GDE_MeshItemData1 *p1;
            GDE_MeshItemData2 *p2;
            GDE_MeshItemData3 *p3;
            GDE_MeshItemData4 *p4;
            uint32_t vbSize = 0;
            if (numVerts) {
                switch (itemKind) {
                case 0:
                    ShiftPointer(&meshItem.m_pVerts, file);
                    static_assert(36 == sizeof(GDE_MeshItemData0));
                    newMeshItem.m_pVerts = p0 = (GDE_MeshItemData0 *)malloc(vbSize = sizeof(GDE_MeshItemData0) * newMeshItem.m_numVerts);
                    GDE_CopyMeshItemData0(p0, (GDE_MeshItemData0 *)meshItem.m_pVerts, newMeshItem.m_numVerts , &minCoord, &maxCoord);
                    for (int i = 0; i < numVerts; ++i)
                        p0[i].m_color = GDE_Color(((GDE_MeshItemData0 *)meshItem.m_pVerts + i)->m_color);
                    break;
                case 4:
                    ShiftPointer(&meshItem.m_pVerts, file);
                    static_assert(48 == sizeof(GDE_MeshItemData4));
                    static_assert(72 == sizeof(GDE_MeshItemData4_file));
                    newMeshItem.m_pVerts = p4 = (GDE_MeshItemData4 *)malloc(vbSize = sizeof(GDE_MeshItemData4) * newMeshItem.m_numVerts);
                    for (int i = 0; i < numVerts; ++i) {
                        auto p4src_i = (GDE_MeshItemData4_file *)meshItem.m_pVerts + i;
                        p4[i].m_point = p4src_i->m_point;
                        static_assert(sizeof(p4[i].m_raw) == sizeof(p4src_i->m_raw));
                        memcpy((char *)&p4[i].m_raw, (char *)&p4src_i->m_raw, sizeof(p4src_i->m_raw));
                        p4[i].m_color1 = GDE_Color(p4src_i->m_color1);
                        p4[i].m_color2 = GDE_Color(p4src_i->m_color2);
                        GDE_NormalizeVector(&p4src_i->m_pointN, &p4src_i->m_pointN);
                        p4[i].m_pointN = p4src_i->m_pointN;
                        p4[i].m_field_27 = 0;
                        p4[i].m_point1 = p4src_i->m_point1;
                        p4[i].m_field_2B = 0;
                        p4[i].m_point2 = p4src_i->m_point2;
                        p4[i].m_field_2F = 0;
                        if (p4src_i->m_point.m_data[0] > maxCoord.m_data[0])
                            maxCoord.m_data[0] = p4src_i->m_point.m_data[0];
                        if (p4src_i->m_point.m_data[1] > maxCoord.m_data[1])
                            maxCoord.m_data[1] = p4src_i->m_point.m_data[1];
                        if (p4src_i->m_point.m_data[2] > maxCoord.m_data[2])
                            maxCoord.m_data[2] = p4src_i->m_point.m_data[2];
                        if (minCoord.m_data[0] > p4src_i->m_point.m_data[0])
                            minCoord.m_data[0] = p4src_i->m_point.m_data[0];
                        if (minCoord.m_data[1] > p4src_i->m_point.m_data[1])
                            minCoord.m_data[1] = p4src_i->m_point.m_data[1];
                        if (minCoord.m_data[2] > p4src_i->m_point.m_data[2])
                            minCoord.m_data[2] = p4src_i->m_point.m_data[2];
                    }
                    break;
                case 1:
                    ShiftPointer(&meshItem.m_pVerts, file);
                    static_assert(44 == sizeof(GDE_MeshItemData1));
                    newMeshItem.m_pVerts = p1 = (GDE_MeshItemData1 *)malloc(vbSize = sizeof(GDE_MeshItemData1) * newMeshItem.m_numVerts);
                    GDE_CopyMeshItemData1(p1, (GDE_MeshItemData1 *)meshItem.m_pVerts, newMeshItem.m_numVerts, &minCoord, &maxCoord);
                    for (int i = 0; i < newMeshItem.m_numVerts; i++)
                        p1[i].m_color = GDE_Color(((GDE_MeshItemData1 *)meshItem.m_pVerts)[i].m_color);
                    break;
                case 2:
                    ShiftPointer(&meshItem.m_pVerts, file);
                    static_assert(40 == sizeof(GDE_MeshItemData2));
                    newMeshItem.m_pVerts = p2 = (GDE_MeshItemData2 *)malloc(vbSize = sizeof(GDE_MeshItemData2) * newMeshItem.m_numVerts);
                    GDE_CopyMeshItemData2(p2, (GDE_MeshItemData2 *)meshItem.m_pVerts, newMeshItem.m_numVerts, &minCoord, &maxCoord);
                    for (int i = 0; i < newMeshItem.m_numVerts; i++) {
                        p2[i].m_color1 = GDE_Color(((GDE_MeshItemData2 *)meshItem.m_pVerts)[i].m_color1);
                        p2[i].m_color2 = GDE_Color(((GDE_MeshItemData2 *)meshItem.m_pVerts)[i].m_color2);
                    }
                    break;
                default: //3?
                    ShiftPointer(&meshItem.m_pVerts, file);
                    static_assert(48 == sizeof(GDE_MeshItemData3));
                    newMeshItem.m_pVerts = p3 = (GDE_MeshItemData3 *)malloc(vbSize = sizeof(GDE_MeshItemData3) * newMeshItem.m_numVerts);
                    GDE_CopyMeshItemData3(p3, (GDE_MeshItemData3 *)meshItem.m_pVerts, newMeshItem.m_numVerts, &minCoord, &maxCoord);
                    for (int i = 0; i < newMeshItem.m_numVerts; i++) {
                        p3[i].m_color1 = GDE_Color(((GDE_MeshItemData3 *)meshItem.m_pVerts)[i].m_color1);
                        p3[i].m_color2 = GDE_Color(((GDE_MeshItemData3 *)meshItem.m_pVerts)[i].m_color2);
                    }
                    break;
                }
            }
            if (numVerts && newMeshItem.m_pVerts) {
                if (newMeshItem.m_instancesCount == 1) {
                    if (newMeshItem.m_pUnused)
                        Log("%s: has data in unused", name);
                    GFX_CreateVertexBuffer(&newMeshItem, vbSize, newMeshItem.m_pVerts);
                    free(newMeshItem.m_pVerts);
                    newMeshItem.m_pVerts = nullptr;
                } else {
                    zassert(0);
                }
            } else {
                LogDebug("Problem with vertexbuffer! filename[%s], lod[%i], numVerts=%i, pointer to verts is valid(0x%p)", name, mi, newMeshItem.m_numVerts, newMeshItem.m_pVerts);
                newMeshItem.m_vbHandle = -1;
            }
            int stripIdx = 0;
            auto curSubItem = meshItem.m_subItemBegPtr;
            while (curSubItem != meshItem.m_subItemEndPtr) {
                ShiftPointer(&curSubItem->m_pIndices, file);
                uint32_t ibSize = ((curSubItem->m_numIndices <= 0xFFFF) ? 2 : 4) * curSubItem->m_numIndices;
                auto pIndices = (uint8_t *)malloc(ibSize);
                memmove(pIndices, curSubItem->m_pIndices, ibSize);
                curSubItem->m_pIndices = pIndices;
                if (curSubItem->m_numIndices && pIndices) {
                    if (newMeshItem.m_instancesCount == 1) {
                        GFX_CreateIndexBuffer(&curSubItem->m_ibHandle, ibSize, pIndices);
                        free(curSubItem->m_pIndices);
                        curSubItem->m_pIndices = nullptr;
                    } else {
                        zassert(0);
                    }
                    switch (itemKind) {
                    case 0:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR1_UV1;
                        break;
                    case 1:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR1_UV2;
                        break;
                    case 2:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR2_UV1;
                        break;
                    default: //3,4
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR2_UV2;
                        break;
                    }
                } else {
                    LogDebug("Problem with indexbuffer! filename[%s], lod[%i], strip[%i], numIndices=%i, pointer to indices is valid(0x%x)", name, mi, stripIdx, curSubItem->m_numIndices, pIndices);
                    newMeshItem.m_vbHandle = -1;
                }
                ++curSubItem;
                ++stripIdx;
            }
            newMesh->m_bounds.m_data[0] = (minCoord.m_data[0] + maxCoord.m_data[0]) * 0.5;
            newMesh->m_bounds.m_data[1] = (minCoord.m_data[1] + maxCoord.m_data[1]) * 0.5;
            newMesh->m_bounds.m_data[2] = (minCoord.m_data[1] + maxCoord.m_data[1]) * 0.5;
            newMesh->m_bounds.m_data[3] = sqrtf((maxCoord.m_data[1] - minCoord.m_data[1]) * 0.5 * (maxCoord.m_data[1] - minCoord.m_data[1]) * 0.5 + (maxCoord.m_data[0] - minCoord.m_data[0]) * 0.5 * (maxCoord.m_data[0] - minCoord.m_data[0]) * 0.5 + (maxCoord.m_data[2] - minCoord.m_data[2]) * 0.5 * (maxCoord.m_data[2] - minCoord.m_data[2]) * 0.5);
            GDE_StoreStat(GS_LEAN, fileSize, file);
        }
    }
    auto &res = g_ENG_InstanceResources[handle];
    res.m_gdeFile = headerCopy;
    if (res.m_gdeName == nullptr) {
        res.m_gdeName = strdup(name);
        res.m_gdeNameCRC = SIG_CalcCaseSensitiveSignature(name);
    }
    res.m_isSkin = false;
    res.m_field_1A_1 = true;
    res.m_state = IRS_INITIAL;
    res.m_manyInstances = nInstances > 1;
    res.m_vramUsed = g_VRAMBytes_VBO - VRAMBytes_VBO;
    res.m_creationTime = timeGetTime();
    res.m_assetCategory = g_CurrentAssetCategory;
    return handle;
}
int LOADER_LoadGdeFile(GDE_Header_360 *file, const char *name, uint32_t fileSize, uint32_t nInstances, int handle) { //LOADER_LoadGdeFile_0
    if (nInstances == 1)
        return LOADER_LoadGdeFile_LEAN(file, name, fileSize, 1, handle);
    //TODO
    return -1;
}
int LOADER_LoadGdeFile(const char *name, bool manyInstances) {
    if (g_UseHWInstancing)
        manyInstances = false;
    int handle = -1;
    if (strstr(name, ".gde")) {
        handle = GDEMESH_Internal_FindLoadedMesh(name, manyInstances);
        if ((unsigned)handle >= _countof(g_ENG_InstanceResources) || nullptr == g_ENG_InstanceResources[handle].m_gdeFile) {
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
            handle = LOADER_LoadGdeFile((GDE_Header_360 *)fileBuf, name, fileSize, manyInstances ? 16 : 1, handle);
            if (!manyInstances) {
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
void LOADER_UnloadGdeFile(int handle) {
    auto &obj = g_ENG_InstanceResources[handle];
    auto gdeFile = obj.m_gdeFile;
    if (!gdeFile)
        return;
    auto mesh = gdeFile->m_mesh;
    if (!obj.m_manyInstances)
        GDE_FreeRuntime(gdeFile);
    if (gdeFile->m_meshSign == GDE_SKIN_MESH) {
        if (mesh) {
            if (obj.m_field_1A_1) {
                for (auto i = 0; i < gdeFile->m_texturesCnt; ++i)
                    free(gdeFile->m_textures[i].m_name);
                for (auto j = 0; j < gdeFile->m_animCnt; ++j) {
                    auto p0 = gdeFile->m_anims[j].m_pAnimators->m_field_0;
                    if (p0) {
                        free(p0->m_arr1);
                        free(p0->m_arr2);
                        free(p0);
                    }
                    free(gdeFile->m_anims[j].m_pAnimators);
                    gdeFile->m_anims[j].m_pAnimators = nullptr;
                }
                free(gdeFile->m_anims);
                gdeFile->m_anims = nullptr;
                free(gdeFile->m_textures);
                gdeFile->m_textures = nullptr;
                free(gdeFile->m_field_28);
                gdeFile->m_field_28 = nullptr;
            }
#if 0
            auto optLod = GDE_OptimizeLod(0, mesh->m_meshItems[3].m_vbHandle);
            v38 = mesh->m_meshItems + 32 * optLod + 8 * optLod; ///???
            while (optLod < mesh->m_meshItems[3].m_vbHandle) {
                v39 = *(v38 + 4);
                for (k = *v38; k != v39; k += 24) {
                    GFX_DestroyVertexBuffer(k + 7);
                    GFX_DestroyVertexBuffer(k + 5);
                }
                if (m_field_1A_1) {
                    free(*v38);
                    *v38 = 0i64;
                    *(v38 + 2) = 0i64;
                    *(v38 + 1) = 0i64;
                    *(v38 + 3) = 0i64;
                }
                ++optLod;
                v38 += 40;
            }
            if (obj.m_field_1A_1) {
                v41 = 0;
                v42 = gdeFile->field_40;
                do {
                    v43 = *(_QWORD *)v42;
                    if (*(_QWORD *)v42) {
                        free(*(void **)(v43 + 8));
                        v44 = *(void **)(v43 + 16);
                        *(_QWORD *)(v43 + 8) = nullptr;
                        free(v44);
                        *(_QWORD *)(v43 + 16) = nullptr;
                    }
                    ++v41;
                    v42 += 8;
                } while (v41 < 6);
            }
#endif //0
        }
    } else if (gdeFile->m_meshSign == GDE_MESH) {
        bool needFree = false;
        if (mesh->m_meshItems[0].m_instancesCount == 1) {
            needFree = 1;
            for (int m = 0; m < gdeFile->m_texturesCnt; ++m)
                free(gdeFile->m_textures[m].m_name);
            for (int n = 0; n < gdeFile->m_animCnt; ++n) {
                auto ptr = gdeFile->m_anims[n].m_pAnimators;
                free(ptr->m_field_0);
                free(ptr->m_pArr1);
                free(ptr->m_pArr2);
                free(ptr);
            }
            free(gdeFile->m_anims);
            free(gdeFile->m_textures);
            free(gdeFile->m_field_28);
        }
        for (auto mi = GDE_OptimizeLod(0, mesh->m_meshItemsCnt); mi < mesh->m_meshItemsCnt; ++mi) {
            auto &pi = mesh->m_meshItems[mi];
            for (auto sbi = pi.m_subItemBegPtr; sbi != pi.m_subItemEndPtr; ++sbi) {
                int v21 = 2;
                if ((obj.m_manyInstances || sbi->m_numIndices > 0xFFFF) && (v21 = 4, obj.m_manyInstances))
                    g_VRAMBytes_VBO -= (sbi->m_numIndices << 6);
                else
                    g_VRAMBytes_VBO -= (sbi->m_numIndices * v21);
                GFX_DestroyBuffer(&sbi->m_ibHandle);
                sbi->m_vertexHandle = -1;
            }
            if (needFree) {
                free(pi.m_subItemBegPtr);
                pi.m_subItemBegPtr = nullptr;
                pi.field_18 = nullptr;
                pi.field_10 = nullptr;
            }
            g_VRAMBytes_VBO -= pi.m_numVerts * (obj.m_manyInstances ? 576 : 36);
            GFX_DestroyBuffer(&pi.m_vbHandle);
        }
    } else {
        zassert(0);
    }
    free(gdeFile->m_mesh);
    gdeFile->m_mesh = nullptr;
    if (obj.m_field_1A_1)
        free(obj.m_gdeFile);
    obj.m_gdeFile = nullptr;
    obj.m_state = IRS_NEED_LOAD;
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
void LOADER_DumpAllGameWorldGDEs() {
    for (int i = 0; i < g_ENG_nResources; ++i) {
        if (g_ENG_InstanceResources[i].m_assetCategory == AC_2)
            LOADER_UnloadGdeFile(i);
    }
}