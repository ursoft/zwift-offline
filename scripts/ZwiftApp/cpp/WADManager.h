#pragma once
enum class WAD_ASSET_TYPE { GDE, SKY, COLL, BOG, SND, ENTITY, MOBY, TIE, SHRUB, TEXTURE, SHADER, PARTICLE, UI, GLOBAL, NAV, PVAR_INCLUDE, TUNING_INCLUDE, CNT };
const char *AssetType(WAD_ASSET_TYPE at);
const int FILE_PATH_SIZE = 96, WAD_VERSION = 11, HASH_BUCKETS = 1024;
struct WAD_FILE_HEADER {
    uint32_t m_nameICRC32 = 0;
    char m_filePath[FILE_PATH_SIZE] = {};
    WAD_ASSET_TYPE m_assetType = WAD_ASSET_TYPE::CNT;
    uint32_t m_fileLength = 0;
    uint32_t m_seqNo = 0; //started from 0, increments by 1 and unique (we invert it to mark "visited")
    WAD_FILE_HEADER *m_nextFileSameHash = nullptr, *m_nextFileSameAsset = nullptr;
    uint64_t f80 = 0;
    WAD_FILE_HEADER *m_link = nullptr;
    int32_t m_crypted = 0, f94 = 0;
    uint64_t f98 = 0, fA0 = 0, fA8 = 0, fB0 = 0, fB8 = 0;
    uint8_t *FirstChar() { return (uint8_t *)this + sizeof(WAD_FILE_HEADER); }
    WAD_FILE_HEADER() { static_assert(sizeof(WAD_FILE_HEADER) == 0xC0); }
    const char *AssetType() {
        WAD_ASSET_TYPE corr = WAD_ASSET_TYPE::CNT;
        if (m_assetType < WAD_ASSET_TYPE::CNT) corr = m_assetType;
        return ::AssetType(corr);
    }
};
struct WAD_HEADER {
    char m_fileSignature[4] = {};
    char m_wadFilePath[FILE_PATH_SIZE] = {};
    uint32_t m_wadFilePathCrc32 = 0;
    WAD_FILE_HEADER *m_assets[(int)WAD_ASSET_TYPE::CNT] = {};
    uint32_t m_crc32 = 0;
    uint32_t m_version = 0;
    uint32_t m_decompressed_size = 0;
    uint32_t m_compressed_size = 0;
    WAD_HEADER() { static_assert(256 == sizeof(WAD_HEADER)); }
};
class WADManager {
public:
    void LoadWADFile(const char *name);
    WAD_FILE_HEADER *GetWadFileHeaderByItemName(const char *pItemPathName, WAD_ASSET_TYPE type, uint64_t *ptr, WAD_FILE_HEADER **ptrFh);
};
void *WAD_FindAssetsByName(const char *pItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader);
void *WAD_FindAssetsByCrcName(uint32_t crcItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader);

inline WADManager g_WADManager;
