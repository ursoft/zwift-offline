#pragma once
inline int64_t g_GroupEventsActive_CurrentLeader, g_GroupEventsActive_CurrentSweeper, g_GroupEventsActive_BroadcastEventId;
struct PrivateEventV2 { //0x108 bytes
    int64_t m_eventId = 0, m_leaderId = 0;
    static void ReportUserResult(const protobuf::SegmentResult &);
    /*PrivateEventV2::AcceptInvitation(void)
PrivateEventV2::BeginPrivateEvent(void)
PrivateEventV2::CalculateMeetupProgression(void)
PrivateEventV2::CancelEventJoin(void)
PrivateEventV2::CloseInvitationPrompt(void)
PrivateEventV2::CloseJoinPrompt(void)
PrivateEventV2::CreateDropinStatusUI(GUI_ListBox *)
PrivateEventV2::EndPrivateEvent(void)
PrivateEventV2::EnsureSegmentRegistered(void)
PrivateEventV2::GetJoinEndTime(void)
PrivateEventV2::GetLineupTime(void)
PrivateEventV2::GetMeetupResults(void)
PrivateEventV2::GetPlayerSlotOffsetBySport(zwift::protobuf::Sport)
PrivateEventV2::GetSlotIndex(void)
PrivateEventV2::HandleEventEndings(void)
PrivateEventV2::InitializeUI(void)
PrivateEventV2::IsInPromptWindow(ulong long)
PrivateEventV2::IssueInvitationPrompt(void)
PrivateEventV2::IssueJoinPrompt(bool)
PrivateEventV2::JoinEvent(bool)
PrivateEventV2::OnEnterState(PrivateEventV2::PrivateEventState)
PrivateEventV2::OnFinishedEvent(void)
PrivateEventV2::OnLeftState(PrivateEventV2::PrivateEventState)
PrivateEventV2::PositionLateJoiner(void)
PrivateEventV2::PositionPlayerInSpawnArea(Route *,int,int)
PrivateEventV2::PrivateEventV2(zwift::protobuf::PrivateEventFeedProto)
PrivateEventV2::ProcessLateJoinResponse(void)
PrivateEventV2::RejectInvitation(void)
PrivateEventV2::ReportUserResult(zwift::protobuf::SegmentResult *)
PrivateEventV2::RequestLateJoin(long long)
PrivateEventV2::SaveSegmentResult(void)
PrivateEventV2::SendDataToAnalytics(char const*)
PrivateEventV2::SetState(PrivateEventV2::PrivateEventState)
PrivateEventV2::SetupNames(void)
PrivateEventV2::Spawn(bool)
PrivateEventV2::UI_PrivateEventV2_DropinStatusWidget::AcceptInvite(void)
PrivateEventV2::UI_PrivateEventV2_DropinStatusWidget::CancelInvite(void)
PrivateEventV2::UI_PrivateEventV2_DropinStatusWidget::UI_PrivateEventV2_DropinStatusWidget(GUI_Obj *,PrivateEventV2*)
PrivateEventV2::UI_PrivateEventV2_DropinStatusWidget::Update(float)
PrivateEventV2::UI_PrivateEventV2_DropinStatusWidget::~UI_PrivateEventV2_DropinStatusWidget()
PrivateEventV2::Update(float)
PrivateEventV2::UpdateEvent(zwift::protobuf::PrivateEventFeedProto const&)
PrivateEventV2::UpdateLateJoin(float)
PrivateEventV2::~PrivateEventV2()*/
};
inline PrivateEventV2 *g_currentPrivateEvent;
struct PrivateEventsManagerV2 { //0xF8 bytes
    bool m_field_DC = false, m_field_DD = false, m_field_DE = false;
    PrivateEventsManagerV2() {
        //TODO
    }
    ~PrivateEventsManagerV2() {
        //TODO
    }
    void GetPrivateEvents();
    /*void CheckForJoinPrompt(void);
    void FindEventByID(ulong long);
    void FindEventBySubgroupID(ulong long);
    void FindEventInState(PrivateEventV2::PrivateEventState);
    void GetCurrentLeader(void);
    void GetEventBySubgroupID(ulong long);
    void GetHideQuitEventButton(void);
    void GetJoinableEvents(std::vector<std::shared_ptr<PrivateEventV2>> &);
    void HandleDetailsRequests(void);
    void HandleEvent(EVENT_ID, std::__va_list);
    void IsFindingJoinee(BikeEntity *);
    void OnJoinEventPressed(void *);
    void OnNotNowPressed(void *);
    void OnPrivateEventEnded(PrivateEventV2 *);
    void PopulateEventListing(GUI_ListBox *);
    void QuitCurrentEvent(void);
    void ReceiveInvitationWorldAttribute(zwift::protobuf::WorldAttribute const *);
    void RefreshMeetupData(void);
    void RefreshMeetupData_Repeat(float);
    //void RegisterPrivateEvent(zwift::protobuf::PrivateEventFeedProto const &); inlined
    void SetCurrentEvent(PrivateEventV2 *);
    void SetHideQuitEventButtonBool(bool);
    void Shutdown(void);
    void StartupAfterLogin(void);
    void Update(float);*/
};
inline PrivateEventsManagerV2 g_PrivateEvents;
struct GroupEventActive {
};
struct GroupEvents {
    struct Rules;
    struct SubgroupState { //?? bytes
        int64_t m_subgroupId = 0;
        int32_t m_field_198 = 0; //or enum = 8
        std::map<int64_t, uint32_t> m_map;
        void AttemptToSetTimeTrialSlot() {
            //TODO
        }
        void EnsureRouteLoaded() {
            //TODO
        }
        void GetResults() {
            //TODO
        }
        void GetResults_RR() {
            //TODO
        }
        GroupEvents::Rules *GetRules();
        void IsTimeLeftToJoin() {
            //TODO
        }
        SubgroupState(int64_t, const GroupEventActive &) {
            //TODO
        }
        ~SubgroupState() {
            //TODO
        }
    };
    struct Rules {
        enum mask : uint64_t { GERM_PWR_CHECK = 0x20'0000'0000 };
        uint64_t m_mask = 0;
        void CacheSurfaceStyleIndexChangeMap() {
            //TODO
        }
        void CheckToChangeSurfaceStyleIndex(int) {
            //TODO
        }
        Rules(const GroupEvents::Rules &) {
            //TODO
        }
        void init() {
            //TODO
        }
        ~Rules() {
            //TODO
        }
    };
    struct EntrantRaw { //0x28 bytes
        uint16_t m_ver, m_len;
        char field_4[4];
        int64_t m_playerIdTx, m_sgEventId;
        bool  m_isSignUp, m_field_19;
        char field_1A[6];
        double m_wt_sec;
    };
    struct Entrant { //0x20 bytes
        Entrant(const EntrantRaw &src) : m_playerIdTx(src.m_playerIdTx), m_wt_sec(src.m_wt_sec), m_field19(src.m_field_19), m_sgEventId(src.m_sgEventId) {}
        uint64_t m_playerIdTx = 0;
        double m_wt_sec = 0.0;
        bool m_field19 = false, m_field_11 = false;
        int64_t m_sgEventId = 0;
    };
    static std::vector<SubgroupState> g_GroupEvents;
    static void Initialize(Experimentation *exp);
    static void Shutdown();
    static SubgroupState *FindSubgroupEvent(int64_t id);
    static bool GetCurrentEvent(int64_t id);
    static void OnWorldAttribute(const protobuf::WorldAttribute &wa);
    static void ReportUserResult(const protobuf::SegmentResult &);
    static void Subgroup_AddEntrantToRegistrants(SubgroupState *, int64_t, double, bool);
    static void Subgroup_AddEntrantToSignups(SubgroupState *, int64_t, double, bool);
    static int64_t GetCurrentBroadcastId_ex();
        /*GroupEvents::AddAdditionalAnalyticsOnEventFinish(std::vector<std::string> *)
GroupEvents::ApplyPhysicsRules(GroupEvents::Rules)
GroupEvents::CheckSubgroupFutures(void)
GroupEvents::CreateSubgroupRegistrationRequest(long)
GroupEvents::CreateSubgroupSignupRequest(long)
GroupEvents::DeleteSubgroupRegistrationRequest(long)
GroupEvents::DeleteSubgroupSignupRequest(long)
GroupEvents::EvalSurfaceChangeVars(char const*,GroupEvents::Rules &)
GroupEvents::EventTypeToString(zwift::protobuf::EventTypeProtobuf,zwift::protobuf::Sport)
GroupEvents::ExtractEventCompletionTracker(GroupEvents::SubgroupState *)
GroupEvents::ExtractRules(GroupEvents::SubgroupState *)
GroupEvents::FindSubgroupEvent(void)
GroupEvents::FindSubgroupEventServerRealmId(long)
GroupEvents::GetBikeOverride(GroupEvents::SubgroupState *)
GroupEvents::GetBikeWheelsOverride(GroupEvents::SubgroupState *)
GroupEvents::GetCurrentEventId(void)
GroupEvents::GetCurrentLeader(void)
GroupEvents::GetCurrentSweeper(void)
GroupEvents::GetEventsRequest(zwift_network::model::EventsSearch const&)
GroupEvents::GetGroupEventState(void)
GroupEvents::GetJersey(GroupEvents::SubgroupState *)
GroupEvents::GetLastEvent(void)
GroupEvents::GetLastEventId(void)
GroupEvents::GetLeaderJersey(GroupEvents::SubgroupState *)
GroupEvents::GetPendingEvent(void)
GroupEvents::GetPendingEventId(void)
GroupEvents::GetRules(ulong long)
GroupEvents::GetSubgroupEvent(long)
GroupEvents::GroupEventActive_AddOrUpdate(zwift::protobuf::EventProtobuf const&)
GroupEvents::GroupEventActive_AddToList(zwift::protobuf::EventProtobuf const&)
GroupEvents::GroupEventActive_Find(long long)
GroupEvents::GroupEventActive_GetRegistrable(bool,VirtualBikeComputer::ZSPORT,bool)
GroupEvents::GroupEventActive_Register(long)
GroupEvents::GroupEventActive_RegisterCheck(long)
GroupEvents::GroupEventActive_Signup(long,bool)
GroupEvents::GroupEventActive_SignupCheck(GroupEvents::SubgroupState *)
GroupEvents::GroupEventActive_SignupCheck(long)
GroupEvents::GroupEventActive_Update(GroupEvents::GroupEventActive &,zwift::protobuf::EventProtobuf const&)
GroupEvents::GroupEventActive_bRegisterSuccess(long)
GroupEvents::GroupEventActive_bSignupSuccess(GroupEvents::SubgroupState *)
GroupEvents::GroupEventActive_bSignupSuccess(long)
GroupEvents::GroupEventsActiveQuery::~GroupEventsActiveQuery()
GroupEvents::HasSubgroupStarted(long)
GroupEvents::InitEventCompletionParameters(GroupEvents::SubgroupState *)
GroupEvents::Initialize(Experiment::IExperimentation<Experiment::Feature> &)
GroupEvents::IsEventPartOfCampaign(ulong long)
GroupEvents::IsFindingJoinee(BikeEntity *)
GroupEvents::IsPackDynamicsV4Enabled(void)
GroupEvents::LeaveCurrentEvent(bool)
GroupEvents::NewSubgroupState(long,GroupEvents::GroupEventActive &,int)
GroupEvents::ParseEventTags(std::string const&,char)
GroupEvents::QueryGroupEvent(long long)
GroupEvents::QueryUpcomingGroupEvents(int)
GroupEvents::QueryUpcomingGroupEvents_Repeat(float)
GroupEvents::RecordFinish(GroupEvents::SubgroupState *,long,float)
GroupEvents::RegistrantList_Find(long long,std::vector<std::unique_ptr<GroupEvents::Entrant>> const&)
GroupEvents::RequestHistoricEventInfo(long long)
GroupEvents::ResetForNewPlayer(void)
GroupEvents::ResetPhysicsRules(void)
GroupEvents::SendTimeDiffForChipTimeToServer(std::shared_ptr<GroupEvents::SubgroupState>,ulong long)
GroupEvents::SetEventListDirty(void)
GroupEvents::SetGroupEventMessageCallback(void (*)(float,GroupEvents::GroupEventMessage,GroupEvents::SubgroupState *))
GroupEvents::SetGroupEventState(GroupEvents::SubgroupState *,GroupEvents::GroupEventState)
GroupEvents::ShouldOverridePlayerJersey(zwift::protobuf::PlayerType,GroupEvents::Rules const&)
GroupEvents::Shutdown(Experiment::IExperimentation<Experiment::Feature> &)
GroupEvents::SignupForSubgroup(long,void (*)(long long,bool,void *),void *)
GroupEvents::SignupQuery::~SignupQuery()
GroupEvents::Subgroup_GetUIColor(uint)
GroupEvents::Subgroup_SetPlayerSlot(GroupEvents::SubgroupState *)
GroupEvents::Subgroup_bAllowRegAttempt(long)
GroupEvents::Subgroup_bWarningTime(long)
GroupEvents::TriggerGroupEventMessage(GroupEvents::SubgroupState *,GroupEvents::GroupEventMessage)
GroupEvents::TryGetRulesTimestamp(GroupEvents::SubgroupState *,ulong long &)
GroupEvents::UpdateFlierDetection(BikeEntity *)
GroupEvents::UpdateRideLeader(void)
GroupEvents::UserRegistered(long long,long long,bool,double)
GroupEvents::UserSignedup(long long,long long,bool,double)
GroupEvents::bHoldPlayerStationary(void)*/
};
inline std::vector<GroupEvents::Entrant> g_SignupList_Orphan, g_RegistrantList_Orphan;
