#include "ZwiftApp.h"
static const char *fmt8601 = "%Y-%m-%dT%H:%M:%SZ";
DateTime DateTime::fromIso8601StringUtc(const std::string &sdt) {
    int Y, m, d, H = 0, M = 0, S = 0;
    auto sc = sscanf_s(sdt.c_str(), "%d-%d-%dT%d:%d:%d", &Y, &m, &d, &H, &M, &S);
    if ((sc != 6 && sc != 3) || m > 12 || m < 1 || d < 1 || d > 31 || H < 0 || H > 23 || M < 0 || M > 59 || S < 0 || S > 59) {
        NetworkingLogError("Invalid parameter on DateTime::fromIso8601String: %s", sdt.c_str());
        return DateTime();
    }
    tm tm_{};
    tm_.tm_year = Y;
    tm_.tm_mon = m - 1;
    tm_.tm_mday = d;
    tm_.tm_hour = H;
    tm_.tm_min = M;
    tm_.tm_sec = S;
    tm_.tm_isdst = -1;
    return DateTime{ _mktime64(&tm_) };
}
std::string DateTime::toIso8601String() const {
    tm tm_{};
    if (m_value == 0 || gmtime_s(&tm_, &m_value)) {
        NetworkingLogError("Invalid time on DateTime::GMT_TIME %lld", m_value);
        tm_.tm_year = 70;
        tm_.tm_mday = 13;
        tm_.tm_hour = 14;
        tm_.tm_min = 15;
        tm_.tm_sec = 16;
    }
    char buf[32];
    if (strftime(buf, sizeof(buf), fmt8601, &tm_))
        return buf;
    NetworkingLogError("Invalid parameter on DateTime::toString: %lld %s", m_value, fmt8601);
    return "1970-01-13T14:15:16.000000+0000"s;
}
