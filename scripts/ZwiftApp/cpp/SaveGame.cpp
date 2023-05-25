#include "ZwiftApp.h"

GenericTrackingData SaveGame::GetTrackingData(uint32_t hash, GenericTrackingData def) {
    auto fnd = m_trackingMap.find(hash);
    if (fnd == m_trackingMap.end())
        return def;
    return fnd->second;
}
