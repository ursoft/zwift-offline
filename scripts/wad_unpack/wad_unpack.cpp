#include <iostream>
#include <cstdio>
#include <filesystem>
#include <windows.h>
#include <map>
#include "WCX-SDK-master\src\wcxhead.h"
#include <cassert>
#include <string>
#include <functional>

const uint32_t g_crc32Table[256] = {
	0, 0x77073096, 0xEE0E612C, 0x990951BA, 0x76DC419, 0x706AF48F,
	0xE963A535, 0x9E6495A3, 0xEDB8832, 0x79DCB8A4, 0xE0D5E91E,
	0x97D2D988, 0x9B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D,
	0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0,
	0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63,
	0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA,
	0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75,
	0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180,
	0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87,
	0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x1DB7106,
	0x98D220BC, 0xEFD5102A, 0x71B18589, 0x6B6B51F, 0x9FBFE4A5,
	0xE8B8D433, 0x7807C9A2, 0xF00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x86D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
	0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B,
	0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA,
	0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541,
	0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC,
	0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F,
	0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E,
	0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x3B6E20C,
	0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x4DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0xD6D6A3E, 0x7A6A5AA8, 0xE40ECF0B,
	0x9309FF9D, 0xA00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2,
	0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671,
	0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8,
	0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767,
	0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6,
	0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795,
	0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
	0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B,
	0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x26D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x5005713, 0x95BF4A82,
	0xE2B87A14, 0x7BB12BAE, 0xCB61B38, 0x92D28E9B, 0xE5D5BE0D,
	0x7CDCEFB7, 0xBDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8,
	0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF,
	0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE,
	0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D,
	0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C,
	0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
	0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02,
	0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
uint32_t SIG_CalcCaseInsensitiveSignature(const char *str) {
	if (str) {
		uint32_t crc32 = 0xFFFFFFFF;
		while (true) {
			switch (*str) {
			case 0:
				return ~crc32;
			case '/': case '\\': 
				crc32 = g_crc32Table[(uint8_t)(crc32 ^ '\\')] ^ (crc32 >> 8);
				str++;
				while (*str && (*str == '/' || *str == '\\')) str++;
				break;
			default:
				crc32 = g_crc32Table[(uint8_t)(crc32 ^ toupper(*str++))] ^ (crc32 >> 8);
				break;
			}
		}
	}
	return 0;
}
const char *AssetType(int at) {
	const char *assetNames[] = { "gde", "sky", "coll", "bog", "snd", "entity", "moby",
		"tie", "shrub", "texture", "shader", "particle", "ui", "global", "nav", "pvar_include",
		"tuning_include", "???" };
	return assetNames[at];
}
enum class WAD_ASSET_TYPE : uint32_t {
	GDE,
	SKY,
	COLL,
	BOG,
	SND,
	ENTITY,
	MOBY,
	TIE,
	SHRUB,
	TEXTURE,
	SHADER,
	PARTICLE,
	UI,
	GLOBAL,
	NAV,
	PVAR_INCLUDE,
	TUNING_INCLUDE,
	CNT
};
const int FILE_PATH_SIZE = 96, WAD_VERSION = 11, HASH_BUCKETS = 1024;
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
struct WadUnpacker {
	int g_ret = 0;
	bool g_bDumpMode;
	tProcessDataProc m_wcxProcess = nullptr;
	operator int() const { return g_ret; }
	std::string m_wadFileName;
	WadUnpacker(const char *fileName, bool bDumpMode = true) : m_wadFileName(fileName) {
		g_bDumpMode = bDumpMode;
		m_curPosition = m_list.cbegin();
		auto err = fopen_s(&g_fwad, fileName, "rb");
		if (err != 0 || g_fwad == nullptr) {
			if (bDumpMode) std::cerr << "wad_unpack error: cannot open '" << fileName << "' for read, error: " << err << std::endl;
			g_ret = -2;
			return;
		}
		WAD_HEADER wad_hdr;
		if (sizeof(wad_hdr) == fread_s(&wad_hdr, sizeof(wad_hdr), 1, sizeof(wad_hdr), g_fwad)) {
			if (wad_hdr.m_fileSignature[0] == 'Z' && wad_hdr.m_fileSignature[1] == 'W' &&
				wad_hdr.m_fileSignature[2] == 'F' && wad_hdr.m_fileSignature[3] == '!') {
				if (wad_hdr.m_version == WAD_VERSION) {
					uint32_t decomp_buf_sz = ((wad_hdr.m_decompressed_size + 263) & 0xFFFFFFF8) + 0x100020;
					m_decomp_buf = (uint8_t *)calloc(decomp_buf_sz, 1);
					if (m_decomp_buf) {
						auto pCompressedPtr = (uint8_t *)calloc(wad_hdr.m_compressed_size, 1);
						if (pCompressedPtr) {
							memcpy(m_decomp_buf, &wad_hdr, sizeof(wad_hdr));
							if (wad_hdr.m_compressed_size) {
								if (wad_hdr.m_compressed_size != fread_s(pCompressedPtr, wad_hdr.m_compressed_size, 1, wad_hdr.m_compressed_size, g_fwad)) {
									g_ret = -10;
									if (bDumpMode) std::cerr << "wad_unpack error: could not read compressed block\n";
									return;
								}
							}
							uint32_t crc32 = 0;
							uint32_t resultLength = TJZIP_Decompress(
								pCompressedPtr,
								m_decomp_buf + sizeof(wad_hdr),
								wad_hdr.m_compressed_size,
								&crc32);
							/*FILE *ftmp = nullptr;
							fopen_s(&ftmp, "c:\\tmp\\decompress.bin", "wb");
							fwrite(m_decomp_buf + sizeof(wad_hdr), resultLength, 1, ftmp);
							fclose(ftmp);
							FILE *ftmp = nullptr;
							auto c2 = TJZIP_Compress(m_decomp_buf + sizeof(wad_hdr), resultLength, pCompressedPtr, &crc32);
							fopen_s(&ftmp, "c:\\tmp\\c3.bin", "wb");
							fwrite(&wad_hdr, 256, 1, ftmp);
							fwrite(pCompressedPtr, wad_hdr.m_compressed_size, 1, ftmp);
							fclose(ftmp);*/
#ifdef _DEBUG
#if 0 //compress test
							uint32_t crc32_chk = 0;
							auto pCompressedPtrChk = (uint8_t *)calloc(wad_hdr.m_compressed_size, 1);
							auto comp_chk_sz = TJZIP_Compress(m_decomp_buf + sizeof(wad_hdr), wad_hdr.m_decompressed_size, pCompressedPtrChk, &crc32_chk);
							assert(comp_chk_sz == wad_hdr.m_compressed_size);
							assert(crc32_chk == crc32);
							assert(0 == memcmp(pCompressedPtrChk, pCompressedPtr, wad_hdr.m_compressed_size));
#endif
#endif

							free(pCompressedPtr);
							WAD_HEADER *wh = (WAD_HEADER *)m_decomp_buf;
							wh->dump(g_bDumpMode);
							if (resultLength == wh->m_decompressed_size) {
								if (wh->m_crc32 != crc32) {
									g_ret = -8;
									if (bDumpMode) std::cerr << "\nwad_unpack error: wad file counted crc32: " << crc32
										<< " but header value is:" << wh->m_crc32 << "\n";
								}
								WAD_OffsetsToPointers(wh);
								m_curPosition = m_list.cbegin();
							} else {
								g_ret = -7;
								if (bDumpMode) std::cerr << "\nwad_unpack error: Decompressed length error: resulting length = " << resultLength << ". Expected length = " <<
									wh->m_decompressed_size << std::endl;
							}
						} else {
							g_ret = -9;
							if (bDumpMode) std::cerr << "wad_unpack error: no memory for compressed, we need: " << wad_hdr.m_compressed_size << std::endl;
						}
					} else {
						g_ret = -6;
						if (bDumpMode) std::cerr << "wad_unpack error: no memory for decompressed, we need: " << decomp_buf_sz << std::endl;
					}
				} else {
					g_ret = -5;
					if (bDumpMode) std::cerr << "wad_unpack error: unexpected wad file version(we support " << WAD_VERSION << "): " << wad_hdr.m_version << std::endl;
				}
			} else {
				g_ret = -4;
				if (bDumpMode) std::cerr << "wad_unpack error: unexpected wad file crc32\n";
			}
		} else {
			g_ret = -3;
			if (bDumpMode) std::cerr << "wad_unpack error: unexpected eof while reading wad header\n";
		}
	}
	void Close() { if (g_fwad) fclose(g_fwad); g_fwad = nullptr; }
	~WadUnpacker() {
		Close();
		if (m_decomp_buf) free(m_decomp_buf);
	}
	FILE *g_fwad = nullptr;
	uint8_t *m_decomp_buf = nullptr;

	void EncryptDecryptWadString(uint8_t *str, uint32_t length) {
		static const uint8_t g_cypher[7] = { 0x35, 0x2e, 0x51, 0x58, 0x32, 0x59, 0x30 };
		for (uint32_t i = 0; i < length; i++) {
			auto j = length - i;
			str[i] ^= (g_cypher[i % 7] - (j % 26) - 65);
		}
	}
	struct ci_less {
		struct nocase_compare {
			bool operator() (const uint8_t &c1, const uint8_t &c2) const { return tolower(c1) < tolower(c2); }
		};
		bool operator() (const std::string &s1, const std::string &s2) const {
			return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare());
		}
	}; 
	struct WAD_FILE_HEADER;
	using WadList = std::map<std::string, WAD_FILE_HEADER *, ci_less>;
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
			return ::AssetType((int)corr);
		}
		uint32_t dump(bool bDumpMode, WadList *pList) {
			//also used to fast find WAD_FILE_HEADER index[0...(HASH_BUCKETS - 1)] (8 * ((ty | (32 * ty)) ^ sign & (HASH_BUCKETS - 1)):
			assert(m_assetType < WAD_ASSET_TYPE::CNT); //not used yet: WAT_LINK = 0x19
			assert(m_crypted == 0 || m_crypted == 1);
			assert(f80 == 0); //copied to link with m_crypted and m_fileLength
			assert(m_link == 0);
			assert(f94 == 0); //reserved?
			assert(f98 == 0);
			assert(fA0 == 0);
			assert(fA8 == 0);
			assert(fB0 == 0);
			assert(fB8 == 0);
			char filePath[sizeof(m_filePath) + 1] = {};
			memcpy(filePath, m_filePath, sizeof(m_filePath));
			std::string fix;
			if (m_nameICRC32 != SIG_CalcCaseInsensitiveSignature(filePath)) {
				assert(m_filePath[sizeof(m_filePath) - 1] != 0);
				//name overflow
				std::string fixed = filePath;
				if (fixed == "Workouts/Return_to_Running_Plan_On_Demand/Week_8/8_6_10minjog5x1minquick1mineasy10mineasycopy.xm") {
					fix = "l";
				} else if (fixed == "bikes/Frames/Specialized_Roubaix/Textures/SpecializedRuby_UltregraUDI2_TurquoiseHyperGreenBlack.") {
					fix = "tgax";
				}
				fixed += fix;
				std::cout << "[name fixed] ";
				assert(m_nameICRC32 == SIG_CalcCaseInsensitiveSignature(fixed.c_str()));
			}
			std::cout << (m_crypted ? '{' : '[') << AssetType() << (m_crypted ? "} " : "] ") << filePath;
			assert(m_filePath[0] != 0);
			for (size_t idx = strlen(filePath); idx < sizeof(filePath); idx++) {
				assert(filePath[idx] == 0);
			}
			if (m_seqNo & 0x80000000) {
				if (bDumpMode) std::cout << " id:" << ~m_seqNo << " skipped (already saved)\n";
			} else {
				char path[sizeof(m_filePath) + 5] = {}; //maximum overflow is 'tgax'
				int lastDelimiter = -1;
				for (int i = 0; i < sizeof(m_filePath); i++) {
					char cin = m_filePath[i];
					if (cin == 0) break;
					switch (cin) {
					case '/': case '\\':
						lastDelimiter = i;
						path[i] = '\\';
						break;
					case '"': case '*': case '<': case '>': case '?': case '|': case ':':
						assert(false);
						path[i] = '#';
						break;
					default:
						path[i] = cin;
						break;
					}
				}
				if (fix.length()) strcat_s(path, fix.c_str());
				(*pList)[path] = this;
				if (bDumpMode) {
					if (lastDelimiter != -1) {
						path[lastDelimiter] = 0;
						if (!std::filesystem::is_directory(path) && !std::filesystem::create_directories(path)) {
							std::cout << " failed to create dirs, quit\n";
							exit(-12);
						}
						path[lastDelimiter] = '\\';
					}
					FILE *f = nullptr;
					fopen_s(&f, path, "wb");
					if (f == nullptr || m_fileLength != fwrite(FirstChar(), 1, m_fileLength, f)) {
						std::cout << " failed to write, quit\n";
						exit(-13);
					}
					std::cout << ' ' << m_fileLength << " bytes saved OK\n";
					fclose(f);
				}
				m_seqNo = ~m_seqNo;
			}
			return ~m_seqNo;
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
		void dump(bool bDumpMode) {
			auto crc = SIG_CalcCaseInsensitiveSignature("f:\\Projects\\ZwiftApp\\assets\\global.wad");
			if (bDumpMode) {
				char wadFilePath[sizeof(m_wadFilePath) + 1] = {};
				memcpy(wadFilePath, m_wadFilePath, sizeof(m_wadFilePath));
#ifdef _DEBUG
				for (size_t idx = strlen(wadFilePath); idx < sizeof(wadFilePath); idx++) {
					assert(wadFilePath[idx] == 0);
				}
				//0x7dc80dd6 is SIG_CalcCaseInsensitiveSignature of "f:\\Projects\\ZwiftApp\\assets\\global.wad"
				//but we have no full path here (first symbol is :)
				//assert(wadFilePath[0] == ':'); //and even it is not every time
				//auto wadFilePathSign = SIG_CalcCaseInsensitiveSignature(m_wadFilePath);
				//assert(m_wadFilePathCrc32 == wadFilePathSign); //did not found this field usage in-game
				assert(m_wadFilePathCrc32 != 0);
#endif
				std::cout << "WADv" << m_version << " int_name: '" << wadFilePath << "'; decompr_sz: " << m_decompressed_size <<
					", compr_sz: " << m_compressed_size << std::endl;
			}
		}
	};
	WadList	m_list;
