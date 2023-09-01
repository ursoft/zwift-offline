//UT Coverage: 23%, 31/137, NEED_MORE
#pragma once
enum BLE_SOURCE { BLES_BUILTIN = 0, BLES_ZCA = 1, BLES_ZH = 2, BLES_WFTN = 3, BLES_CNT };
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
struct DeviceComponent { //24 (0x18) bytes
    ExerciseDevice *m_owner = nullptr;
    enum ComponentType { CPT_SPD = 0, CPT_CAD = 1, CPT_RUN_SPD = 2, CPT_RUN_CAD = 3, CPT_HR = 4, CPT_5 = 5, CPT_PM = 6, CPT_7 = 7, CPT_CTRL = 8, CPT_9 = 9, CPT_10 = 10, CPT_STEER = 16 } m_type = CPT_SPD;
    int m_packId = -999, m_packTs = 0, m_field_20 /*TODO:enum*/= 0;
    bool m_bInitState = true;
    DeviceComponent(ComponentType ct = CPT_SPD) : m_type(ct) {}
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
enum RoadFeelType { RF_0, RF_1, RF_2, RF_WOOD = 3, RF_BRICKS_HARD = 4, RF_BRICKS_SOFT = 5, RF_GRAVEL1 = 6, RF_GRAVEL2 = 7, RF_GRAVEL_SOFT = 8 };
enum GearingType { GT_0 };
enum ShiftDirection { SD_0 };
struct TrainerControlComponent : public DeviceComponent { //0x68 bytes
    virtual void ProcessANTEvent(uint8_t) = 0; //[00]
    virtual void InitSpindown() = 0; //[01]
    virtual void SetERGMode(int) = 0; //[02]
    virtual void SetRoadMode() = 0; //[03]
    virtual void SetSimulationMode() = 0; //[04]
    virtual bool SupportsRoadTexture() { return false; } //[05]
    virtual void SetRoadTexture(RoadFeelType, float) { /*empty*/ } //[06]
    virtual void SetGradeLookAheadSecs(float f) { m_gradeLookAheadSecs = f; } //[07]
    virtual void SetSimulationGrade(float) = 0; //[08]
    virtual void SetWindSpeed(float) = 0; //[09]
    virtual void Update(float) { /*empty*/ } //[10]
    virtual void OnPaired() { /*empty*/ } //[11]
    virtual void SetRiderWeightKG(float f) { m_riderWeightKG = f; } //[12]
    virtual void SetGearingShiftType(GearingType) { /*empty*/ } //[13]
    virtual void ShiftCassetteCog(ShiftDirection) { /*empty*/ } //[14]
    virtual void ShiftChainRing(ShiftDirection) { /*empty*/ } //[15]
    virtual void Shift_EZTap_Event(bool, ShiftDirection) { /*empty*/ } //[16]
    virtual void Shift_ZTap_Event(bool, ShiftDirection) { /*empty*/ } //[17]
    enum ProtocolType { P_0, FTMS_V1, FTMS_V2, FTMS_V3, P_4, ZAP_PROTOCOL = 5 } m_protocolType = P_0;
    ProtocolType GetProtocolType() { return m_protocolType; }
    TrainerControlComponent() : DeviceComponent(DeviceComponent::CPT_CTRL) {}
    bool IsFitTech();
    void UpdateShifting() { /*empty*/ }
    int64_t m_field_2C = -1;
    float m_gradeLookAheadSecs = 0.0f, m_riderWeightKG = 73.0f, m_gradePercent = 0.0f, m_field_40 = 0.0f, m_field_58 = 0.004f, m_field_5C = 0.368f, m_lastTimeSec = 0.0f;
    int m_field_20 = 11 /*TODO:enum*/, m_field_28 = 0, m_erg = 0;
    bool m_field_34 = true, m_field_35 = true, m_field_36 = false, m_field_37 = false;
    char m_field_60 = 2;
};
struct SarisControlComponent : public TrainerControlComponent { //0x78 bytes, PC only
    void ProcessANTEvent(uint8_t) override { /*later*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override { /*later*/ } //[02]
    void SetRoadMode() override { /*later*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
struct KICKRControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override {} //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override {} //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { //[09]
        /* later ANDR:result = ANT_IsSearchEnabled();
        if ((result & 1) != 0)
            return result;
        result = timeGetTime();
        if ((int)result - dword_2586920 <= (unsigned int)g_PacketSendDelay)
            return result;
        result = timeGetTime();
        dword_2586920 = result;
        v6 = *((_DWORD *)this + 8);
        if ((unsigned int)(v6 - 2) >= 4)
        {
            return (*(__int64(__fastcall **)(KICKRControlComponent *))(*(_QWORD *)this + 32LL))(this);
        } else if (v6 != 5)
        {
            result = Log(8u, "KICKR changing wind speed to %3.2f", v5);
            v7 = *((_QWORD *)this + 14);
            if (v7)
            {
                result = *(unsigned int *)(v7 + 588);
                if ((result & 0x80000000) == 0)
                {
                    *((float *)this + 16) = a2;
                    *((_DWORD *)this + 8) = 5;
                    *((_BYTE *)this + 121) = 1;
                    WF_SetRoadCurveMode_Burst();
                    v8 = (double)(unsigned int)timeGetTime() / 1000.0;
                    *((float *)this + 19) = v8;
                    result = timeGetTime();
                    v9 = *((_QWORD *)this + 14);
                    v10 = *(_DWORD *)(v9 + 1632);
                    *(_DWORD *)(v9 + 4LL * (unsigned __int8)v10 + 1636) = result;
                    *(_DWORD *)(v9 + 1632) = v10 + 1;
                }
            }
        }*/
    }
//        KICKRControlComponent::SendUserParametersToEquipment(void)
//        KICKRControlComponent::SetSimulationGrade(float, bool)
};
struct FECTrainerControlComponent : public TrainerControlComponent { //0x78 bytes
    static inline int g_flag;
    static inline uint32_t g_lastSpindownTs;
    void ProcessANTEvent(uint8_t) override {} //[00]
    void InitSpindown() override { //[01]
        g_flag = 0;
        doInitSpindown();
    }
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override {} //[03]
    void SetSimulationMode() override {} //[04]
    bool SupportsRoadTexture() override {} //[05]
    void SetRoadTexture(RoadFeelType, float) override {} //[06]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    void doInitSpindown() {
        //TODO
    }
};
struct EliteControlComponent : public TrainerControlComponent{ //0x90 bytes
    void ProcessANTEvent(uint8_t) override {} //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override {} //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void Update(float) override {} //[10]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
struct TACX_BLE_ControlComponent : public TrainerControlComponent { //0x78 bytes
    TACX_BLE_ControlComponent(BLEDevice *parent) : m_parent(parent) { m_gradeLookAheadSecs = 2.0f; }
    BLEDevice *m_parent;
    bool m_field_70 = true, m_field_71 = false;
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override; //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override { m_field_20 = 3; } //[04]
    bool SupportsRoadTexture() override; //[05]
    void SetRoadTexture(RoadFeelType ty, float strength) override; //[06]
    void SetSimulationGrade(float) override; //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
struct ELITE_BLE_ControlComponent : public TrainerControlComponent{ //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
struct WATTBIKE_BLE_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    void OnPaired() override {} //[11]
};
struct TechnoGym_BLE_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    void Update(float) override {} //[10]
};
struct KICKR_BLEM_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override {} //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    void StartRolldown(int) { /*TODO*/ }
};
struct KINETIC_BLE_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
struct FTMS_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override {} //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override { /*empty*/ } //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    FTMS_ControlComponent(BLEDevice *dev);
    BLEDevice *m_bleDevice;
};
struct FTMS_ControlComponent_v2 : public TrainerControlComponent { //0xA40 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { m_bSpinDown = true; } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float f) override { //[09]
        m_field_20 = 3;
        m_windSpeed = f;
    }
    void Update(float) override {} //[10]
    float m_windSpeed = 0.0f;
    bool m_bSpinDown = false;
    FTMS_ControlComponent_v2(BLEDevice *dev);
};
struct FTMS_ControlComponent_v3 : public TrainerControlComponent { //0xB40 bytes
    float m_windSpeed = 0.0f;
    int m_field_68 = 0; //TODO: enum
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { //[01]
        if (m_FID_FTMS_SD_fix) {
            if (m_field_68 > 8 && m_field_68 < 15)
                return;
            m_bSpinDown = true;
        } else if (m_field_68 == 14 || m_field_68 == 12)
            return;
        m_bSpinDown = true;
    }
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float f) override { //[09]
        if (m_FID_FTMS_SD_fix && m_field_20 <= 7)
            return;
        m_field_20 = 3;
        m_windSpeed = f;
    }
    void Update(float) override {} //[10]
    BLEDevice *m_bleDevice;
    int m_field_B18;
    bool m_FID_FTMS_SD_fix, m_bSpinDown = false, m_field_B3D = false;
    char m_field_B38;
    FTMS_ControlComponent_v3(BLEDevice *dev);
    /*FTMS_ControlComponent_v3::CheckState_BikeSimMode(float)
FTMS_ControlComponent_v3::CheckState_SimMode(float)
FTMS_ControlComponent_v3::FTMS_RequestControl(void)
FTMS_ControlComponent_v3::FTMS_RequestSpindown(void)
FTMS_ControlComponent_v3::FTMS_ResetTrainer(void)
FTMS_ControlComponent_v3::FTMS_SetBikeSimParms(void)
FTMS_ControlComponent_v3::FTMS_SetTargetWatts(void)
FTMS_ControlComponent_v3::FTMS_StartSession(void)
FTMS_ControlComponent_v3::GetTrainerSpinDownTargetSpeedInMPH(ExerciseDevice const*,FTMS_ControlComponent_v3::RequestResult const&)
FTMS_ControlComponent_v3::HandleControlPointResultCode(FTMS_ControlComponent_v3::RequestResult &,FTMS_ControlComponent_v3::FTMS_TRAINER_CONTROL_STATE &,FTMS_ControlComponent_v3::FTMS_CONTROL_OPCODE &)
FTMS_ControlComponent_v3::HandleState_Ready(float)
FTMS_ControlComponent_v3::HandleState_RequestControl(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_RequestControlAndReset(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_RequestSetBikeSimParms(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_RequestSetTargetPower(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_RequestSpinDown(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_RequestTrainerReset(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_SpinDownClearSimParms(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_SpinDownClearTargetPower(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_StartSession(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::HandleState_WaitForSpindown(float,FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::IntervalTimeHasElapsed(float)
FTMS_ControlComponent_v3::LogOpcodeParameters(FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::ProcessANTEvent(uchar)
FTMS_ControlComponent_v3::RequestHasCompleted(FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::RequestHasError(FTMS_ControlComponent_v3::RequestResult &)
FTMS_ControlComponent_v3::SendPacketData(char const*,uchar const*,int)
FTMS_ControlComponent_v3::SetERGMode(int)
FTMS_ControlComponent_v3::SetERGMode(int,bool)
FTMS_ControlComponent_v3::SetNextState(FTMS_ControlComponent_v3::FTMS_TRAINER_CONTROL_STATE,FTMS_ControlComponent_v3::FTMS_CONTROL_OPCODE)
FTMS_ControlComponent_v3::SetRequestResult(uchar const*,uint)
FTMS_ControlComponent_v3::SetRiderWeight(void)
FTMS_ControlComponent_v3::SetRoadMode(void)
FTMS_ControlComponent_v3::SetSimulationGrade(float)
FTMS_ControlComponent_v3::SetSimulationMode(void)
FTMS_ControlComponent_v3::UpdateStateMachine(float)*/
};
struct WhisperSmart_BLE_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    void Update(float) override {} //[10]
};
struct MAGDAYS_BLE_ControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override { /*empty*/ } //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
    void Update(float) override {} //[10]
};
struct CompuTrainerControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override {} //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
struct EliteWiredControlComponent : public TrainerControlComponent { //0x78 bytes
    void ProcessANTEvent(uint8_t) override { /*empty*/ } //[00]
    void InitSpindown() override { /*empty*/ } //[01]
    void SetERGMode(int) override {} //[02]
    void SetRoadMode() override {} //[03]
    void SetSimulationMode() override {} //[04]
    void SetSimulationGrade(float) override {} //[08]
    void SetWindSpeed(float) override { /*empty*/ } //[09]
};
enum SensorType { ST_GENERIC, ST_ELITE_STEER, ST_JB_STEER };
struct SensorValueComponent : public DeviceComponent { //40 (0x28) bytes
    using DeviceComponent::DeviceComponent;
    virtual SensorType GetSensorType() { return ST_GENERIC; }
    float m_val = 0.f;
    int gap;
};
struct Component_7 : public DeviceComponent { //48 (0x30) bytes - AccelerationComponent?
    using DeviceComponent::DeviceComponent;
    double m_prevSpeed = 0.0, m_curSpeed = 0.0, m_deltaSec = 0.0;
};
struct Component_9 : public DeviceComponent { //80 (0x50) bytes
    using DeviceComponent::DeviceComponent;
};
struct Bowflex_BLE_ControlComponent : public SensorValueComponent { //56 (0x38) bytes
    Bowflex_BLE_ControlComponent() : SensorValueComponent(CPT_RUN_SPD) {}
    uint16_t m_field_30 = 1;
    void Bowflex_ParseStream(uint8_t *, uint32_t, Bowflex_BLE_ControlComponent *, SensorValueComponent *);
    void InitStreaming();
};
struct EliteSteeringComponent : public SensorValueComponent { //72 (0x48) bytes
    SensorType GetSensorType() override { return ST_ELITE_STEER; }
    BLEDevice *m_parent;
    uint32_t m_ts, m_field_30 /*TODO:enum*/ = 0, m_field_34 = 0, m_field_40 = 0, m_field_3C;
    EliteSteeringComponent(BLEDevice *parent) : SensorValueComponent(CPT_STEER), m_parent(parent) {
        m_ts = timeGetTime();
        srand(m_ts);
        auto v5 = rand();
        rand();
        auto v6 = rand();
        m_field_3C = (v5 << 16) | v6;
    }
    void OnDeviceActivated() {
        m_field_30 = 3;
        SetUpdateFrequency();
    }
    void SendActivationCommand(uint32_t cmd);
    void SetUpdateFrequency();
    void SetCenter();
    void SendActivationRequest(uint32_t rq);
    //inlined void OnConnect();
    void HandleNoResponse();
    void HandleActivationRequest();
    void HandleImpostor();
    //inlined void OnDisconnect();
    bool FirmwareUpdateRequired();
    static void DisplayDialog(const char *, const char *);
};
struct JetBlackSteeringComponent : public SensorValueComponent { //72 (0x48) bytes
    SensorType GetSensorType() override { return ST_JB_STEER; }
    static bool IsFeatureFlagEnabled();
    BLEDevice *m_parent;
    int m_field_34 = 0;
    JetBlackSteeringComponent(BLEDevice *parent) : SensorValueComponent(CPT_STEER), m_parent(parent) {
        //TODO
    }
};
struct ExerciseDevice { //0x290 bytes
    char m_name[128]{};
    char m_nameIdBuf[128]{};
    std::vector<DeviceComponent *> m_components;
    //OMIT std::vector<std::string> m_devAnalytics;
    std::string m_manufName, m_hwRev, m_swVersion, m_fwVersion, m_address;
    std::mutex m_mutex;
    uint32_t m_last_time_ms = 0, m_rssiTime = 0, m_fwVersionInt = 0;
    int m_rssi = 0, m_field_11C = 0, m_field_118 = -1 /*TODO:enum*/;
    uint32_t m_prefsID = (uint32_t)-1;
    DeviceProtocol m_protocol = DP_UNKNOWN;
    uint16_t m_modelNumber = 0;
    bool m_hidden = false, m_is560017 = false, m_field_1FE = false, m_field_1FC = false, m_field_1FD = false, m_field_200 = false;
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
struct ANTDevice : public ExerciseDevice { // 0xB90 bytes
    ANTDevice(uint8_t, uint16_t, uint8_t);
    void Pair(bool p) override;   //[01]
    void UnPair() override;   //[02]
    bool IsPaired() const override; //[04]
    void Update(float) override;
    virtual void OnReboot(); //[13]
    virtual void ProcessANTBroadcastData(uint8_t *); //[14]
    virtual void ProcessANTEvent(uint8_t); //[15]
};
struct ZMLAUXDevice : public ExerciseDevice { //0xb28 bytes
    inline static const uint32_t PREFS_ID = (uint32_t)-23;
    void Pair(bool p) override;   //[01]
    void UnPair() override;   //[02]
    bool IsPaired() const override; //[04]
    ZMLAUXDevice(bool hasHR, bool hasSPEED) {
        m_protocol = DP_ZML;
        m_prefsID = PREFS_ID;
        if (hasSPEED)
            if (!FindComponentOfType(DeviceComponent::CPT_RUN_SPD))
                AddComponent(new SensorValueComponent(DeviceComponent::CPT_RUN_SPD));
        if (hasHR)
            if (!FindComponentOfType(DeviceComponent::CPT_HR))
                AddComponent(new SensorValueComponent(DeviceComponent::CPT_HR));
    }
    void Update(float) override;
    uint32_t m_tsLastSPD = 0, m_tsLastHR = 0;
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
    static inline ExerciseDevice *m_pSelectedHRDevice, *m_pSelectedDi2Device, *m_pSelectedPowerDevice, *m_pSelectedSpeedDevice, *m_pSelectedCadenceDevice, *m_pSelectedRunSpeedDevice, *m_pSelectedRunCadenceDevice, *m_pSelectedControllableTrainerDevice, *m_pSelectedSteeringDevice, *m_pSelectedBrakingDevice, *m_pSelectedAuthoritativeDevice;
    static inline uint32_t m_lastBLESearchTime;
    static inline float m_SpindownSpeedInMPH;
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
    static int GetSelectedBLEDeviceCount();
    static bool IsThisDevicePaired(uint32_t prefsId);
    static int GetUnpairedBLEDeviceCount();
    static bool AreAnyBLEDevicesCurrentlyPaired();
    static std::string GetEquipmentTypesString(const BLEDevice *);
    static bool TrainerSetGradeLookAheadSecs(float f);
    static bool TrainerSetWindSpeed(float);
    static std::string GetDeviceNameAndModel(const ExerciseDevice &);
    /*
FitnessDeviceManager::AddDevicesToReconnectAfterBackgrounding(void)
FitnessDeviceManager::AddLostDevice(uint)
FitnessDeviceManager::ApplyRotationDeadZone(float,float)
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
FitnessDeviceManager::GetEliteSteeringComponent(ExerciseDevice *)
FitnessDeviceManager::GetJetBlackSteeringComponent(ExerciseDevice *)
FitnessDeviceManager::GetLastConfirmationDelay(void)
FitnessDeviceManager::GetPairingSport(void)
FitnessDeviceManager::GetPreferedBLEConnectionType(void)
FitnessDeviceManager::GetSteeringSensitivity(void)
FitnessDeviceManager::GetTrainerGradeLookAhead(void)
FitnessDeviceManager::GetTrainerSlope(void)
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
FitnessDeviceManager::TrainerSetRiderWeightKG(float)
FitnessDeviceManager::TrainerSetRoadMode(void)
FitnessDeviceManager::TrainerSetRoadTexture(RoadFeelType,float)
FitnessDeviceManager::TrainerSetSimGrade(float)
FitnessDeviceManager::TrainerSetSimMode(void)
FitnessDeviceManager::TrainerSetSpindownStatus(uint)
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