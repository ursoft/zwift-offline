#pragma once
uint32_t SIG_CalcCaseInsensitiveSignature(const char *str);
uint32_t SIG_CalcCaseSensitiveSignature(const char *str);
extern const uint32_t g_crc32Table[];
namespace CCRC32 {
uint32_t FileCRC(const char *);
bool FileCRC(const char *, uint32_t *, uint64_t);
void FullCRC(const uint8_t *, uint64_t, uint32_t *);
uint32_t FullCRC(const uint8_t *buf, uint64_t sz);
};