#ifdef _DEBUG
	std::map<uint32_t, uint32_t> m_seqCheck;
#endif
	void doDump(WAD_FILE_HEADER *obj) {
		uint32_t ret = obj->dump(g_bDumpMode, &m_list);
#ifdef _DEBUG
		m_seqCheck[ret]++;
#endif
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
				doDump(pfh);
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
					doDump(pnfh);
					pfh = pnfh;
				}
			}
		}
		auto ptrAfterHeader = (int64_t *)(bwh + sizeof(WAD_HEADER));
		for (int64_t dirIdx = 0; dirIdx != HASH_BUCKETS; dirIdx++) {
			auto dirOffset = ptrAfterHeader[dirIdx];
			if (dirOffset) {
				auto dirPtr = (WAD_FILE_HEADER *)(bwh + dirOffset);
				doDump(dirPtr);
				ptrAfterHeader[dirIdx] = (int64_t)dirPtr;
				if (dirPtr) {
					while (dirPtr->m_nextFileSameHash) {
						auto filePtr = (WAD_FILE_HEADER *)(bwh + (int64_t)dirPtr->m_nextFileSameHash);
						doDump(filePtr);
						dirPtr->m_nextFileSameHash = filePtr;
						dirPtr = filePtr;
					}
				}
			}
		}
