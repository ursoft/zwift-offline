#include "ZwiftApp.h"
GenericTrackingData SaveGame::GetTrackingData(uint32_t hash, const GenericTrackingData &def) {
    auto fnd = m_trackingMap.find(hash);
    if (fnd == m_trackingMap.end())
        return def;
    return fnd->second;
}
void SaveGame::SetTrackingData(uint32_t hash, const GenericTrackingData &data) {
    m_trackingMap[hash] = data;
}