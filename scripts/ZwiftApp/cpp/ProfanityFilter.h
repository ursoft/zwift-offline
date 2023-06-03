#pragma once
struct ProfanityFilter {
    static void LoadProfanityList();
    static bool PlayerOldEnoughToMessage();
    /*CheckMessageForProfaneWords(uint16_t *, uint16_t *, uint64_t, bool);
    PlayerAgeInYears();
    ProfanityFilter();
    ~ProfanityFilter();*/
};