#pragma once
using ZSPORT = protobuf::Sport;
struct VirtualBikeComputer {
    float GetDistance(UnitType ut, bool) { /*TODO*/ return 0.0; }
    void SetTireSize(uint32_t tireCirc) { m_tireCirc = tireCirc; }
    uint64_t m_lastPower = 0;
    uint32_t m_tireCirc = 2105, m_powerSmoothing = 1, m_field_118 = 0, m_field_128 = 0;
    int32_t m_field_1E8 = 0;
    float m_distance = 0.0f, m_power = 0.0f, m_field_19C = 0.0f, m_speed = 0.0f, m_field_18C = 0.0f, m_field_198 = 0.0f, m_field_1E0 = 0.0f, m_sensor_f2 = 0.0f, m_total_smth = 0.0f, 
        m_heart_f2 = 0.0f;
    protobuf::Sport m_sport = protobuf::CYCLING;
    bool m_bool = false, m_bool1 = false, m_workoutPaused = false;
};
struct SaveGame;
struct ConfettiComponent {
    static void SpawnPersonalConfetti(BikeEntity *pbe, NOTABLEMOMENT_TYPE nmt, float, int64_t) {
        //TODO
    }
}; 
/*ConfettiSpawn(VEC3, VEC3, float, uint *, uint)
CreateAudioEvent(char const *, int)
SpawnPersonalConfetti(BikeEntity &, NOTABLEMOMENT_TYPE, float, long long)
Update(BikeEntity &, zwift::context::UpdateContext &, float)
Update(BikeEntity &, zwift::context::UpdateContext &, float)
UpdateWhileSleeping(BikeEntity &, float, zwift::context::UpdateContext &, float)
UpdateWhileSleeping(BikeEntity &, float, zwift::context::UpdateContext &, float)
~ConfettiComponent()*/
struct Heading {
    float m_cos;
    int32_t m_heading2;
    float m_sin, m_angleRad;
    Heading() {
        m_cos = 1.0f;
        m_heading2 = 0;
        m_sin = -0.0f; //QUEST: what for
        m_angleRad = 0.0f;
    }
    void SetDir(float a2, float a3) {
        auto v4 = atan2f(a3, a2);
        SetRadians(-v4);
    }
    void SetDir(const VEC3 &a2) { SetDir(a2.m_data[2], a2.m_data[0]); }
    float GetRadians() { return m_angleRad; }
    static float doNorm(float rad) {
        if (rad < -3.1416f)
            return rad + 6.2832f;
        else if (rad >= 3.1416f)
            return rad - 6.2832f;
        return rad;
    }
    static float NormalizeRadians(float rad) { return doNorm(fmodf(rad, 6.2832f)); }
    void SetRadians(float rad) {
        m_angleRad = NormalizeRadians(rad);
        float sinx;
        __libm_sse2_sincosf_(m_angleRad, &sinx, &m_cos);
        m_heading2 = 0;
        m_sin = -sinx;
    }
    static float InterpolateRadians(float a2, float a3, float a4) {
        float v7 = NormalizeRadians(a2);
        float v9 = NormalizeRadians(a3);
        float d = doNorm(v9 - v7);
        float result = a4 * d + v7;
        return NormalizeRadians(result);
    }
    /* Heading::GetDir(void) - not implemented in Android */
};
struct GroupRideFence {
    struct Component {
        //TODO
        char field_0[36];
        uint16_t m_ver;
        uint16_t m_len;
        char field_28;
        char field_29;
        char field_2A;
        char field_2B;
        char m_bool2C;
        char field_2D;
        char field_2E;
        char field_2F;
        char field_30[16];
        int m_field_40;
        char gap44[4];
        char field_48[224];
        uint64_t m_field_128;
        char field_130[20];
        int m_field_144;
        char gap148[8];
        int m_field_150;

