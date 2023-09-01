#include "ZwiftApp.h"
void GroupEvents::Initialize(Experimentation *exp) {
    exp->IsEnabled(FID_EV_CSF, [](ExpVariant val) {
        //OMIT doClientTelemetryFID_EVENTCO
        });
    exp->IsEnabled(FID_EVENTQI, [](ExpVariant val) {
        //OMIT doClientTelemetryEVENTQU
        });
}
void GroupEvents::Shutdown() {
    /* TODO v22 = *(__int64 **)&g_sExperimentationUPtr;
    sub_7FF658BAE530((__int128 ***)&qword_7FF65A1E6DD8);
    Mylast = g_GroupEvents._Mylast;
    for (i = (__int128 *)g_GroupEvents._Myfirst; i != (__int128 *)Mylast; ++i) {
        v25 = *v22;
        v46 = *i;
        (*(void(__fastcall **)(__int64 *, __int128 *))(v25 + 40))(v22, &v46);
    }*/
}
GroupEvents::SubgroupState *GroupEvents::FindSubgroupEvent(int64_t id) {
    //TODO
    return nullptr;
}
bool GroupEvents::GetCurrentEvent(int64_t id) {
    //TODO
    return false;
}
void GroupEvents::OnWorldAttribute(const protobuf::WorldAttribute &wa) {
    //TODO
}
void GroupEvents::ReportUserResult(const protobuf::SegmentResult &) {
    //TODO
}
void GroupEvents::Subgroup_AddEntrantToRegistrants(SubgroupState *, int64_t, double, bool) {
    //TODO
}
void GroupEvents::Subgroup_AddEntrantToSignups(SubgroupState *, int64_t, double, bool) {
    //TODO
}
int64_t GroupEvents::GetCurrentBroadcastId_ex() {
    auto ret  = g_GroupEventsActive_BroadcastEventId;
    if (!ret) {
        if (g_currentPrivateEvent)
            ret = g_currentPrivateEvent->m_eventId;
        else
            ret = 0;
    }
    return ret;
}
GroupEvents::SubgroupState *FindSubgroupEventSharedPtr(int64_t id) {
    //TODO
    return nullptr;
}
bool GroupEvents::HasSubgroupStarted(int64_t id) { //was inlined
    SubgroupState *sge = FindSubgroupEventSharedPtr(id);
    return sge && g_CachedWorldTime >= (time_t)sge->GetEventSubgroup()->event_subgroup_start_wt();
}
void GroupEvents::RequestHistoricEventInfo(int groupid) {
    //TODO
}
void PrivateEventsManagerV2::GetPrivateEvents() {
    //TODO
}
void ReportUserResult(const protobuf::SegmentResult &) {
    //TODO
}
void PrivateEventV2::ReportUserResult(const protobuf::SegmentResult &) {
    //TODO
}
GroupEvents::Rules *GroupEvents::SubgroupState::GetRules() {
    //TODO
    return nullptr;
}
