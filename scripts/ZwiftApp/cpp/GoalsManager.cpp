#include "ZwiftApp.h" //READY for testing
GoalsManager::GoalsManager(EventSystem *ev) : EventObject(ev) {
    ev->Subscribe(EV_RESET, this);
}
void GoalsManager::HandleEvent(EVENT_ID e, va_list va) { if (e == EV_RESET) Load(); }
void GoalsManager::AddPersonalGoal(const protobuf::Goal &g) {
    if (!m_saving) {
        ++g_nGoalsAdded;
        m_save_future = zwift_network::save_goal(g);
        m_saving = true;
    }
}
void GoalsManager::ConvertOldGoals() {
    auto mb = BikeManager::Instance()->m_mainBike;
    auto sg = mb->GetSaveGame();
    for (auto &i : sg->m_oldGoals) {
        protobuf::Goal v14;
        v14.set_player_id(mb->m_playerIdTx);
        if (i.m_type == protobuf::TIME) {
            v14.set_type(protobuf::TIME);
            v14.set_name("Time Goal"s);
            v14.set_target_duration(i.m_val / 60.0f);
        } else {
            v14.set_type(protobuf::DISTANCE);
            v14.set_name("Distance Goal"s);
            v14.set_target_distance(i.m_val);
        }
        v14.set_periodicity((i.m_isWeekly != 1) ? protobuf::MONTHLY : protobuf::WEEKLY);
        v14.set_status(protobuf::ACTIVE);
        v14.set_sport(protobuf::CYCLING);
        AddPersonalGoal(v14);
    }
    sg->m_oldGoals.clear();
    m_oldConverted = true;
}
void GoalsManager::FinishLoad(const NetworkResponse<protobuf::Goals> &resp) {
    for (auto &i : resp.m_T.goals())
        m_goals.push_back(i);
}
float GoalsManager::GetActiveGoalCompletion(protobuf::GoalType gt) {
    float ret = 0.0f;
    auto g = GetActivePersonalGoalOfType(gt);
    if (g) {
        auto bc = BikeManager::Instance()->m_mainBike->m_bc;
        if (gt == protobuf::TIME) {
            ret = (g->actual_duration() + (bc->m_current_duration / 1000) / 60.0f) / g->target_distance(); //QUEST: why 'target_distance', not target_duration?
        } else if (gt == protobuf::DISTANCE) {
            ret = (bc->GetDistance(UT_METRIC, true) + g->actual_distance()) / g->target_distance();
        } else zassert(0);
    }
    return ret;
}
const protobuf::Goal *GoalsManager::GetActivePersonalGoalOfType(protobuf::GoalType gt) {
    if (m_loadedOk) {
        auto bc = BikeManager::Instance()->m_mainBike->m_bc;
        for (auto &g : m_goals) {
            if (bc->m_sport == g.sport() && g.status() == protobuf::ACTIVE && gt == g.type()) {
                return &g;
            }
        }
    }
    return nullptr;
}
std::vector<const protobuf::Goal *> GoalsManager::GetActivePersonalGoals() { //not used, not found
    std::vector<const protobuf::Goal *> ret;
    auto bc = BikeManager::Instance()->m_mainBike->m_bc;
    for (auto &g : m_goals)
        if (bc->m_sport == g.sport() && g.status() == protobuf::ACTIVE)
            ret.push_back(&g);
    return ret;
}
const protobuf::Goal *GoalsManager::GetNewestInactivePersonalGoalOfType(protobuf::GoalType gt) { //not used, not found
    const protobuf::Goal *ret = nullptr;
    auto bc = BikeManager::Instance()->m_mainBike->m_bc;
    for (auto &g : m_goals) {
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
    //+216 is not m_get_future.get();
}
void GoalsManager::Reset() {
    m_goals.clear();
    m_activeDistanceGoal = m_activeDurationGoal = nullptr;
    m_saving = m_oldConverted = m_loadedOk = m_loadingNow = false;
    //QUEST: where used:
    //*(_QWORD *)&v89->field_30 = 0i64;
    //*(_QWORD *)&v89->field_60 = 0i64;
    //v89->field_68._Mylast = v89->field_68._Myfirst;
    //v89->field_38._Mylast = v89->field_38._Myfirst;
}
void GoalsManager::Update(float dt) {
    auto be = BikeManager::Instance()->m_mainBike;
    if (m_saving) {
        if (std::future_status::ready == m_save_future.wait_for(std::chrono::seconds(0))) {
            m_goals.push_back(m_save_future.get().m_T);
            m_saving = false;
        }
    }
    if (m_oldConverted) {
        m_timeAcc += dt;
        if (m_timeAcc >= 1.0f) {
            Load();
            m_oldConverted = false;
            m_timeAcc = 0.0f;
        }
    }
    if (m_loadingNow) {
        if (std::future_status::ready == m_get_future.wait_for(std::chrono::seconds(0))) {
            FinishLoad(m_get_future.get());
            m_loadedOk = true;
            UpdateGoals();
        }
        m_loadingNow = false;
    }
    if (m_loadedOk) {
        if (Experimentation::Instance()->IsEnabled(FID_GOAL_NM)) {
            auto actTimeGoalCompletion = GetActiveGoalCompletion(protobuf::TIME);
            auto actDistGoalCompletion = GetActiveGoalCompletion(protobuf::DISTANCE);
            if (actTimeGoalCompletion != 0.0f || actDistGoalCompletion != 0.0f) {
                static float g_actTimeGoalCompletion = actTimeGoalCompletion, g_actDistGoalCompletion = actDistGoalCompletion;
                if (g_actTimeGoalCompletion != 0.0f && g_actTimeGoalCompletion < 1.0f && actTimeGoalCompletion >= 1.0f) {
                    UI_NotificationQueue::QueueGoalMilestone(1);
                    auto act = GetActivePersonalGoalOfType(protobuf::TIME);
                    g_pNotableMomentsMgr.OnNotableMoment(protobuf::NMT_COMPLETED_GOAL, be->GetPosition(), 1, act->periodicity(), act->target_duration());
                    ZNETWORK_BroadcastLocalPlayerNotableMoment(protobuf::NMT_COMPLETED_GOAL, 1, 0.0f);
                }
                if (g_actDistGoalCompletion != 0.0f && g_actDistGoalCompletion < 1.0f && actDistGoalCompletion >= 1.0f) {
                    UI_NotificationQueue::QueueGoalMilestone(1);
                    auto act = GetActivePersonalGoalOfType(protobuf::DISTANCE);
                    g_pNotableMomentsMgr.OnNotableMoment(protobuf::NMT_COMPLETED_GOAL, be->GetPosition(), 0, act->periodicity(), act->target_distance());
                    ZNETWORK_BroadcastLocalPlayerNotableMoment(protobuf::NMT_COMPLETED_GOAL, 0, 0.0f);
                }
                g_actTimeGoalCompletion = actTimeGoalCompletion;
                g_actDistGoalCompletion = actDistGoalCompletion;
            }
        }
    }
}
void GoalsManager::UpdateGoals() {
    if (!m_loadedOk || m_goals.size() == 0) {
        m_activeDistanceGoal = nullptr;
        m_activeDurationGoal = nullptr;
    } else {
        m_activeDistanceGoal = GetActivePersonalGoalOfType(protobuf::DISTANCE);
        m_activeDurationGoal = GetActivePersonalGoalOfType(protobuf::TIME);
    }
}
GoalsManager::~GoalsManager() {
    m_eventSystem->Unsubscribe(EV_RESET, this);
}
