#include "ZwiftApp.h"
ConnectionManager::ConnectionManager() {}
ConnectionManager::~ConnectionManager() {}
void ConnectionManager::DestroyInstance() {
    g_ConnectionManagerUPtr.reset();
}
void ConnectionManager::Unsubscribe(ConnectionSubscriber *s) {
    std::erase_if(m_subscribers, [s](ConnectionSubscriber *i) { return s == i; });
}
void ConnectionManager::Initialize() {
    g_ConnectionManagerUPtr.reset(new ConnectionManager());
}
void ConnectionManager::Subscribe(ConnectionSubscriber *s) {
    m_subscribers.push_back(s);
}
ConnectionManager *ConnectionManager::GetInstance() { zassert(IsInitialized()); return g_ConnectionManagerUPtr.get(); }
bool ConnectionManager::IsInitialized() { return g_ConnectionManagerUPtr.get() != nullptr; }
bool ConnectionManager::IsSystemAlertFeatureEnabled() {
    return Experimentation::Instance()->IsEnabled(FID_SYSTEMA);
}
bool ConnectionManager::IsPairedDevicesAlertFeatureEnabled() {
    return Experimentation::Instance()->IsEnabled(FID_PAIR_DA);
}
void ConnectionManager::UpdateZwiftServerConnectionStatus(float /*dt*/) {
    if (ZNETWORK_IsLoggedIn() && ConnectionManager::IsSystemAlertFeatureEnabled() && g_gameStartupFlowState == ZSF_11) {
        auto now = ZNETWORK_GetNetworkSyncedTimeGMT();
        if (now - m_lastConnCheck > 10) {
            ConnectivityInfo ci;
            zwift_network::get_connection_metrics(&ci);
            if ((ci.m_tcpPackets && ci.m_tcpPackets <= m_tcpPacketsLast) || (ci.m_udpPackets && ci.m_udpPackets <= m_udpPacketsLast)) {
                if (!m_disconnStart)
                    m_disconnStart = now;
                if (now - m_disconnStart > 10 && !m_bDisconnNotified) {
                    Notify(CT_SERVER, CNT_SERVER_LOST, NDT_1);
                    m_bDisconnNotified = true;
                    //OMIT zwift_analytics::ConnectivityAnalytics::AddNewEvent(this_, 1, 1, 1, now);// ConnectionManager::AddNewConnectivityAnalyticsEvent
                }
                Log("The server connection has been disconnected for %d seconds", now - m_disconnStart);
                if (m_bDisconnNotified)
                    m_field_81 = false;
            } else {
                if (m_bDisconnNotified || m_field_81) {
                    if (m_restoredStart) {
                        if (now - m_restoredStart > 10) {
                            m_field_58 = m_restoredStart = m_disconnStart = 0;
                            Notify(CT_SERVER, CNT_SERVER_RESTORED, NDT_1);
                            m_field_81 = m_bDisconnNotified = false;
                            //OMIT UpdateAnalyticsForResolvedNotification(CT_SERVER, now);
                        }
                    } else {
                        m_restoredStart = now;
                    }
                }
            }
            m_tcpPacketsLast = ci.m_tcpPackets;
            m_udpPacketsLast = ci.m_udpPackets;
            m_lastConnCheck = now;
        }
    } else if (m_disconnStart || m_field_58 || m_restoredStart || m_bDisconnNotified || m_field_81) {
        m_restoredStart = m_disconnStart = 0;
        m_field_58 = 0;
        m_bDisconnNotified = m_field_81 = false;
    }
}
void ConnectionManager::UpdateAnalyticsForResolvedNotification(ConnectionType, int64_t) {
    //OMIT
}
void ConnectionManager::UpdateConnectedDeviceStatus() {
    if (!ZNETWORK_IsLoggedIn() || !IsPairedDevicesAlertFeatureEnabled() || g_gameStartupFlowState != ZSF_11) {
        m_devProblemReported = false;
        m_infos.clear();
        m_connDevs.clear();
        return;
    }
    auto anyd = false;
    auto sport = BikeManager::Instance()->m_mainBike->m_bc->m_sport;
    m_bled = m_antd = false;
    if (sport == protobuf::RUNNING) {
        if (FitnessDeviceManager::m_pSelectedRunSpeedDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedRunSpeedDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedRunSpeedDevice, DeviceComponent::CPT_RUN_SPD);
        }
        if (FitnessDeviceManager::m_pSelectedRunCadenceDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedRunCadenceDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedRunCadenceDevice, DeviceComponent::CPT_RUN_CAD);
        }
        if (FitnessDeviceManager::m_pSelectedHRDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedHRDevice)) {
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedHRDevice, DeviceComponent::CPT_HR);
            anyd = true;
        }
    } else if (sport == protobuf::CYCLING) {
        if (FitnessDeviceManager::m_pSelectedPowerDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedPowerDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedPowerDevice, DeviceComponent::CPT_PM);
        }
        if (FitnessDeviceManager::m_pSelectedSpeedDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedSpeedDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedSpeedDevice, DeviceComponent::CPT_SPD);
        }
        if (FitnessDeviceManager::m_pSelectedCadenceDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedCadenceDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedCadenceDevice, DeviceComponent::CPT_CAD);
        }
        if (FitnessDeviceManager::m_pSelectedHRDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedHRDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedHRDevice, DeviceComponent::CPT_HR);
        }
        if (FitnessDeviceManager::m_pSelectedControllableTrainerDevice && CheckIfDeviceIsDisconnected(FitnessDeviceManager::m_pSelectedControllableTrainerDevice)) {
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedControllableTrainerDevice, DeviceComponent::CPT_CTRL);
        }
    }
    auto steerd = DeviceInputManager::Instance()->DetermineInputDeviceType();
    if (steerd && !FitnessDeviceManager::DoesSteeringDeviceHaveSignal(steerd)) {
        switch (steerd) {
        default:
            break;
        case IDT_STANDALONE:
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedSteeringDevice, DeviceComponent::CPT_STEER);
            break;
        case IDT_SMARTBIKE:
            anyd = true;
            CollectDisconnectedDeviceInfo(FitnessDeviceManager::m_pSelectedControllableTrainerDevice, DeviceComponent::CPT_STEER);
            break;
        }
    }
    //auto now = ZNETWORK_GetNetworkSyncedTimeGMT();
    auto news = std::ranges::any_of(m_connDevs, [](auto &p) { return p.second == false; });
    if (anyd) {
        if (news) {
            Notify(CT_DEVICE, CNT_DEV_LOST, NDT_2);
            m_devProblemReported = true;
            // OMIT zwift_analytics::ConnectivityAnalytics::AddNewEvent(this, 2, 8, 4, now);
            for (auto &p : m_connDevs)
                p.second = true;
        }
    } else if (!news && m_devProblemReported) {
        Notify(CT_DEVICE, CNT_DEV_RESTORED, NDT_2);
        m_devProblemReported = false;
        m_connDevs.clear();
        //OMIT ConnectionManager::UpdateAnalyticsForResolvedNotification(this, 4, now);
        m_infos.clear();
    }
}
void ConnectionManager::CollectDisconnectedDeviceInfo(ExerciseDevice *dev, DeviceComponent::ComponentType ct) {
    auto f = m_infos.find(dev->m_prefsID);
    if (f != m_infos.end()) {
        f->second.m_types.insert(ct);
        return;
    }
    m_infos[dev->m_prefsID].m_types.insert(ct);
    m_connDevs[dev->m_prefsID] = false;
}
void ConnectionManager::Notify(ConnectionType ct, NotificationType nt, NotificationDisplayType dt) {
    for (auto i : m_subscribers)
        i->HandleNotification(ct, nt, dt);
}
bool ConnectionManager::CheckIfDeviceIsDisconnected(ExerciseDevice *dev) {
    bool result = (int)(timeGetTime() - dev->m_last_time_ms) > 20'000;
    if (result) {
        switch (dev->m_protocol) {
        case DP_BLE: case DP_BLE_MOBILE:
            m_bled = true;
            break;
        default:
            m_antd = true;
            break;
        }
    }
    return result;
}
void ConnectionManager::UpdateHardwareProtocolConnectionStatus() {
    if (ZNETWORK_IsLoggedIn() && ConnectionManager::IsSystemAlertFeatureEnabled() && g_gameStartupFlowState == ZSF_11) {
        //auto now = ZNETWORK_GetNetworkSyncedTimeGMT();
        if (m_lastAntConn) {
            if (!g_bANTConnected) {
                if (!m_antProblemReported) {
                    Notify(CT_ANT, CNT_ANT_LOST, NDT_1);
                    m_antProblemReported = true;
                    //OMIT zwift_analytics::ConnectivityAnalytics::AddNewEvent(this, 1, 4, 2, now);
                }
            }
        } else if (g_bANTConnected) {
            if (m_antProblemReported) {
                Notify(CT_ANT, CNT_ANT_RESTORED, NDT_1);
                m_antProblemReported = false;
                //OMIT ConnectionManager::UpdateAnalyticsForResolvedNotification(this, 2, now);
            }
        }
        if (!m_lastBleAvail || BLEModule::Instance()->IsBLEAvailable() || m_bleProblemReported) {
            if (BLEModule::Instance()->IsBLEAvailable()) {
                if (m_bleProblemReported) {
                    Notify(CT_BLE, CNT_BLE_RESTORED, NDT_1);
                    m_bleProblemReported = false;
                    //OMIT ConnectionManager::UpdateAnalyticsForResolvedNotification(this, CT_BLE, now);
                }
            }
        } else {
            Notify(CT_BLE, CNT_BLE_LOST, NDT_1);
            m_bleProblemReported = true;
            //OMIT zwift_analytics::ConnectivityAnalytics::AddNewEvent(this, 1, 6, 3, now);
        }
        m_lastAntConn = g_bANTConnected;
        m_lastBleAvail = BLEModule::Instance()->IsBLEAvailable();
    } else if (m_bleProblemReported || m_antProblemReported || m_lastAntConn || m_lastBleAvail)
        m_bleProblemReported = m_antProblemReported = m_lastAntConn = m_lastBleAvail = false;
}
struct SupportedDeviceHelper {
    static bool IsSmartBikeSteeringSupported(const char *name) {
        return strstr(name, "Stages Bike") || strstr(name, "KICKR BIKE");
    }
};
struct GameplayEventsManager {
    int m_steering = 0;
    /*void ~GameplayEventsManager();
void CalculateEarnedStars(RoadSegment *);
void CalculateTotalStars(void);
void DestroyDialogs(void);
void DrawStarBar(RoadSegment *);
void GetBikeOverride(BikeEntity *);
void GetFrontWheelOverride(BikeEntity *);
void GetRearWheelOverride(BikeEntity *);
void Initialize(bool);
void IsBikeInFreeRideAssistZone(BikeEntity const *);
void IsBikeInOffRoadSegment(BikeEntity const *);
void LoadRoadSegment(RoadSegment **, int);
void OffRoadDecline(bool);
void Reset(void);
void ResetPlayerBike(void);
void Self(void);
void SendOffRoadAnalytics(void);
void SetRideReportText(void);
void StartOffRoadSegment(void);
void Update(float);
*/
} g_gameplayEventsManager;
DeviceInputManager *DeviceInputManager::Instance() { return &g_devInMgr; }
InputDeviceType DeviceInputManager::DetermineInputDeviceType() {
    if (FitnessDeviceManager::m_pSelectedSteeringDevice) // FitnessDeviceManager::IsAnalogSteeringPaired
        m_type = IDT_STANDALONE;
    else if (FitnessDeviceManager::m_pSelectedControllableTrainerDevice && 
        SupportedDeviceHelper::IsSmartBikeSteeringSupported(FitnessDeviceManager::m_pSelectedControllableTrainerDevice->m_name))
        m_type = IDT_SMARTBIKE;
    else if (g_gameplayEventsManager.m_steering <= 0)
        m_type = IDT_NONE;
    else
        m_type = IDT_1;
    return m_type;
}
bool DeviceInputManager::IsButtonDown(ZwiftButtonMap btn) {
    zassert(ZwiftButtonMap::INVALID < btn && btn < ZwiftButtonMap::MAX_BUTTONS);
    return m_buttonDown.test(btn);
}
bool DeviceInputManager::IsStagesSteeringEnabled() {
    static bool g_bStagesSteering = g_UserConfigDoc.GetBool("ZWIFT\\DEVCTRL\\STAGESSTEERING", false, true),
        g_featStagesSteering = Experimentation::Instance()->IsEnabled(FID_ENA_SS);
    return g_bStagesSteering && g_bStagesSteering;
}
void DeviceInputManager::PushInputEvent(InputEvent *ev) {
    auto btn = ev->m_button;
    zassert(ZwiftButtonMap::INVALID < btn && btn < ZwiftButtonMap::MAX_BUTTONS);
    InputCallback *cb;
    switch (ev->m_type) {
    case 0:
        cb = GetCallbackForKey(ev->m_par.m_down, btn);
        if (*cb)
            cb->operator()(0.0f);
        m_buttonDown[btn] = ev->m_par.m_down;
        break;
    case 1:
        cb = GetCallbackForKey(ev->m_par.m_fpar > 0.0f, btn);
        if (*cb)
            cb->operator()(ev->m_par.m_fpar);
        m_buttonDown[btn] = (ev->m_par.m_fpar > 0.0f);
        break;
    }
}
struct ControllerBinding {
    void ParseCFGFile(XMLDoc *pSrc, InputCallback *down, InputCallback *up) {
        //TODO later: kickrbike, stagesbike
    }
};
void DeviceInputManager::LoadKeyBindings(const std::string &file, KeyCatcher kk) {
    static ControllerBinding s_ctrlBinding;
    XMLDoc doc;
    if (!doc.Load(file.c_str(), nullptr)) {
        Log("Failed to open %s Retry: -1", file.c_str()); //OMIT 5 retries
        s_ctrlBinding.ParseCFGFile(&doc, m_callbacks[KP_PRESS][kk], m_callbacks[KP_RELEASE][kk]);
    }
}
void DeviceInputManager::LoadDeviceBindings() {
    if (!FitnessDeviceManager::m_pSelectedControllableTrainerDevice)
        return;
    if (strstr(FitnessDeviceManager::m_pSelectedControllableTrainerDevice->m_name, "KICKR BIKE")) {
        LoadKeyBindings("data/configs/buttons_kickrbike_gameplay.xml"s, KK_GAMEPLAY);
        LoadKeyBindings("data/configs/buttons_smartbike_ui.xml"s, KK_GUI);
    }
    if (strstr(FitnessDeviceManager::m_pSelectedControllableTrainerDevice->m_name, "Stages Bike")) {
        LoadKeyBindings("data/configs/buttons_stagesbike_gameplay.xml"s, KK_GAMEPLAY);
        LoadKeyBindings("data/configs/buttons_smartbike_ui.xml"s, KK_GUI);
    }
}
