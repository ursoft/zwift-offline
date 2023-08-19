ZwiftAuthenticationManager vptrs:
dtr: vptr[0]
isAccessTokenInvalidOrExpired: vptr[1] (+8)
getAccessTokenHeader: vptr[2] (+16)
getRefreshTokenStr: vptr[3]
getRefreshToken: vptr[4]
getSessionStateFromToken: vptr[5]
getSubjectFromToken: vptr[6]
getOauthClient: vptr[7]
setLoggedIn: vptr[8]
getLoggedIn: vptr[9]
attendToAccessToken: vptr[10]
setRequestId: vptr[11]
getRequestId: vptr[12]
setAccessTokenAsExpired: vptr[13]
resetCredentials: vptr[14]
setCredentials: 15
setCredentialsOld 16
setCredentialsMid 17

CurlHttpConnection vptrs:
dtr: vptr[0]
setAuthorizationHeader 1
clearAuthorizationHeader 2
setTokenInfo 3
clearTokenInfo 4
setSessionIdHeader 5
setRequestId 6
setTimeout 7
getTimeout 8
setUploadTimeout 9
[10, 80] performGet
[11, 88] performDelete
[12, 96] performPostStr
[13, 104] performPostVec
[14, 112] performPost(str)
[15, 120] performPost 15
performPostWithTimeout 16
[17, 136] performPostWithHash 17
[18, 144] performPut ct,at
performPut ct 19
[20, 160] performPut ac 20
[21, 168] performPutOrPost
performPutOrPost 22
escapeUrl 23

std::_Associated_state (=std::future)
dtr: 0
_Wait(void): 1
Get_value(bool): 2
Has_deferred_function(void): 3
Run_deferred_function(class std::unique_lock<class std::mutex> &): 4
Do_notify(class std::unique_lock<class std::mutex> *,bool): 5

WorldClockService
[0,  0] dtr
[1,  8] isInitialized()
[2, 16] getWorldTime()

HashSeedService
[0,  0] dtr
[1,  8] calculateRefreshRelaySessionInAdvance(std::chrono::duration<long,std::ratio<60l,1l>>)
[2, 16] onHashSeedsReceived(std::shared_ptr<zwift_network::NetworkResponse<protobuf::HashSeeds> const> const&)
[3, 24] onRefreshRelaySession(std::shared_ptr<zwift_network::NetworkResponse<protobuf::RelaySessionRefreshResponse> const> const&)

NetworkClientImpl
[0,  0] NetworkClientImpl__handleDisconnectRequested
[1,  8] handleWorldAndMapRevisionChanged
[2, 16] dtr

CriticalPowerCurve
[0,  0] dtr
[1,  8] RecorderComponent::Init
[2, 16] CriticalPowerCurve::Update
[3, 24] CriticalPowerCurve::InitializeDrawData
[4, 32] CriticalPowerCurve::GetBestTimeAverage
[5, 40] CriticalPowerCurve::Reset
[6, 48] CriticalPowerCurve::GetDataPoints
[7, 56] RecorderComponent::GetDataPointsVEC3
[8, 64] CriticalPowerCurve::AddDataPoint
[9, 72] RecorderComponent::AddDataPointVEC3