#ifdef _DEBUG
		uint32_t min = 0xFFFFFFFF, max = 0;
		if (m_seqCheck.size()) {
			for (auto chk : m_seqCheck) {
				assert(chk.second == 2);
				if (chk.first > max) max = chk.first;
				if (chk.first < min) min = chk.first;
			}
			assert(min == 0);
			assert(max == m_seqCheck.size() - 1);
		}
		for (auto chk : m_list) {
			auto hash = chk.second->m_nameICRC32;
			auto ty = (int)chk.second->m_assetType;
			auto bucketIdx = ((ty | (32 * ty)) ^ hash) & (HASH_BUCKETS - 1);
			auto bucket = ((WAD_FILE_HEADER **)(m_decomp_buf + sizeof(WAD_HEADER)))[bucketIdx];
			while (bucket != chk.second) {
				bucket = bucket->m_nextFileSameHash;
				assert(bucket); //если мы сломались тут, значит hash не соответствует bucket 
			}
		}
		FILE *f[(int)WAD_ASSET_TYPE::CNT] = {};
		for (int i = 0; i < (int)WAD_ASSET_TYPE::CNT; i++) { fopen_s(f + i, (::AssetType(i) + std::string(".lst")).c_str(), "a"); }
		for (auto chk : m_list) {
			auto ty = chk.second->m_assetType;
			switch (ty) {
			case WAD_ASSET_TYPE::GDE: //by extension .gde
			case WAD_ASSET_TYPE::SHADER: //by extension end: 'sh'
			case WAD_ASSET_TYPE::TEXTURE: //by extension .tgax/.ztx
			case WAD_ASSET_TYPE::GLOBAL:
				assert(GuessAssetType(chk.first.c_str()) == ty);
				break;
			default:
				assert(::AssetType(int(ty)) == nullptr);
			}
			fprintf_s(f[(int)ty], "%96s\n", chk.second->m_filePath);
		}
		for (auto pf : f) { fclose(pf); }
