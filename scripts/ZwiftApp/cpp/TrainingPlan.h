#pragma once
struct Workout;
struct TrainingPlanEnrollmentInstance;
struct TrainingPlan {
    struct PlanBlock {
        PlanBlock(const TrainingPlan::PlanBlock &);
        void Load(tinyxml2::XMLElement *);
    };
    struct PlanEntry {
        void ActivateThisActivity();
        void DoesEventQualify(GroupEvents::SubgroupState *, int);
        void FindSuitableGroupEvent();
        void GetDoByTime(bool *, bool);
        //TODO void GroupEventOverrideInfo::GroupEventOverrideInfo(tinyxml2::XMLElement *);
        void Load(tinyxml2::XMLElement *);
        PlanEntry(TrainingPlan::PlanEntry const &);
        ~PlanEntry();
    };
    void CalculateAverages();
    std::vector<PlanEntry *> GetActivities(TrainingPlanEnrollmentInstance *, uint64_t, uint64_t, bool, int);
    void GetActivity(uint32_t);
    void GetBlock(uint32_t);
    void GetBlockStart(uint64_t,uint32_t);
    void GetEntryEnd(const TrainingPlan::PlanEntry &, uint64_t);
    void GetEntryStart(const TrainingPlan::PlanEntry &, uint64_t, uint64_t);
    float GetHoursUntilAvailable(TrainingPlanEnrollmentInstance *, uint64_t, const TrainingPlan::PlanEntry &);
    void GetLogoMark();
    void GetShortLogomark();
    //TODO void GetTotalActivityCount(bool,Workouts::Sport);
    float GetValidHoursRemaining(TrainingPlanEnrollmentInstance *, uint64_t, const TrainingPlan::PlanEntry &, bool *, bool);
    void HandleEndingPlan(TrainingPlanEnrollmentInstance *);
    void IsPlanLockedByEntitlement();
    void IsPlanLockedByTime(long);
    void Load(char const*);
    TrainingPlan &operator=(const TrainingPlan &);
    ~TrainingPlan();
};
struct TrainingPlanEnrollmentInstance {
    uint32_t m_planId = 0;
    void AddActivityRecord(uint64_t, uint32_t, int64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint32_t, uint32_t, tinyxml2::XMLDocument *);
    void AddOutsideActivityRecord(uint32_t);
    void CheckPlanCompletion();
    void GetBlock(int);
    void GetBlockOrder(TrainingPlan::PlanBlock *);
    void GetFailedActivitiesCount();
    void GetOutsideRecordCount();
    void GetPlannedEndDate();
    void GetTargetEndDate();
    void GetTotalActivityCount();
    void GetTotalRecordedValues(float *, float *, float *, uint32_t *, uint32_t *);
    void GetWeekCount();
    void HasAnyActivitiesRemaining();
    void IsValidWorkoutForPlan(Workout *,TrainingPlan::PlanEntry **);
    void Load(uint32_t);
    void OnPlanCancelled();
    void RemoveActivityRecord(uint32_t);
    void Save(bool);
};
struct TrainingPlanManager : public EventObject { //0x110 bytes
    TrainingPlanEnrollmentInstance *m_pcurEnrollment = nullptr;
    TrainingPlanManager();
    static TrainingPlanManager *Instance() {
        static TrainingPlanManager *ret = new TrainingPlanManager();
        return ret;
    }
    TrainingPlan *GetTrainingPlan(uint32_t id);
    bool CanShowAvailableTrainingTasks();
    void CancelEnrollmentInTrainingPlan();
    void DeleteWorkoutsFromAttributes();
    void EnrollInTrainingPlan(uint32_t, uint32_t);
    void GetTimesCompleted(TrainingPlan *);
    void HandleEvent(EVENT_ID, va_list);
    void IsTrainingPlanSummaryFixEnabled();
    void Load(const char *);
    void LoadNetworkData();
    void LogTrainingPlanCancellation(TrainingPlan *, TrainingPlanEnrollmentInstance *);
    void LogTrainingPlanCompletion(TrainingPlan *, TrainingPlanEnrollmentInstance *);
    void OnCompletedWorkout(Workout *);
    void ProcessBlackouts();
    void SaveWorkoutsToAttributes();
    void UndoCancelEnrollment(uint32_t);
    void Update(float);
    void UpdateActivityInfoCache(TrainingPlanEnrollmentInstance *, const TrainingPlan::PlanEntry *);
    void UpdateReminders(TrainingPlanEnrollmentInstance *);
    ~TrainingPlanManager();
};
uint64_t TP_GetNetworkTime();