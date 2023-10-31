#pragma once
enum ConnectionType { CT_0, CT_SERVER, CT_ANT, CT_BLE, CT_DEVICE };
enum NotificationDisplayType { NDT_0, NDT_1, NDT_2 };
enum NotificationType { CNT_SERVER_LOST = 1, CNT_SERVER_RESTORED = 3, CNT_ANT_LOST = 4, CNT_ANT_RESTORED = 5, CNT_BLE_LOST = 6, CNT_BLE_RESTORED = 7, CNT_DEV_LOST = 8, CNT_DEV_RESTORED = 9 };
struct ConnectionSubscriber {
    virtual void HandleNotification(ConnectionType, NotificationType, NotificationDisplayType) = 0;
};
struct SaveActivityService : public ConnectionSubscriber { //0x128 bytes
    SaveActivityService(XMLDoc *doc, Experimentation *exp, EventSystem *ev);
    static void Shutdown();
    static bool IsInitialized() { return g_SaveActivityServicePtr.get() != nullptr; }
    static void Initialize(XMLDoc *doc, Experimentation *exp, EventSystem *ev);
    inline static std::unique_ptr<SaveActivityService> g_SaveActivityServicePtr;
    void HandleNotification(ConnectionType, NotificationType, NotificationDisplayType) override;

/*AutosaveCurrentActivity(BikeEntity const&)
CreateNewActivity(BikeEntity const&,int)
CreateNewActivity(BikeEntity const&,int,SaveActivity::Sport)
CreateNewActivity(BikeEntity const&,int,SaveActivity::Sport,ulong long)
DeleteActivityFromDisk(void)
DeleteCurrentActivity(BikeEntity const&)
EndCurrentActivity(BikeEntity const&)
EndCurrentActivity(BikeEntity const&,std::string const&,SaveActivity::Privacy,bool)
EndCurrentActivity(BikeEntity const&,std::string const&,SaveActivity::Privacy,bool,std::string const&)
FinalizeCurrentActivity(BikeEntity const&,std::string const&,SaveActivity::Privacy,bool)
GameLoadLevel(int)
GenerateActivityName(int)
GetCurrentActivityWorldId(void)
HandleEvent(EVENT_ID,std::__va_list)
HandleLogout(void)
HasCurrentActivity(void)
Initialize(XMLDoc &,Experiment::IExperimentation<Experiment::Feature> &,EventSystem &,ConnectionManager &)
InitializeNewActivity(BikeEntity const&,int,SaveActivity::Sport,ulong long)
Instance(void)
IsCurrentActivityWorkout(void)
IsDeletingActivity(void)
IsInitialized(void)
IsSavingActivity(void)
OnNetworkRequestFailure(SaveActivity::StateMachine<SaveActivityService> const&,std::string const&)
OnNewActivityID(SaveActivity::StateMachine<SaveActivityService> const&,ulong)
OnRequestExceededRetryCount(SaveActivity::StateMachine<SaveActivityService> &)
OnSentNetworkRequest(SaveActivity::StateMachine<SaveActivityService> const&)
OnTerminalState(SaveActivity::StateMachine<SaveActivityService> const&)
PendingActivityData::~PendingActivityData()
PersistCurrentlySavingActivity(void)
PlayerData::~PlayerData()
ReplaceCurrentActivity(BikeEntity &,SaveActivity::Sport)
ReplaceCurrentActivity(BikeEntity &,int)
ReplaceCurrentActivity(BikeEntity &,int,SaveActivity::Sport,ulong long)
ReplaceCurrentActivity(BikeEntity &,int,ulong long)
ResetCurrentPlayerClub(void)
ResetPendingActivityData(void)
RestoreActivity(BikeEntity const&,int,ulong)
RestoreFITFile(bool)
RestorePreviousActivity(BikeEntity const&,int,ulong)
RetireCurrentActivity(EVENT_ID)
SaveActivityFromDisk(std::string const&)
SaveActivityService(XMLDoc &,Experiment::IExperimentation<Experiment::Feature> &,EventSystem &)
ScreenshotUploaded(std::string const&)
SetCurrentActivity(SaveActivity::ActivityData &&)
SetCurrentActivityEventComplete(GroupEvents::SubgroupState const&)
SetCurrentActivityEventStarted(GroupEvents::SubgroupState &,std::string const&)
SetCurrentActivityGroupWorkoutStarted(GroupEvents::SubgroupState const&,Workout const&)
SetCurrentActivityMeetupStarted(zwift::protobuf::PrivateEventFeedProto const&)
SetCurrentPlayerClub(std::string const&)
SetPacePartnerName(std::string const&)
SetRouteName(std::string const&)
Shutdown(ConnectionManager &)
UpdatePacePartnerData(void)
WriteActivityToDisk(SaveActivity::StateMachine<SaveActivityService> &)
~SaveActivityService()*/
};
