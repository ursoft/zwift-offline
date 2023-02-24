#pragma once
enum EGameMode { GM_CNT };
inline bool g_bMapSchedule_DLFailed, g_bMapSchedule_Ready, g_UseErgModeInWorkouts, g_ShowGraph;

void GAME_onFinishedDownloadingMapSchedule(const std::string &, int err);
void GAME_GetSuppressedLogTypes();
void GAME_SetUseErgModeInWorkouts(bool);
void GAME_Initialize();
void GAME_AddMode(EGameMode);
void GAME_AudioUpdate(GameWorld *, Camera *, float);
void GAME_CancelWorkout();
//void GAME_CheckEventBikeFrameChange(bool, GroupEvents::SubgroupState *)
void GAME_CheckForKonaHeatItUp_Unlocks(BikeEntity *, float, float);
void GAME_CheckForSpecializedMixpanelUnlocks(bool);
void GAME_CheckForWahooClimbingMissionUnlocks(float);
/*void GAME_CheckForZwiftAcademyExtraCreditUnlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftAcademyTri2020Unlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftAcademyUnlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftCampaignOTMUnlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftCampaignRWRUnlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftCampaignTDZUnlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftCampaignTOEUnlocks(zwift::protobuf::CampaignSummaryProtobuf)
void GAME_CheckForZwiftCampaignTOWUnlocks(zwift::protobuf::CampaignSummaryProtobuf)*/
void GAME_CleanupTempFiles();
void GAME_ClearScreenshot();
void GAME_ClearScreenshotUploadQueue();
/*void GAME_ClubParticipationAwardHandler::Render();
void GAME_ClubParticipationAwardHandler::TriggerCompletion();
void GAME_ClubParticipationAwardHandler::Update(float)
void GAME_ClubParticipationAwardHandler::~GAME_ClubParticipationAwardHandler()*/
void GAME_DecreaseFlatRoadTrainerResistance();
void GAME_DeleteSavedStateSnapshot();
void GAME_Destroy();
void GAME_DisableAllScreenshots();
//void GAME_DoReturnToWorldDialog(uint32_t, uint32_t, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS))
//void GAME_DoSounds(Workout::Event const *, float)
void GAME_DrawEventFinishLine(float);
void GAME_DrawFinishMiniMap(const VEC3 &, const float &);
void GAME_DrawWarningNotifications(float);
//void GAME_EnqueueScreenshotUpload(GAME_Screenshot const *)
void GAME_EventSystemCallback(EVENT_ID, va_list);
void GAME_FetchLevelFromRoute(uint32_t);
//void GAME_FinishedGroupEvent(GroupEvents::SubgroupState *)
//void GAME_FinishedGroupWorkout(GroupEvents::SubgroupState *)
void GAME_FlatRoadTrainerResistanceWithinRangeValue();
void GAME_GODUpdate(float);
void GAME_GetCurrentPlayerSport();
void GAME_GetCurrentPlayerTrainingMetric();
void GAME_GetCurrentPlayerTrainingMetric();
void GAME_GetDistanceToFinishInMeters();
void GAME_GetLastScreenshotTakenType();
void GAME_GetLogLevel();
void GAME_GetMapForTime(const uint32_t *);
void GAME_GetMapForTime_New(const uint32_t *);
void GAME_GetMemoryWarningCount();
void GAME_GetScreenShotFromUploadQueue(uint32_t);
//void GAME_GetScreenshot(GAME_Screenshot const *&)
void GAME_GetScreenshotUploadQueueSize();
void GAME_GetSteeringMode();
void GAME_GetTimeToFinishInSeconds();
void GAME_GetTrainerSlopeModifier();
void GAME_GetTrainerSlopeModifierBackup();
void GAME_GetUserPreferredRoute(int);
void GAME_GetWorldData(int);
//void GAME_GetWorldFull(GroupEvents::SubgroupState *)
void GAME_GetWorldTime(int &, int &);
void GAME_GivePlayerBatteryCharge();
//void GAME_GivePlayerPowerup(POWERUP_TYPE);
void GAME_GivePlayerRandomPowerup();
//void GAME_GroupEventActuallyBegan(GroupEvents::SubgroupState *)
//void GAME_GroupEventCallback(float, GroupEvents::GroupEventMessage, GroupEvents::SubgroupState *)
//void GAME_HandleCompletedTrainingPlan(TrainingPlanEnrollmentInstance *);
void GAME_HasSavedStateSnapshot();
void GAME_InMode(EGameMode);
void GAME_IncreaseFlatRoadTrainerResistance();
//void GAME_InitMarketingBox(GUI_Obj *)
//void GAME_Internal_EquipMovemberItem(Accessory::Type, uint32_t)
void GAME_IsAfterMapTime(time_t *, time_t *);
void GAME_IsBrakingAvailable();
void GAME_IsRealCadence();
void GAME_IsSteeringAvailable();
void GAME_IsSteeringRoad(int, int);
void GAME_IsUsingPhoneSteering();
void GAME_IsWorldIDAvailableViaPrefsFile(int);
void GAME_JoinEvent(int64_t);
void GAME_JoinNextWeeksEvent();
void GAME_JoinPendingEvent();
void GAME_KillAllTutorials();
void GAME_LoadFinishLineMesh(int &);
void GAME_LoadLevel(int);
void GAME_MaxStoredScreenshots();
void GAME_NumberStoredScreenshots();
void GAME_OnEventEnd();
void GAME_OnMemoryWarning();
//void GAME_OnStartedAGroupEvent(GroupEvents::SubgroupState *);
void GAME_OpenRouteNotFoundDialog();
void GAME_PlacePlayerAtStartGate(uint32_t, int64_t, uint32_t);
void GAME_PlayerActivatePowerup();
//void GAME_PlayerPassedFinish(TimingArchEntity *)
void GAME_ProcessInputComponents();
//void GAME_QueueScreenshot(GAME_ScreenshotParams const &)
void GAME_QuitGroupEvent();
void GAME_ReEnableAllScreenshots();
void GAME_RemoveEventLineupPromptDialog();
void GAME_RemoveMode(EGameMode);
void GAME_RenderWorkoutTopBar(float, float, float, float, void *);
//void GAME_RequestMemorableScreenShot(NOTABLEMOMENT_TYPE, bool, float)
void GAME_ResetEventFinishLineParams();
void GAME_ResetScreenshotsForActivity();
void GAME_RestoreFitFile();
void GAME_RestoreSavedStateSnapshot();
//void GAME_RestoreTrainerDifficultyAfterEvent(BikeEntity *, GroupEvents::SubgroupState *);
void GAME_SaveStateSnapshot(bool);
void GAME_SaveStockImage(uint32_t, int, int64_t);
void GAME_SaveStockImage_Finish();
//void GAME_ScreenshotParams::Default(ScreenshotSource);
//void GAME_ScreenshotParams::GAME_ScreenshotParams(GAME_ScreenshotParams const &);
void GAME_SetBrakingAvailable(bool);
//void GAME_SetChangeTrainerEffectSignal(std::weak_ptr<UI_Refactor::ChangeTrainerEffectSignal>);
void GAME_SetMode(EGameMode);
void GAME_SetSaveGameSettings(std::string, uint64_t, uint64_t, uint64_t, uint64_t);
void GAME_SetSnapshotConsumed(bool);
void GAME_SetSteeringAvailable(bool);
//void GAME_SetSteeringMode(SteeringMode, bool);
void GAME_SetTrainerSlopeModifier(float);
void GAME_SetTrainerSlopeModifierBackup(float);
void GAME_SetVRViewPosDirection(VEC3, VEC3);
void GAME_SetWorldTime(float);
void GAME_ShouldAskUserToRateApp(bool);
void GAME_ShouldDrawEventFinishLine();
void GAME_ShouldUseErgModeInWorkouts();
void GAME_Shutdown();
void GAME_ShutdownProfile();
void GAME_StartRideTutorial();
//void GAME_StartWorkout(Workout *);
//void GAME_StartupGroupWorkout(Workout *, float);
void GAME_SwitchLevel(int);
//void GAME_TakeScreenshot(const GAME_Screenshot *, ScreenshotContent);
void GAME_TakeScreenshotThisFrame();
void GAME_UnloadLevel(int);
//void GAME_Update(zwift::context::UpdateContext &, float)
void GAME_UpdateViewProjMatrices();
void GAME_UserExit(bool);
/*void GAME_glue_RunningPRs_EarlyWarning::OnBrokePR(bool, BestTimeForDistanceEntry *)
void GAME_glue_RunningPRs_to_Leaderboards::HandleEvent(EVENT_ID, std::__va_list)
void GAME_glue_RunningPRs_to_Leaderboards::OnBrokePR(bool, BestTimeForDistanceEntry *)
void GAME_glue_RunningPRs_to_Leaderboards::~GAME_glue_RunningPRs_to_Leaderboards()*/
void INTERNAL_GAME_ChangedGameMode();