BikeEntity
[00,  0] BikeEntity_dtr
[01,  8] ret 0 Entity::HandleEvent(EVENT_ID,std::__va_list)
[02, 16] Entity::GetPosition
[03, 24] Entity::NetworkUpdate
[04, 32] Entity::Initialize(tinyxml2::XMLElement const*)
[05, 40] Entity::InitializeFromPrefabData(tinyxml2::XMLElement const*)
[06, 48] Entity::InitializeEnd(void)
[07, 56] *::Update(float)
[08, 64] *::Render(void)
[09, 72] Entity::Render2D(void)
[10, 80] Entity::PostInitialize(void)
[11, ] Entity::PostLoadThreadedInitialize(void)
[12, ] Entity::SpecialRender(GFX_RenderPass)
[13, ] Entity::GetLastSelectedEntityType(void)
[14, ] Entity::DidInitializeCommands(void)
[15, ] Entity::GetExposedCommands(void)
[16, ] Entity::PathUpdateCallback(double,VEC3 const&,VEC3 const&,VEC3 const&,Entity*)
[17, ] Entity::SetEnabled(bool)
[18, ] Entity::IsSnapToLand(void)
[19, ] Entity::HasNonEventMesh(void)
[20, ] Entity::SwapMesh(std::string)
[21, ] Entity::SwapAnim(std::string,std::string)
[22, ] Entity::SwapTexture(char const*,int,int)
[23, ] Entity::SwapSounds(std::string,std::string)
[24, ] Entity::StopEntitySound(void)
[25, ] Entity::SetDefaultLODDistances(void)
[26, ] Entity::SetupExposedCommandsForTriggers(void)
[27, ] Entity::ReadSelectedResponses(void)
[28, ] Entity::OnNotify(std::string)
[29, ] Entity::OnTriggerRename(std::string,std::string)
[30, ] Entity::OnTriggerReadded(std::string)
[31, ] Entity::OnTriggerDeleted(std::string)
[32, ] Entity::DoesHaveChildEntities(void)
[33, ] Entity::AnimEventResponse(void)
[34, ] Entity::InitializeHolidayOverrides(tinyxml2::XMLElement const*)
[35, ] Entity::InitializeEventOverrides(tinyxml2::XMLElement const*)
[36, ] BikeEntity::PreRender(void)
[37, ] BikeEntity::CreateNamePlate(bool)
[38, ] BikeEntity::Update(zwift::context::UpdateContext &,float)
[39, 312] BikeEntity::UpdateAnimation(float)
[40, ] BikeEntity::UpdateRoadContactElev(float,bool)
[41, ] BikeEntity::CreateLights(void)
[42, ] BikeEntity::DestroyLights(void)
[43, ] BikeEntity::IsRightTraffic(void)
[44, ] BikeEntity::PlayerBikeFullProfileUpdate(zwift::protobuf::PlayerProfile const&)
[45, ] BikeEntity::PlayerBikeLimitedProfileUpdate(zwift::protobuf::PlayerProfile const&)
[46, ] BikeEntity::NetworkedBikeProfileUpdate(zwift::protobuf::PlayerProfile const&)
[47, ] BikeEntity::UnlockDigitalEntitlements(void)
[48, ] BikeEntity::WakeupAnim(void)
[49, ] BikeEntity::JoinWorld(long,bool)
[50, ] BikeEntity::SendPlayerState(long,zwift::protobuf::PlayerState const&)
[51, ] BikeEntity::IsDialogOfTypeOpen(UI_DIALOGS)
[52, ] BikeEntity::ResetRoadDirVec(void)
[53, ] BikeEntity::RegisterTailLightParticles(void)
[54, ] BikeEntity::RegisterTurnSignalParticles(void)

RoadManager
[0,  0] - dtr
[1,  8] - GetRoadSegment(int)
[2, 16] - GetRoadCount(void)

RouteManager
[0] GetRoute

