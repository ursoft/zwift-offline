#pragma once
enum BikeEntityChangeFlags { BCH_UNITS = 0x400000 };

class BikeEntity {
public:
    DWORD m_changeFlags;
    bool m_writable;
    bool m_metricUnits;
    void SaveProfile(bool, bool);
};