#include "ZwiftApp.h"
GenericTrackingData SaveGame::GetTrackingData(uint32_t hash, const GenericTrackingData &def) {
    auto fnd = m_trackingMap.find(hash);
    if (fnd == m_trackingMap.end())
        return def;
    return fnd->second;
}
uint64_t SaveGame::GetSystemDateTime() {
    auto Time = _time64(nullptr);
    auto v0 = _localtime64(&Time);
    return (v0->tm_year + 1900) | (uint64_t((uint8_t(v0->tm_mon) + 1) | ((uint8_t(v0->tm_mday) | ((v0->tm_min + 100 * v0->tm_hour) << 8)) << 8)) << 32);
}
void SaveGame::SetTrackingData(uint32_t hash, const GenericTrackingData &data) {
    m_trackingMap[hash] = data;
}