RoadSegment (abstract), RoadSegmentPlaceholder:RoadSegment(*=RoadSegment), RoadSegmentWorld:RoadSegment(*=RoadSegmentWorld)
[000, 000] dtr, =, RoadSegmentWorld_dtr
[001, ] pure_virtual, RoadSegmentPlaceholder::IsPlaceholder:ret1, RoadSegmentWorld::IsPlaceholder: ret0
[002, ] pure_virtual, RoadSegmentPlaceholder::IsAvailable:ret0, RoadSegmentWorld::IsAvailable(void)
[003, ] pure_virtual, RoadSegmentPlaceholder::IsSportAllowOnRoad:ret0, RoadSegmentWorld::IsSportAllowOnRoad(ulong long)
[004, ] pure_virtual, RoadSegmentPlaceholder::GetSportAllowed:ret1, RoadSegmentWorld::GetSportAllowed(void)
[005, ] pure_virtual, RoadSegmentPlaceholder::IsPaddock: ret0, RoadSegmentWorld::IsPaddock(void)
[006, ] *::FindClosestPointOnRoad(VEC3 const&,double,int)
[007, ] *::FindClosestPointOnRoadF(VEC3 const&,float,int)
[008, ] *::FindClosestPointOnRoad2D(VEC3,float,int), =,
[009, ] *::CalcPosByShortDistWithOffset(double,double,double,double,VEC3 *)
[010, ] *::CalculateRoadPositionByDist(double,double,VEC3 *)
[011, ] *::CalculateRoadPositionAtTime(double,bool)
[012, ] *::CalculateRoadPositionAtTimeF(float,bool)
[013, ] *::GetRoadWidth(double,bool,float *,float *,RoadMarkerEntity const**)
[014, ] *::GetRoadOffset(double)
[015, ] *::GetShoulderHalfWidth(double)
[016, ] *::GetEdgeHalfWidth(double)
[017, ] *::GetLoiterHalfWidth(double)
[018, ] *::GetRoadColor(double)
[019, ] *::GetMarkerInfluence(double,RoadMarkerEntity const*)
[020, ] *::GetCyclistAltitudeAtRoadTime(VEC3 const&,double,bool)
[021, ] *::GetAbsoluteSplineTimeOffsetFromAtoB(double,double,bool)
[022, ] *::UpdateCyclistWheelAltitudesAtRoadTime(VEC3 &,double,VEC3 &,double,bool)
[023, ] *::Render(GFX_RenderPass)
[024, ] *::NavRender(VEC4 const&,double,double)
[025, ] *::GenerateRoadMesh(void)
[026, ] *::GenerateNavRoadMesh(void)
[027, ] *::Load(tinyxml2::XMLElement *)
[028, ] *::PostInitialize(void)
[029, ] *::CalculateSplinePoints(void)
[030, ] *::GetNeighboringRoadNodes(RoadNode *,RoadNode **,RoadNode **)
[031, ] *::IsPointInRoadRegion(VEC3 const&)
[032, ] *::IsPointInRoadRegion2D(VEC3 const&)
[033, ] *::GetNodeBasedOnRoadTime(double)
[034, ] *::GetRoadNodesPtr(void)
[035, ] *::GetRoadNodeCount(void)
[036, ] *::GetRoadNodeAt(int)
[037, ] *::GetRoadNodeAt(int)
[038, ] *::GetFirstNode(void)
[039, ] *::GetFirstNode(void)
[040, ] *::GetSecondNode(void)
[041, ] *::GetSecondNode(void)
[042, ] *::GetLastNode(void)
[043, ] *::GetLastNode(void)
[044, ] *::GetSecondLastNode(void)
[045, ] *::GetSecondLastNode(void)
[046, ] *::IsFirstNode(RoadNode const*)
[047, ] *::IsSecondNode(RoadNode const*)
[048, ] *::IsSecondLastNode(RoadNode const*)
[049, ] *::IsLastNode(RoadNode const*)
[050, ] *::IndexOfNode(RoadNode const*)
[051, ] *::IsPreviousNodeStraight(RoadNode const*)
[052, ] *::GetRoadPointLinearDistanceAt(int)
[053, ] *::GetSmallDistanceRoadTime(void)
[054, ] *::Update(float)
[055, ] *::ConvertDistanceToRoadTime(float)
[056, ] *::ConvertRoadTimeToDistance(double)
[057, ] *::GetMinimumTessellationDistance(void)
[058, ] *::GetMaxInstersectionTime(bool,ulong long,uint)
[059, ] *::GetRoadMarkersPtr(void)
[060, ] *::GetRoadMarkers(void)
[061, ] *::AddMarkers(Entity *,int *)
[062, ] *::IsRoadInvisible(double)
[063, ] *::GetStyleIndex(double,uint &,uint *)
[064, ] *::GetFirstRoadMarkerBasedOnRoadTime(double,bool)
[065, ] *::GetClosestRoadMarkerBasedOnRoadTime(double,uint *)
[066, ] *::GetAllRoadMarkersBasedOnRoadTime(double,std::vector<RoadMarkerEntity const*> *)
[067, ] *::GetIntersectionMarkerBasedOnRoadTime(double)
[068, ] *::GetClosestIntersectionMarkerToRoadTime(double,uint *)
[069, ] *::GetClosestForwardIntersectionMarkerToRoadTime(double,bool,uint *)
[070, ] *::GetClosestIntersectionMarkerToRoadTimeWithValidOption(double,bool,uint *)
[071, ] *::GetClosestForwardIntersectionMarkerToRoadTimeWithValidOption(double,bool,uint *)
[072, ] *::GetIntersectionMarkerBasedOnId(int)
[073, ] *::GetCourseDistanceAtSplineTime(double,double *)
[074, ] *::GetSplineTimeAtCourseDistance(float)
[075, ] *::GetCourseDistanceBetweenSplineTimes(double,double,bool,double *)
[076, ] *::GetCourseHeightAtDistance(float)
[077, ] *::GetDefaultWidth(void)
[078, ] *::GetDefaultOffset(void)
[079, ] *::GetDefaultShoulderWidth(void)
[080, ] *::GetRiderBoundsRatio(void)
[081, ] *::GetCenterRightRatio(void)
[082, ] *::GetCourseLength(void)
[083, ] *::GetMinAltitude(void)
[084, ] *::GetMaxAltitude(void)
[085, ] *::GetMinExtents(void)
[086, ] *::GetMaxExtents(void)
[087, ] *::GetMaxGradient(void)
[088, ] *::GetMetersClimbed(void)
[089, ] *::GetCurvePointCount(void)
[090, ] *::GetGlobalNodeVisualOffset(void)
[091, ] *::GetPhysicSlope(void)
[092, ] *::IsLooped(void)
[093, ] *::SnapMarkersToTesselation(void)
[094, ] *::IsOneWay(void)
[095, ] *::AllowFullWidthSteering(void)
[096, ] *::GetDefaultRoadStyleIndex(void)
[097, ] *::IsRightTraffic(void)
[098, ] *::GetTesselationCount(void)
[099, ] *::SetBoundingRegion(VEC3 const&,VEC3 const&)
[100, ] *::ShouldDrawTransparentInNav(int)
[101, ] *::GetNavDrawLayer(void)
[102, ] *::SetSportAllowOnRoad(ulong long,bool)
[103, ] *::GetRoadTextureLength(void)
[104, ] *::GetRoadTextureWidthScale(void)
[105, ] *::SetRoadTextureLength(float)
[106, ] *::SetRoadTextureWidthScale(float)
[107, ] *::GetRoadSamplePoints(void)
[108, ] *::GetSpeedGates(void)
[109, ] *::AddSpeedGate(SpeedGateEntity *)
[110, ] *::CheckForSpeedGates(float,float,bool)
[111, ] *::RecordRiderOffCourse(float,bool)
[112, ] *::ResetSpeedGates(void)
[113, ] *::SetGatesReady(bool)
[114, ] *::AddPOI(POIEntity *)
[115, ] *::CheckForPOI(float)
[116, ] *::GetPaddockExitRoadTime(void)
[117, ] *::GetSplineType(void)
[118, ] *::SetSplineType(RoadSegmentSplineType::Type)
[119, ] *::SetDefaultStyleColor(uint)
[120, ] *::GetDefaultStyleColor(void)
[121, ] *::SetTintColor(uint)
[122, ] *::GetTintColor(void)
[123, ] *::GetRiderOffsetBlendDistance(void)
[124, ] *::SetRiderOffsetBlendDistance(float)
[125, ] *::GetDefaultLoiterOffset(void)
[126, ] *::SetDefaultLoiterOffset(float)
[127, ] *::GetRoadEdge(VEC3 const&,VEC3,double,float)

