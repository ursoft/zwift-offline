#include "ZwiftApp.h"
std::unique_ptr<PlayerAchievementService> g_sPlayerAchievementServicePtr;
void PlayerAchievementService::Initialize(EventSystem *ev) {
	zassert(g_sPlayerAchievementServicePtr.get() == nullptr);
	g_sPlayerAchievementServicePtr.reset(new PlayerAchievementService(ev));
}
PlayerAchievementService *PlayerAchievementService::Instance() {
	zassert(g_sPlayerAchievementServicePtr.get() != nullptr);
	return g_sPlayerAchievementServicePtr.get();
}
bool PlayerAchievementService::IsInitialized() {
	return !!g_sPlayerAchievementServicePtr.get();
}
PlayerAchievementService::PlayerAchievementService(EventSystem *ev) : EventObject(ev) {
    m_repeatPeriod = 15;
	ev->Subscribe(EV_RESET, this);
    m_lastPeriodEnd = timeGetTime();
    //TODO
}
void PlayerAchievementService::HandleEvent(EVENT_ID e, va_list va) {
	if (e == EV_RESET) {
        if (m_field18) {
            m_field18 = nullptr;
            memset(m_bitField, 0, sizeof(m_bitField));
            m_stateLoad = m_stateSave = SLS_INITIAL;
        }
        /* TODO v4 = *(int *)(a3 + 24);
        if ((v4 & 0x80000000) != 0 && (*(_DWORD *)(a3 + 24) = v4 + 8, (int)v4 + 8 <= 0)) {
            v5 = (void **)(*(_QWORD *)(a3 + 8) + v4);
        } else {
            v5 = *(void ***)a3;
            *(_QWORD *)a3 += 8LL;
        }
        m_field18 = va_arg(va, void *);*/
        LoadAchievements();
        LoadRideHistory();
    }
}
void PlayerAchievementService::HandleLogout() {
    m_field18 = nullptr;
    memset(m_bitField, 0, sizeof(m_bitField));
    m_stateLoad = m_stateSave = SLS_INITIAL;
}
void PlayerAchievementService::LoadAchievements() {
    if (m_stateLoad == SLS_FAILED) {
        m_stateLoad = SLS_INPROCESS;
        //TODO
    }
}
void PlayerAchievementService::LoadAchievementsFailure(ZNet::Error, void *a3) {
    if (a3 == m_field18) {
        //TODO
        m_stateLoad = SLS_FAILED;
        //TODO
    }
}
void PlayerAchievementService::LoadAchievementsSuccess(const protobuf::Achievements &pb, void *a3) {
    if (a3 == m_field18) {
        m_stateLoad = SLS_DONE;
        //TODO
    }
}
void PlayerAchievementService::LoadRideHistory() {
	//TODO
}
void PlayerAchievementService::PersistAchievements() {
    auto data = listPlayerAchievements();
    if (data.size()) {
        m_stateSave = SLS_INPROCESS;
	    //TODO
    }
}
void PlayerAchievementService::PersistAchievementsFailure(std::vector<int>, ZNet::Error) {
    m_stateSave = SLS_FAILED;
    //TODO
}
void PlayerAchievementService::PersistAchievementsSuccess() {
    m_stateSave = SLS_DONE;
}
void PlayerAchievementService::SetAchievement(Achievement a) {
    m_bitField[a / 8] |= (1 << (a & 7));
    m_changeFlag = true;
}
void PlayerAchievementService::Shutdown() {
    //not sure
    g_sPlayerAchievementServicePtr.reset(nullptr);
}
void PlayerAchievementService::Update(float t) {
    if (m_field18) {
        if (m_stateLoad == SLS_INITIAL)
            PlayerAchievementService::LoadAchievements();
        if (m_changeFlag && m_stateLoad == SLS_DONE) {
            if (m_stateSave == SLS_INPROCESS)
                return;
            PlayerAchievementService::PersistAchievements();
            m_changeFlag = false;
        }
        if (m_stateLoad == SLS_FAILED || m_stateSave == SLS_FAILED) {
            auto now_t = timeGetTime();
            if (now_t - m_lastPeriodEnd > 1000 * m_repeatPeriod) {
                if (m_stateLoad == SLS_FAILED)
                    PlayerAchievementService::LoadAchievements();
                if (m_stateSave == SLS_FAILED)
                    PlayerAchievementService::PersistAchievements();
                m_lastPeriodEnd = now_t;
            }
        }
    }
}
std::vector<int> PlayerAchievementService::listPlayerAchievements() {
    std::vector<int> ret(ACH_CNT);
    for (int i = 0; i < ACH_CNT; i++) {
        if (HasAchievement((Achievement)i))
            ret.push_back(i);
    }
    return ret;
}
PlayerAchievementService::~PlayerAchievementService() {
	m_eventSystem->Unsubscribe(EV_RESET, this);
	/*TODO
  v5[0] = (__int64)"PlayerAchievementService";
  v5[1] = 24i64;
  sub_7FF658A17260((__int128 ***)&this->field_28, (__int64)v5);
  sub_7FF658A16620((__int64 *)&this->field_98);
  sub_7FF658A17F60((__int64)&this->field_88, (__int64)&this->field_88, *(__int64 **)(*(_QWORD *)&this->field_88 + 8i64));
  j_j_free(*(_QWORD *)&this->field_88, 64i64);
  sub_7FF658A13A40((__int64 *)&this->field_28);
*/
}
