#pragma once
class NotableMoment {};
class NotableMomentType {};
enum NOTABLEMOMENT_TYPE { NMT_CNT };
class NotableMomentsManager { //984 bytes
public:
    NotableMomentsManager();
    static void Init();

    bool IsMomentNotableEnoughForServer(const NotableMoment &);
    bool IsMomentNotableEnoughForServer(const NotableMomentType &);
    bool IsMomentNotableEnoughForGame(const NotableMoment &);

    void OnNotableMoment_OnEntitlementGranted(VEC3, int);
    void OnNotableMoment(NOTABLEMOMENT_TYPE, VEC3, uint64_t, uint64_t, double);
    void OnNotableMoment_NewPR(VEC3, int, const char *, float, float, void *);
    void OnNotableMoment_AchievementUnlocked(VEC3, int, const char *, const char *, void *);
    void OnNotableMoment_WorkoutComplete(VEC3, const char *, uint32_t, int, int);
    void OnNotableMoment_LevelUp(VEC3, int, int);
    void OnNotableMoment_GotJersey(VEC3, uint32_t, const char *, int, void *);
};
inline bool g_pNotableMomentsMgrInited;
inline NotableMomentsManager g_pNotableMomentsMgr;
