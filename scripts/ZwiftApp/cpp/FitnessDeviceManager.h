#pragma once
enum BLE_SOURCE { BLES_BUILTIN = 0, BLES_ZCA = 1, BLES_2 = 2, BLES_WFTN = 3 };
enum BLE_ERROR_TYPE { BLER_0 = 0 };
enum DeviceProtocol { DP_ANT, DP_BLE, DP_SERIAL, DP_BLE_MOBILE, DP_WATCH, DP_WAHOO_ANT, DP_ZML, DP_USB, DP_WIFI, DP_UNKNOWN, DP_CNT };
struct ExerciseDevice;
struct BLEDevice;
struct WFTNPDeviceManager {
    static void WriteCharacteristic(const protobuf::BLEPeripheralRequest &);
    static void UnPair(BLEDevice *);
    /*void AddDevice(uint, std::string const &, std::string const &, std::vector<uchar> const &, std::vector<uchar> const &);
    void AdvertisePreviouslyDiscovered(float);
    void BinaryUuidToShortID(std::vector<uchar> const &, ulong);
    void BinaryUuidToString(std::vector<uchar> const &, ulong);
    void CharacteristicIsSupported(std::vector<uchar> const &, int, std::vector<std::string> const &);
    void DiscoverServices(uint);
    void DisplayConnectionErrorUI(char const *, char const *, char const *, char const *);
    void GetIDByNetworkHandle(uint);
    void GetIDBySerial(std::string const &);
    void HandleConnect(uint);
    void HandleDisconnect(uint);
    void HandleDiscoverCharacteristics(uint, std::vector<uchar> const &);
    void HandleDiscoverServices(uint, std::vector<uchar> const &);
    void HandleError(zwift_network::LanExerciseDeviceInfo const &);
    void HandleNoSignal(BLEDevice *);
    void IsTimeToAdvertise(float);
    void Pair(BLEDevice *);
    void ProcessMessage(uint, std::vector<uchar> const &);
    void SendBLEResponseToGame(WFTNPDeviceState *, std::vector<uchar> const &, zwift::protobuf::BLEPeripheralResponse_PeripheralResponseType);
    void SendMessage(uint, uchar, uchar, std::vector<uchar> &);
    void ServiceIsSupported(std::vector<uchar> const &, int, zwift::protobuf::BLEPeripheralRequest const &);
    void ShouldUseNewCode(void);
    void StringUuidToBinary(std::string const &, std::vector<uchar> &, ulong);
    void UpdateDevices(float);*/
};
struct DeviceComponent { //24 bytes?
    ExerciseDevice *m_owner = nullptr;
    enum ComponentType { CPT_SPD = 0, CPT_CAD = 1, CPT_RUN_SPD = 2, CPT_RUN_CAD = 3, CPT_HR = 4, CPT_5 = 5, CPT_PM = 6, CPT_7 = 7, CPT_CTRL = 8, CPT_STEER = 16 } m_type;
    int m_packId = -999, m_packTs = 0;
    bool m_bInitState = true;
    DeviceComponent(ComponentType ct) : m_type(ct) {}
    ExerciseDevice *GetOwner() { return m_owner; }
    int GetPacketId() { return m_packId; }
    int GetPacketTimeStamp() { return m_packTs; }
    ComponentType GetType() { return m_type; }
    const char *GetTypeName(ComponentType ct);
    void InitialStateProcessed() { m_bInitState = false; }
    bool IsInitialState() { return m_bInitState; }
    void SetOwner(ExerciseDevice *p) { m_owner = p; }
    void SetPacketId(int id) { m_packId = id; }
    void SetPacketTimeStamp(int ts) { m_packTs = ts; }
};
struct ExerciseDevice { //0x290 bytes
    char m_name[256]{};
    std::vector<DeviceComponent *> m_components;
    //OMIT std::vector<std::string> m_devAnalytics;
    std::string m_str1, m_str2, m_str3, m_str4, m_str5;
    std::mutex m_mutex;
    uint32_t m_last_time_ms = 0, m_rssiTime = 0;
    int m_rssi = 0;
    uint32_t m_prefsID = (uint32_t)-1;
    DeviceProtocol m_protocol = DP_UNKNOWN;
    bool m_hidden = false, m_is560017 = false;
    void UpdateTimeStamp(uint32_t ts) { m_last_time_ms = ts; }
    int GetTimeSinceLastSeen(uint32_t ts) { return int(ts - m_last_time_ms); }
    void SetSignalStrength(int rssi) {
        m_rssiTime = timeGetTime();
        m_rssi = rssi;
    }
    enum SignalStrengthGroup { LOW, LOW_ONE_BAR, LOW_TWO_BARS, NORM_3_BARS, GOOD_4_BARS };
    SignalStrengthGroup GetSignalStrengthGroup();
    bool IsHidden() { return m_hidden; }
    bool SetHidden(bool h) { m_hidden = h; }
    ExerciseDevice();
    virtual ~ExerciseDevice() {}               //[00]
    virtual void Pair(bool p) {}               //[01]
    virtual void UnPair() {}                   //[02]
    virtual void Destroy() {}                  //[03]
    virtual bool IsPaired() const { return false; }   //[04]
    virtual bool IsActivelyPaired() { return false; } //[05]
    virtual void Update(float) = 0;            //[06]
    virtual void StartWorkout() {}             //[07]
    virtual void EndWorkout() {}               //[08]
    //OMIT virtual void FirmwareVersionCheck(void (*)(eCheckResult,sDeviceFirmwareInfo *,void *),void *) [9]
    //OMIT void FirmwareUpdate(FirmwareUpdate::eUserChoice,void (*)(eUpdateResult,sDeviceFirmwareInfo *,void *),void *) [10]
    //OMIT void GetFirmwareUpdateProgress(void) [11]
    virtual uint32_t GetPrefsID() { return m_prefsID; } //[12]
    std::vector<DeviceComponent *> &GetComponentList() { return m_components; }
    DeviceComponent *FindComponentOfType(DeviceComponent::ComponentType ct) {
        std::lock_guard l(m_mutex);
        for (auto c : m_components)
            if (c->m_type == ct)
                return c;
        return nullptr;
    }
    void RemoveComponent(DeviceComponent *devComp) {
        std::erase_if(m_components, [devComp](const DeviceComponent *i) { return i == devComp; });
    }
    void AddComponent(DeviceComponent *devComp);
    /* later:
    void ProcessUserInput_WattBike(const uint8_t *data, uint32_t len) {
    }
void ProcessUserInput_KickrBike(uchar const*)
void ProcessUserInput_StagesBike(uchar const*,uint)
OMIT:
void AddDeviceAnalytics(std::string const &key, std::string const& val)
void GetDeviceAnalytics(std::vector<std::string> &)
 */
};
struct ZMLAUXDevice : public ExerciseDevice { //0xb28 bytes
    bool IsPaired() const;
    void Pair(bool p);
    //UnPair();
    //Update(float);
    //ZMLAUXDevice(bool, bool);
    ~ZMLAUXDevice();
    void Update(float) override;
};
struct EliteWiredTrainerDevice : public ExerciseDevice { //0x2f8 bytes
    void Update(float) override;
};
struct CompuTrainerDevice : public ExerciseDevice { //0x3a0 bytes
    void Update(float) override;
};
struct AntDevice : public ExerciseDevice { //0xb90 bytes
    uint16_t m_prefsAntID;
    uint8_t m_may17;
    void Update(float) override;
};
struct EliteSCPowerDevice : public AntDevice { //0xb98 bytes

};
struct DeviceDbItem { //0x18 bytes
    char *m_name;
    DeviceProtocol m_protocol;
    uint32_t m_hash, m_mfgModel;
};
struct FitnessDeviceManager {
    static inline protobuf::Sport m_PairingSport;
    static inline std::vector<DeviceDbItem *> m_DeviceDB;
    static inline std::vector<ExerciseDevice *> m_DeviceList;
    static inline uint32_t m_lastBLESearchTime;
    static inline std::mutex g_FDM_DeviceListMutex;
    static void TrainerSetSimGrade(float v);
    static void Initialize();
    static void SetInitialRotationZC();
    static void HandleBLEReconnect(uint32_t hash);
    static void HandleBLEConnect(uint32_t hash);
    static void HandleBLEError(uint32_t hash, BLE_ERROR_TYPE err, uint32_t a5);
    static ExerciseDevice *FindDevice(uint32_t hash);
    static uint32_t FindMfgAndModel(uint32_t hash);
    static void AddDevice(ExerciseDevice *dev, const char *nameId);
    static void AddDevice(DeviceDbItem *dev, const char *nameId);
    static void AddDeviceToKnownDatabase(DeviceProtocol prot, uint32_t hash, const char *nameId, uint32_t mfgModel);
    static void PairAuthoritativeSensor(ExerciseDevice *);
    static void PairBrakingSensor(ExerciseDevice *);
    static void PairCadenceSensor(ExerciseDevice *);
    static void PairControllableTrainer(ExerciseDevice *);
    static void PairDi2Sensor(ExerciseDevice *);
    static void PairHRSensor(ExerciseDevice *);
    static void PairPowerSensor(ExerciseDevice *);
    static void PairRunCadenceSensor(ExerciseDevice *);
    static void PairRunSpeedSensor(ExerciseDevice *);
    static void PairSpeedSensor(ExerciseDevice *);
    static void PairSteeringSensor(ExerciseDevice *);
    static void RemoveDevice(ExerciseDevice *, bool);

