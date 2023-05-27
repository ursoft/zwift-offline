#include "ZwiftApp.h"
int g_textureToHandleTable[1024]; //TODO
template<class T>
void GDE_PackIndices(uint32_t **dest, int offset, T *src, unsigned int numIndices) {
    **dest = offset + *src;
    (*dest)++;
    **dest = offset + *src;
    (*dest)++;
    for (uint32_t i = 0; i < numIndices; i++) {
        **dest = offset + src[i];
        (*dest)++;
    }
    **dest = offset + src[numIndices - 1];
    (*dest)++;
    if ((numIndices & 1) == 0) {
        **dest = offset + src[numIndices - 1];
        (*dest)++;
    }
}
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
void GDE_UpgradeMesh(GDE_Mesh_VERT_BUF *pMesh) {
    if (pMesh->m_version == 9) {
        for (int i = 0; i < pMesh->m_lodMax; i++)
            pMesh->m_data[i].m_itemKind = 4;
    }
}
bool GFX_PerfPenalty() {
    return (g_GFX_PerformanceFlags & (GPF_BIG_PERF_PENALTY | GPF_SMALL_PERF_PENALTY)) != 0;
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
bool GFX_Internal_LoadOnDemandMeshHandle(int handle, GDE_MeshKind gmk) {
    auto m_gdeName = g_ENG_InstanceResources[handle].m_gdeName;
    int handleReturned = (gmk == GMK_VERT_BUF) ? LOADER_LoadGdeFile(m_gdeName, g_ENG_InstanceResources[handle].m_manyInstances) : LOAD_CHARACTER_SkinGdeFile(m_gdeName);
    zassert(handleReturned == handle);
    if (handleReturned == -1)
        return false;
    ++g_nMeshReloads;
    //nop("Reloaded mesh %s (instanced = %s) [nReloads = %d]");
    return true;
}
void LOADER_LoadGdeFileAnim(GDE_Header_360 *file, GDE_Material *pAnim, int *texs) {
    static_assert(sizeof(GDE_Tex) == 32);
    static_assert(sizeof(GDE_Material) == 0x170);
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
            memmove(pNewAnimators_0, pAnimators_0, 16); //QUEST: why 16, not 8?
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
        pAnim->m_texGlid[0] = ((uint32_t)pAnim->m_texIdx[i] >= file->m_texturesCnt) ? -1 : texs[pAnim->m_texIdx[i]];
    for (int i = 0; i < _countof(pAnim->m_pUsage); i++) {
        ShiftPointer(&pAnim->m_pUsage->m_pConstantParams, file);
        ShiftPointer(&pAnim->m_pUsage->m_somePtr, file);
        ShiftPointer(&pAnim->m_pUsage->m_pAnumBindings, file);
    }
}
void LOADER_LoadGdeFileRuntime(GDE_Header_360 *file, GDE_Header_360 *headerCopy) {
    static_assert(sizeof(GDE_Runtime) == 0x28);
    static_assert(sizeof(GDE_RuntimeItem) == 152);
    if (!file->m_runtime || file == headerCopy)
        return;
    if (headerCopy)
        headerCopy->m_runtime = nullptr;
    GDE_Runtime **pNextRuntimeDest = nullptr;
    bool ok = true;
    for (auto &runtime = file->m_runtime; ok && runtime; runtime = runtime->m_next) {
        ShiftPointer(&runtime, file);
        if (!headerCopy)
            continue;
        if (pNextRuntimeDest == nullptr)
            pNextRuntimeDest = &headerCopy->m_runtime;
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
int GDE_OptimizeLod(uint32_t a1, uint32_t lodMax) {
    if (GFX_PerfPenalty() && lodMax > 1 && a1 == 0)
        return 1;
    return std::min(a1, lodMax - 1);
}
void GDE_CopyMeshItemData0(GDE_MeshItemData0 *dest, GDE_MeshItemData0 *src, uint32_t cnt, VEC3 *mn, VEC3 *mx) {
    for (uint32_t i = 0; i < cnt; i++) {
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
    for (uint32_t i = 0; i < cnt; i++) {
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
    for (uint32_t i = 0; i < cnt; i++) {
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
    for (uint32_t i = 0; i < cnt; i++) {
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
    if (handle == -1)
        handle = GDEMESH_GetFreeMeshResourceHandle();
    if (handle == -1)
        return -1;
    const uint32_t expGdeVer = 0x10012;
    if (file->m_ver != expGdeVer) {
        LogLev(LL_WARNING, "Game expects GDE file version 0x%08X, but GDE file \"%s\" is actually version 0x%08X.", expGdeVer, name, file->m_ver);
        return -1;
    }
    ShiftPointer(&file->m_materials, file);
    ShiftPointer(&file->m_textures, file);
    ShiftPointer(&file->m_shaders, file);
    headerCopy->m_materials = (GDE_Material *)malloc(sizeof(GDE_Material) * file->m_materialsCnt);
    headerCopy->m_textures = (GDE_Tex *)malloc(sizeof(GDE_Tex) * file->m_texturesCnt);
    headerCopy->m_shaders = (GDE_Shader *)malloc(sizeof(GDE_Shader) * file->m_shadersCnt);
    zassert(file->m_texturesCnt < _countof(g_textureToHandleTable));
    auto destTextures = headerCopy->m_textures;
    auto srcTextures = file->m_textures;
    int texHandles[_countof(g_textureToHandleTable)];
    for (uint32_t t = 0; t < file->m_texturesCnt; t++) {
        if (srcTextures[t].m_name) {
            memmove(destTextures + t, srcTextures + t, sizeof(GDE_Tex));
            ShiftPointer(&srcTextures[t].m_name, file);
            destTextures[t].m_name = _strdup(srcTextures[t].m_name);
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
                    int destNameLen = (int)strlen(destTextures[t].m_name);
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
    for (uint32_t uIdx = 0; uIdx < file->m_materialsCnt; ++uIdx) {
        auto uSrc = file->m_materials + uIdx, uDest = headerCopy->m_materials + uIdx;
        LOADER_LoadGdeFileAnim(file, uSrc, texHandles);
        *uDest = *uSrc;
    }
    LOADER_LoadGdeFileRuntime(file, headerCopy);
    auto VRAMBytes_VBO = g_VRAMBytes_VBO;
    if (file->m_meshKind == GMK_VERT_BUF) {
        ShiftPointer(&file->m_mesh.VERT_BUF, file);
        static_assert(sizeof(GDE_Mesh_VERT_BUF) == 0x1F8);
        auto newMesh = (GDE_Mesh_VERT_BUF *)malloc(sizeof(GDE_Mesh_VERT_BUF));
        headerCopy->m_mesh.VERT_BUF = newMesh;
        auto mesh = file->m_mesh.VERT_BUF;
        *newMesh = *mesh;
        if (mesh->m_version != 10) {
            LogDebug("LOADER_LoadGdeFile_LEAN() Loading old version of mesh, %s, filesize, %d", name, fileSize);
        }
        g_ENG_InstanceResources[handle].m_bounds = mesh->m_bounds;
        GDE_UpgradeMesh(mesh);
        auto optLod = GDE_OptimizeLod(0, mesh->m_lodMax);
        auto checkedInstCnt = std::clamp(nInstances, 1u, 16u);
        newMesh->m_data[0].m_instancesCount = checkedInstCnt;
        VEC3 minCoord{ +1.0e10f, +1.0e10f, +1.0e10f };
        VEC3 maxCoord{ -1.0e10f, -1.0e10f, -1.0e10f };
        for (auto mi = optLod; mi < mesh->m_lodMax; mi++) {
            static_assert(sizeof(GDE_Mesh_VERT_BUFi) == 80);
            auto &meshItem = mesh->m_data[mi];
            auto &newMeshItem = newMesh->m_data[mi];
            auto subItemArSize = (meshItem.m_subItemEndPtr - meshItem.m_subItemBegPtr) * sizeof(GDE_MeshSubItem);
            auto newMeshSubItems = (GDE_MeshSubItem *)malloc(subItemArSize);
            memmove(newMeshSubItems, (char *)file + (uint64_t)meshItem.m_subItemBegPtr, subItemArSize);
            //newMeshItem = meshItem; //OMIT: already copied!
            newMeshItem.m_vbHandle = newMeshItem.m_field_4 = -1;
            auto off = (char *)newMeshSubItems - (char *)newMeshItem.m_subItemBegPtr;
            newMeshItem.m_subItemBegPtr = newMeshSubItems;
            newMeshItem.m_instancesCount = checkedInstCnt;
            ShiftPointer(&newMeshItem.m_field_10, off);
            ShiftPointer(&newMeshItem.m_field_18, off);
            ShiftPointer(&newMeshItem.m_field_20, off);
            ShiftPointer(&newMeshItem.m_subItemEndPtr, off);
            auto itemKind = meshItem.m_itemKind;
            auto numVerts = meshItem.m_numVerts;
            GDE_MeshItemData0 *p0;
            GDE_MeshItemData1 *p1;
            GDE_MeshItemData2 *p2;
            GDE_MeshItemData3 *p3;
            GDE_MeshItemData4 *p4;
            uint32_t vbSize = 0;
            auto pVerts = meshItem.m_pVerts;
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
                        p4[i].field_14 = p4src_i->field_14;
                        p4[i].field_18 = p4src_i->field_18;
                        p4[i].m_instIdx = p4src_i->m_instIdx;
                        p4[i].gap = p4src_i->gap;
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
                    GFX_CreateVertexBuffer(&newMeshItem.m_vbHandle, vbSize, newMeshItem.m_pVerts);
                    free(newMeshItem.m_pVerts);
                    newMeshItem.m_pVerts = nullptr;
                } else {
                    zassert(0);
                }
            } else {
                LogDebug("Problem with vertexbuffer! filename[%s], lod[%i], numVerts=%i, pointer to verts is valid(0x%x)", name, mi, newMeshItem.m_numVerts, pVerts);
                newMeshItem.m_vbHandle = -1;
            }
            int stripIdx = 0;
            auto curSubItem = newMeshItem.m_subItemBegPtr;
            while (curSubItem != newMeshItem.m_subItemEndPtr) {
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
            GDE_StoreStat(GS_VERT_BUF, fileSize, file); //QUEST: why not outside loop?
        }
    }
    auto &res = g_ENG_InstanceResources[handle];
    res.m_gdeFile = headerCopy;
    if (res.m_gdeName == nullptr) {
        res.m_gdeName = _strdup(name);
        res.m_gdeNameCRC = SIG_CalcCaseSensitiveSignature(name);
    }
    res.m_isSkin = false;
    res.m_heapUsed = true;
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
    if (handle == -1)
        handle = GDEMESH_GetFreeMeshResourceHandle();
    if (handle == -1)
        return -1;
    const uint32_t expGdeVer = 0x10012;
    if (file->m_ver != expGdeVer) {
        LogLev(LL_WARNING, "Game expects GDE file version 0x%08X, but GDE file \"%s\" is actually version 0x%08X.", expGdeVer, name, file->m_ver);
        return -1;
    }
    ShiftPointer(&file->m_materials, file);
    ShiftPointer(&file->m_textures, file);
    ShiftPointer(&file->m_shaders, file);
    zassert(file->m_texturesCnt < _countof(g_textureToHandleTable));
    auto srcTextures = file->m_textures;
    int texHandles[_countof(g_textureToHandleTable)];
    for (uint32_t t = 0; t < file->m_texturesCnt; t++) {
        if (srcTextures[t].m_name) {
            ShiftPointer(&srcTextures[t].m_name, file);
            auto v38 = strstr(srcTextures[t].m_name, "YorkShire\\Textures");
            if (v38)
                v38[4] = 's';
            auto v39 = strstr(srcTextures[t].m_name, ".htx");
            if (v39)
                strcpy(v39, ".tga");
        }
        if (srcTextures[t].m_name == nullptr || strlen(srcTextures[t].m_name)) {
            if (LOADER_FindAssetsBySignature(srcTextures[t].m_assetCrc, WAD_ASSET_TYPE::TEXTURE)) {
                texHandles[t] = 0;
                zassert(0);
            } else {
                auto tgaHandle = GFX_CreateTextureFromTGAFile(srcTextures[t].m_name, -1, true);
                if (tgaHandle == -1) {
                    const char *tgaAltName = nullptr;
                    int srcNameLen = int(strlen(srcTextures[t].m_name));
                    if (srcNameLen > 0) {
                        auto destName = srcTextures[t].m_name;
                        const char *dch = destName + srcNameLen;
                        do {
                            if (*dch == '\\')
                                break;
                            --dch;
                            tgaAltName = destName + srcNameLen;
                            srcNameLen--;
                        } while (srcNameLen > 0);
                        if (tgaAltName)
                            tgaHandle = GFX_CreateTextureFromTGAFile(tgaAltName, -1, true);
                    }
                }
                texHandles[t] = tgaHandle;
                if (tgaHandle == -1) {
                    LogDebug("LOADER_LoadGdeFile() Unable to load texture file %s, from %s", name, srcTextures[t].m_name);
                    texHandles[t] = 0;
                }
            }
        }
    }
    for (uint32_t uIdx = 0; uIdx < file->m_materialsCnt; ++uIdx)
        LOADER_LoadGdeFileAnim(file, file->m_materials + uIdx, texHandles);
    LOADER_LoadGdeFileRuntime(file, nullptr);
    uint32_t lod = 0, checkedInstCnt = 0;
    GDE_Mesh_VB_CLUST *vbc = nullptr;
    switch (file->m_meshKind) {
    case GMK_VB_CLUSTER:
        static_assert(sizeof(GDE_Cluster) == 72);
        static_assert(sizeof(GDE_Group) == 32);
        static_assert(sizeof(GDE_SimpleMaterial) == 16);
        vbc = ShiftPointer(&file->m_mesh.VB_CLUST, file);
        if (vbc->m_version != 6)
            return -1;
        GDE_StoreStat(GS_VB_CLUST, fileSize, file);
        ShiftPointer(&vbc->m_materials, file);
        for (int materialIdx = 0; materialIdx < vbc->m_materialsCnt; ++materialIdx) {
            auto pSimpleMat = vbc->m_materials + materialIdx;
            ShiftPointer(&pSimpleMat->m_groups, file);
            for (int groupIdx = 0; groupIdx < pSimpleMat->m_groupsCnt; groupIdx++) {
                auto curGroup = pSimpleMat->m_groups + groupIdx;
                ShiftPointer(&curGroup->m_clusters, file);
                for (int clusterIdx = 0; clusterIdx < curGroup->m_clustersCnt; clusterIdx++) {
                    auto curCluster = curGroup->m_clusters + clusterIdx;
                    if (!curCluster->m_numVerts || !curCluster->m_pVerts)
                        LogDebug("Problem with vertexbuffer! filename[%s], material[%i], group[%i], cluster[%i], numVerts=%i, pointer to verts is valid(0x%x)",
                            name, materialIdx, groupIdx, clusterIdx, curCluster->m_numVerts, curCluster->m_pVerts);
                    auto numIndices = curCluster->m_numIndices;
                    if (!numIndices || !curCluster->m_pIndices)
                        LogDebug("Problem with indexbuffer! filename[%s], material[%i], group[%i], cluster[%i], numIndices=%i, pointer to indices is valid(0x%x)",
                            name, materialIdx, groupIdx, clusterIdx, curCluster->m_numIndices, curCluster->m_pIndices);
                    if (numIndices && curCluster->m_numVerts) {
                        ShiftPointer(&curCluster->m_pVerts, file);
                        ShiftPointer(&curCluster->m_pIndices, file);
                        GFX_CreateVertexBuffer(&curCluster->m_vbHandle, 72 /*FIXME - sizeof struct */ * curCluster->m_numVerts, curCluster->m_pVerts);
                        GFX_CreateIndexBuffer(&curCluster->m_ibHandle, 2 * curCluster->m_numIndices, curCluster->m_pIndices);
                    } else {
                        curCluster->m_vbHandle = -1;
                        curCluster->m_ibHandle = -1;
                    }
                }
            }
        }
        ShiftPointer(&vbc->m_field_20, file);
        break;
    case GMK_SHRUB:
        ShiftPointer(&file->m_mesh.SHRUB, file);
        if (file->m_mesh.SHRUB->m_version != 1) {
            LogLev(LL_WARNING, "Game expects SHRUB version 0x%08X, but SHRUB  \"%s\" is actually version 0x%08X.", 1, name, file->m_mesh.SHRUB->m_version);
            return -1;
        }
        ShiftPointer(&file->m_mesh.SHRUB->m_field_20, file);
        ShiftPointer(&file->m_mesh.SHRUB->m_field_28, file);
        break;
    case GMK_VERT_BUF:
        ShiftPointer(&file->m_mesh.VERT_BUF, file);
        GDE_UpgradeMesh(file->m_mesh.VERT_BUF);
        g_ENG_InstanceResources[handle].m_bounds = file->m_mesh.VERT_BUF->m_bounds;
        lod = std::max(1u, nInstances);
        checkedInstCnt = std::min(lod, 16u);
        for (auto mi = 0; mi < file->m_mesh.VERT_BUF->m_lodMax; mi++) {
            auto &meshItem = file->m_mesh.VERT_BUF->m_data[mi];
            ShiftPointer(&meshItem.m_subItemBegPtr, file);
            ShiftPointer(&meshItem.m_field_10, file);
            ShiftPointer(&meshItem.m_field_18, file);
            ShiftPointer(&meshItem.m_field_20, file);
            ShiftPointer(&meshItem.m_subItemEndPtr, file);
            meshItem.m_itemKind = meshItem.m_itemKind;
            meshItem.m_instancesCount = checkedInstCnt;
            if (!meshItem.m_numVerts || meshItem.m_pVerts == nullptr) {
                LogDebug("Problem with vertexbuffer! filename[%s], lod[%i], numVerts=%i, pointer to verts is valid(0x%x)",
                    name, mi, meshItem.m_numVerts, meshItem.m_pVerts);
                meshItem.m_vbHandle = -1;
            } else {
                ShiftPointer(&meshItem.m_pVerts, file);
                for (int vi = 0; vi < meshItem.m_numVerts; vi++) {
                    GDE_MeshItemData0 *p0;
                    GDE_MeshItemData1 *p1;
                    GDE_MeshItemData2 *p2;
                    GDE_MeshItemData3 *p3;
                    GDE_MeshItemData4_file *p4;
                    switch (meshItem.m_itemKind) {
                    case 0:
                        p0 = (GDE_MeshItemData0 *)meshItem.m_pVerts + vi;
                        p0->m_color = GDE_Color(p0->m_color);
                        break;
                    case 1:
                        p1 = (GDE_MeshItemData1 *)meshItem.m_pVerts + vi;
                        p1->m_color = GDE_Color(p1->m_color);
                        break;
                    case 2:
                        p2 = (GDE_MeshItemData2 *)meshItem.m_pVerts + vi;
                        p2->m_color1 = GDE_Color(p2->m_color1);
                        p2->m_color2 = GDE_Color(p2->m_color2);
                        break;
                    case 3:
                        p3 = (GDE_MeshItemData3 *)meshItem.m_pVerts + vi;
                        p3->m_color1 = GDE_Color(p3->m_color1);
                        p3->m_color2 = GDE_Color(p3->m_color2);
                        break;
                    default: //4 ?
                        p4 = (GDE_MeshItemData4_file *)meshItem.m_pVerts + vi;
                        p4->m_color1 = GDE_Color(p4->m_color1);
                        p4->m_color2 = GDE_Color(p4->m_color2);
                        break;
                    }
                }
                if (meshItem.m_instancesCount == 1) {
                    switch (meshItem.m_itemKind) {
                    case 0:
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, sizeof(GDE_MeshItemData0) * meshItem.m_numVerts, meshItem.m_pVerts);
                        break;
                    case 1:
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, sizeof(GDE_MeshItemData1) * meshItem.m_numVerts, meshItem.m_pVerts);
                        break;
                    case 2:
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, sizeof(GDE_MeshItemData2) * meshItem.m_numVerts, meshItem.m_pVerts);
                        break;
                    case 3:
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, sizeof(GDE_MeshItemData3) * meshItem.m_numVerts, meshItem.m_pVerts);
                        break;
                    default: //4?
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, sizeof(GDE_MeshItemData4_file) * meshItem.m_numVerts, meshItem.m_pVerts);
                        break;
                    }
                } else {
                    auto totalVerts = meshItem.m_instancesCount * meshItem.m_numVerts;
                    if (meshItem.m_itemKind == 0) {
                        static_assert(sizeof(GDE_MeshItemData0_ext) == 44);
                        auto vbSize = sizeof(GDE_MeshItemData0_ext) * totalVerts;
                        auto vbDest = (GDE_MeshItemData0_ext *)malloc(vbSize);
                        auto vbi = (GDE_MeshItemData0 *)meshItem.m_pVerts;
                        auto vbo = vbDest;
                        for (int in = 0; in < meshItem.m_instancesCount; ++in) {
                            for (int vi = 0; vi < meshItem.m_numVerts; vi++) {
                                *vbo = *vbi;
                                vbo->m_instIdx1 = (float)in;
                                vbo->m_instIdx2 = (float)in;
                                vbo++; vbi++;
                            }
                        }
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, uint32_t(vbSize), vbDest);
                        free(vbDest);
                    } else if (meshItem.m_itemKind == 1) {
                        auto vbSize = sizeof(GDE_MeshItemData1) * totalVerts;
                        auto vbDest = (GDE_MeshItemData1 *)malloc(vbSize);
                        auto vbi = (GDE_MeshItemData1 *)meshItem.m_pVerts;
                        auto vbo = vbDest;
                        for (int in = 0; in < meshItem.m_instancesCount; ++in) {
                            for (int vi = 0; vi < meshItem.m_numVerts; vi++) {
                                *vbo = *vbi;
                                vbo->m_instIdx = (float)in;
                                vbo++; vbi++;
                            }
                        }
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, uint32_t(vbSize), vbDest);
                        free(vbDest);
                    } else if (meshItem.m_itemKind == 2) {
                        static_assert(sizeof(GDE_MeshItemData2_ext) == 48);
                        auto vbSize = sizeof(GDE_MeshItemData2_ext) * totalVerts;
                        auto vbDest = (GDE_MeshItemData2_ext *)malloc(vbSize);
                        auto vbi = (GDE_MeshItemData2 *)meshItem.m_pVerts;
                        auto vbo = vbDest;
                        for (int in = 0; in < meshItem.m_instancesCount; ++in) {
                            for (int vi = 0; vi < meshItem.m_numVerts; vi++) {
                                *vbo = *vbi;
                                vbo->m_instIdx1 = (float)in;
                                vbo->m_instIdx2 = (float)in;
                                vbo++; vbi++;
                            }
                        }
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, uint32_t(vbSize), vbDest);
                        free(vbDest);
                    } else if (meshItem.m_itemKind == 3) {
                        auto vbSize = sizeof(GDE_MeshItemData3) * totalVerts;
                        auto vbDest = (GDE_MeshItemData3 *)malloc(vbSize);
                        auto vbi = (GDE_MeshItemData3 *)meshItem.m_pVerts;
                        auto vbo = vbDest;
                        for (int in = 0; in < meshItem.m_instancesCount; ++in) {
                            for (int vi = 0; vi < meshItem.m_numVerts; vi++) {
                                *vbo = *vbi;
                                vbo->m_instIdx = (float)in; //-5 from next
                                vbo++; vbi++;
                            }
                        }
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, uint32_t(vbSize), vbDest);
                        free(vbDest);
                    } else { //4
                        auto vbi = (GDE_MeshItemData4_file *)meshItem.m_pVerts;
                        auto vbSize = sizeof(GDE_MeshItemData4_file) * totalVerts;
                        auto vbDest = (GDE_MeshItemData4_file *)malloc(vbSize);
                        auto vbo = vbDest;
                        for (int in = 0; in < meshItem.m_instancesCount; ++in) {
                            for (int vi = 0; vi < meshItem.m_numVerts; vi++) {
                                *vbo = *vbi;
                                vbo->m_instIdx = (float)in; //-13 from next
                                vbo++; vbi++;
                            }
                        }
                        GFX_CreateVertexBuffer(&meshItem.m_vbHandle, uint32_t(vbSize), vbDest);
                        free(vbDest);
                    }
                }
            }
            int stripIdx = 0;
            auto curSubItem = meshItem.m_subItemBegPtr;
            while (curSubItem != meshItem.m_subItemEndPtr) {
                ShiftPointer(&curSubItem->m_pIndices, file);
                if (curSubItem->m_numIndices && curSubItem->m_pIndices) {
                    if (meshItem.m_instancesCount == 1) {
                        uint32_t ibSize = ((curSubItem->m_numIndices <= 0xFFFF) ? 2 : 4) * curSubItem->m_numIndices;
                        GFX_CreateIndexBuffer(&curSubItem->m_ibHandle, ibSize, curSubItem->m_pIndices);
                    } else {
                        auto allocSize = 4 * meshItem.m_instancesCount * (((curSubItem->m_numIndices & 1) == 0) + curSubItem->m_numIndices + 3);
                        auto m = (uint32_t **)malloc(allocSize);
                        for (auto in = 0; in < meshItem.m_instancesCount; ++in) {
                            auto off = meshItem.m_numVerts * in;
                            if (curSubItem->m_numIndices <= 0xFFFF)
                                GDE_PackIndices(m, off, (uint16_t *)curSubItem->m_pIndices, curSubItem->m_numIndices);
                            else
                                GDE_PackIndices(m, off, (uint32_t *)curSubItem->m_pIndices, curSubItem->m_numIndices);
                        }
                        GFX_CreateIndexBuffer(&curSubItem->m_ibHandle, allocSize, m);
                        free(m);
                    }
                    switch (meshItem.m_itemKind) {
                    case 0:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR1_UV1;
                        break;
                    case 1:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR1_UV2;
                        break;
                    case 2:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR2_UV1;
                        break;
                    case 3:
                        curSubItem->m_vertexHandle = g_hTRI_VERT_COLOR2_UV2;
                        break;
                    case 4:
                        curSubItem->m_vertexHandle = g_hGDE_TRI_VERT;
                        break;
                    }
                } else {
                    LogDebug("Problem with indexbuffer! filename[%s], lod[%i], strip[%i], numIndices=%i, pointer to indices is valid(0x%x)", 
                        name, mi, stripIdx, curSubItem->m_numIndices, curSubItem->m_pIndices);
                    meshItem.m_vbHandle = -1;
                }
                ++curSubItem;
                ++stripIdx;
            }
        }
        GDE_StoreStat(GS_VERT_BUF, fileSize, file);
        break;
    default:
        zassert(0);
    }
    auto &res = g_ENG_InstanceResources[handle];
    res.m_gdeFile = file;
    if (res.m_gdeName == nullptr) {
        res.m_gdeName = _strdup(name);
        res.m_gdeNameCRC = SIG_CalcCaseSensitiveSignature(name);
    }
    res.m_isSkin = false;
    res.m_heapUsed = true;
    res.m_state = IRS_INITIAL;
    res.m_manyInstances = nInstances > 1;
    res.m_creationTime = timeGetTime();
    res.m_assetCategory = g_CurrentAssetCategory;
    return handle;
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
    if (!obj.m_manyInstances)
        GDE_FreeRuntime(gdeFile);
    if (gdeFile->m_meshKind == GMK_SKIN) {
        auto mesh = gdeFile->m_mesh.SKIN;
        static_assert(sizeof(GDE_Mesh_SKIN) == 264);
        static_assert(sizeof(GDE_SkinVB) == 40);
        static_assert(sizeof(GDE_SkinVB_Item) == 96);
        if (mesh) {
            if (obj.m_heapUsed) {
                for (uint32_t i = 0; i < gdeFile->m_texturesCnt; ++i)
                    free(gdeFile->m_textures[i].m_name);
                for (uint32_t j = 0; j < gdeFile->m_materialsCnt; ++j) {
                    auto p0 = gdeFile->m_materials[j].m_pAnimators->m_field_0;
                    if (p0) {
                        free(p0->m_arr1);
                        free(p0->m_arr2);
                        free(p0);
                    }
                    free(gdeFile->m_materials[j].m_pAnimators);
                    gdeFile->m_materials[j].m_pAnimators = nullptr;
                }
                free(gdeFile->m_materials);
                gdeFile->m_materials = nullptr;
                free(gdeFile->m_textures);
                gdeFile->m_textures = nullptr;
                free(gdeFile->m_shaders);
                gdeFile->m_shaders = nullptr;
            }
            for (auto optLod = GDE_OptimizeLod(0, mesh->m_lodMax); optLod < mesh->m_lodMax; optLod++) {
                auto pVB = mesh->m_vbs + optLod;
                for (auto k = pVB->m_pItems; k != pVB->m_pEndItem; k++) {
                    GFX_DestroyBuffer(&k->m_vbHandle);
                    GFX_DestroyBuffer(&k->m_ibHandle);
                }
                if (obj.m_heapUsed) {
                    free(pVB->m_pItems);
                    pVB->m_field_8 = nullptr;
                    pVB->m_field_10 = nullptr;
                    pVB->m_field_18 = nullptr;
                    //QUEST: m_pEndItem not set to 0
                }
            }
            if (obj.m_heapUsed) {
                for (auto ps : obj.m_gdeFile->m_pSkelInfo) {
                    if (ps) {
                        free(ps->m_pBoneInfoArray);
                        ps->m_pBoneInfoArray = nullptr;
                        free(ps->m_pDefaultComponents);
                        ps->m_pDefaultComponents = nullptr;
                        //QUEST: why ps not deleted
                    }
                }
            }
        }
    } else if (gdeFile->m_meshKind == GMK_VERT_BUF) {
        auto mesh = gdeFile->m_mesh.VERT_BUF;
        bool needFree = false;
        if (mesh->m_data[0].m_instancesCount == 1) {
            needFree = 1;
            for (uint32_t m = 0; m < gdeFile->m_texturesCnt; ++m)
                free(gdeFile->m_textures[m].m_name);
            for (uint32_t n = 0; n < gdeFile->m_materialsCnt; ++n) {
                auto ptr = gdeFile->m_materials[n].m_pAnimators;
                free(ptr->m_field_0);
                free(ptr->m_pArr1);
                free(ptr->m_pArr2);
                free(ptr);
            }
            free(gdeFile->m_materials);
            free(gdeFile->m_textures);
            free(gdeFile->m_shaders);
        }
        for (auto mi = GDE_OptimizeLod(0, mesh->m_lodMax); mi < mesh->m_lodMax; ++mi) {
            auto &pi = mesh->m_data[mi];
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
                pi.m_field_18 = nullptr;
                pi.m_field_10 = nullptr;
            }
            g_VRAMBytes_VBO -= pi.m_numVerts * (obj.m_manyInstances ? 576 : 36);
            GFX_DestroyBuffer(&pi.m_vbHandle);
        }
    } else {
        zassert(0);
    }
    free(gdeFile->m_mesh.SHRUB); //any OK
    if (obj.m_heapUsed)
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
                memmove(&dest[ret], chunk, dec_sz);
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
bool GDE_CheckSkin(GDE_Header_360 *file, const char *name, bool createVB, bool shiftPtrLevel1) {
    if (file->m_ver != 0x10012) {
        LogDebug("GDE mesh version incorrect: \"%s\" is version 0x%08X instead of 0x%08X.");
        return false;
    }
    if (!file->m_mesh.SKIN || !file->m_materials) {
        LogDebug("Bad header on gdeFile \"%s\".", name);
        return false;
    }
    if (file->m_meshKind != GMK_SKIN) {
        LogDebug("Unsupported mesh type 0x%08X for GDE file \"%s\".", file->m_meshKind, name);
        return false;
    }
    ShiftPointer(&file->m_mesh.SKIN, file);
    const int expMeshVersion = 8;
    if (file->m_mesh.SKIN->m_version != expMeshVersion) {
        LogDebug("Skin mesh version incorrect: \"%s\" is version %d instead of %d.", name, file->m_mesh.SKIN->m_version, expMeshVersion);
        return false;
    }
    for(int lod = 0; lod < file->m_mesh.SKIN->m_lodMax; ++lod) {
        auto VB = file->m_mesh.SKIN->m_vbs + lod;
        ShiftPointer(&VB->m_pItems, file);
        ShiftPointer(&VB->m_field_8, file);
        ShiftPointer(&VB->m_field_10, file);
        ShiftPointer(&VB->m_field_18, file);
        int strip = 0;
        for (auto VBi = VB->m_pItems; VBi != VB->m_pEndItem; VBi++, strip++) {
            auto pIndices = VBi->m_pIndices;
            auto pVerts = VBi->m_pVerts;
            ShiftPointer(&VBi->m_pIndices, file);
            ShiftPointer(&VBi->m_pVerts, file);
            if (createVB) {
                if (VBi->m_numVerts && pVerts) {
                    GFX_CreateVertexBuffer(&VBi->m_vbHandle, VBi->m_numVerts * ((VBi->m_flags & 0x10000000) != 0 ? 64 : 44), VBi->m_pVerts);
                } else {
                    LogDebug("Problem with vertexbuffer! filename[%s], lod[%i], strip[%i], numVerts=%i, pointer to verts is valid(0x%x)", name, lod, strip, VBi->m_numVerts, pVerts);
                    VBi->m_vbHandle = -1;
                }
                if (VBi->m_numIndices && pIndices) {
                    GFX_CreateIndexBuffer(&VBi->m_ibHandle, 2 * VBi->m_numIndices, VBi->m_pIndices);
                } else {
                    LogDebug("Problem with indexbuffer! filename[%s], lod[%i], strip[%i], numIndices=%i, pointer to indices is valid(0x%x)", name, lod, strip, VBi->m_numIndices, pIndices);
                    VBi->m_ibHandle = -1;
                }
            }
        }
    }
    for (auto &si : file->m_pSkelInfo) {
        if (si) {
            ShiftPointer(&si, file);
            ShiftPointer(&si->m_pBoneInfoArray, file);
            ShiftPointer(&si->m_pDefaultComponents, file);
            if (si->m_field_18)
                ShiftPointer(&si->m_field_18, file);
            if (si->m_field_20)
                ShiftPointer(&si->m_field_18, file);
        }
    }
    if (!shiftPtrLevel1)
        return true;
    ShiftPointer(&file->m_materials, file);
    ShiftPointer(&file->m_textures, file);
    ShiftPointer(&file->m_shaders, file);
    return true;
}
int LOAD_CHARACTER_SkinGdeFile_LEAN(GDE_Header_360 *file, char *name, uint32_t fileLength, int handle) {
    if (handle == -1) {
        handle = GDEMESH_GetFreeMeshResourceHandle();
        if (handle == -1)
            return -1;
    }
    auto *headerCopy = (GDE_Header_360 *)malloc(sizeof(GDE_Header_360));
    if (!headerCopy) {
        LogTyped(LOG_ERROR, "Out of memory (Mesh)!");
        return -1;
    }
    *headerCopy = *file;
    auto _VRAMBytes_VBO = g_VRAMBytes_VBO;
    ShiftPointer(&file->m_materials, file);
    ShiftPointer(&file->m_textures, file);
    static_assert(sizeof(GDE_Shader) == 16);
    ShiftPointer(&file->m_shaders, file);
    headerCopy->m_materials = (GDE_Material *)malloc(sizeof(GDE_Material) * file->m_materialsCnt);
    if (!headerCopy->m_materials) {
        LogTyped(LOG_ERROR, "Out of memory (materials)!");
        return -1;
    }
    headerCopy->m_textures = (GDE_Tex *)malloc(sizeof(GDE_Tex) * file->m_texturesCnt);
    if (!headerCopy->m_textures) {
        LogTyped(LOG_ERROR, "Out of memory (textures)!");
        return -1;
    }
    headerCopy->m_shaders = (GDE_Shader *)malloc(sizeof(GDE_Shader) * file->m_shadersCnt);
    if (!headerCopy->m_shaders) {
        LogTyped(LOG_ERROR, "Out of memory (shaders)!");
        return -1;
    }
    int texHandles[_countof(g_textureToHandleTable)];
    auto srcTextures = file->m_textures, destTextures = headerCopy->m_textures;
    for (uint32_t t = 0; t < file->m_texturesCnt; t++) {
        if (srcTextures[t].m_name) {
            memmove(destTextures + t, srcTextures + t, sizeof(GDE_Tex));
            ShiftPointer(&srcTextures[t].m_name, file);
            destTextures[t].m_name = _strdup(srcTextures[t].m_name);
            if (!destTextures[t].m_name) {
                LogTyped(LOG_ERROR, "Out of memory (filename)!");
                return -1;
            }
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
                    int destNameLen = int(strlen(destTextures[t].m_name));
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
    for (uint32_t uIdx = 0; uIdx < file->m_materialsCnt; ++uIdx) {
        auto uSrc = file->m_materials + uIdx, uDest = headerCopy->m_materials + uIdx;
        LOADER_LoadGdeFileAnim(file, uSrc, texHandles);
        *uDest = *uSrc;
    }
    LOADER_LoadGdeFileRuntime(file, headerCopy);
    if (!GDE_CheckSkin(file, name, false, false))
        return -1; //Ursoft FIXED: was 0
    static_assert(sizeof(GDE_BoneInfo) == 80);
    static_assert(sizeof(GDE_DefaultComponents) == 48);
    static_assert(sizeof(GDE_SkelInfo) == 0x40);
    auto newMesh = (GDE_Mesh_SKIN *)malloc(sizeof(GDE_Mesh_SKIN));
    auto mesh = file->m_mesh.SKIN;
    headerCopy->m_mesh.SKIN = newMesh;
    if (newMesh) {
        *newMesh = *mesh;
        int lod = GDE_OptimizeLod(0, mesh->m_lodMax);
        for (; lod < mesh->m_lodMax; lod++) {
            auto src = mesh->m_vbs + lod;
            auto &dest = newMesh->m_vbs[lod];
            auto cnt = src->m_pEndItem - src->m_pItems;
            auto itemsMem = cnt * sizeof(GDE_SkinVB_Item);
            dest.m_pItems = (GDE_SkinVB_Item *)malloc(itemsMem);
            if (dest.m_pItems) {
                LogTyped(LOG_ERROR, "Out of memory! (STRIP)");
                return -1;
            }
            memmove(dest.m_pItems, src->m_pItems, itemsMem);
            dest.m_field_8 = dest.m_pItems + (src->m_field_8 - src->m_pItems);
            dest.m_field_10 = dest.m_pItems + (src->m_field_10 - src->m_pItems);
            dest.m_field_18 = dest.m_pItems + (src->m_field_18 - src->m_pItems);
            dest.m_pEndItem = dest.m_pItems + cnt;
            int totalVerts = 0, totalIndices = 0;
            auto curItem = src->m_pItems;
            while (curItem != src->m_pEndItem) {
                totalVerts += curItem->m_numVerts;
                totalIndices += curItem->m_numIndices;
                ++curItem;
            }
            dest.m_pItems->m_pVerts = (uint32_t *)malloc(44 * totalVerts);
            if (!dest.m_pItems->m_pVerts) {
                LogTyped(LOG_ERROR, "Out of memory! (pVerts)");
                return -1;
            }
            memmove(dest.m_pItems->m_pVerts, src->m_pItems->m_pVerts, 44 * totalVerts);
            dest.m_pItems->m_pIndices = (uint16_t *)malloc(2 * totalIndices);
            if (!dest.m_pItems->m_pIndices) {
                LogTyped(LOG_ERROR, "Out of memory! (pIndices)");
                return -1;
            }
            memmove(dest.m_pItems->m_pIndices, src->m_pItems->m_pIndices, 2 * totalVerts);
            curItem = src->m_pItems;
            auto destItem = dest.m_pItems;
            int runIndices = 0;
            while (curItem != src->m_pEndItem) {
                *destItem = *curItem;
                if (curItem != src->m_pItems) {
                    destItem->m_pIndices = nullptr;
                    destItem->m_pVerts = nullptr;
                    destItem->m_vbHandle = -1;
                    destItem->m_ibHandle = -1;
                }
                destItem->m_runIndices = runIndices;
                runIndices += curItem->m_numIndices;
                curItem++;
                destItem++;
            }
            GFX_CreateVertexBuffer(&dest.m_pItems->m_vbHandle, 44 * totalVerts, dest.m_pItems->m_pVerts);
            GFX_CreateIndexBuffer(&dest.m_pItems->m_ibHandle, 2 * totalIndices, dest.m_pItems->m_pIndices);
            free(dest.m_pItems->m_pVerts);
            dest.m_pItems->m_pVerts = nullptr;
            free(dest.m_pItems->m_pIndices);
            dest.m_pItems->m_pIndices = nullptr;
            auto srci = src->m_pItems;
            for (auto desti = dest.m_pItems; desti != src->m_pEndItem; ++desti, srci++) {
                desti->m_vbHandle = srci->m_vbHandle;
                desti->m_ibHandle = srci->m_ibHandle;
            }
        }
        if (lod >= mesh->m_lodMax) {
            memset(newMesh->gap, 0, sizeof(newMesh->gap));
            for(int v80 = 0; v80 < _countof(file->m_pSkelInfo); v80++) {
                auto src = file->m_pSkelInfo[v80];
                if (src) {
                    auto dest = (GDE_SkelInfo *)malloc(sizeof(GDE_SkelInfo));
                    if (!dest) {
                        LogTyped(LOG_ERROR, "Out of memory! (pSkelInfo)");
                        return -1;
                    }
                    headerCopy->m_pSkelInfo[v80] = dest;
                    *dest = *src;
                    dest->m_pDefaultComponents = nullptr;
                    dest->m_field_18 = nullptr;
                    dest->m_field_20 = nullptr;
                    dest->m_pBoneInfoArray = (GDE_BoneInfo *)malloc(sizeof(GDE_BoneInfo) * src->m_arraysCnt);
                    if (!dest->m_pBoneInfoArray) {
                        LogTyped(LOG_ERROR, "Out of memory! (pBoneInfoArray)");
                        return -1;
                    }
                    memmove(dest->m_pBoneInfoArray, src->m_pBoneInfoArray, sizeof(GDE_BoneInfo) * src->m_arraysCnt);
                    dest->m_pDefaultComponents = (GDE_DefaultComponents *)malloc(sizeof(GDE_DefaultComponents) * src->m_arraysCnt);
                    if (!dest->m_pDefaultComponents) {
                        LogTyped(LOG_ERROR, "Out of memory! (pDefaultComponents)");
                        return -1;
                    }
                    memmove(dest->m_pDefaultComponents, src->m_pDefaultComponents, sizeof(GDE_DefaultComponents) * src->m_arraysCnt);
                }
            }
            GDE_StoreStat(GS_SKIN, fileLength, nullptr);
            auto &obj = g_ENG_InstanceResources[handle];
            obj.m_gdeFile = headerCopy;
            if (!obj.m_gdeName) {
                obj.m_gdeName = _strdup(name);
                if (!obj.m_gdeName) {
                    LogTyped(LOG_ERROR, "Out of memory! (sourceFilename)");
                    return -1;
                }
                obj.m_gdeNameCRC = SIG_CalcCaseSensitiveSignature(name);
            }
            obj.m_vramUsed = g_VRAMBytes_VBO - _VRAMBytes_VBO;
            obj.m_manyInstances = false;
            obj.m_isSkin = true;
            obj.m_heapUsed = true;
            obj.m_state = IRS_INITIAL;
            obj.m_creationTime = timeGetTime();
            obj.m_assetCategory = g_CurrentAssetCategory;
            return handle;
        }
    } else {
        LogTyped(LOG_ERROR, "Out of memory! (SKIN)");
    }
    return -1;
}
