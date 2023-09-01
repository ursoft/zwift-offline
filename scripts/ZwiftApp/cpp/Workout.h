#pragma once
void IncreaseWorkoutIntensityClicked();
void DecreaseWorkoutIntensityClicked();
inline bool g_bSkipThisWorkoutSection;
struct WorkoutDatabase {
    static inline WorkoutDatabase *g_Instance;
    void *m_field_98 = nullptr;
    WorkoutDatabase() {
        //TODO
    }
    static WorkoutDatabase *Self() {
        if (!g_Instance) {
            g_Instance = new WorkoutDatabase();
            Log("WorkoutDatabase Constructed");
        }
        return WorkoutDatabase::g_Instance;
    }
    /*
WorkoutDatabase::DeleteWorkout(uint)
WorkoutDatabase::Destroy(void)
WorkoutDatabase::EnsureStartup(void)
WorkoutDatabase::FindWorkoutFiles(char const*,bool)
WorkoutDatabase::ForceReloadWorkout(char const*)
WorkoutDatabase::GetAllWorkoutsWithTag(char const*,bool,Workout::WorkoutGameMode)
WorkoutDatabase::GetAnalyticsName(void)
WorkoutDatabase::GetCurrentWorkoutEvent(void)
WorkoutDatabase::GetCustomWorkoutTagList(void)
WorkoutDatabase::GetDescriptionForWorkout(char *,char *)
WorkoutDatabase::GetNextWorkoutInPlan(Workout *)
WorkoutDatabase::GetRPEEntriesForWorkout(Workout *)
WorkoutDatabase::GetWTEForWorkout(Workout *)
WorkoutDatabase::GetWorkout(char const*,char const*,char const*)
WorkoutDatabase::GetWorkoutByHash(uint,bool)
WorkoutDatabase::GetWorkoutCategories(void)
WorkoutDatabase::GetWorkoutDictByGameMode(Workout::WorkoutGameMode)
WorkoutDatabase::GetWorkoutName(void)
WorkoutDatabase::GetWorkoutSubcatsByCategory(char const*)
WorkoutDatabase::GetWorkoutsByCategory(char const*,char const*,Workout::WorkoutGameMode)
WorkoutDatabase::GetWorkoutsByGameMode(Workout::WorkoutGameMode)
WorkoutDatabase::GetWorkoutsWithTag(char const*)
WorkoutDatabase::HandleEvent(EVENT_ID,std::__va_list)
WorkoutDatabase::HasCompletedAnyWorkout(void)
WorkoutDatabase::HasCompletedWorkout(uint)
WorkoutDatabase::IsStarted(void)
WorkoutDatabase::IsWorkoutHUDEnabled(void)
WorkoutDatabase::LoadPartnersCustomWorkout(long,std::string const&,zwift_network::model::WorkoutPartnerEnum)
WorkoutDatabase::LoadWorkout(tinyxml2::XMLDocument *,uint,bool,bool,char const*,char const*,bool,bool)
WorkoutDatabase::LoadWorkoutCategories(tinyxml2::XMLDocument *)
WorkoutDatabase::LoadWorkoutWAD(char const*)
WorkoutDatabase::LoadWorkoutsFromFolder(char const*,char const*,bool)
WorkoutDatabase::RemoveLastWorkoutFromHistory(void)
WorkoutDatabase::Self(void)
WorkoutDatabase::SetActiveWorkout(Workout *)
WorkoutDatabase::SetActiveWorkoutSource(WorkoutDatabase::ActiveWorkoutSource)
WorkoutDatabase::WorkoutDatabase(void)
WorkoutDatabase::~WorkoutDatabase()    */
};
