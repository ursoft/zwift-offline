#include "ZwiftApp.h"
const char *AssetType(WAD_ASSET_TYPE at) {
    const char *assetNames[] = { "gde", "sky", "coll", "bog", "snd", "entity", "moby",
        "tie", "shrub", "texture", "shader", "particle", "ui", "global", "nav", "pvar_include",
        "tuning_include", "???" };
    return assetNames[(int)at];
}
WAD_ASSET_TYPE GuessAssetType(const char *filePath) {
    assert(*filePath);
    if (*filePath == 0) return WAD_ASSET_TYPE::GLOBAL;
    auto sz = strlen(filePath);
    const char *ptr = filePath + sz - 1;
    if (*ptr == 'e') {
        if (sz > 3 && ptr[-1] == 'd' && ptr[-2] == 'g' && ptr[-3] == '.') return WAD_ASSET_TYPE::GDE;
    } else if (*ptr == 'x') {
        if (sz > 3 && ptr[-1] == 't') {
            if (ptr[-2] == 'z' && ptr[-3] == '.') return WAD_ASSET_TYPE::TEXTURE;
        } else if (sz > 4 && ptr[-1] == 'a') {
            if (ptr[-2] == 'g' && ptr[-3] == 't' && ptr[-4] == '.') return WAD_ASSET_TYPE::TEXTURE;
        }
    } else if (sz > 4 && *ptr == 'h' && ptr[-1] == 's') { return WAD_ASSET_TYPE::SHADER; }
    return WAD_ASSET_TYPE::GLOBAL;
}
int TJZIP_ParseDictionaryCode(uint8_t **pSrcPtr, uint8_t **pDestPtr, uint32_t *crc32) {
	int32_t backOff = 0;
	int len = 0;
	int result;
	auto code = **pSrcPtr;
	(*pSrcPtr)++;
	auto chr1 = **pSrcPtr;
	(*pSrcPtr)++;
	uint8_t chr2, chr3;
	switch (code & 0xE0) {
	case 0xE0:
		chr2 = **pSrcPtr;
		(*pSrcPtr)++;
		if ((code & 0xF) != 0) {
			len = (code & 0xF) + 3;
			backOff = (code << 10) & 0x4000 | (chr1 >> 2 << 8) | chr2;
			result = chr1 & 3;
		} else {
			chr3 = **pSrcPtr;
			(*pSrcPtr)++;
			if (chr1) {
				len = chr1 + 18;
				backOff = (code << 10) & 0x4000 | (chr2 >> 2 << 8) | chr3;
				result = chr2 & 3;
			} else {
				auto chr4 = **pSrcPtr;
				(*pSrcPtr)++;
				auto chr5 = **pSrcPtr;
				(*pSrcPtr)++;
				len = chr3 | (chr2 << 8);
				result = chr4 & 3;
				if (len)
					backOff = (code << 10) & 0x4000 | (chr4 >> 2 << 8) | chr5;
			}
		}
		break;
	case 0xC0:
		chr2 = **pSrcPtr;
		(*pSrcPtr)++;
		len = (code & 0x1F) + 4;
		backOff = chr2 | (chr1 >> 2 << 8);
		result = chr1 & 3;
		break;
	default:
		len = (code >> 5) + 4;
		result = code & 3;
		backOff = chr1 | (((code >> 2) & 3) << 8);
		break;
	}
	auto pDict = (*pDestPtr) - backOff;
	while (len--) {
		auto chr = *pDict++;
		**pDestPtr = chr;
		(*pDestPtr)++;
		*crc32 = g_crc32Table[(uint8_t)*crc32 ^ chr] ^ (*crc32 >> 8);
	}
	return result;
}
void TJZIP_ParseRawDataBlock(uint8_t **pSrcPtr, uint8_t **pDestPtr, uint32_t *crc32) {
	uint8_t chr0 = **pSrcPtr;
	(*pSrcPtr)++;
	int len;
	if (chr0) {
		len = chr0 + 2;
	} else {
		uint8_t chr1 = **pSrcPtr;
		(*pSrcPtr)++;
		uint8_t chr2 = **pSrcPtr;
		(*pSrcPtr)++;
		if (chr2) {
			len = (chr2 << 8) | chr1;
		} else {
			uint8_t chr3 = **pSrcPtr;
			(*pSrcPtr)++;
			uint8_t chr4 = **pSrcPtr;
			(*pSrcPtr)++;
			if (chr3) {
				len = (chr3 << 16) | (chr4 << 8) | chr1;
			} else {
				uint8_t chr5 = **pSrcPtr;
				(*pSrcPtr)++;
				uint8_t chr6 = **pSrcPtr;
				(*pSrcPtr)++;
				len = (chr4 << 24) | (chr5 << 8) | (chr6 << 16) | chr1;
			}
		}
	}
	while (len--) {
		**pDestPtr = **pSrcPtr;
		*crc32 = g_crc32Table[(uint8_t)*crc32 ^ **pSrcPtr] ^ (*crc32 >> 8);
		(*pSrcPtr)++;
		(*pDestPtr)++;
	}
}
uint32_t TJZIP_Decompress(uint8_t *pDecompressPtr, uint8_t *m_decomp_buf, uint32_t compressed_size, uint32_t *crc32) {
	uint8_t *destPtr = m_decomp_buf;
	uint8_t *srcPtr = pDecompressPtr;
	*crc32 = 0;
	TJZIP_ParseRawDataBlock(&srcPtr, &destPtr, crc32);
	while (srcPtr - pDecompressPtr < (int)compressed_size) {
		int dCode = TJZIP_ParseDictionaryCode(&srcPtr, &destPtr, crc32);
		if (dCode != 3) {
			if (dCode == 0) {
				TJZIP_ParseRawDataBlock(&srcPtr, &destPtr, crc32);
			} else {
				if (srcPtr - pDecompressPtr < (int)compressed_size) {
					for (int i = 0; i < dCode; i++) {
						uint8_t chr = *srcPtr++;
						*destPtr++ = chr;
						*crc32 = g_crc32Table[(uint8_t)*crc32 ^ chr] ^ (*crc32 >> 8);
					}
				}
			}
		}
		if (pDecompressPtr < m_decomp_buf != srcPtr < destPtr)
			return 0;
	}
	*crc32 = ~*crc32;
	return uint32_t(destPtr - m_decomp_buf);
}
bool WAD_Load(const char *pFilename, WAD_HEADER **ppDest, bool restorePointers /*= true*/) {
	FILE *fwad;
	auto err = fopen_s(&fwad, pFilename, "rb");
	if (err != 0 || fwad == nullptr)
		return false;
	WAD_HEADER wad_hdr;
	bool ret = false;
	if (sizeof(wad_hdr) == fread_s(&wad_hdr, sizeof(wad_hdr), 1, sizeof(wad_hdr), fwad)) {
		if (wad_hdr.m_fileSignature[0] == 'Z' && wad_hdr.m_fileSignature[1] == 'W' &&
			wad_hdr.m_fileSignature[2] == 'F' && wad_hdr.m_fileSignature[3] == '!') {
			if (wad_hdr.m_version == WAD_VERSION) {
				uint32_t decomp_buf_sz = ((wad_hdr.m_decompressed_size + 263) & 0xFFFFFFF8) + 0x100020;
				auto decomp_buf = (uint8_t *)calloc(decomp_buf_sz, 1);
				if (decomp_buf) {
					auto pCompressedPtr = (uint8_t *)calloc(wad_hdr.m_compressed_size, 1);
					if (pCompressedPtr) {
						memcpy(decomp_buf, &wad_hdr, sizeof(wad_hdr));
						if (wad_hdr.m_compressed_size) {
							if (wad_hdr.m_compressed_size != fread_s(pCompressedPtr, wad_hdr.m_compressed_size, 1, wad_hdr.m_compressed_size, fwad)) {
								free(pCompressedPtr);
								free(decomp_buf);
								fclose(fwad);
								return ret;
							}
						}
						uint32_t crc32 = 0;
						uint32_t resultLength = TJZIP_Decompress(
							pCompressedPtr,
							decomp_buf + sizeof(wad_hdr),
							wad_hdr.m_compressed_size,
							&crc32);
						free(pCompressedPtr);
						WAD_HEADER *wh = (WAD_HEADER *)decomp_buf;
						if (resultLength == wh->m_decompressed_size) {
                            if (wh->m_crc32 == crc32) {
                                WAD_OffsetsToPointers(wh);
                                *ppDest = wh;
                                ret = true;
                            }
						}
					}
					if (!ret)
						free(decomp_buf);
				}
			}
		}
	}
	fclose(fwad);
    return ret;
}
void WADManager::LoadWADFile(const char *pFilename) {
    zassert(pFilename && "LoadWADFile failed - Must have non-NULL parameter");
    if (pFilename) {
        auto crc = SIG_CalcCaseInsensitiveSignature(pFilename);
        if (IsWADFileLoaded(crc)) {
            Log("Wad %s at 0x08X already loaded", pFilename, crc);
        } else {
            WAD_HEADER *pRes = nullptr;
            if (WAD_Load(pFilename, &pRes)) {
                struct _stat64i32 v9;
                _stat64i32(pFilename, &v9);
                m_wads[crc] = LoadedWad{crc, v9.st_mtime, pRes };
            } else {
                LogTyped(LOG_ERROR, "Could not load wad file: %s", pFilename);
            }
        }
    }
}
bool WADManager::IsWADFileLoaded(const char *name) { return IsWADFileLoaded(SIG_CalcCaseInsensitiveSignature(name)); }
WAD_FILE_HEADER *WAD_FindAssetsBySignature(uint32_t crcItemPathName, WAD_ASSET_TYPE type, WAD_HEADER *pHeader) {
    if (!crcItemPathName || !pHeader)
        return nullptr;
    int ty = (int)type;
    auto bucketIdx = ((ty | (32 * ty)) ^ crcItemPathName) & (HASH_BUCKETS - 1);
    auto ret = ((WAD_FILE_HEADER **)((char *)pHeader + sizeof(WAD_HEADER)))[bucketIdx];
    if (ret) {
        do {
            if (ret->m_nameICRC32 == crcItemPathName && (ret->m_assetType == type || ret->m_assetType == WAD_ASSET_TYPE::LINK_TO))
                break;
            ret = ret->m_nextFileSameHash;
        } while (ret);
        if (ret && ret->m_assetType == WAD_ASSET_TYPE::LINK_TO && ret->m_link) {
            if (!ret->f80 && ret->m_link->f80) {
                ret->f80 = ret->m_link->f80;
                ret->m_fileLength = ret->m_link->m_fileLength;
                ret->m_crypted = ret->m_link->m_crypted;
            }
            return ret->m_link;
        }
    }
    return ret;
}
WAD_FILE_HEADER *WADManager::GetWadFileHeaderByItemName(const char *pItemPathName, WAD_ASSET_TYPE type, time_t *wadTouchTime) {
    WAD_FILE_HEADER *ret = nullptr;
    auto crc = SIG_CalcCaseInsensitiveSignature(pItemPathName);
    for (auto i : m_wads) {
        ret = WAD_FindAssetsBySignature(crc, type, i.second.m_wadHeader);
        if (ret) {
            if (wadTouchTime)
                *wadTouchTime = i.second.m_time;
            break;
        }
    }
    return ret;
}
void WADManager::DeleteAllWadFiles() {
    for (auto i : m_wads)
        free(i.second.m_wadHeader);
    m_wads.clear();
}
void WADManager::DeleteWADFile(const char *name) {
    auto it = m_wads.extract(SIG_CalcCaseInsensitiveSignature(name));
    if (!it.empty()) {
        free(it.mapped().m_wadHeader);
    }
}
WAD_HEADER *WADManager::GetWadHeaderByFileName(const char *name, time_t *t) {
    auto res = m_wads.find(SIG_CalcCaseInsensitiveSignature(name));
    if (res != m_wads.end()) {
        if (t) *t = res->second.m_time;
        return res->second.m_wadHeader;
    }
    return nullptr;
}
void WAD_FindAssetsByType(WAD_ASSET_TYPE, WAD_HEADER *) {
    //OMIT (not used or inlined?)
}
void WAD_NextAssetByType(WAD_FILE_HEADER *) {
    //OMIT (not used or inlined?)
}
void EncryptDecryptWadString(uint8_t *str, uint32_t length) {
    static const uint8_t g_cypher[7] = { 0x35, 0x2e, 0x51, 0x58, 0x32, 0x59, 0x30 };
    for (uint32_t i = 0; i < length; i++) {
        auto j = length - i;
        str[i] ^= (g_cypher[i % 7] - (j % 26) - 65);
    }
}
void WAD_OffsetsToPointers(WAD_HEADER *wh) {
	uint8_t *bwh = (uint8_t *)wh;
	for (int assetIdx = 0; assetIdx < (int)WAD_ASSET_TYPE::CNT; assetIdx++) {
		if (wh->m_assets[assetIdx]) {
			auto pfh = (WAD_FILE_HEADER *)(bwh + (uint64_t)wh->m_assets[assetIdx]);
			assert((int)pfh->m_assetType == assetIdx);
			wh->m_assets[assetIdx] = pfh;
			if (pfh->m_crypted)
				EncryptDecryptWadString(pfh->FirstChar(), pfh->m_fileLength);
			if (pfh->m_link) {
				pfh->m_link = (WAD_FILE_HEADER *)(bwh + (uint64_t)pfh->m_link);
			}
			while (pfh->m_nextFileSameAsset) {
				auto pnfh = (WAD_FILE_HEADER *)(bwh + (uint64_t)pfh->m_nextFileSameAsset);
				assert(pfh->m_assetType == pnfh->m_assetType);
				pfh->m_nextFileSameAsset = pnfh;
				if (pnfh->m_link) {
					pnfh->m_link = (WAD_FILE_HEADER *)(bwh + (uint64_t)pnfh->m_link);
				}
				if (pnfh->m_crypted)
					EncryptDecryptWadString(pnfh->FirstChar(), pnfh->m_fileLength);
				pfh = pnfh;
			}
		}
	}
	auto ptrAfterHeader = (int64_t *)(bwh + sizeof(WAD_HEADER));
	for (int64_t dirIdx = 0; dirIdx != HASH_BUCKETS; dirIdx++) {
		auto dirOffset = ptrAfterHeader[dirIdx];
		if (dirOffset) {
			auto dirPtr = (WAD_FILE_HEADER *)(bwh + dirOffset);
			ptrAfterHeader[dirIdx] = (int64_t)dirPtr;
			if (dirPtr) {
				while (dirPtr->m_nextFileSameHash) {
					auto filePtr = (WAD_FILE_HEADER *)(bwh + (int64_t)dirPtr->m_nextFileSameHash);
					dirPtr->m_nextFileSameHash = filePtr;
					dirPtr = filePtr;
				}
			}
		}
	}
}

//Unit Tests
TEST(SmokeTest, Wads) {
	g_WADManager.LoadWADFile("assets\\global.wad");
	time_t t;
	auto map = g_WADManager.GetWadFileHeaderByItemName("MapSchedule_v2.xml", WAD_ASSET_TYPE::GLOBAL, &t);
	EXPECT_EQ(0x0000000063892f2b, t);
	EXPECT_STREQ("global", map->AssetType());
	EXPECT_STREQ("MapSchedule_v2.xml", map->m_filePath);
	EXPECT_EQ(2621, map->m_fileLength);
	EXPECT_EQ(0x86, map->m_seqNo);
	EXPECT_EQ(0x7dd5d10d, map->m_nameICRC32);
	EXPECT_EQ(WAD_ASSET_TYPE::GLOBAL, map->m_assetType);
	EXPECT_STREQ("<?xml", std::string((char *)map->FirstChar(), 5).c_str());
	EXPECT_EQ(2605, int(strstr((char *)map->FirstChar(), "</MapSchedule>") - (char *)map->FirstChar()));
	g_WADManager.DeleteAllWadFiles();
}
