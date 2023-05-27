#pragma once
struct RaceDictionary {
    static RaceDictionary *Init();
    bool LoadFromData();
};
inline RaceDictionary *g_RaceDictionary;
