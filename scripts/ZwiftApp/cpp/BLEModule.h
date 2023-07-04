//UT Coverage: 100%, 4/4
#pragma once
inline bool g_BLE_LoggingON;
void InitializeBLESearchParameters(protobuf::BLEPeripheralRequest *rq);
void BLEDevice_StartSearchForLostDevices();
bool IsNewBLEMiddlewareEnabled();
struct BLEDevice : public ExerciseDevice { //0x360 bytes
    std::string m_devId, m_nameId;
    BLE_SOURCE m_bleSubType = BLES_BUILTIN;
    uint32_t m_charId = 0, m_hash = 0, m_lastCadTs = 0, m_lastSpdTs = 0;
    bool m_isPaired = false, m_field_2CC = false, m_field_29D = false, m_field_29C = false;
    BLEDevice(const std::string &, const std::string &, uint32_t, uint32_t, BLE_SOURCE src);
    static uint32_t CreateUniqueID(uint32_t hf1 /*, uint32_t hf2*/);
    static uint32_t CreateUniqueID(const std::string &f1/*, const std::string &f2*/);
    virtual void SetPaired(bool p);
    void Update(float dt) override;
    void ProcessBLEData(const protobuf::BLEPeripheralResponse &);
    void Pair(bool) override;
    bool IsActivelyPaired() override;
    bool IsPaired() const override;
    uint32_t GetPrefsID() override;
    void UnPair() override;
    void LogBleRxPacket(const protobuf::BLEPeripheralResponse &resp);
        /*void 
void EndWorkout(void)
void GetEliteSteeringComponent(void)
void GetFirmwareUpdateProgress(void)
void GetJetBlackSteeringComponent(void)
void HubFirmwareUpdate(std::string,std::string,std::string)
void LogBleTxPacket(char const*,char const*,zwift::protobuf::BLEPeripheralRequest &)
void OnDeviceConnected(void)
void OnDeviceDisconnected(void)
void ProcessBLEData(zwift::protobuf::BLEPeripheralResponse const&)
void ProcessBattery(CharacteristicInfo const&)
void ProcessBowFlexTreadmillStatus(CharacteristicInfo const&)
void ProcessBowFlexTreadmillStream(CharacteristicInfo const&)
void ProcessCharacteristic(zwift::protobuf::BLEPeripheralResponse const&)
void ProcessCycleOpsControlPoint(CharacteristicInfo const&)
void ProcessEliteActivationControlPoint(CharacteristicInfo const&)
void ProcessEliteActivationResponsePoint(CharacteristicInfo const&)
void ProcessEliteAux(CharacteristicInfo const&)
void ProcessEliteResponsePoint(CharacteristicInfo const&)
void ProcessFTMSBikeData(CharacteristicInfo const&)
void ProcessFTMSControlPoint(CharacteristicInfo const&)
void ProcessFTMSFeatures(CharacteristicInfo const&)
void ProcessFTMSMachineStatus(CharacteristicInfo const&)
void ProcessFTMSTreadmillData(CharacteristicInfo const&)
void ProcessFirmwareVersion(CharacteristicInfo const&)
void ProcessHardwareRevision(CharacteristicInfo const&)
void ProcessHeartRate(CharacteristicInfo const&)
void ProcessInRideData(CharacteristicInfo const&)
void ProcessJetBlackSWB(CharacteristicInfo const&)
void ProcessKickrBikeInput(CharacteristicInfo const&)
void ProcessKickrControlPoint(CharacteristicInfo const&)
void ProcessKineticSmartData(CharacteristicInfo const&)
void ProcessMagneticDaysControlPoint1(CharacteristicInfo const&)
void ProcessManufacturerName(CharacteristicInfo const&)
void ProcessMilestonePodToHost(CharacteristicInfo const&)
void ProcessMilestonePodVersion(CharacteristicInfo const&)
void ProcessModelNumber(CharacteristicInfo const&)
void ProcessPower(CharacteristicInfo const&)
void ProcessRunSpeedCadence(CharacteristicInfo const&)
void ProcessSerialID(CharacteristicInfo const&)
void ProcessSoftwareVersion(CharacteristicInfo const&)
void ProcessSpeedCadence(CharacteristicInfo const&)
void ProcessStagesBikeInput(CharacteristicInfo const&)
void ProcessSystemID(CharacteristicInfo const&)
void ProcessTacxControlPoint(CharacteristicInfo const&)
void ProcessTechnoGymControlPoint(CharacteristicInfo const&)
void ProcessTemperature(CharacteristicInfo const&)
void ProcessTickrXControlPoint(CharacteristicInfo const&)
void ProcessUserDataWeight(CharacteristicInfo const&)
void ProcessWahooGymConnectDeviceData(CharacteristicInfo const&)
void ProcessWahooGymConnectDeviceType(CharacteristicInfo const&)
void ProcessWattBikeControlPoint(CharacteristicInfo const&)
void ProcessWattBikeInput(CharacteristicInfo const&)
void ProcessWhisperSmartControlPointFast(CharacteristicInfo const&)
void ProcessWhisperSmartControlPointSlow(CharacteristicInfo const&)
void RequestUpdatedRSSIInfo(void)
void StartWorkout(void)
void SwapLegacyControlComponentForFTMS(void)
*/
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
