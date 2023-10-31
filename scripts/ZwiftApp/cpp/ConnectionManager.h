#pragma once
enum InputDeviceType : int { IDT_NONE, IDT_1, IDT_STANDALONE, IDT_SMARTBIKE };
enum ZwiftButtonMap { INVALID = -1, MAX_BUTTONS = 16 };
struct InputEvent {
    ZwiftButtonMap m_button{ INVALID };
    int m_type{};
    union {
        float m_fpar; bool m_down;
    } m_par{};
};
using InputCallback = std::function<bool(float)>;
enum KeyCatcher { KK_GAMEPLAY, KK_GUI, KK_CNT };
enum KeyPos { KP_PRESS, KP_RELEASE, KP_CNT };
struct DeviceInputManager { //0x1034 bytes
    static DeviceInputManager *Instance();
    bool m_inited{};
    InputDeviceType m_type = IDT_NONE;
    std::bitset<MAX_BUTTONS> m_buttonDown;
    enum KeyCatcher m_keySet{ KK_GAMEPLAY };
    InputCallback m_callbacks[KP_CNT][KK_CNT][MAX_BUTTONS];
    int m_field1030{};
    InputDeviceType DetermineInputDeviceType();
    bool IsButtonDown(ZwiftButtonMap);
    static bool IsStagesSteeringEnabled();
    void PushInputEvent(InputEvent *ptr);
    InputCallback *GetCallbackForKey(bool pressed, ZwiftButtonMap);
    void LoadDeviceBindings();
    void LoadKeyBindings(const std::string &file, KeyCatcher kk);

        /*ApplyDeadZone(int,int,int) : inlined
AreAnyButtonsPressed(uint)
CalibrateFitTechAnalog(void)
DeviceInputManager(void)
GetBrakingValue(void)
GetSteeringValue(void)
GetUniformAnalogValue(int,uint,uint)
NumberOfButtonsDown(void)
PreLoadBindings(void)
ProcessSteeringInput_FT(void)
ProcessSteeringInput_SB(void)
RecordRawAnalogValue(ZwiftAnalogMap,signed char)
Render(void)
SetActiveKeyCatcher(KeyCatcher)
SetAnalogValue(ZwiftAnalogMap,float)
~DeviceInputManager()

empty:
DeviceInputManager_RegisterCommands(void)*/
};
inline DeviceInputManager g_devInMgr;
struct ConnectionSubscriber;
struct DisconnectedDeviceInfo {
    std::set<DeviceComponent::ComponentType> m_types;
};
struct ConnectionManager { // 0x138 bytes
public:
    time_t m_lastConnCheck = 0, m_disconnStart = 0, m_restoredStart = 0, m_field_58 = 0;
    uint64_t m_tcpPacketsLast = 0, m_udpPacketsLast = 0;
    std::vector<ConnectionSubscriber *> m_subscribers;
    std::unordered_map<uint32_t, bool> m_connDevs;
    std::unordered_map<uint32_t, DisconnectedDeviceInfo> m_infos;
    bool m_bDisconnNotified = false, m_field_81 = false, m_lastAntConn = false, m_lastBleAvail = false, m_antProblemReported = false, m_bleProblemReported = false, m_devProblemReported = false;

    ConnectionManager();
    ~ConnectionManager();
    static void DestroyInstance();
    void Subscribe(ConnectionSubscriber *s);
    void Unsubscribe(ConnectionSubscriber *s);
    static ConnectionManager *GetInstance();
    static bool IsInitialized();
    static void Initialize();
    void UpdateZwiftServerConnectionStatus(float dt);
    bool IsSystemAlertFeatureEnabled();
    bool IsPairedDevicesAlertFeatureEnabled();
    void UpdateAnalyticsForResolvedNotification(ConnectionType, int64_t);
    void UpdateConnectedDeviceStatus();
    void Notify(ConnectionType, NotificationType, NotificationDisplayType);
    bool CheckIfDeviceIsDisconnected(ExerciseDevice *a2);
    void Update(float dt) {
        UpdateZwiftServerConnectionStatus(dt);
        UpdateHardwareProtocolConnectionStatus();
        UpdateConnectedDeviceStatus();
    }
    void UpdateHardwareProtocolConnectionStatus();
    bool m_antd = false, m_bled = false;
    bool IsConnectedANTDeviceDisconnected() { return m_antd; }
    bool IsConnectedBLEDeviceDisconnected() { return m_bled; }
    void CollectDisconnectedDeviceInfo(ExerciseDevice *dev, DeviceComponent::ComponentType ct);
/*OMIT AddNewConnectivityAnalyticsEvent(NotificationDisplayType,NotificationType,ConnectionType,long)
SendUnresolvedAnalytics()

inlined, all called from UpdateConnectedDeviceStatus:
    CollectDisconnectedSteeringDeviceInfo(InputDeviceType)
    RunningDeviceDisconnected
    CyclingDeviceDisconnected
also inlined:
    ResetHardwareNotificationMetrics()
    ResetServerNotificationMetrics()
*/
};
inline std::unique_ptr<ConnectionManager> g_ConnectionManagerUPtr;
