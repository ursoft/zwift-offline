#pragma once
using ZSPORT = protobuf::Sport;
class VirtualBikeComputer {
public:
    float GetDistance(UnitType ut, bool) { /*TODO*/ return 0.0; }
    void SetTireSize(uint32_t tireCirc) { m_tireCirc = tireCirc; }
    uint64_t m_lastPower = 0;
    uint32_t m_tireCirc = 2105, m_powerSmoothing = 1, m_field_118 = 0, m_field_128 = 0;
    float m_distance = 0.0f, m_field_178 = 0.0f, m_field_19C = 0.0f, m_field_188 = 0.0f, m_field_18C = 0.0f, m_field_198 = 0.0f, m_field_1E0 = 0.0f, m_sensor_f2 = 0.0f, m_total_smth = 0.0f, 
        m_heart_f2 = 0.0f;
    protobuf::Sport m_sport = protobuf::CYCLING;
    bool m_bool = false, m_bool1 = false;
};
enum BikeEntityChangeFlags0 { BCH0_JERSEY = 0x8000000 };
enum BikeEntityChangeFlags1 { BCH1_UNITS = 0x400000 };
struct Entity {
    VEC3 m_pos{};
    virtual const VEC3 &GetPosition() { return m_pos; }
};
class BikeEntity : public Entity {
public:
    BikeEntity();
    int64_t m_playerId = 0, m_curEventId = 0, m_field_14F0 = 0;
    VirtualBikeComputer *m_bc = nullptr;
    protobuf::PlayerProfile m_profile;
    uint32_t m_race_f14 = 0, m_changeFlags0, m_changeFlags1, m_fwGdeSignature = 0, m_rwGdeSignature = 0, m_jersey = 0, m_yellowJersey = 0;
    protobuf::PlayerType m_playerType = protobuf::PlayerType::NORMAL;
    bool m_writable = false, m_metricUnits = true, m_field_C98 = false, m_race_f15 = false, m_race_f16 = false, m_sensor_f11 = false;
    void SaveProfile(bool, bool);
    bool IsPacerBot() { return m_playerType == protobuf::PlayerType::PACER_BOT; }
    int64_t GetEventID();
};