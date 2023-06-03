#include "ZwiftApp.h"
void ProfanityFilter::LoadProfanityList() {
    //TODO
}
bool ProfanityFilter::PlayerOldEnoughToMessage() {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    int d, m, y;
    if (!mainBike || !mainBike->m_profile.has_dob() || 3 != sscanf_s(mainBike->m_profile.dob().c_str(), "%d/%d/%d", &m, &d, &y))
        return true;
    auto sdt = SaveGame::GetSystemDateTime();
    int mon = uint8_t(sdt >> 32);
    int day = uint8_t(sdt >> 40);
    auto age = uint32_t(sdt) - y - 1;
    if (mon > m || (mon == m && day >= d))
        age++;
    return age <= 0 || age >= 13;
}