CameraManager
[0] dtr
[1] GetSelectedCamera

Camera()
[0] dtr
[1] Update(float, float)
[2] Entity *GetLookAtEntity()

XMLDoc
[0] dtr
[1] Load
[2] SaveAs(char const*,bool)
[3] UserLoad
[4] UserSaveAs(char const*,bool)
[5] BufferLoad

XMLNode

BLEModule
[0] dtr
[1] DidReceiveError
[2] DidRecover
[3] DidConnect
[4] ReceivedRSSI
[5] PairDevice(std::string)
[6] ProcessDiscovery
[7] ProcessBLEResponse

BLEModule::LegacyBLEImpl
[00]  ~LegacyBLEImpl()
[01] InitializeBLE
[02] HasBLE
[03] IsBLEAvailable
[04] IgnoreReceivedBluetoothPackets(bool)
[05] EnableDeviceDiscovery(bool)
[06] SetDeviceConnectedFlag(protobuf::BLEPeripheralResponse const&,bool)
[07] StartScan(protobuf::BLEPeripheralRequest const&)
[08] StopScan
[09] IsScanning
[10] IsAnyDeviceConnecting
[11] StartSearchForLostDevices
[12] StopSearchForLostDevices(void)
[13] IsRecoveringLostDevices(void)
[14] PairDevice(BLEDevice const&) PairDevice_bd=ret0
[15] PairDevice(std::string const&)
[16] UnpairDevice(BLEDevice const&)
[17] SendValueToDevice(protobuf::BLEPeripheralRequest const&,BLE_SOURCE)
[18] ProcessDiscovery(protobuf::BLEAdvertisement const&,BLE_SOURCE)
[19] ProcessAdvertisedServiceUUIDs=ret0
[20] ProcessAdvertisementManufacturerData(protobuf::BLEAdvertisement const&,std::string const&,BLE_SOURCE)
[21] ProcessBLEResponse(protobuf::BLEPeripheralResponse const&,BLE_SOURCE)
[22] GetRSSI(char const*)=ret0
[23] ReceivedRSSI(int,char const*,char const*)
[24] DoHardwarePrompt(void)
[25] IsAutoConnectPairingOn(void)
[26] SetAutoConnectPairingMode(bool)
[27] DidReceiveError(char const*,char const*,BLE_ERROR_TYPE,uint)
[28] DidRecover(char const*,char const*)
[29] DidConnect(char const*,char const*)
[30] PurgeDeviceList(void)
[31] StartBackgroundBLECommunication(void)=ret0
[32] StopBackgroundBLECommunication(void)=ret0

