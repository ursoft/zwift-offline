#pragma once
class VirtualBikeComputer {
public:
    float GetDistance(UnitType ut, bool) { /*TODO*/ return 0.0; }
    void SetTireSize(uint32_t tireCirc) { m_tireCirc = tireCirc; }
    protobuf::Sport m_sport = protobuf::CYCLING;
    uint32_t m_tireCirc = 2105, m_powerSmoothing = 1;;
};
enum BikeEntityChangeFlags { BCH_UNITS = 0x400000 };
class BikeEntity {
public:
    int64_t m_playerId = 0;
    VirtualBikeComputer *m_bc = nullptr;
    DWORD m_changeFlags, m_fwGdeSignature = 0, m_rwGdeSignature = 0;
    bool m_writable = false;
    bool m_metricUnits = true;
    protobuf::PlayerProfile m_profile;
    void SaveProfile(bool, bool);
};