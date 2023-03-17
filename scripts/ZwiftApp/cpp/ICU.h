#pragma once
bool InitICUBase();
UChar *ToUTF8(const char *src, UChar *dest, size_t length);
UChar *ToUTF8_ib(const char *src);
char *FromUTF8(UChar *src, char *dest, size_t length);