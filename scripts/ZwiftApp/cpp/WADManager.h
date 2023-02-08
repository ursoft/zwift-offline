#pragma once
struct WAD_FILE_HEADER {
    uint32_t f0;
    char m_filePath[96];
    uint32_t m_assetType, m_fileLength, m_unk;
    WAD_FILE_HEADER *m_nextFile, *m_nextFileSameAsset;
    uint64_t f80;
    void *f88_visited;
    int32_t m_crypted, f94;
    uint64_t f98, fA0, fA8, fB0, fB8;
    uint8_t m_firstChar;
};
struct WAD_HEADER {
    char m_fileSignature[4];
    uint32_t arr25[25];
    WAD_FILE_HEADER *m_assets[17];
    uint32_t m_signature;
    uint32_t m_version;
    uint32_t m_decompressed_size;
    uint32_t m_compressed_size;
};

enum class WAD_ASSET_TYPE { GDE, SKY, COLL, BOG, SND, ENTITY, MOBY, TIE, SHRUB, TEXTURE, SHADER, PARTICLE, UI, GLOBAL, NAV, PVAR_INCLUDE, TUNING_INCLUDE, WAT_CNT };
class WADManager {
public:
    void LoadWADFile(const char *name);
    WAD_FILE_HEADER *GetWadFileHeaderByItemName(const char *pItemPathName, WAD_ASSET_TYPE type, uint64_t *ptr, WAD_FILE_HEADER **ptrFh);
    static const char *AssetType(WAD_ASSET_TYPE at);
};
extern WADManager g_WADManager;
void *WAD_FindAssetsByName(const char *pItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader);
void *WAD_FindAssetsByCrcName(uint32_t crcItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader);
