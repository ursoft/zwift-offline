#include "ZwiftApp.h"
std::vector<TrainingPlan::PlanEntry *> TrainingPlan::GetActivities(TrainingPlanEnrollmentInstance *, uint64_t, uint64_t, bool, int) {
    std::vector<TrainingPlan::PlanEntry *> ret;
    //TODO
    return ret;
}
float TrainingPlan::GetHoursUntilAvailable(TrainingPlanEnrollmentInstance *, uint64_t, const TrainingPlan::PlanEntry &) {
    //TODO
    return 0.0f;
}
TrainingPlanManager::TrainingPlanManager() : EventObject(EventSystem::GetInst()) {
    //TODO
}
void TrainingPlanManager::HandleEvent(EVENT_ID, va_list) {
    //TODO
}
TrainingPlanManager::~TrainingPlanManager() {
    //TODO
}
TrainingPlan *TrainingPlanManager::GetTrainingPlan(uint32_t id) {
    //TODO
    return nullptr;
}
uint64_t TP_GetNetworkTime() {
    //TODO
    return 0;
}