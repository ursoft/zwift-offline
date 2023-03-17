#include "ZwiftApp.h"
bool g_ICU_open_fail_logged, g_ToUTF8_NullFail_logged, g_ToUTF8_TruncFail_logged, g_ToUTF8_Conv_fail_logged, g_FromUTF8_NullFail_logged, g_FromUTF8_Conv_fail_logged;
UConverter *g_utfConverter;
bool InitICUBase() {
    UErrorCode uc_err = U_ZERO_ERROR;
    u_init(&uc_err);
    g_utfConverter = ucnv_open("UTF8", &uc_err);
    if (uc_err <= 0)
        return true;
    if (!g_ICU_open_fail_logged) {
        g_ICU_open_fail_logged = 1;
        LogTyped(LOG_ERROR, "UTF8--Failed to initialize ICU");
    }
    return false;
}
UChar *ToUTF8(const char *src, UChar *dest, size_t length) {
    if (!src) {
        if (!g_ToUTF8_NullFail_logged) {
            g_ToUTF8_NullFail_logged = 1;
            LogTyped(LOG_ERROR, "ToUTF8--Passing null as the source text");
        }
        return nullptr;
    }
    memset(dest, 0, 2 * length);
    auto src_len = strlen(src);
    if (src_len >= length) {
        if (!g_ToUTF8_TruncFail_logged) {
            g_ToUTF8_TruncFail_logged = 1;
            LogTyped(LOG_WARNING, "ToUTF8--Truncation, %lld >= %lld, pText=%s", src_len, length, src);
        }
        src_len = length - 1;
    }
    UErrorCode uc_err = U_ZERO_ERROR;
    ucnv_toUChars(g_utfConverter, dest, length, src, src_len, &uc_err);
    if (uc_err > 0) {
        if (!g_ToUTF8_Conv_fail_logged) {
            g_ToUTF8_Conv_fail_logged = 1;
            LogTyped(LOG_ERROR, "ToUTF8--Conversion failed, pText=%s", src);
        }
        return nullptr;
    }
    return dest;
}
char *FromUTF8(UChar *src, char *dest, size_t length) {
    if (!src) {
        if (!g_FromUTF8_NullFail_logged) {
            g_FromUTF8_NullFail_logged = 1;
            LogTyped(LOG_ERROR, "FROM-UTF8--Passing null as the source text.");
        }
        return nullptr;
    }
    memset(dest, 0, length);
    UErrorCode uc_err = U_ZERO_ERROR;
    ucnv_fromUChars(g_utfConverter, dest, length, src, u_strlen(src), &uc_err);
    if (uc_err > 0) {
        if (!g_FromUTF8_Conv_fail_logged) {
            g_FromUTF8_Conv_fail_logged = 1;
            LogTyped(LOG_ERROR, "FROM-UTF8--Conversion failed, pText=%s", src);
        }
        return nullptr;
    }
    return dest;
}
static UChar g_UtfBuffer[46000];
UChar *ToUTF8_ib(const char *src) { return ToUTF8(src, g_UtfBuffer, _countof(g_UtfBuffer)); }