#endif //_DEBUG
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
		while(len--) {
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
		while(len--) {
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
	void TJZIP_OutputRaw(uint8_t **ppDest, uint8_t *src, size_t n) {
		if (n <= 2) {
			*(*ppDest - 2) |= n;
		} else if (n <= 257) {
			**ppDest = uint8_t(n - 2);
			++(*ppDest);
		} else if (n <= 65535) {
			**ppDest = 0;
			++(*ppDest);
			**ppDest = uint8_t(n);
			++(*ppDest);
			**ppDest = uint8_t(n >> 8);
			++(*ppDest);
		} else if (0 == (n >> 24)) {
			**ppDest = 0;
			++(*ppDest);
			**ppDest = uint8_t(n);
			++(*ppDest);
			**ppDest = 0;
			++(*ppDest);
			**ppDest = uint8_t(n >> 16);
			++(*ppDest);
			**ppDest = uint8_t(n >> 8);
			++ *ppDest;
		} else if (!(n >> 28)) {
			**ppDest = 0;
			++(*ppDest);
			**ppDest = uint8_t(n);
			++(*ppDest);
			**ppDest = 0;
			++(*ppDest);
			**ppDest = 0;
			++(*ppDest);
			**ppDest = uint8_t(n >> 8);
			++(*ppDest);
			**ppDest = uint8_t(n >> 24);
			++(*ppDest);
			**ppDest = uint8_t(n >> 16);
			++(*ppDest);
		} else {
			assert(false);
			return;
		}
		memcpy(*ppDest, src, n);
		*ppDest += n;
	}
	void TJZIP_OutputCode(uint8_t **ppDest, uint32_t offs, uint32_t len) {
		if (offs <= 0x3FF && len <= 9) {
			**ppDest = ((offs >> 6) & 0xC | (32 * len)) + 0x80;
			++(*ppDest);
			**ppDest = offs;
			++(*ppDest);
		} else if (!(offs >> 14) && len <= 0x23) {
			**ppDest = (len + 60) | 0xC0;
			(*ppDest)++;
			**ppDest = (offs >> 6) & 0xFC;
			(*ppDest)++;
			**ppDest = offs;
			++(*ppDest);
		} else {
			if (offs >> 15) { assert(false); return; }
			if (len <= 0x12) {
				**ppDest = (offs >> 10) & 0x10 | (len + 29) | 0xE0;
				(*ppDest)++;
				**ppDest = (offs >> 6) & 0xFC;
				(*ppDest)++;
				**ppDest = offs;
				++(*ppDest);
			} else if (len <= 0x111) {
				**ppDest = (offs >> 10) & 0x10 | 0xE0;
				(*ppDest)++;
				**ppDest = len - 18;
				(*ppDest)++;
				**ppDest = (offs >> 6) & 0xFC;
				(*ppDest)++;
				**ppDest = offs;
				++(*ppDest);
			} else if (!HIWORD(len)) {
				**ppDest = (offs >> 10) & 0x10 | 0xE0;
				++(*ppDest);
				**ppDest = 0;
				++(*ppDest);
				**ppDest = len >> 8;
				++(*ppDest);
				**ppDest = len;
				++(*ppDest);
				**ppDest = (offs >> 6) & 0xFC;
				++(*ppDest);
				**ppDest = offs;
				++(*ppDest);
			} else {
				assert(false);
			}
		}
	}
	uint32_t TJZIP_Compress(uint8_t *pSrc, uint32_t pSrcLength, uint8_t *pDest, uint32_t *crc32) {
		uint8_t *pDestCur = pDest;
		uint8_t *pSrcCur = pSrc;
		auto pSrcEnd = &pSrc[pSrcLength - 1];
		uint8_t *dictionary = (uint8_t *)calloc(1, 0x8000000u);
		uint32_t offs = 0;
		uint8_t *dictSentence = nullptr;
		auto *pSrcCur_ = pSrc;
		auto v9 = pSrcCur_ + 1;
		uint32_t hash;
		int four[4] = {};
		goto LABEL_5;
	LABEL_2:
		v9 = pSrcCur_ + 1;
	LABEL_3:
		if (v9 <= pSrcEnd) {
			do {
				while (1) {
					pSrcCur_ = v9;
					if (v9 + 3 > pSrcEnd)
						goto LABEL_2;
				LABEL_5:
					four[0] = pSrcCur_[0];
					four[1] = pSrcCur_[1];
					four[2] = pSrcCur_[2];
					four[3] = pSrcCur_[3];
					v9 = pSrcCur_ + 1;
					hash = 16777619 * ((16777619 * ((16777619 * ((16777619 * (four[0] ^ 0x811C9DC5)) ^ four[1])) ^ four[2])) ^ four[3]);
					hash = hash & 0xFFFFFF ^ (hash >> 24);
					dictSentence = *(uint8_t **)&dictionary[8 * hash];
					if (dictSentence) {
						if (pSrcCur_ - dictSentence >= 0)
							offs = uint32_t(pSrcCur_ - dictSentence);
						else
							assert(0);
							//offs = dictSentence - pSrcCur_;
						if (offs < 4 || pSrcEnd - pSrcCur_ < 4)
							goto LABEL_3;
						if (!(offs >> 15))
							break;
					}
LABEL_28:
					*(uint64_t *)&dictionary[8 * hash] = (uint64_t)pSrcCur_;
					if (v9 > pSrcEnd)
						goto LABEL_32;
				}
				if (dictSentence[0] != four[0] || dictSentence[1] != four[1] || dictSentence[2] != four[2] || dictSentence[3] != four[3]) {
					goto LABEL_28;
				}
				uint32_t len = 4LL;
				if (pSrcCur_ + len < pSrcEnd) {
					do {
						if (pSrcCur_ == dictSentence + len)
							break;
						if (dictSentence[len] != pSrcCur_[len])
							break;
						++len;
					} while (&pSrcCur_[len] < pSrcEnd);
				}
				if (pSrcCur == pSrcCur_)
					*(pDestCur - 2) |= 3u;
				else
					TJZIP_OutputRaw(&pDestCur, pSrcCur, pSrcCur_ - pSrcCur);
				TJZIP_OutputCode(&pDestCur, offs, len);
				pSrcCur = &pSrcCur_[len];
				v9 = pSrcCur;
			} while (pSrcCur <= pSrcEnd);
		}
LABEL_32:
		if (pSrcCur != v9) TJZIP_OutputRaw(&pDestCur, pSrcCur, v9 - pSrcCur);
		*crc32 = 0;
		for (auto p = pSrc; p <= pSrcEnd; p++) *crc32 = g_crc32Table[(uint8_t)*crc32 ^ *p] ^ (*crc32 >> 8);
		*crc32 = ~*crc32;
		free(dictionary);
		return uint32_t(pDestCur - pDest);
	}
	WadList::const_iterator m_curPosition;
	bool ReadHeader(tHeaderData *HeaderData) {
		if (m_curPosition == m_list.cend()) {
			m_curPosition = m_list.cbegin();
			return false;
		}
		strcpy_s(HeaderData->FileName, m_curPosition->first.c_str());
		HeaderData->UnpSize = HeaderData->PackSize = m_curPosition->second->m_fileLength;
		HeaderData->FileAttr = (m_curPosition->second->m_crypted) ? 0x2 : 0;
		return true;
	}
	int DeleteFiles(char *DeleteList) {
		while (*DeleteList) {
			auto len = strlen(DeleteList);
			auto last = DeleteList + len - 1;
			if (len > 3 && *last == '*' && last[-1] == '.' && last[-2] == '*') {
				std::list<std::string> todel;
				std::string path(DeleteList, last - 3);
				for (auto i : m_list)
					if (i.first.find(path) == 0) todel.push_back(i.first);
				for (auto i : todel)
					m_list.erase(i);
			} else {
				/*auto erased = */ m_list.erase(DeleteList);
				//assert(erased);
			}
			DeleteList += len + 1;
		}
		return Save();
	}
	int PackFiles(char *SubPath, char *SrcPath, char *AddList, int Flags) {
		std::list<WAD_FILE_HEADER *> pool;
		//std::list<std::string> srcList;
		int ret = 0;
		while (*AddList) {
			while (*AddList == '\\') AddList++;
			char *name = AddList, *nextAdd = AddList + strlen(AddList) + 1;
			if (nextAdd[-2] != '\\') {
				char arcPathName[MAX_PATH] = {}, srcPathName[MAX_PATH] = {};
				if (SubPath && *SubPath) {
					strcpy_s(arcPathName, SubPath);
					auto pos = strlen(arcPathName);
					if (pos < MAX_PATH && arcPathName[pos - 1] != '\\') arcPathName[pos] = '\\';
				}
				if (0 == (Flags & PK_PACK_SAVE_PATHS)) {
					char *newName = nextAdd;
					while (--newName > name) {
						if (*newName == '\\') {
							newName++;
							break;
						}
					}
					name = newName;
				}
				strcat_s(arcPathName, name);
				if (SrcPath && *SrcPath) {
					strcpy_s(srcPathName, SrcPath);
					auto pos = strlen(srcPathName);
					if (pos < MAX_PATH && srcPathName[pos - 1] != '\\') srcPathName[pos] = '\\';
				}
				strcat_s(srcPathName, AddList);
				FILE *f = nullptr;
				fopen_s(&f, srcPathName, "rb");
				if (f == nullptr) {
					ret = E_EREAD;
					break;
				}
				//if (Flags & PK_PACK_MOVE_FILES) srcList.push_back(srcPathName);
				fseek(f, 0, SEEK_END);
				auto fileSize = ftell(f);
				fseek(f, 0, SEEK_SET);
				if (fileSize > 1024 * 1024 * 1024) {
					ret = E_NO_MEMORY;
					fclose(f);
					break;
				}
				auto fh = (WAD_FILE_HEADER *)calloc(1, fileSize + sizeof(WAD_FILE_HEADER));
				if (fh == nullptr) {
					ret = E_NO_MEMORY;
					fclose(f);
					break;
				}
				pool.push_back(fh);
				fh->m_nameICRC32 = SIG_CalcCaseInsensitiveSignature(arcPathName);
				auto copy = strlen(arcPathName);
				if (copy > FILE_PATH_SIZE) copy = FILE_PATH_SIZE;
				memcpy(fh->m_filePath, arcPathName, copy);
				for (auto &ch : fh->m_filePath) if (ch == '\\') ch = '/';
				fh->m_assetType = GuessAssetType(srcPathName);
				fh->m_fileLength = (uint32_t)fileSize;
				if (fileSize != fread_s(fh->FirstChar(), fileSize, 1, fileSize, f)) {
					ret = E_EREAD;
					fclose(f);
					break;
				}
				fclose(f);
				//all other fh fields should be set on save
				m_list[arcPathName] = fh;
			}
			AddList = nextAdd;
		}
		if (ret == 0)
			ret = Save();
		m_list.clear(); //good to clear after asve and before pool free
		m_curPosition = m_list.cbegin();
		for (auto pi : pool) free(pi);
		if (ret) return ret;
		/*if (Flags & PK_PACK_MOVE_FILES) {
			BOOL allDeleted = 1;
			for (auto s : srcList) {
				allDeleted &= ::DeleteFileA(s.c_str());
			}
			//if (allDeleted == 0) return E_
		}*/
		return 0;
	}
	int Save() {
		int ret = 0;
		Close();
		//I hope we should not keep original sequence and m_crypted flag
		//WAD_FILE_HEADER: f80 and links are saved as 0 (FIXME when this fields will used)
		FILE *f = nullptr;
		fopen_s(&f, m_wadFileName.c_str(), "wb");
		if (f == nullptr) {
			return E_EWRITE;
		}
		uint32_t seqNo = 0;
		WAD_HEADER wad_header;
		memcpy_s(wad_header.m_fileSignature, sizeof(wad_header.m_fileSignature), "ZWF!", 4);
		size_t nameOffset = 0;
		if (m_wadFileName.length() > sizeof(wad_header.m_wadFilePath)) {
			nameOffset = m_wadFileName.length() - sizeof(wad_header.m_wadFilePath);
		}
		memcpy_s(wad_header.m_wadFilePath, sizeof(wad_header.m_wadFilePath), m_wadFileName.c_str() + nameOffset, 
			m_wadFileName.length() - nameOffset);
		wad_header.m_wadFilePathCrc32 = SIG_CalcCaseInsensitiveSignature(m_wadFileName.c_str());
		wad_header.m_version = WAD_VERSION;
		WAD_FILE_HEADER *lastAssets[(int)WAD_ASSET_TYPE::CNT] = {};
		WAD_FILE_HEADER *buckets[HASH_BUCKETS] = {}, *lastBuckets[HASH_BUCKETS] = {};
		uint64_t offset = sizeof(WAD_HEADER) + sizeof(buckets);
		for (auto i: m_list) {
			i.second->m_seqNo = seqNo++;
			i.second->m_nextFileSameHash  = nullptr;
			i.second->m_nextFileSameAsset = nullptr;
			i.second->m_crypted = false;
			int ty = (int)i.second->m_assetType;
			WAD_FILE_HEADER **ppAsset = wad_header.m_assets + ty;
			if (*ppAsset == nullptr) {
				*ppAsset = (WAD_FILE_HEADER *)offset;
			} else {
				lastAssets[ty]->m_nextFileSameAsset = (WAD_FILE_HEADER *)offset;
			}
			lastAssets[ty] = i.second;
			auto bucketIdx = ((ty | (32 * ty)) ^ i.second->m_nameICRC32) & (HASH_BUCKETS - 1);
			WAD_FILE_HEADER **ppBucket = buckets + bucketIdx;
			if (*ppBucket == nullptr) {
				*ppBucket = (WAD_FILE_HEADER *)offset;
			} else {
				lastBuckets[bucketIdx]->m_nextFileSameHash = (WAD_FILE_HEADER *)offset;
			}
			lastBuckets[bucketIdx] = i.second;
			offset += sizeof(WAD_FILE_HEADER) + i.second->m_fileLength;
		}
		wad_header.m_decompressed_size = (uint32_t)offset - sizeof(WAD_HEADER);
		auto compBufSize = offset + 7 + 7 * (offset / 256);
		uint8_t *decompBuf = (uint8_t *)calloc(1, offset), *compBuf = (uint8_t *)calloc(1, compBufSize);
		if (decompBuf && compBuf) {
			offset = sizeof(buckets);
			memcpy_s(decompBuf, offset, buckets, offset);
			for (auto i : m_list) {
				auto size = sizeof(WAD_FILE_HEADER);
				memcpy_s(decompBuf + offset, size, i.second, size);
				offset += size;
				memcpy_s(decompBuf + offset, i.second->m_fileLength, i.second->FirstChar(), i.second->m_fileLength);
				offset += i.second->m_fileLength;
			}
			wad_header.m_compressed_size = TJZIP_Compress(decompBuf, wad_header.m_decompressed_size, compBuf, &wad_header.m_crc32);
			/*FILE *ftmp = nullptr;
			fopen_s(&ftmp, "c:\\tmp\\compress.bin", "wb");
			fwrite(decompBuf, wad_header.m_decompressed_size, 1, ftmp);
			fclose(ftmp);*/
			assert(wad_header.m_compressed_size <= compBufSize);
			int ret = 0;
			if (sizeof(wad_header) != fwrite(&wad_header, 1, sizeof(wad_header), f))
				ret = E_EWRITE;
			else if (wad_header.m_compressed_size != fwrite(compBuf, 1, wad_header.m_compressed_size, f))
				ret = E_EWRITE;
			free(decompBuf);
			free(compBuf);
		} else {
			if (decompBuf) free(decompBuf);
			if (compBuf) free(compBuf);
			ret = E_NO_MEMORY;
		}
		fclose(f);
		return ret;
	}
};

int main(int argc, char **argv) {
	int ret = 0;
	switch(argc) {
	case 2:
		return WadUnpacker(argv[1]);
	case 0: case 1: //no args: read line-by-line from stdin
		std::cout << "wad_unpack v100966+ usage:\nwad_unpack.exe file.wad\nNow working from stdin\n";
		while(true) {
			std::cout << "Input WAD filePath: ";
			std::string fn;
			getline(std::cin, fn);
			if (fn.length() == 0) break;
			WadUnpacker next(fn.c_str());
			assert(0 == int(next));
			std::cout << "Result: " << int(next) << std::endl;
			if (ret == 0) ret = int(next);
#ifdef _DEBUG
#if 0 //test
			next.m_wadFileName = "c:\\Users\\build\\Downloads\\tmp\\assets" + next.m_wadFileName.substr(1 + strlen("c:\\Program Files(x86)\\Zwift\\assets"));
			next.Save();
			WadUnpacker check(next.m_wadFileName.c_str());
			assert(0 == int(check));
#endif
#endif
		}
		break;
	default: //too many args
		std::cout << "wad_unpack v100966+ usage:\nwad_unpack.exe file.wad\n";
		ret = -1;
		break;
	}
	return ret;
}
extern "C" {
/*OpenArchive should perform all necessary operations when an archive is to be opened.
OpenArchive should return a unique handle representing the archive. The handle should remain valid until CloseArchive is called. 
If an error occurs, you should return zero, and specify the error by setting OpenResult member of ArchiveData.
You can use the ArchiveData to query information about the archive being open, and store the information in ArchiveData to some location 
that can be accessed via the handle. */
	__declspec(dllexport) HANDLE __stdcall OpenArchive(tOpenArchiveData *ArchiveData) {
		auto ret = new WadUnpacker(ArchiveData->ArcName, false);
		if (ret->g_ret != 0) {
			delete ret;
			ArchiveData->OpenResult = E_BAD_ARCHIVE;
			return 0;
		}
		return ret;
	}
/*Totalcmd calls ReadHeader to find out what files are in the archive.
ReadHeader is called as long as it returns zero (as long as the previous call to this function returned zero). Each time it is called, 
HeaderData is supposed to provide Totalcmd with information about the next file contained in the archive. When all files in the archive 
have been returned, ReadHeader should return E_END_ARCHIVE which will prevent ReaderHeader from being called again. If an error occurs, 
ReadHeader should return one of the error values or 0 for no error.
hArcData contains the handle returned by OpenArchive. The programmer is encouraged to store other information in the location that can 
be accessed via this handle. For example, you may want to store the position in the archive when returning files information in ReadHeader.
In short, you are supposed to set at least PackSize, UnpSize, FileTime, and FileName members of tHeaderData. Totalcmd will use this 
information to display content of the archive when the archive is viewed as a directory.*/
	__declspec(dllexport) int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData) {
		auto obj = (WadUnpacker *)hArcData;
		return (hArcData != INVALID_HANDLE_VALUE && obj && obj->ReadHeader(HeaderData)) ? 0 : E_END_ARCHIVE;
	}
/*ProcessFile should unpack the specified file or test the integrity of the archive.
ProcessFile should return zero on success, or one of the error values otherwise.
hArcData contains the handle previously returned by you in OpenArchive. Using this, you should be able to find out information (such as 
the archive filename) that you need for extracting files from the archive.
Unlike PackFiles, ProcessFile is passed only one filename. Either DestName contains the full path and file name and DestPath is NULL, or 
DestName contains only the file name and DestPath the file path. This is done for compatibility with unrar.dll.
When Total Commander first opens an archive, it scans all file names with OpenMode==PK_OM_LIST, so ReadHeader() is called in a loop with 
calling ProcessFile(...,PK_SKIP,...). When the user has selected some files and started to decompress them, Total Commander again calls 
ReadHeader() in a loop. For each file which is to be extracted, Total Commander calls ProcessFile() with Operation==PK_EXTRACT immediately 
after the ReadHeader() call for this file. If the file needs to be skipped, it calls it with Operation==PK_SKIP.
Each time DestName is set to contain the filename to be extracted, tested, or skipped. To find out what operation out of these last three 
you should apply to the current file within the archive, Operation is set to one of the following:
Constant Value Description
PK_SKIP 0 Skip this file
PK_TEST 1 Test file integrity
PK_EXTRACT 2 Extract to disk */
	__declspec(dllexport) int __stdcall ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName) {
		auto obj = (WadUnpacker *)hArcData;
		if (hArcData == INVALID_HANDLE_VALUE || obj == nullptr || obj->m_curPosition == obj->m_list.cend()) return E_NO_FILES;
		if (Operation == PK_EXTRACT) {
			char fullPath[260] = {};
			if (DestPath)
				strcpy_s(fullPath, DestPath);
			size_t len = strnlen_s(fullPath, 259);
			if (DestName) {
				if (len > 1 && fullPath[len - 1] != '\\')
					fullPath[len - 1] = '\\';
				strcat_s(fullPath, DestName);
			}
			FILE *f = nullptr;
			fopen_s(&f, fullPath, "wb");
			if (f == nullptr) {
				obj->m_curPosition++;
				return E_ECREATE;
			}
			auto pf = obj->m_curPosition->second;
			//TODO: support slow media and write by chunks with m_wcxProcess call
			if (pf->m_fileLength != fwrite(pf->FirstChar(), 1, pf->m_fileLength, f)) {
				fclose(f);
				obj->m_curPosition++;
				return E_EWRITE;
			}
			fclose(f);
		}
		obj->m_curPosition++;
		return 0;
	}