BLEDeviceManager
[00] BLEDeviceManager_dtr_0
[01] _purecall
[02] StartScan
[03] StopScan
[04] PurgeDeviceList
[05] SendValueToDevice
[06] StartSearchForLostDevices
[07] StopSearchForLostDevices
[08] PairDevice
[09] UnpairDevice

ExerciseDevice
[00] dtr
[01] Pair(bool)
[02] UnPair(void)
[03] Destroy(void)
[04] IsPaired(void)
[05] IsActivelyPaired(void)
[06] Update(float)
[07] StartWorkout(void)
[08] EndWorkout(void)
[09] FirmwareVersionCheck(void (*)(eCheckResult,sDeviceFirmwareInfo *,void *),void *)
[10] FirmwareUpdate(FirmwareUpdate::eUserChoice,void (*)(eUpdateResult,sDeviceFirmwareInfo *,void *),void *)
[11] GetFirmwareUpdateProgress(void)
[12] GetPrefsID(void)

BLEDevice : ExerciseDevice
[13] SetPaired(bool)
[14] ProcessBLEData(zwift::protobuf::BLEPeripheralResponse const&)

SarisControlComponent 0x78 bytes, CPT_CTRL: TrainerControlComponent, DeviceComponent

public SensorValueComponent: CPT_SPD, CPT_RUN_SPD, CPT_RUN_CAD, CPT_5, CPT_CAD, CPT_PM
  public /* offset 0x8 */ DeviceComponent
[0] GetSensorType

public FECTrainerControlComponent : //CPT_CTRL
  public /* offset 0x0 */ TrainerControlComponent :
    public /* offset 0x8 */ DeviceComponent
[00] FECTrainerControlComponent::ProcessANTEvent(uchar)
[01] FECTrainerControlComponent::InitSpindown(void)
[02] FECTrainerControlComponent::SetERGMode(int)
[03] FECTrainerControlComponent::SetRoadMode(void)
[04] FECTrainerControlComponent::SetSimulationMode(void)
[05] FECTrainerControlComponent::SupportsRoadTexture(void)
[06] FECTrainerControlComponent::SetRoadTexture(RoadFeelType,float)
[07] TrainerControlComponent::SetGradeLookAheadSecs(float)
[08] FECTrainerControlComponent::SetSimulationGrade(float)
[09] FECTrainerControlComponent::SetWindSpeed(float)
[10] TrainerControlComponent::Update(float)
[11] TrainerControlComponent::OnPaired(void)
[12] TrainerControlComponent::SetRiderWeightKG(float)
[13] TrainerControlComponent::SetGearingShiftType(GearingType)
[14] TrainerControlComponent::ShiftCassetteCog(ShiftDirection)
[15] TrainerControlComponent::ShiftChainRing(ShiftDirection)
[16] TrainerControlComponent::Shift_EZTap_Event(bool,ShiftDirection)
[17] TrainerControlComponent::Shift_ZTap_Event(bool,ShiftDirection)

StaticAssetLoader
[00] dtr
[01] StaticAssetLoader::LoadGdeFile(char const*,bool) = LOADER_LoadGdeFile
[02] StaticAssetLoader::CreateTextureFromTGAFile(char const*)
[03] StaticAssetLoader::CreateShaderFromFile(char const*)
[04] StaticAssetLoader::AddMesh(InstancedObjects *,char const*,ulong long)
[05] StaticAssetLoader::SkinGdeFile(std::string const&)
[06] StaticAssetLoader::GetMesh(int)
[07] StaticAssetLoader::GetDefaultNormalTexture(Accessory::Type,Gender)
[08] StaticAssetLoader::LoadWADFile(const char *)