        void OnReceiveRiderStats(const ZNETWORK_GRFenceRiderStats &) {
            //TODO
        }
        bool IsFenceGenerator() {
            //TODO
            return false;
        }
        void ToggleFenceLeaderUI() {
            //TODO
        }
    };
    //TODO
    /*GroupRideFence::Component::ApplyFence(void)
GroupRideFence::Component::Authority_AdjustFenceTimeAhead(uchar)
GroupRideFence::Component::Authority_AdjustFlyTimeout(int)
GroupRideFence::Component::Authority_RPC_SendFenceConfig(void)
GroupRideFence::Component::Authority_TurnFenceOff(void)
GroupRideFence::Component::Authority_TurnFenceOn(void)
GroupRideFence::Component::CalculateFenceVectors(float)
GroupRideFence::Component::CleanupArtifacts(void)
GroupRideFence::Component::Component(BikeEntity const&)
GroupRideFence::Component::GetFenceGenerator(void)
GroupRideFence::Component::GetGeneratorUIHeight(void)
GroupRideFence::Component::GetLapCount(void)
GroupRideFence::Component::GetLeader(void)
GroupRideFence::Component::GetRoute(bool)
GroupRideFence::Component::GetWorldForward(void)
GroupRideFence::Component::GetWorldPosition(void)
GroupRideFence::Component::GetWorldRight(void)
GroupRideFence::Component::HandleEvent(EVENT_ID,std::__va_list)
GroupRideFence::Component::HasLateJoined(void)
GroupRideFence::Component::HasReliableSplineDistance(void)
GroupRideFence::Component::IsFenceEnabled(void)
GroupRideFence::Component::IsFlyerHUDShown(void)
GroupRideFence::Component::IsGeneratorUIShown(void)
GroupRideFence::Component::IsInKickZone(void)
GroupRideFence::Component::IsLeader(void)
GroupRideFence::Component::IsNetworked(void)
GroupRideFence::Component::IsPlayer(void)
GroupRideFence::Component::IsZwiftStaff(void)
GroupRideFence::Component::LateInit(void)
GroupRideFence::Component::OnReceiveFenceConfig(ZNETWORK_GRFenceConfig const&)
GroupRideFence::Component::OnToggleFence(bool)
GroupRideFence::Component::RPC_SendRiderStats(void)
GroupRideFence::Component::Render(void)
GroupRideFence::Component::ReportUserKickedByFence(void)
GroupRideFence::Component::ShouldDrawFenceOnMinimap(void)
GroupRideFence::Component::ShouldShowFenceActionButton(void)
GroupRideFence::Component::ShouldShowGeneratorUI(void)
GroupRideFence::Component::Update(ulong long,float)
GroupRideFence::Component::UpdateCulling(void)
GroupRideFence::Component::UpdateEventParams(void)
GroupRideFence::Component::UpdateFence(BikeEntity const&,float)
GroupRideFence::Component::UpdateNextIntersection(BikeEntity const&)
GroupRideFence::Component::UpdateRPCs(float)
GroupRideFence::Component::UpdateRiderFenceStats(float)
GroupRideFence::Component::UpdateUI(float)
GroupRideFence::Component::{unnamed type#1}::~Component()
GroupRideFence::Component::~Component()
GroupRideFence::FencePositioner::GetMinMetersAheadOfGenerator(float)
GroupRideFence::FencePositioner::GetMinMetersBehindToDraw(void)
GroupRideFence::FencePositioner::Import(ZNETWORK_GRFenceConfig const&)
GroupRideFence::FencePositioner::SetFlyOutLocation(BikeEntity const&,GroupRideFence::FenceLocation &)
GroupRideFence::Status::Import(ZNETWORK_GRFenceConfig const&)
GroupRideFence::Status::IsFenceSupported(void)
GroupRideFence::Status::ShouldDrawFence(bool)
GroupRideFence::Status::ShouldEnforceFence(void)
GroupRideFence::operator!=(GroupRideFence::RoadPoint const&,GroupRideFence::RoadPoint const&)
GroupRideFence::operator==(GroupRideFence::RoadPoint const&,GroupRideFence::RoadPoint const&)*/
};
struct IncomingPlayerStateComponent {
    protobuf::PlayerState m_pbState;
    protobuf::PowerType m_pty = protobuf::PT_VIRTUAL;
};
struct BikeManager;
struct BikeEntity : public Entity { //0x1948 bytes
    static inline StaticAssetLoader s_staticAssetLoader;
    struct RideOnAnim { //0x30 bytes
        float m_f0 = 0.0f, m_f1 = 0.0f, m_f2 = 500.0f, m_f3 = 0.0f;
        float m_f4 = 0.001f, m_f5 = 0.0f, m_f6 = -18.0f, m_f7 = 0.0f;
        int64_t m_fromPlayerId;
        bool m_field_28 = false, m_field_29 = false;
    };
    BikeEntity(BikeManager *bm, bool bIsNetworked, bool bIsPlayer);
    ~BikeEntity();
    int64_t m_curEventId = 0, m_cheatBits = 0;
    VirtualBikeComputer *m_bc = nullptr;
    IncomingPlayerStateComponent *m_ipsc = nullptr;
    RoadSegment *m_road = nullptr;
    protobuf::PlayerProfile m_profile;
    SaveGame *m_pSaveGame = nullptr;
    RouteComputer *m_routeComp = nullptr;
    PrivateAttributesHelper m_pah;
    std::list<RideOnAnim> m_rxRideonsList;
    GroupRideFence::Component *m_grFenceComponent = nullptr;
    EbikeBoost *m_eboost = nullptr;
    double m_field_888 = 0.0;
    Heading m_heading;
    VEC3 m_teleportPos{};
    UChar m_uname[824/2]; //TODO: really not too big
    int m_rxRideons = 0, m_field_1814 = -1 /*not sure*/, m_field_8F0 = 0, m_field_940 = 0;
    float m_field_AA8 = 0.0f, m_field_8EC = 0.0f;
    int32_t m_field_AAC = 0, m_field_3D4 = 0 /*enum*/, m_field_3CC = 0 /*enum*/, m_field_13C = 0;
    uint32_t m_race_f14 = 0, m_fwGdeSignature = 0, m_rwGdeSignature = 0, m_yellowJersey = 0;
    protobuf::POWERUP_TYPE m_pendPU = protobuf::POWERUP_NONE;
    int32_t m_skillWKG = -1, m_eventPos = 0, m_field_11DC = 0, m_msToLeader = 0;
    float m_field_59C = 0.0f, m_field_5A0 = 1.0f;
    bool m_writable = false, m_isCheater = false, m_isSandbagger = false, m_sensor_f11 = false, m_field_806 = false,
        m_immuneFromCheating = false, m_boolCheatSmth = false, m_joinedWorld = false, m_field_488 = false, m_field_3D8 = false, m_field_3D9 = false,
        m_field_8B8 = false, m_field_CC1 = false;
    void SaveProfile(bool, bool);
    bool IsPacerBot() { return m_profile.player_type() == protobuf::PlayerType::PACER_BOT; }
    int64_t GetEventID();
    SaveGame *GetSaveGame() {
        zassert(m_pSaveGame)
        return m_pSaveGame;
    }
    float GetFTP(bool *bEstimated = nullptr);
    bool GetShouldUseSkillLevel();
    float GetSkillLevelWKG();
    float GetRiderWeightKG(bool a2);
    void RequestProfileFromServer();
    void GiveRideOn(int64_t fromPlayerId);
    enum USER_ACTION { UA_ELBOW, UA_WAVE, UA_2, UA_RIDEON, UA_HAMMERTIME, UA_NICE, UA_BRINGIT, UA_TOAST, UA_BELL, UA_9 };
    void PerformAction(protobuf::UserBikeAction);
    void ClearPowerups();
    void Respawn(int segment, double a2, bool a3, bool a4);
    void AdjustRandomXZ(/*float*/);
    void ActivatePowerUp();
    /* TODO:
void WakeupAnim(void);
void UpdateWhileSleeping(zwiftUpdateContext &,float);
void UpdateWheelRot(float);
void UpdateVisualLean(float,float,float);
void UpdateTimeNearRiders(float);
void UpdateSplineDistance(void);
void UpdateSound_Running(void);
void UpdateSound_Cycling(void);
void UpdateSound(void);
void UpdateSleepState(bool,float);
void UpdateRoadStyle(float);
void UpdateRoadContactElev(float,bool);
void UpdateRideOns(void);
void UpdateRideOnAnimations(float);
void UpdateRenderingMask(zwiftUpdateContext &);
void UpdatePowerups(float);
void UpdatePowerupDrawState(float);
void UpdatePositionCommon(float,float);
void UpdatePosition(float);
void UpdateLurking(float);
void UpdateHoloTrainerDrawState(float);
void UpdateHeading(float,float &);
void UpdateForwardFlag(void);
void UpdateEventPaddock(void);
void UpdateEventJerseys(long);
void UpdateEbikeBoost(float);
void UpdateDonutFailSafe(VEC3 const&);
void UpdateCurrentRoad(float,bool);
void UpdateCachedRoadStyle(void);
void UpdateBranchingRoad(float);
void UpdateBikeOverrides(long);
void UpdateAttachmentData(void);
void UpdateAnimation(float);
void UpdateAnimFileMap(void);
void Update(zwiftUpdateContext &,float);
void Update(float);
void UnlockDigitalEntitlements(void);
void ToggleHolidayBikeIDOverride(bool);
void TeleportToStart(RoadSegment const&,double);
void SwitchRoadBasedOnTime(void);
void SteerTowardsTarget(float);
void StateSnapshot_Valid(void);
void StateSnapshot_Restore(zwiftUpdateContext &);
void StateSnapshot_Restore(void);
void StateSnapshot_Make(zwiftUpdateContext &);
void StateSnapshot_Make(void);
void SpeedBasedYawAdjustment(float &,float);
void ShouldRenderPowerupWheels(void);
void ShouldRenderPowerupBikeFrame(void);
void ShouldRenderPowerupAccessory(Accessory const*);
void ShouldRenderHolographicWheels(void);
void ShouldRenderHolographicBikeFrame(void);
void ShouldRenderHolographicAccessory(Accessory const*);
void ShouldRenderAccessory(Accessory const*);
void ShouldDrawGhostlyHolo(BikeEntity const*,VEC4 *);
void ShouldApplyExtraProjectionImprovements(void);
void SetSpawnDirection(bool);
void SetShouldUseSkillLevel(bool,int);
void SetRightCollisionFlags(void);
void SetPowerupShaderForWheel(VEC4 &,bool &);
void SetPlayerPaused(bool);
void SetPacerBotRouteHash(void);
void SetOnboardProgress(BikeEntity::ONBOARD_CARD,long long);
void SetMissionBikeOverride(uint);
void SetLeftCollisionFlags(void);
void SetJerseyOverride(uint);
void SetJerseyAsset(void);
void SetHolographicShaderForWheel(VEC4 &,bool &);
void SetHasLeaderJerseyType(TimingArchEntity *,std::string,std::string,bool,bool);
void SetFTP(float);
void SetDraft(float);
void SetDesiredAnimation(BikeEntity::CYCLIST_ANIMATION,bool,bool);
void SetCoastingAnim(float,TurnDirection);
void SetBranchingPreference(IntersectionOption::PREFERENCE);
void SetBikeOverride(uint);
void SetAIYaw(float,float,float,bool);
void SetAI(std::string,bool,int,int,bool,uint,int,bool,uint,uint,uint,uint,int,bool,uint,bool,bool,uint,bool,bool,uint,uint,uint,uint,uint,uint,int,uint,uint,bool,float);
void SendPlayerState(long,protobuf::PlayerState const&);
void SelectRoadBasedOnPreference(IntersectionMarkerEntity *,float);
void SearchFutureRoad(int);
void SearchForNearbyRoad(int);
void SaveProfile(bool,bool);
void RoadOverbound(void);
void RidingPosition(void);
void ResetSplineDistance(float);
void ResetRoadDirVec(void);
void ResetProximity(void);
void ResetCollisionFlags(void);
void ResetBikeIDOverrideCache(void);
void ResetBikeForNewSession(void);
void ResetBikeForNewLevel(bool,bool,Route const*);
void RenderPartnerCrown(MATRIX44 const*,VEC4,int);
void RenderLateralCollisionEffect(bool);
void RenderCassette(GDE_Header_360 *,int,bool,VEC4);
void RenderAlpha(void);
void RenderAccessory(Accessory const*,int,ulong long,int,VEC4,bool,bool,float);
void Render(void);
void RegisterTurnSignalParticles(void);
void RegisterTailLightParticles(void);
void RandomSelectRoad(std::vector<IntersectionOption *> *,IntersectionMarkerEntity *);
void ProcessPlayerState(zwiftUpdateContext &,protobuf::PlayerState const&,float);
void PreRender(void);
void PostBloomRender(void);
void PositionInTTEventPaddock(int,VEC3,float,bool,int,bool);
void PositionInEventPaddock(int,VEC3,float,int,int,bool,int);
void PlayerProfileUpdate(protobuf::PlayerProfile const&,bool);
void PlayerCalcAim(float);
void PlayerBikeLimitedProfileUpdate(protobuf::PlayerProfile const&);
void PlayerBikeFullProfileUpdate(protobuf::PlayerProfile const&);
void PerformUTurn(void);
void OneTimeXpToDropsConversion(bool);
void OneTimePowerUserLevelingSelection(void);
void OneTimeNewAccessoriesUnlock(void);
void OnWake(void);
void OnSleep(zwiftUpdateContext &);
void OnRoadCalcAim(float);
void OnCalculateETAJobFinished(int,JobData *);
void OffRoadCalcAim(float);
void NetworkedBikeProfileUpdate(protobuf::PlayerProfile const&);
void NetworkDelayRoadCalcAim(float);
void NearbyRiderPathVoting(float);
void MoveLoiteringPlayer(float);
void MapDirectionToSelection(std::vector<IntersectionOption> const*,int);
void LoadNewRouteComputer(int,bool,bool);
void LoadMeshes(void);
void JoinWorld(long,bool);
void IsSameSportAsPlayer(BikeEntity const*);
void IsRightTraffic(void);
void IsProPlayerOrZwiftStaff(void);
void IsPowerupAllowed(POWERUP_TYPE);
void IsPacerBot(BikeEntity const*);
void IsOnroad(void);
void IsOffroad(void);
void IsNetworkedNinja(void);
void IsHandCycle(void);
void IsEventRiderStationary(void);
void IsDialogOfTypeOpen(UI_DIALOGS);
void IsCurrentlyReceivingARideOn(void);
void IsBrakeLightEnabled(void);
void IsBrakeLightActive(void);
void IsBikeStillOnEventPaddock(void);
void IsAheadOnSameRoad(BikeEntity const*,BikeEntity const*);
void InterpolateSplineDistance(void);
void InitializeBike(void);
void InitAnim(void);
void HasLocalRiderDelayedPacket(void);
void GivePowerUp(POWERUP_TYPE);
void GetWKG(void);
void GetVRHeadPosition(void);
void GetSocksAccessoryOverride(bool,AccessoryConfig const*,bool,ulong long *);
void GetShouldUseSkillLevel(void);
void GetServerDirection(void);
void GetSelectedRoute(void);
void GetRoadId(void);
void GetRiderHeightCM(void);
void GetRearDerailleurOverride(void);
void GetProximity(Proximity::Axis const&);
void GetPlayerRunSpeed(void);
void GetPlayerLevel(void);
void GetPlayerAge(uint,uint,uint);
void GetParentEventID(void);
void GetPaintJobOverrideTextureHandle(int *,int);
void GetPacketAgeInSeconds(void);
void GetPace(void);
void GetNetworkQualityModifier(void);
void GetMaxHR(void);
void GetKitTextureHandle(AccessoryConfig const*,int *,int *);
void GetInstantYaw(void);
void GetHolidayWheelOverride(void);
void GetHolidayBikeIDOverride(uint,bool);
void GetGlassesAccessoryOverride(Accessory const*);
void GetForceLOD(void);
void GetFlagTexture(void);
void GetFTP(bool *);
void GetEventID(void);
void GetCurrentRoadType(void);
void GetCurrentGarageBikeFrameHash(void);
void GetBikeWheel(uint,Accessory::Type);
void GetBikeWeightKG(void);
void GetBikeRollingResistance(void);
void GetBikePresetConfig(uint);
void GetBikeModifiedBrakingForce(void);
void GetBikeCdABias(void);
void GetAnimationReferenceMesh(zwift::entity::bike::PLAYER_BODY_TYPES);
void GetAnimDuration(int const&);
void GetAnim(int const&);
void FillAvatarAssets(void);
void EstimatePositionXMetersAhead(float,RoadSegment **,VEC3 *);
void EstimatePositionNSecondsAhead(float,float,RoadSegment **,float *,VEC3 *,float *);
void EstimatePositionNSecondsAhead(float,RoadSegment **,float *,VEC3 *,float *);
void DropsContainer::DropsContainer(void);
void DrawRider(bool,uint,float,bool);
void DrawDisplayWheels(uint,uint,float);
void DrawDebugProximity(void);
void DrawDebugInfo(void);
void DrawBike(bool,uint,bool,bool,uint,uint,uint,bool,float,uint,float);
void DoPhysics(float);
void DoFirstTime(bool);
void DoCalculateETAJob(JobData *);
void DidGetRideOnFrom(long long);
void DestroyLights(void);
void CreateNewPacket(zwiftUpdateContext &,float);
void CreateNamePlate(bool);
void CreateLights(void);
void CreateBikeMatrix(void);
void CleanupProximity(void);
void CheckOnboardProgress(BikeEntity::ONBOARD_CARD);
void CheckHolidayProgress(void);
void CanUTurn(void);
void CalculateETAInSeconds(double,double,std::atomic<double> *,std::atomic<double> *);
void CalculateCassetteLOD(uint,float,float);
void ByPassBikeOverride(void);
void BroadcastAction(BikeEntity::USER_ACTION);
void BikeAnimCallback(AnimTuning_Event *,void *);
void AutoSelectRoad(IntersectionMarkerEntity *,float);
void ApplyHeadingClamps(float,float &,float &);
void AdjustSideways(float);
void AdjustJoinPosition(float,float);
void AdjustForward(float);
void AccumulateProximity(BikeEntity const&)    */;
};

inline int64_t g_GroupEventsActive_CurrentEventId;
