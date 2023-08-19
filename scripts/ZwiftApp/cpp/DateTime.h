#pragma once  //READY for testing
struct DateTime {
    time_t m_value = 0;
    static DateTime fromIso8601StringUtc(const std::string &std);
    std::string toIso8601String() const;
};
