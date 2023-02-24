#include "ZwiftApp.h"
GameHolidayManager *GameHolidayManager::Instance() { return &g_GameHolidayManager; }
bool GameHolidayManager::IsHoliday(GameHolidayType, bool) {
    //TODO
    return true;
}
void GameHolidayManager::GetAprilFoolsEventXpMultiplier() {
    //TODO
}
void GameHolidayManager::GetCurrentHoliday(bool) {
    //TODO
}
void GameHolidayManager::GetHoliday(int64_t) {
    //TODO
}
void GameHolidayManager::GetHolidayTypeFromName(const char *) {
    //TODO
}
bool GameHolidayManager::IsAprilFools(time_t) {
    //TODO
    return true;
}
bool GameHolidayManager::IsHalloween(time_t) {
    //TODO
    return true;
}
bool GameHolidayManager::IsHoliday(const char *, bool) {
    //TODO
    return true;
}
bool GameHolidayManager::IsHoliday() {
    //TODO
    return true;
}
bool GameHolidayManager::IsHolidayFeatureEnabled(HolidayFeatureType) {
    //TODO
    return true;
}
bool GameHolidayManager::IsNewYears(time_t) {
    //TODO
    return true;
}
bool GameHolidayManager::IsThanksgiving(time_t) {
    //TODO
    return true;
}
bool GameHolidayManager::IsXmas(time_t) {
    //TODO
    return true;
}
void GameHolidayManager::SetupCurrentHoliday(int64_t) {
    //TODO
}
GameHolidayManager::~GameHolidayManager() {
    //TODO
}
