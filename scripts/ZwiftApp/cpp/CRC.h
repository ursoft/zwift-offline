#pragma once

uint32_t SIG_CalcCaseInsensitiveSignature(const char *str);
extern const uint32_t g_crc32Table[];
class CCRC32 {
public:
    static uint32_t FileCRC(const char *);
    static bool FileCRC(const char *, uint32_t *, uint64_t);
    static void FullCRC(const uint8_t *, uint64_t, uint32_t *);
    static uint32_t FullCRC(const uint8_t *buf, uint64_t sz);
};