/*CloseArchive should perform all necessary operations when an archive is about to be closed.
CloseArchive should return zero on success, or one of the error values otherwise. It should free all the resources associated with the 
open archive. The parameter hArcData refers to the value returned by a programmer within a previous call to OpenArchive.*/
	__declspec(dllexport) int __stdcall CloseArchive(HANDLE hArcData) {
		auto obj = (WadUnpacker *)hArcData;
		if (hArcData != INVALID_HANDLE_VALUE && obj) delete obj;
		return 0;
	}
	__declspec(dllexport) void __stdcall SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1) {}
	__declspec(dllexport) void __stdcall SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc) {
		auto obj = (WadUnpacker *)hArcData;
		if (hArcData != INVALID_HANDLE_VALUE && obj) obj->m_wcxProcess = pProcessDataProc;
	}
/*GetBackgroundFlags is called to determine whether a plugin supports background packing or unpacking.
GetBackgroundFlags should return one of the following values:
Constant               Value Description
BACKGROUND_UNPACK        1 Calls to OpenArchive, ReadHeader(Ex), ProcessFile and CloseArchive are thread-safe (unpack in background)
BACKGROUND_PACK          2 Calls to PackFiles are thread-safe (pack in background)
BACKGROUND_MEMPACK       4 Calls to StartMemPack, PackToMem and DoneMemPack are thread-safe
To make your packer plugin thread-safe, you should remove any global variables which aren't the same for all pack or unpack operations.
For example, the path to the ini file name can remain global, but something like the compression ratio, or file handles need to be stored
separately. 
Packing: The PackFiles function is just a single call, so you can store all variables on the stack (local variables of that function).
Unpacking: You can allocate a struct containing all the variables you need across function calls, like the compression method and ratio,
and state variables, and return a pointer to this struct as a result to OpenArchive. This pointer will then passed to all other functions
like ReadHeader as parameter hArcData. 
Pack in memory: You can do the same in StartMemPack as described under Unpacking.*/
	__declspec(dllexport) int __stdcall GetBackgroundFlags() { return BACKGROUND_UNPACK | BACKGROUND_PACK | BACKGROUND_MEMPACK; }