    /*
FitnessDeviceManager::AddDevicesToReconnectAfterBackgrounding(void)
FitnessDeviceManager::AddLostDevice(uint)
FitnessDeviceManager::ApplyRotationDeadZone(float,float)
FitnessDeviceManager::AreAnyBLEDevicesCurrentlyPaired(void)
FitnessDeviceManager::ClearAllLastSelectedDevicesFromPrefs(void)
FitnessDeviceManager::ClearLastSelectedDeviceFromPrefs(DeviceComponent::ComponentType)
FitnessDeviceManager::ClearLostDevices(void)
FitnessDeviceManager::ClearSelectedDevice(ExerciseDevice *)
FitnessDeviceManager::CreateDeviceAnalyticsProperties(BLEDevice const*)
FitnessDeviceManager::CreateErrorEventName(BLE_ERROR_TYPE)
FitnessDeviceManager::CurrentConfigurationSupportsERGMode(void)
FitnessDeviceManager::DisableAll(void)
FitnessDeviceManager::DoesSteeringDeviceHaveSignal(InputDeviceType)
FitnessDeviceManager::EndWorkout(void)
FitnessDeviceManager::FindFriendlyName(uint)
FitnessDeviceManager::ForceDeviceChangeSignal(void)
FitnessDeviceManager::GetAcceleration(int &,int &,double &,double &,double &)
FitnessDeviceManager::GetConfirmedTrainerSlope(void)
FitnessDeviceManager::GetCurrentBrakingForce(void)
FitnessDeviceManager::GetCurrentCadence(void)
FitnessDeviceManager::GetCurrentCalorieRate(void)
FitnessDeviceManager::GetCurrentDeltaRollZC(void)
FitnessDeviceManager::GetCurrentDeltaYaw(void)
FitnessDeviceManager::GetCurrentHeartRate(void)
FitnessDeviceManager::GetCurrentMachineState(void)
FitnessDeviceManager::GetCurrentPower(void)
FitnessDeviceManager::GetCurrentRotationLimit(void)
FitnessDeviceManager::GetCurrentRowingSpeed(void)
FitnessDeviceManager::GetCurrentSpeed(void)
FitnessDeviceManager::GetCurrentSteeringHeading(void)
FitnessDeviceManager::GetCurrentWheelRPM(void)
FitnessDeviceManager::GetDeviceNameAndModel(ExerciseDevice const*)
FitnessDeviceManager::GetEliteSteeringComponent(ExerciseDevice *)
FitnessDeviceManager::GetEquipmentTypesString(BLEDevice const*)
FitnessDeviceManager::GetJetBlackSteeringComponent(ExerciseDevice *)
FitnessDeviceManager::GetLastConfirmationDelay(void)
FitnessDeviceManager::GetPairingSport(void)
FitnessDeviceManager::GetPreferedBLEConnectionType(void)
FitnessDeviceManager::GetSelectedBLEDeviceCount(void)
FitnessDeviceManager::GetSteeringSensitivity(void)
FitnessDeviceManager::GetTrainerGradeLookAhead(void)
FitnessDeviceManager::GetTrainerSlope(void)
FitnessDeviceManager::GetUnpairedBLEDeviceCount(void)
FitnessDeviceManager::Get_BLEState(void)
FitnessDeviceManager::HandleCentralManagerResetting(void)
FitnessDeviceManager::HandleDisconnectError(uint,uint)
FitnessDeviceManager::Initialize(void)
FitnessDeviceManager::InitializeBLEStatistics(void)
FitnessDeviceManager::IsAnalogSteeringPaired(void)
FitnessDeviceManager::IsComponentPresent(int)
FitnessDeviceManager::IsDeviceBeingUsedElsewhere(ExerciseDevice *,uint)
FitnessDeviceManager::IsLost(uint)
FitnessDeviceManager::IsNameInList(char const*)
FitnessDeviceManager::IsPairedForCurrentSport(void)
FitnessDeviceManager::IsPowerFromLegitPowerMeter(void)
FitnessDeviceManager::IsPowerFromSmartTrainer(void)
FitnessDeviceManager::IsSmartBikeBrakingPaired(void)
FitnessDeviceManager::IsSmartBikeSteeringPaired(void)
FitnessDeviceManager::IsSmartTrainerAndPowerSourceTheSameDevice(void)
FitnessDeviceManager::IsSteeringDevicePaired(InputDeviceType)
FitnessDeviceManager::IsThisDevicePaired(uint)
FitnessDeviceManager::LoadDeviceProfiles(char const*)
FitnessDeviceManager::PreferAllSourcesBLE(void)
FitnessDeviceManager::PreferHubBLE(void)
FitnessDeviceManager::PreferNativeBLE(void)
FitnessDeviceManager::PurgeAllDevices(bool,bool)
FitnessDeviceManager::PurgeAllNonConnectedDevices(bool)
FitnessDeviceManager::RecoverDeviceToReconnectAfterBackgrounding(uint)
FitnessDeviceManager::RecoverLostDevice(uint)
FitnessDeviceManager::ResetAcceleration(void)
FitnessDeviceManager::RestoreAll(void)
FitnessDeviceManager::SaveDeviceProfiles(char const*)
FitnessDeviceManager::SendAnalyticsEvent(std::string const&,BLEDevice const*,std::vector<std::string> const&)
FitnessDeviceManager::SendDataLossEvent(ExerciseDevice const*)
FitnessDeviceManager::SendErrorAnalytics(uint,BLE_ERROR_TYPE,uint)
FitnessDeviceManager::SetPairingSport(uint)
FitnessDeviceManager::SetTrainerTargetSpeed(float)
FitnessDeviceManager::SetUISignals(std::weak_ptr<UI_Refactor::DeviceChangedSignal>)
FitnessDeviceManager::Set_BLEState(uint)
FitnessDeviceManager::Set_PreferAllSourcesBLE(bool)
FitnessDeviceManager::Set_PreferHubBLE(bool)
FitnessDeviceManager::Set_PreferNativeBLE(bool)
FitnessDeviceManager::Shutdown(void)
FitnessDeviceManager::StartWorkout(void)
FitnessDeviceManager::TrackDeviceChanges(bool)
FitnessDeviceManager::TrainerCanSpindown(void)
FitnessDeviceManager::TrainerGetSpeed(void)
FitnessDeviceManager::TrainerGetTargetSpeed(void)
FitnessDeviceManager::TrainerInitSpindown(void)
FitnessDeviceManager::TrainerSetERG(int)
FitnessDeviceManager::TrainerSetGradeLookAheadSecs(float)
FitnessDeviceManager::TrainerSetRiderWeightKG(float)
FitnessDeviceManager::TrainerSetRoadMode(void)
FitnessDeviceManager::TrainerSetRoadTexture(RoadFeelType,float)
FitnessDeviceManager::TrainerSetSimGrade(float)
FitnessDeviceManager::TrainerSetSimMode(void)
FitnessDeviceManager::TrainerSetSpindownStatus(uint)
FitnessDeviceManager::TrainerSetWindSpeed(float)
FitnessDeviceManager::TrainerSpindownStatus(void)
FitnessDeviceManager::TrainerSupportErgMode(void)
FitnessDeviceManager::TrainerSupportErgResume(void)
FitnessDeviceManager::TrainerSupportsRoadTexture(void)
FitnessDeviceManager::UnpairAllPairedDevices(bool,bool)
FitnessDeviceManager::UnpairAllPairedDevices_V2(bool)
FitnessDeviceManager::UnpairAuthoritativeSensor(bool)
FitnessDeviceManager::UnpairBrakingSensor(bool)
FitnessDeviceManager::UnpairCadenceSensor(bool)
FitnessDeviceManager::UnpairControllableTrainer(bool)
FitnessDeviceManager::UnpairDi2Sensor(bool)
FitnessDeviceManager::UnpairHRSensor(bool)
FitnessDeviceManager::UnpairPowerSensor(bool)
FitnessDeviceManager::UnpairRunCadenceSensor(bool)
FitnessDeviceManager::UnpairRunSpeedSensor(bool)
FitnessDeviceManager::UnpairSpeedSensor(bool)
FitnessDeviceManager::UnpairSteeringSensor(bool)
FitnessDeviceManager::Update(float)
FitnessDeviceManager::UpdateScanning(float)*/
};