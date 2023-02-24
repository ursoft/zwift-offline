#pragma once
enum GameHolidayType { GHT_CNT };
enum HolidayFeatureType { HFT_CNT };
class GameHolidayManager { //? bytes
public:
    static GameHolidayManager *Instance();
    bool IsHoliday(GameHolidayType, bool);
    void GetAprilFoolsEventXpMultiplier();
    void GetCurrentHoliday(bool);
    void GetHoliday(int64_t);
    void GetHolidayTypeFromName(const char *);
    bool IsAprilFools(time_t);
    bool IsHalloween(time_t);
    bool IsHoliday(const char *, bool);
    bool IsHoliday();
    bool IsHolidayFeatureEnabled(HolidayFeatureType);
    bool IsNewYears(time_t);
    bool IsThanksgiving(time_t);
    bool IsXmas(time_t);
    void SetupCurrentHoliday(int64_t);
    ~GameHolidayManager();
};
inline GameHolidayManager g_GameHolidayManager;