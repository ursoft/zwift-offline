#include "ZwiftApp.h"
GoalsManager::GoalsManager(EventSystem *ev) : EventObject(ev) {
    ev->Subscribe(EV_RESET, this);
    //TODO - simple field ctrs?
}
void GoalsManager::HandleEvent(EVENT_ID e, va_list va) { if (e == EV_RESET) Load(); }
void GoalsManager::AddPersonalGoal(const protobuf::Goal &g) {
    if (!m_hasPersonalGoal) {
        ++g_nGoalsAdded;
        zwift_network::save_goal(g);
        //TODO 
        /* android v3 = *(unsigned __int64 **)&this->field_8;
        *(_QWORD *)&this->field_8 = v6;
        if (v3) {
            v4 = v3 + 1;
            do
                v5 = __ldaxr(v4);
            while (__stlxr(v5 - 1, v4));
            if (!v5)
                (*(void(__fastcall **)(unsigned __int64 *))(*v3 + 16))(v3);
        }*/
        m_hasPersonalGoal = true;
    }
}
void GoalsManager::ConvertOldGoals() {
    //TODO - deprecated
}
void GoalsManager::FinishLoad(const NetworkResponse<protobuf::Goals> &resp) {
    m_goals = resp;
}
float GoalsManager::GetActiveGoalCompletion(protobuf::GoalType gt) {
    float ret = 0.0f;
    auto g = GetActivePersonalGoalOfType(gt);
    if (g) {
        auto bc = BikeManager::Instance()->m_mainBike->m_bc;
        if (gt == protobuf::TIME) {
            //TODO float x = (float)((float)(*(_QWORD *)(v5 + 296) = current_duration ?
            float x = 0.0;
            ret = (g->actual_duration() + x / 1000.0 / 60.0) / g->target_distance(); //TODO: fix wrong name 'target_distance'
        } else if (gt == protobuf::DISTANCE) {
            ret = (bc->GetDistance(UT_METRIC, true) + g->actual_distance()) / g->target_distance();
        } else zassert(0);
    }
    return ret;
}
const protobuf::Goal *GoalsManager::GetActivePersonalGoalOfType(protobuf::GoalType gt) {
    if (m_loadedOk) {
        auto bc = BikeManager::Instance()->m_mainBike->m_bc;
        for (auto &g : m_goals.goals()) {
            if (bc->m_sport == g.sport() && g.status() == protobuf::ACTIVE && gt == g.type()) {
                return &g;
            }
        }
    }
    return nullptr;
}
std::vector<const protobuf::Goal *> GoalsManager::GetActivePersonalGoals() {
    std::vector<const protobuf::Goal *> ret;
    auto bc = BikeManager::Instance()->m_mainBike->m_bc;
    for (auto &g : m_goals.goals()) {
        if (bc->m_sport == g.sport() && g.status() == protobuf::ACTIVE) {
            ret.push_back(&g);
        }
    }
    return ret;
}
const protobuf::Goal *GoalsManager::GetNewestInactivePersonalGoalOfType(protobuf::GoalType gt) {
    const protobuf::Goal *ret = nullptr;
    auto bc = BikeManager::Instance()->m_mainBike->m_bc;
    for (auto &g : m_goals.goals()) {
        if (bc->m_sport == g.sport() && g.status() == protobuf::RETIRED && gt == g.type()) {
            if (ret == nullptr || g.created_on() > ret->created_on())
                ret = &g;
        }
    }
    return ret;
}
void GoalsManager::Load() {
    m_loadingNow = true;
    m_get_future = zwift_network::get_goals(BikeManager::Instance()->m_mainBike->m_playerIdTx);
    m_get_future.get();
    //TODO
    /* android
    v3 = *(unsigned __int64 **)&result->field_10;
    *(_QWORD *)&result->field_10 = v6;
    if (v3) {
        v4 = v3 + 1;
        do
            v5 = __ldaxr(v4);
        while (__stlxr(v5 - 1, v4));
        if (!v5)
            (*(void(__fastcall **)(unsigned __int64 *))(*v3 + 16))(v3);
    }*/
}
void GoalsManager::Reset() {
    m_goals.Clear();
    m_activeDistanceGoal = m_activeDurationGoal = nullptr;
    m_hasPersonalGoal = m_oldConverted = m_loadedOk = m_loadingNow = false;
    //TODO 0->all or some fields
}
void GoalsManager::Update(float dt) {
    //TODO
}
void GoalsManager::UpdateGoals() {
    if (!m_loadedOk || m_goals.goals_size() == 0) {
        m_activeDistanceGoal = nullptr;
        m_activeDurationGoal = nullptr;
    } else {
        m_activeDistanceGoal = GetActivePersonalGoalOfType(protobuf::DISTANCE);
        m_activeDurationGoal = GetActivePersonalGoalOfType(protobuf::TIME);
    }
}
GoalsManager::~GoalsManager() {
    m_eventSystem->Unsubscribe(EV_RESET, this);
    //TODO - simple field dtrs? delete field_58, delete field_28, field_8->vtbl dtr (?)
}
