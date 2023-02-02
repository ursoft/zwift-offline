#pragma once
class VirtualBikeComputer {
public:
    float GetDistance(UnitType ut, bool) { /*TODO*/ return 0.0; }
    protobuf::Sport m_sport = protobuf::CYCLING;
};
enum BikeEntityChangeFlags { BCH_UNITS = 0x400000 };
class BikeEntity {
public:
    int64_t m_playerId;
    VirtualBikeComputer *m_bc;
    DWORD m_changeFlags;
    bool m_writable;
    bool m_metricUnits;
    protobuf::PlayerProfile m_profile;
    void SaveProfile(bool, bool);
};