/*GetPackerCaps tells Totalcmd what features your packer plugin supports.
Implement GetPackerCaps to return a combination of the following values:
Constant                Value Description
PK_CAPS_NEW               1 Can create new archives
PK_CAPS_MODIFY            2 Can modify existing archives
PK_CAPS_MULTIPLE          4 Archive can contain multiple files
PK_CAPS_DELETE            8 Can delete files
PK_CAPS_OPTIONS          16 Has options dialog
PK_CAPS_MEMPACK          32 Supports packing in memory
PK_CAPS_BY_CONTENT       64 Detect archive type by content
PK_CAPS_SEARCHTEXT      128 Allow searching for text in archives created with this plugin
PK_CAPS_HIDE            256 Don't show packer icon, don't open with Enter but with Ctrl+PgDn
PK_CAPS_ENCRYPT         512 Plugin supports encryption.
Omitting PK_CAPS_NEW and PK_CAPS_MODIFY means PackFiles will never be called and so you don't have to implement PackFiles. Omitting 
PK_CAPS_MULTIPLE means PackFiles will be supplied with just one file. Leaving out PK_CAPS_DELETE means DeleteFiles will never be called;
leaving out PK_CAPS_OPTIONS means ConfigurePacker will not be called. PK_CAPS_MEMPACK enables the functions StartMemPack, PackToMem and
DoneMemPack. If PK_CAPS_BY_CONTENT is returned, Totalcmd calls the function CanYouHandleThisFile when the user presses Ctrl+PageDown on an
unknown archive type. Finally, if PK_CAPS_SEARCHTEXT is returned, Total Commander will search for text inside files packed with this
plugin. This may not be a good idea for certain plugins like the diskdir plugin, where file contents may not be available. If PK_CAPS_HIDE
is set, the plugin will not show the file type as a packer. This is useful for plugins which are mainly used for creating files, e.g. to
create batch files, avi files etc. The file needs to be opened with Ctrl+PgDn in this case, because Enter will launch the associated
application.
Important note:
If you change the return values of this function, e.g. add packing support, you need to reinstall the packer plugin in Total Commander,
otherwise it will not detect the new capabilities.*/
	__declspec(dllexport) int __stdcall GetPackerCaps() { return PK_CAPS_BY_CONTENT | PK_CAPS_SEARCHTEXT | PK_CAPS_NEW |
		PK_CAPS_MODIFY | PK_CAPS_MULTIPLE | PK_CAPS_DELETE; }
	__declspec(dllexport) BOOL __stdcall CanYouHandleThisFile(char *FileName) {
		FILE *f = nullptr;
		fopen_s(&f, FileName, "rb");
		if (f) {
			char buf[4] = {};
			fread_s(buf, sizeof(buf), 1, sizeof(buf), f);
			fclose(f);
			return buf[0] == 'Z' && buf[1] == 'W' && buf[2] == 'F' && buf[3] == '!';
		}
		return 0;
	}
