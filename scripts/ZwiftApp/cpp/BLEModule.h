//UT Coverage: 100%, 4/4
#pragma once
inline bool g_BLE_LoggingON;
void InitializeBLESearchParameters(protobuf::BLEPeripheralRequest *rq);
void BLEDevice_StartSearchForLostDevices();
bool IsNewBLEMiddlewareEnabled();
struct CharacteristicInfo;
struct BLEDevice : public ExerciseDevice { //0x360 bytes
    std::string m_devId, m_nameId, m_scharId;
    BLE_SOURCE m_bleSrc = BLES_BUILTIN;
    uint32_t m_charId = 0, m_hash = 0, m_lastCadTs = 0, m_lastSpdTs = 0, m_lastTorTs = 0, m_lastSpdTs2 = 0, m_cumulativeWheelRevolutions = 0, m_cumulativeCrankRevolutions = 0, 
        m_cumulativeWheelRevolutionsPm = 0, m_cumulativeCrankRevolutionsPm = 0, m_swVersionInt = 0, m_torque = 0, m_field_118 = 0 /*TODO:enum*/;
    uint16_t m_lastWheelEventTime = 0, m_lastCrankEventTime = 0, m_lastWheelEventTimePm = 0, m_lastCrankEventTimePm = 0, m_riderWeight = 0;
    bool m_isPaired = false, m_field_2CC = false, m_field_29D = false, m_field_29C = false, m_spdInitialized = false, m_cadInitialized = false;
    BLEDevice(const std::string &devId, const std::string &devName, uint32_t charId, uint32_t hash, BLE_SOURCE src);
    static uint32_t CreateUniqueID(uint32_t hdevId /*, uint32_t hf2*/);
    static uint32_t CreateUniqueID(const std::string &devId/*, const std::string &f2*/);
    virtual void SetPaired(bool p);
    void Update(float dt) override;
    void ProcessBLEData(const protobuf::BLEPeripheralResponse &);
    void Pair(bool) override;
    bool IsActivelyPaired() override;
    bool IsPaired() const override;
    uint32_t GetPrefsID() override;
    void UnPair() override;
    static void LogBleRxPacket(const protobuf::BLEPeripheralResponse &resp);
    static void LogBleTxPacket(const char *funcName, const char *devName, protobuf::BLEPeripheralRequest &req);
    void ProcessCharacteristic(const protobuf::BLEPeripheralResponse &resp);
    void ProcessSystemID(const CharacteristicInfo &ci);
    void ProcessSerialID(const CharacteristicInfo &ci);
    void ProcessFirmwareVersion(const CharacteristicInfo &ci);
    void ProcessHardwareRevision(const CharacteristicInfo &ci);
    void ProcessSoftwareVersion(const CharacteristicInfo &ci);
    void ProcessManufacturerName(const CharacteristicInfo &ci);
    void ProcessHeartRate(const CharacteristicInfo &ci);
    void ProcessRunSpeedCadence(const CharacteristicInfo &ci);
    void ProcessSpeedCadence(const CharacteristicInfo &ci);
    void ProcessPower(const CharacteristicInfo &ci);
    void ProcessFTMSFeatures(const CharacteristicInfo &ci);
    void ProcessFTMSTreadmillData(const CharacteristicInfo &ci);
    void ProcessFTMSBikeData(const CharacteristicInfo &ci);
    FTMS_ControlComponent *SwapLegacyControlComponentForFTMS();
    EliteSteeringComponent *GetEliteSteeringComponent();
    void ProcessFTMSControlPoint(const CharacteristicInfo &ci);
    void ProcessFTMSMachineStatus(const CharacteristicInfo &ci);
    void ProcessTacxControlPoint(const CharacteristicInfo &ci);
    //inlined void OnDeviceConnected();
    //inlined void OnDeviceDisconnected();
        /*void
    void ProcessTickrXControlPoint(const CharacteristicInfo &ci);
void EndWorkout(void)
void GetFirmwareUpdateProgress(void)
void GetJetBlackSteeringComponent(void)
void HubFirmwareUpdate(std::string,std::string,std::string)
void ProcessBLEData(zwift::protobuf::BLEPeripheralResponse const&)
void ProcessBattery(const CharacteristicInfo &ci)
void ProcessBowFlexTreadmillStatus(const CharacteristicInfo &ci)
void ProcessBowFlexTreadmillStream(const CharacteristicInfo &ci)
void ProcessCycleOpsControlPoint(const CharacteristicInfo &ci)
void ProcessEliteActivationControlPoint(const CharacteristicInfo &ci)
void ProcessEliteActivationResponsePoint(const CharacteristicInfo &ci)
void ProcessEliteAux(const CharacteristicInfo &ci)
void ProcessEliteResponsePoint(const CharacteristicInfo &ci)
void ProcessInRideData(const CharacteristicInfo &ci)
void ProcessJetBlackSWB(const CharacteristicInfo &ci)
void ProcessKickrBikeInput(const CharacteristicInfo &ci)
void ProcessKickrControlPoint(const CharacteristicInfo &ci)
void ProcessKineticSmartData(const CharacteristicInfo &ci)
void ProcessMagneticDaysControlPoint1(const CharacteristicInfo &ci)
void ProcessMilestonePodToHost(const CharacteristicInfo &ci)
void ProcessMilestonePodVersion(const CharacteristicInfo &ci)
void ProcessModelNumber(const CharacteristicInfo &ci)
void ProcessStagesBikeInput(const CharacteristicInfo &ci)
void ProcessTechnoGymControlPoint(const CharacteristicInfo &ci)
void ProcessTemperature(const CharacteristicInfo &ci)
void ProcessUserDataWeight(const CharacteristicInfo &ci)
void ProcessWahooGymConnectDeviceData(const CharacteristicInfo &ci)
void ProcessWahooGymConnectDeviceType(const CharacteristicInfo &ci)
void ProcessWattBikeControlPoint(const CharacteristicInfo &ci)
void ProcessWattBikeInput(const CharacteristicInfo &ci)
void ProcessWhisperSmartControlPointFast(const CharacteristicInfo &ci)
void ProcessWhisperSmartControlPointSlow(const CharacteristicInfo &ci)
void RequestUpdatedRSSIInfo(void)
void StartWorkout(void)
*/
};
struct RunPod_BLE : public BLEDevice { //0x878 bytes
    RunPod_BLE(const std::string &devId, const std::string &devName, uint32_t charId, uint32_t hash, BLE_SOURCE src) : BLEDevice(devId, devName, charId, hash, src) {
        //later
    }
};
struct BLEModule : public EventObject {
    struct LegacyBLEImpl {
        void DidConnect(const char *, const char *);
        void DidReceiveError(const char *, const char *, BLE_ERROR_TYPE, uint32_t);
        void DidRecover(const char *, const char *);
        void DoHardwarePrompt();
        void EnableDeviceDiscovery(bool);
        void GetRSSI(const char *);
        bool HasBLE();
        void IgnoreReceivedBluetoothPackets(bool);
        void InitializeBLE();
        bool IsAnyDeviceConnecting();
        bool IsAutoConnectPairingOn();
        bool IsBLEAvailable();
        bool IsRecoveringLostDevices();
        bool IsScanning();
        void PairDevice(const BLEDevice &);
        void ProcessAdvertisedServiceUUIDs(const protobuf::BLEAdvertisement &, const std::string &, protobuf::BLEAdvertisementDataSection_Type, BLE_SOURCE);
        void ProcessAdvertisementManufacturerData(const protobuf::BLEAdvertisement &, const std::string &, BLE_SOURCE);
        void ProcessBLEResponse(const protobuf::BLEPeripheralResponse &, BLE_SOURCE);
        void ProcessDiscovery(const protobuf::BLEAdvertisement &, BLE_SOURCE);
        void PurgeDeviceList();
        void ReceivedRSSI(int, const char *, const char *);
        void SendValueToDevice(const protobuf::BLEPeripheralRequest &, BLE_SOURCE);
        void SetAutoConnectPairingMode(bool);
        void SetDeviceConnectedFlag(const protobuf::BLEPeripheralResponse &, bool);
        void StartBackgroundBLECommunication();
        void StartScan(const protobuf::BLEPeripheralRequest &);
        void StartSearchForLostDevices();
        void StopBackgroundBLECommunication();
        void StopScan();
        void StopSearchForLostDevices();
        void UnpairDevice(const BLEDevice &);
    } *m_bleImpl = nullptr;
    inline static std::unique_ptr<BLEModule> g_BLEModule;
    BLEModule(Experimentation *exp);
    ~BLEModule();
    static void Initialize(Experimentation *exp);
    static bool IsInitialized() { return g_BLEModule.get() != nullptr; }
    static BLEModule *Instance() { zassert(g_BLEModule.get() != nullptr); return g_BLEModule.get(); }
    static void Shutdown();
    void HandleEvent(EVENT_ID, va_list) override;
    void StopScan();
    void ProcessDiscovery(const protobuf::BLEAdvertisement &, BLE_SOURCE);
    void ProcessBLEResponse(const protobuf::BLEPeripheralResponse &, BLE_SOURCE);
    void callVoidImplMethodOrLogIfUninitialized(const std::function<void(void)> &func, const char *method);
    bool callBoolImplMethodOrLogIfUninitialized(const std::function<bool(void)> &func, const char *method);
    void PairDevice(const BLEDevice &dev);
    void DidRecover(const char *a2, const char *a3);
    void DidConnect(const char *a2, const char *a3);
    void ReceivedRSSI(int a2, const char *a3, const char *a4);
    void DidReceiveError(const char *, const char *, BLE_ERROR_TYPE, uint32_t);
    void SetDeviceConnectedFlag(const protobuf::BLEPeripheralResponse &resp, bool flag);
    void StartScan(const protobuf::BLEPeripheralRequest &req);
    void ProcessAdvertisementManufacturerData(const protobuf::BLEAdvertisement &adv, const std::string &a3, BLE_SOURCE src);
    void EnableDeviceDiscovery(bool en);
    void StartSearchForLostDevices();
    void SendValueToDevice(const protobuf::BLEPeripheralRequest &req, BLE_SOURCE src);
    bool IsScanning();
    bool HasBLE();
    bool IsBLEAvailable();
    bool IsRecoveringLostDevices();
    bool IsAutoConnectPairingOn();
    void PurgeDeviceList();
    void DoHardwarePrompt();
    void IgnoreReceivedBluetoothPackets(bool ign);
    void ProcessAdvertisedServiceUUIDs(const protobuf::BLEAdvertisement &adv, const std::string &a2, protobuf::BLEAdvertisementDataSection_Type dst, BLE_SOURCE src);
    void StopSearchForLostDevices();
    void UnpairDevice(const BLEDevice &dev);
    void InitializeBLE();
};
