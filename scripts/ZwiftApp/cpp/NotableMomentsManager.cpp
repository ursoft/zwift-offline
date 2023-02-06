#include "ZwiftApp.h"
NotableMomentsManager::NotableMomentsManager() {
    //TODO
}
void NotableMomentsManager::Init() {
    //TODO
    if (!g_pNotableMomentsMgrInited) {
        /*v90 = g_pNotableMomentsMgr + 288;
        v91 = *(_QWORD *)(g_pNotableMomentsMgr + 288);
        if (*(_QWORD *)(g_pNotableMomentsMgr + 296) == 0xAAAAAAAAAAAAAAAi64)
            Xlength_error("list too long");
        coa1.vptr = (void *)(g_pNotableMomentsMgr + 288);
        coa1.m_countPtr = 0i64;
        v92 = operator new(0x18ui64);
        v92[2] = LvlUp::onHandleLevelUpMoment;
        ++ *(_QWORD *)(v90 + 8);
        v93 = *(_QWORD **)(v91 + 8);
        *v92 = v91;
        v92[1] = v93;
        coa1.m_countPtr = 0i64;
        *(_QWORD *)(v91 + 8) = v92;
        *v93 = v92;*/
        g_pNotableMomentsMgrInited = true;
    }
}

/*    bool IsMomentNotableEnoughForGame(const NotableMoment &);
    bool IsMomentNotableEnoughForServer(const NotableMomentType &);
    void OnNotableMoment_OnEntitlementGranted(VEC3, int);
    void OnNotableMoment(NOTABLEMOMENT_TYPE, VEC3, uint64_t, uint64_t, double);
    void OnNotableMoment_NewPR(VEC3, int, const char *, float, float, void *);
    void OnNotableMoment_AchievementUnlocked(VEC3, int, const char *, const char *, void *);
    void IsMomentNotableEnoughForServer(const NotableMoment &);
    void OnNotableMoment_WorkoutComplete(VEC3, const char *, uint, int, int);
    void OnNotableMoment_LevelUp(VEC3, int, int);
    void OnNotableMoment_GotJersey(VEC3, uint, const char *, int, void *);
    void OnNotableMoment(NOTABLEMOMENT_TYPE, VEC3, uint64_t, uint64_t, double);
    void OnNotableMoment_NewPR(VEC3, int, const char *, float, float, void *);
    void OnNotableMoment_AchievementUnlocked(VEC3, int, const char *, const char *, void *);
    void OnNotableMoment_LevelUp(VEC3, int, int);
    void OnNotableMoment_WorkoutComplete(VEC3, const char *, uint, int, int);
    void OnNotableMoment_OnEntitlementGranted(VEC3, int);
    void OnNotableMoment_GotJersey(VEC3, uint, const char *, int, void *);
    void IsMomentNotableEnoughForGame(const NotableMoment &);
    bool IsMomentNotableEnoughForServer(const NotableMoment &);
    bool IsMomentNotableEnoughForServer(const NotableMomentType &);
*/
bool g_pNotableMomentsMgrInited;
NotableMomentsManager g_pNotableMomentsMgr;