/*PackFiles specifies what should happen when a user creates, or adds files to the archive.
PackFiles should return zero on success, or one of the error codes otherwise.
PackedFile refers to the archive that is to be created or modified. The string contains the full path.
SubPath is either NULL, when the files should be packed with the paths given with the file names, or not NULL when they should be placed 
below the given subdirectory within the archive. Example:
    SubPath="subdirectory"
    Name in AddList="subdir2\filename.ext"
-> File should be packed as "subdirectory\subdir2\filename.ext"
SrcPath contains path to the files in AddList. SrcPath and AddList together specify files that are to be packed into PackedFile. Each 
string in AddList is zero-delimited (ends in zero), and the AddList string ends with an extra zero byte, i.e. there are two zero bytes 
at the end of AddList.
Flags can contain a combination of the following values reflecting the user choice from within Totalcmd:
Constant                 Value Description
PK_PACK_MOVE_FILES         1 Delete original after packing
PK_PACK_SAVE_PATHS         2 Save path names of files
PK_PACK_ENCRYPT            4 Ask user for password, then encrypt file with that password */
	__declspec(dllexport) int __stdcall PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags) {
		WadUnpacker exist(PackedFile, false);
		exist.Close();
		FILE *f = nullptr;
		auto err = fopen_s(&f, PackedFile, "a");
		if (f) fclose(f);
		if (err != 0 || f == nullptr) {
			return E_EWRITE;
		}
		return exist.PackFiles(SubPath, SrcPath, AddList, Flags);
	}
/*DeleteFiles should delete the specified files from the archive
DeleteFiles should return zero on success, or one of the error codes otherwise.
PackedFile contains full path and name of the the archive.
DeleteList contains the list of files that should be deleted from the archive. The format of this string is the same as AddList
within PackFiles.*/
	__declspec(dllexport) int __stdcall DeleteFiles(char *PackedFile, char *DeleteList) {
		WadUnpacker ret(PackedFile, false);
		if (ret.g_ret != 0) {
			return E_BAD_ARCHIVE;
		}
		return ret.DeleteFiles(DeleteList);
	}
}
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}