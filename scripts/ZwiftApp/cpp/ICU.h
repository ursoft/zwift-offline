#pragma once //READY for testing
struct TempUTF16 {
    UChar m_chars[256];
    int m_wrCounter = 0;
    TempUTF16 *GetWritable();
};
bool InitICUBase();
UChar *ToUTF8(const char *src, UChar *dest, size_t length);
UChar *ToUTF8_ib(const char *src);
char *FromUTF8(const UChar *src, char *dest, size_t length);
char *SafeFromUTF8(const UChar *src, TempUTF16 *a2);