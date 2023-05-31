#pragma once
using ZSPORT = protobuf::Sport;
struct RouteComputer {
    uint64_t m_selRoute = 0;
    int m_field_10 = 0, m_field_18 = 0;
    /*RouteComputer::CheckRouteLapProgress(void)
RouteComputer::DestroyDynamicArches(void)
RouteComputer::EstimateHCPPosTimeForward(float,float,uint *)
RouteComputer::FilloutRouteRecordData(RouteFinishData &)
RouteComputer::FindFutureRoadBasedOnRoute(int &,int,int,double,bool,VirtualBikeComputer::ZSPORT,int &,bool &,double &,IntersectionMarkerEntity **)
RouteComputer::FindNextDecisionForGivenRoad(int &,int,VirtualBikeComputer::ZSPORT)
RouteComputer::FixLoopingRouteHRC(void)
RouteComputer::FlipRoute(void)
RouteComputer::GetClosestHCheckpoint(VEC3 const&,bool)
RouteComputer::GetClosestHCheckpointTime(VEC3 const&,bool)
RouteComputer::GetClosestHCheckpointToHCheckpoint(uint,bool)
RouteComputer::GetCompletionPct(bool,int)
RouteComputer::GetCurrentDecision(VirtualBikeComputer::ZSPORT)
RouteComputer::GetDecisionAt(VirtualBikeComputer::ZSPORT,int)
RouteComputer::GetEventBranchDecision(int,VirtualBikeComputer::ZSPORT,Route const*)
RouteComputer::GetLeadInPct(int)
RouteComputer::GetNextHRCOnRoad(uint)
RouteComputer::GetPosAtCheckpointTime(float)
RouteComputer::GetSelectedRoute(void)
RouteComputer::GetSelectedRouteHash(void)
RouteComputer::GetTotalDistanceOfCurrentRoute(int const&)
RouteComputer::HitCheckpointCount(void)
RouteComputer::InitialiseRouteProgress(void)
RouteComputer::IsOnLeadin(void)
RouteComputer::IsRoutePerfectLoop(void)
RouteComputer::OnCompletedRoute(Route *)
RouteComputer::OnValidRoute(void)
RouteComputer::ProjectAlongRoute(VEC3,float,bool,bool *,double *,int *)
RouteComputer::Reset(void)
RouteComputer::ResetCheckpoints(void)
RouteComputer::ResetRoutePlayerTrackers(void)
RouteComputer::RetrieveNextDecision(Route const*,int &,int &,bool &)
RouteComputer::RouteComputer(BikeEntity *,int)
RouteComputer::SetDecisionStateToOffroute(void)
RouteComputer::SetRoute(Route *,bool,bool,std::string const&)
RouteComputer::SetRouteProgressTimeout(float)
RouteComputer::SetValidRouteProgress(bool)
RouteComputer::ShowRouteProgress(void)
RouteComputer::ShowingRouteCompleted(void)
RouteComputer::SpawnDynamicArch(int,double,ArchAsset const&)
RouteComputer::Update(float)
RouteComputer::UpdateCheckpointState(void)
RouteComputer::UpdateDecisionState(VirtualBikeComputer::ZSPORT)
RouteComputer::UpdateRouteProgressTimeout(float)
RouteComputer::~RouteComputer()*/
};
struct VirtualBikeComputer {
    float GetDistance(UnitType ut, bool) { /*TODO*/ return 0.0; }
    void SetTireSize(uint32_t tireCirc) { m_tireCirc = tireCirc; }
    uint64_t m_lastPower = 0;
    uint32_t m_tireCirc = 2105, m_powerSmoothing = 1, m_field_118 = 0, m_field_128 = 0;
    int32_t m_field_1E8 = 0;
    float m_distance = 0.0f, m_power = 0.0f, m_field_19C = 0.0f, m_field_188 = 0.0f, m_field_18C = 0.0f, m_field_198 = 0.0f, m_field_1E0 = 0.0f, m_sensor_f2 = 0.0f, m_total_smth = 0.0f, 
        m_heart_f2 = 0.0f;
    protobuf::Sport m_sport = protobuf::CYCLING;
    bool m_bool = false, m_bool1 = false;
};
struct Entity {
    VEC3 m_pos{};
    virtual const VEC3 &GetPosition() { return m_pos; }
};
struct SaveGame;
struct BikeEntity : public Entity { //0x1948 bytes
    struct RideOnAnim { //0x30 bytes
        float m_f0 = 0.0f, m_f1 = 0.0f, m_f2 = 500.0f, m_f3 = 0.0f;
        float m_f4 = 0.001f, m_f5 = 0.0f, m_f6 = -18.0f, m_f7 = 0.0f;
        int64_t m_fromPlayerId;
        bool m_field_28 = false, m_field_29 = false;
    };
    BikeEntity();
    int64_t m_playerIdTx = 0, m_curEventId = 0, m_cheatBits = 0;
    VirtualBikeComputer *m_bc = nullptr;
    protobuf::PlayerProfile m_profile;
    SaveGame *m_pSaveGame = nullptr;
    RouteComputer *m_routeComp = nullptr;
    PrivateAttributesHelper m_pah;
    std::list<RideOnAnim> m_rxRideonsList;
    int m_rxRideons = 0;
    float m_field_AA8 = 0.0;
    int32_t m_field_AAC = 0;
    uint32_t m_race_f14 = 0, m_fwGdeSignature = 0, m_rwGdeSignature = 0, m_yellowJersey = 0;
    protobuf::POWERUP_TYPE m_pendPU = protobuf::POWERUP_NONE;
    int32_t m_skillWKG = -1, m_x = 0, m_y_alt = 0, m_z = 0, m_eventPos = 0, m_field_11DC = 0, m_msToLeader = 0;
    float m_field_59C = 0.0f, m_field_5A0 = 1.0f;
    bool m_writable = false, m_field_C98 = false, m_race_f15 = false, m_race_f16 = false, m_sensor_f11 = false, m_field_806 = false,
        m_immuneFromCheating = false, m_boolCheatSmth = false, m_joinedWorld = false, m_field_488 = false;
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

    /* TODO:
void ~BikeEntity();
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
void Respawn(int,double,bool,bool);
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
void PerformAction(BikeEntity::USER_ACTION);
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
void ClearPowerups(void);
void CleanupProximity(void);
void CheckOnboardProgress(BikeEntity::ONBOARD_CARD);
void CheckHolidayProgress(void);
void CanUTurn(void);
void CalculateETAInSeconds(double,double,std::atomic<double> *,std::atomic<double> *);
void CalculateCassetteLOD(uint,float,float);
void ByPassBikeOverride(void);
void BroadcastAction(BikeEntity::USER_ACTION);
void BikeEntity(BikeManager &,bool,bool);
void BikeAnimCallback(AnimTuning_Event *,void *);
void AutoSelectRoad(IntersectionMarkerEntity *,float);
void ApplyHeadingClamps(float,float &,float &);
void AdjustSideways(float);
void AdjustRandomXZ(float);
void AdjustJoinPosition(float,float);
void AdjustForward(float);
void ActivatePowerUp(void);
void AccumulateProximity(BikeEntity const&)    */;
};