#pragma once
void ZML_SendGameSessionInfo();
void ZML_StartAuxPairing();
void ZML_SendAuxPairingSelection(bool, protobuf::SportsDataSample_Type);
void ZML_EndAuxPairing();
void ZML_SendSegmentLeaderboardResult(protobuf::LeaderboardType ty, const protobuf::SegmentResult &);
bool HasPairedToZML();
void ZML_SendRemoveSegmentLeaderboardResultLive(const protobuf::SegmentResult &);
void ZML_RequestEffectPlay(protobuf::EffectRequest_Effect ef, bool, bool);
void ZML_HandleCustomButton(uint32_t);
void ZML_ReceivedWorkoutAction(protobuf::WorkoutActionRequest_Type);
void ZML_ReceivedClientAction(const protobuf::ClientAction &);
void ZML_DetonateRideOnBomb();
/*INTERNAL_ZML_GamePacketSportFromGameSport(VirtualBikeComputer::ZSPORT)
ZML_ActBusy(void)
ZML_DismissExitGame(void)
ZML_DismissSaveActivity(void)
ZML_EndAuxPairing(void)
ZML_ForceResendingWorkoutInfo(void)
ZML_GetButtonSceheme(void)
ZML_GetGameMajorMinorVersion(void)
ZML_IsConnected(void)
ZML_Meetup_NotifyInvitationReceived(void)
ZML_Meetup_NotifyJoined(ulong long)
ZML_PromptExitGame(void)
ZML_ResetAuxPairing(void)
ZML_ResetContextSync(void)
ZML_ResetSteeringCallBack(UI_TwoButtonsDialog::DIALOG_RESULTS)
inlined ZML_RestoreAuxPairing(void)
ZML_SelectAuxSport(zwift::protobuf::GamePacket_Sport)
ZML_SendBoostModeUpdate(EbikeBoostOption,float,float,float,float)
ZML_SendMobileAlert(char const*,char const*,ZML_MobileAlert_Option const*,int,uint)
ZML_SendMobileAlertCancel(int)
ZML_SendPlayerInfo(long long,bool)
ZML_SendPlayerSegmentStatus(long long,zwift::protobuf::PlayerSegmentInfo_Status,char const*)
ZML_SendPlayerStopwatchSegment(TimingArchEntity *,int,float)
ZML_SendSegmentLeaderboardResult30Day(zwift::protobuf::SegmentResult const&)
ZML_SendSegmentLeaderboardResultLive(zwift::protobuf::SegmentResult const&)
ZML_SendSegmentNewLeader(zwift::protobuf::SegmentResult)
ZML_SendTurningButtonConfiguration(void)
ZML_SetButtonScheme(ZMLBS)
ZML_SetButtonScheme(ZML_ButtonActions *,int)
ZML_SetConnectedVersion(int,int,int)
ZML_Update(float)
ZML_UpdateFullLeaderboards(bool)
ZML_UpdateWorkoutState(void)
ZML_internal_PopulateActiveSegments(void)
ZML_internal_PopulateAnnotationFromBikeEntity(zwift::protobuf::MappingAnnotation *,BikeEntity *,bool,bool)
ZML_internal_SegmentStatusTitle(zwift::protobuf::PlayerSegmentInfo_Status)
ZML_internal_SendActiveSegments(void)
ZML_internal_UpdateRiderLocationData(void)
ZML_internal_ZwifterNameFromProfile(zwift::protobuf::PlayerProfile const&)
ZML_internal_populateSegmentLeaderboardResult(zwift::protobuf::SegmentLeaderboardResult *,zwift::protobuf::SegmentResult const&)*/