#pragma once
class RaceDictionary {
public:
    static RaceDictionary *Init();
    bool LoadFromData();
};
inline RaceDictionary *g_RaceDictionary;
