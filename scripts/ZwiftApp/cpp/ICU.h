#pragma once
bool InitICUBase();
UChar *ToUTF8(const char *src, UChar *dest, size_t length);
char *FromUTF8(UChar *src, char *dest, size_t length);