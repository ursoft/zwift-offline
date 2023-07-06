//UT Coverage: 50%, 249/502, NEED_MORE
#include "ZwiftApp.h"
enum BLESearchSource { BSS_BUILTIN = 1, BSS_ZCA = 2 };
int g_BLESearchSources;
bool IsNewBLEMiddlewareEnabled() {
    static bool g_bIsNewBLEMiddlewareEnabled = Experimentation::Instance()->IsEnabled(FID_BLEMIDD);
    return g_bIsNewBLEMiddlewareEnabled;
}
bool JetBlackSteeringComponent::IsFeatureFlagEnabled() {
    return Experimentation::IsEnabledCached<FID_ADDDEVI>();
}
bool IsWdcErrorDialogs() {
    static bool ret = Experimentation::Instance()->IsEnabled(FID_WDCERRO);
    return ret;
}
void BLEDevice_StartSearchForLostDevices() {
    if (!g_BLESearchSources)
        zassert(!"No search types have been specified");
    if (g_BLESearchSources & BSS_BUILTIN) {
        auto bm = BLEModule::Instance();
        if (bm->HasBLE() && bm->IsBLEAvailable()) {
            protobuf::BLEPeripheralRequest rq;
            InitializeBLESearchParameters(&rq);
            LogTyped(LOG_BLE, "Starting Native BLE search");
            BLEModule::Instance()->StartScan(rq);
        }
    }
    if (g_BLESearchSources & BSS_ZCA && zwift_network::is_paired_to_phone()) {
        protobuf::BLEPeripheralRequest rq;
        InitializeBLESearchParameters(&rq);
        Log("Starting BLE search over Zwift Companion");
        zwift_network::send_ble_peripheral_request(rq);
    }
}
FTMS_ControlComponent_v3 *CreateReplaceOrGetFTMSControlComponent_v3(BLEDevice *dev) {
    FTMS_ControlComponent_v3 *ret = nullptr;
    if (dev) {
        auto pControllableTrainer = (TrainerControlComponent *)dev->FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (pControllableTrainer) {
            zassert(pControllableTrainer->GetProtocolType() != TrainerControlComponent::ZAP_PROTOCOL);
            if (pControllableTrainer->GetProtocolType() > TrainerControlComponent::P_1)
                return ret;
            dev->RemoveComponent(pControllableTrainer);
        }
        dev->m_field_1FE = true;
        ret = new FTMS_ControlComponent_v3(dev);
        dev->AddComponent(ret);
    }
    return ret;
}
void BLEDevice_CreateTrainerST3(const protobuf::BLEAdvertisement &adv, uint8_t a2, uint16_t id, BLE_SOURCE src) {
    auto hash = SIG_CalcCaseInsensitiveSignature(adv.per().device_id().c_str());
    if (adv.per().has_device_id() && adv.per().has_device_name() && !FitnessDeviceManager::FindDevice(hash & 0xFFFFFFF | 0x10000000)) {
        auto v20 = new BLEDevice(adv.per().device_id(), adv.per().device_name(), 0, hash, src);
        sprintf_s(v20->m_nameIdBuf, "%s %04X", adv.per().device_name().c_str(), id);
        v20->m_field_11C = a2;
        FitnessDeviceManager::AddDevice(v20, v20->m_nameIdBuf);
        if (!v20->FindComponentOfType(DeviceComponent::CPT_PM))
            v20->AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
        if (!v20->FindComponentOfType(DeviceComponent::CPT_CAD))
            v20->AddComponent(new SensorValueComponent(DeviceComponent::CPT_CAD));
        if (!v20->FindComponentOfType(DeviceComponent::CPT_HR))
            v20->AddComponent(new SensorValueComponent(DeviceComponent::CPT_HR));
        auto ftms = CreateReplaceOrGetFTMSControlComponent_v3(v20);
        if (ftms)
            ftms->m_field_36 = true;
    }
}
bool IsWahooDirectConnectEnabled() { return Experimentation::IsEnabledCached<FID_WAHOOD>(); }
bool g_IgnoreReceivedBluetoothPackets;
protobuf::BLEPeripheralResponse_Error g_lastBLEError = protobuf::BL_ERR_UNK;
void BLE_GenerateFriendlyName(const std::string &src, char *dest) {
    auto csrc = src.c_str();
    const char *v6 = csrc;
    if (csrc[0] == 'D' && csrc[1] == 'I' && !csrc[2])
        v6 = "Direto";
    else if (csrc[0] == 'D' && csrc[1] == 'R' && !csrc[2])
        v6 = "Drivo";
    else if (csrc[0] == 'R' && csrc[1] == 'M' && !csrc[2])
        v6 = "Rampa";
    strncpy(dest, v6, 255);
    dest[255] = 0;
}
bool g_EnableDeviceDiscovery = true;
bool isJetBlackSteering(const std::string &name) { return name.find("Smart Wheel Block"s) != std::string::npos; }
bool isEliteSteering(const std::string &name) { return name.find("STERZO"s) != std::string::npos && name.find("RIZER"s) != std::string::npos; }
void BLEDevice_ProcessBLEResponse(const protobuf::BLEPeripheralResponse &resp, BLE_SOURCE src) {
    if (g_IgnoreReceivedBluetoothPackets)
        return;
    if (resp.has_err_msg()) {
        Log("BLE Response error %d : %s", resp.err_kind(), resp.err_msg().c_str());
        g_lastBLEError = resp.err_kind();
        return;
    }
    g_lastBLEError = protobuf::BL_ERR_UNK;
    uint32_t charId = 0;
    if (1 != sscanf(resp.chr().id().c_str(), "%x", &charId) && resp.type() != protobuf::BL_TY_3 && resp.type() != protobuf::BL_TY_4)
        return LogTyped(LOG_BLE, "Failed to parse characteristic UUID in response (type %d) for device \"%s\"", resp.type(), resp.per().device_name().c_str());
    if (charId == 0x2A53 && FitnessDeviceManager::m_PairingSport != protobuf::RUNNING)
        return;
    if (charId == 0x2A63 && FitnessDeviceManager::m_PairingSport == protobuf::RUNNING)
        return;
    switch (charId) {
    case 0xA026E005: case 0x6E40FEC2: case 0x6E40FEC3:
        charId = 0x2A63;
        break;
    case 0xE9410102:
        charId = 0xE9410101;
        break;
    case 0xE9410203:
        charId = 0xE9410201;
        break;
    case 0xA026E01D:
        charId = 0xA026E01F;
        break;
    case 0xC46BE5F:
        charId = 0xC46BE60;
        break;
    }
    auto device_id = resp.per().device_id();
    auto device_name = resp.per().device_name();
    auto cdevice_id = device_id.c_str();
    auto cdevice_name = device_name.c_str();
    auto hash = SIG_CalcCaseInsensitiveSignature(cdevice_id);
    auto uid = BLEDevice::CreateUniqueID(hash);
    auto Device = (BLEDevice *)FitnessDeviceManager::FindDevice(uid);
    if (Device && Device->m_devId != device_id)
        LogTyped(LOG_BLE, "Hash Collision: '%s' & '%s'", Device->m_devId.c_str(), cdevice_id);
    if (resp.type() != protobuf::BL_TY_4) {
        bool zcaOk = (g_BLESearchSources & BSS_ZCA) && (src == BLES_ZCA);
        bool builtinOk = ((g_BLESearchSources & BSS_BUILTIN) && src == BLES_BUILTIN && BLEModule::Instance()->IsScanning()) || BLEModule::Instance()->IsRecoveringLostDevices();
        bool netwOk = (src == BLES_WFTN) && IsWahooDirectConnectEnabled();
        bool runPod = false;
        char nameBuf[256]{}, nameMfg[256]{};
        if (!Device && g_EnableDeviceDiscovery && (zcaOk || builtinOk || netwOk)) {
            if (charId != 0x2A53 && (strstr(cdevice_name, "Milestone") || strstr(cdevice_name, "Zwift RunPod"))) {
                runPod = true;
                Device = new RunPod_BLE(device_id, device_name, charId, hash, src);
            } else {
                Device = new BLEDevice(device_id, device_name, charId, hash, src);
            }
            switch (charId)
            {
            case 0xA026E01F:
                sprintf_s(nameBuf, "%s %d", cdevice_name, hash);
                sprintf_s(nameMfg, "???");
                break;
            case 0xE9410101:
                sprintf_s(nameBuf, "%s %d", cdevice_name, hash);
                sprintf_s(nameMfg, "inRide");
                break;
            case 0xE9410201:
                sprintf_s(nameBuf, "%s %d", cdevice_name, hash);
                sprintf_s(nameMfg, "Kinetic");
                break;
            case 0x2A63: case 0x2A37: case 0x2A53: case 0x2A5B:
                if (runPod)
                    sprintf_s(nameBuf, "%s", cdevice_name);
                else
                    sprintf_s(nameBuf, "%s %d", cdevice_name, hash);
                sprintf_s(nameMfg, "%s", cdevice_name);
                break;
            default:
                BLE_GenerateFriendlyName(device_name, nameBuf);
                BLE_GenerateFriendlyName(device_name, nameMfg);
                break;
            }
            strcpy_s(Device->m_nameIdBuf, nameMfg);
            FitnessDeviceManager::AddDevice(Device, nameBuf);
            Device->m_rssiTime = timeGetTime();
            Device->m_rssi = resp.per().rssi();
        } else {
            Device->m_rssiTime = timeGetTime();
            Device->m_rssi = resp.per().rssi();
            switch (charId) {
            case 0x347B0031: case 0x347B0032: case 0xA026E010: case 0x347B0011:
                break;
            case 0xE9410101:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_PM))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
                if (!Device->FindComponentOfType(DeviceComponent::CPT_CAD))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_CAD));
                break;
            case 0xE9410201:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_PM))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
                break;
            case 0x347B0030:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_STEER) && isEliteSteering(Device->m_name))
                    Device->AddComponent(new EliteSteeringComponent(Device));
                break;
            case 0x347B0010: case 0x58094966: case 0x6E40FEC2: case 0x6E40FEC3:
                if (!isEliteSteering(Device->m_name) && !Device->FindComponentOfType(DeviceComponent::CPT_PM))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
                break;
            case 0x26D42A4D:
                if (JetBlackSteeringComponent::IsFeatureFlagEnabled() && !Device->FindComponentOfType(DeviceComponent::CPT_STEER))
                    if (isJetBlackSteering(Device->m_name))
                        Device->AddComponent(new JetBlackSteeringComponent(Device));
                break;
            case 0x2A37:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_HR))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_HR));
                break;
            case 0x2A63:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_PM))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
                break;
            case 0x2ACD:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_RUN_SPD))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_RUN_SPD));
                break;
            case 0x2AD2:
                if (!strstr(Device->m_name, "WattbikeAtom")) {
                    if (!Device->FindComponentOfType(DeviceComponent::CPT_PM))
                        Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
                    Device->m_field_1FD = true;
                }
                break;
            case 0x2AD9:
                if (!strstr(Device->m_name, "WattbikeAtom"))
                    Device->m_field_1FE = true;
                break;
            case 0x2A53: {
                bool v68 = false;
                if (!Device->FindComponentOfType(DeviceComponent::CPT_RUN_CAD)) {
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_RUN_CAD));
                    v68 = true;
                }
                if (Device->FindComponentOfType(DeviceComponent::CPT_RUN_SPD)) {
                    if (v68)
                        Device->m_field_1FC = true;
                } else {
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_RUN_SPD));
                    Device->m_field_1FC = true;
                }}
                break;
            case 0x2A5B:
                if (!Device->FindComponentOfType(DeviceComponent::CPT_SPD))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_SPD));
                if (!Device->FindComponentOfType(DeviceComponent::CPT_CAD))
                    Device->AddComponent(new SensorValueComponent(DeviceComponent::CPT_CAD));
                if (!Device->FindComponentOfType(DeviceComponent::CPT_7))
                    Device->AddComponent(new Component_7(DeviceComponent::CPT_7));
                break;
            default:
#if 0 //later
                if (charId > 0xFFB2 && !Device->FindComponentOfType(DeviceComponent::CPT_CTRL))
                Device->m_scharId = resp.chr().id();
                per_f3 = operator new(0x78ui64);
                v95 = KICKR_BLEM_ControlComponent_ctr(per_f3, v94, Device);
                v96 = (v95 + 8);
                if (!v95)
                    v96 = 0i64;
                ExerciseDevice::AddComponent(Device, v96);
                v97 = &device_name;
                v98 = device_name._Myres >= 0x10ui64;
                v99 = device_name._Bx._Ptr;
                if (device_name._Myres >= 0x10ui64)
                    v97 = device_name._Bx._Ptr;
                v100 = device_name._Mysize;
                if (device_name._Mysize < 5ui64)
                    goto LABEL_286;
                v101 = &v97->_Bx._Buf[device_name._Mysize];
                v102 = memchr(v97, 75, device_name._Mysize - 4);
                if (!v102)
                    goto LABEL_286;
                while (1)
                {
                    v103 = 0i64;
                    while (1)
                    {
                        v104 = v102[v103++];
                        if (v104 != aKickr[v103 - 1])
                            break;
                        if (v103 == 5)
                        {
                            v105 = 0;
                            goto LABEL_282;
                        }
                    }
                    v105 = v104 < aKickr[v103 - 1] ? -1 : 1;
                LABEL_282:
                    if (!v105)
                        break;
                    v102 = memchr(v102 + 1, 75, v101 - 4 - (v102 + 1));
                    if (!v102)
                        goto LABEL_286;
                }
                if (v102 - v97 == -1)
                {
                LABEL_286:
                    v106 = &device_name;
                    if (v98)
                        v106 = v99;
                    if (v100 < 0xD)
                        goto LABEL_292;
                    v107 = memchr(v106, 77, v100 - 12);
                    if (!v107)
                        goto LABEL_292;
                    while (memcmp(v107, "MINOURA SMART", 0xDui64))
                    {
                        v107 = memchr(v107 + 1, 77, v106 + v100 - 12 - (v107 + 1));
                        if (!v107)
                            goto LABEL_292;
                    }
                    if (v107 - v106 == -1)
                    {
                    LABEL_292:
                        v108 = 0;
                        goto LABEL_293;
                    }
                }
                v108 = 1;
            LABEL_293:
                *(v95 + 54) = v108;
                v109 = &device_name;
                v110 = device_name._Myres >= 0x10ui64;
                if (device_name._Myres >= 0x10ui64)
                    v109 = device_name._Bx._Ptr;
                v111 = device_name._Mysize;
                if (device_name._Mysize < 6ui64)
                    goto LABEL_308;
                v112 = &v109->_Bx._Buf[device_name._Mysize];
                v113 = memchr(v109, 77, device_name._Mysize - 5);
                if (!v113)
                    goto LABEL_308;
                while (1)
                {
                    v114 = 0i64;
                    while (1)
                    {
                        v115 = v113[v114++];
                        if (v115 != aMagnus[v114 - 1])
                            break;
                        if (v114 == 6)
                        {
                            v116 = 0;
                            goto LABEL_304;
                        }
                    }
                    v116 = v115 < aMagnus[v114 - 1] ? -1 : 1;
                LABEL_304:
                    if (!v116)
                        break;
                    v113 = memchr(v113 + 1, 77, v112 - 5 - (v113 + 1));
                    if (!v113)
                        goto LABEL_308;
                }
                if (v113 - v109 == -1)
                {
                LABEL_308:
                    v117 = &device_name;
                    if (v110)
                        v117 = device_name._Bx._Ptr;
                    if (v111 < 6)
                        goto LABEL_157;
                    v118 = memchr(v117, 72, v111 - 5);
                    if (!v118)
                        goto LABEL_157;
                    while (1)
                    {
                        v119 = 0i64;
                        while (1)
                        {
                            v120 = v118[v119++];
                            if (v120 != aHammer[v119 - 1])
                                break;
                            if (v119 == 6)
                            {
                                v121 = 0;
                                goto LABEL_317;
                            }
                        }
                        v121 = v120 < aHammer[v119 - 1] ? -1 : 1;
                    LABEL_317:
                        if (!v121)
                            break;
                        v118 = memchr(v118 + 1, 72, v117 + v111 - 5 - (v118 + 1));
                        if (!v118)
                            goto LABEL_157;
                    }
                    if (v118 - v117 == -1)
                        goto LABEL_157;
                }
                *(v95 + 55) = 1;
#endif
                break;
            }
        }
    }
    switch (resp.type()) {
    case protobuf::BL_TY_0: case protobuf::BL_TY_5:
        if (Device)
            Device->m_last_time_ms = timeGetTime();
        break;
    case protobuf::BL_TY_2:
        Device->ProcessBLEData(resp);
        break;
    case protobuf::BL_TY_3:
        if (Device) {
            static uint32_t g_lastTY3ts;
            if (GFX_GetFrameCount() != g_lastTY3ts) {
                g_lastTY3ts = GFX_GetFrameCount();
                Device->SetPaired(true);
                auto steer = (SensorValueComponent *)Device->FindComponentOfType(DeviceComponent::CPT_STEER);
                if (steer && steer->GetSensorType() == ST_ELITE_STEER)
                    ((EliteSteeringComponent *)steer)->m_ts = timeGetTime();
                if (JetBlackSteeringComponent::IsFeatureFlagEnabled()) {
                    if (steer && steer->GetSensorType() == ST_JB_STEER) {
                        auto jb = (JetBlackSteeringComponent *)steer;
                        jb->m_field_34 = 0;
                        jb->m_val = 0.0f;
                        Log("JetBlackSteeringComponent::SetCenter()");
                    }
                }
            }
            Device->ProcessBLEData(resp);
        }
        break;
    case protobuf::BL_TY_4:
        if (Device) {
            Device->SetPaired(true);
            auto steer = (SensorValueComponent *)Device->FindComponentOfType(DeviceComponent::CPT_STEER);
            if (steer && steer->GetSensorType() == ST_ELITE_STEER)
                ((EliteSteeringComponent *)steer)->m_parent->SetPaired(false);
            if (JetBlackSteeringComponent::IsFeatureFlagEnabled()) {
                if (steer && steer->GetSensorType() == ST_JB_STEER) {
                    auto jb = (JetBlackSteeringComponent *)steer;
                    jb->m_parent->SetPaired(false);
                }
            }
            Device->ProcessBLEData(resp);
        }
        break;
    }
}
struct dllBLEPeripheralResponseChr {
    std::string m_id;
    char *m_buffer;
    uint32_t m_bufferSize;
};
struct dllBLEPeripheralResponseSrv {
    std::string m_id;
    std::vector<dllBLEPeripheralResponseChr> m_chrValues;
};
struct dllBLEPeripheralResponse {
    protobuf::BLEPeripheralResponse_Type m_type;
    int gap;
    std::string m_deviceId, m_deviceName;
    int16_t m_rssi;
    std::vector<dllBLEPeripheralResponseSrv> m_servs;
};
struct dllBLEAdvertisementDataSection { //32 bytes
    char m_type;
    std::vector<char> m_manufData;
};
struct dllBLEAdvertisement { //0x60 bytes
    uint16_t m_per_f3;
    std::string m_deviceId, m_deviceName;
    std::vector<dllBLEAdvertisementDataSection> m_dataSects;
};
struct dllBLEPeripheralRequest { //56 bytes
    std::string m_id;
    std::vector<std::string> m_chars;
};
std::vector<dllBLEPeripheralRequest> g_dllBLEPeripheralRequests;
void parseDllBLEPeripheralResponse(const dllBLEPeripheralResponse &src, protobuf::BLEPeripheralResponse *dest) {
    if (src.m_type >= protobuf::BL_TY_0 && src.m_type <= protobuf::BL_TY_5) { //TACX 1816/2A55(any): BL_TY_5
        dest->set_type(src.m_type);
    }
    auto per = dest->mutable_per();
    per->set_rssi(src.m_rssi); //TACX 1816/2A55: FFB6; TACX 1818/2A55:FFB3
    per->set_device_id(src.m_deviceId); //TACX 1816/2A55(any): 274523460006625
    per->set_device_name(src.m_deviceName);
}
void parseDllBLEAdvertisement(const dllBLEAdvertisement &src, protobuf::BLEAdvertisement *dest) {
    auto per = dest->mutable_per();
    per->set_rssi(src.m_per_f3);
    per->set_device_id(src.m_deviceId);
    per->set_device_name(src.m_deviceName);
    for (auto &dsi : src.m_dataSects)
        if (dsi.m_type == -1) {
            auto v16 = dest->add_sects();
            v16->set_type(protobuf::MANUFACTURER_DATA);
            v16->set_data(std::string(dsi.m_manufData.data(), dsi.m_manufData.size()));
        }
}
void cbProcessBLEResponseChr(const dllBLEPeripheralResponseChr &chr, const dllBLEPeripheralResponse &resp) {
    protobuf::BLEPeripheralResponse v20;
    v20.set_type(protobuf::BL_TY_2);
    auto per = v20.mutable_per();
    per->set_device_name(resp.m_deviceName);
    per->set_device_id(resp.m_deviceId);
    per->set_rssi(resp.m_rssi);
    auto lchr = v20.mutable_chr();
    auto v19 = chr.m_id;
    lchr->set_id(v19);
    v19.resize(8);
    lchr->set_value(v19);
    BLEModule::Instance()->ProcessBLEResponse(v20, BLES_BUILTIN);
}
void cbProcessBLEResponse(dllBLEPeripheralResponse *resp);
void cbConnectionStatusCBFunc(dllBLEPeripheralResponse *resp) {
    if (resp->m_type == protobuf::BL_TY_3) {
        cbProcessBLEResponse(resp);
    } else if (resp->m_type == protobuf::BL_TY_4) {
        protobuf::BLEPeripheralResponse v4;
        parseDllBLEPeripheralResponse(*resp, &v4);
        BLEModule::Instance()->ProcessBLEResponse(v4, BLES_BUILTIN);
    }
}
void cbPeripheralDiscoveryFunc(dllBLEAdvertisement *adv) {
    protobuf::BLEAdvertisement pbadv;
    parseDllBLEAdvertisement(*adv, &pbadv);
    BLEModule::Instance()->ProcessDiscovery(pbadv, BLES_BUILTIN);
}
void cbOnPairCB(dllBLEPeripheralResponse *resp) {
    protobuf::BLEPeripheralResponse v14;
    parseDllBLEPeripheralResponse(*resp, &v14);
    BLEModule::Instance()->SetDeviceConnectedFlag(v14, true);
}
void cbAssertFunc(bool cond, const char *msg) {
    if (!cond) {
        if (ZwiftAssert::IsBeingDebugged()) __debugbreak();
        if (ZwiftAssert::BeforeAbort(msg, __FILE__, __LINE__)) 
            ZwiftAssert::Abort();
    }
}
void cbLogFunc(const char *msg) {
    LogTyped(LOG_BLE, msg);
}
void cbEnableSupportsBLEFunc(bool en);
void cbBLEIsAvailableFunc(bool yes);
struct dllBLEError {
    void *gap;
    std::string _a2, _a3;
};
void cbBLEErrorFunc(dllBLEError *pDllStruct, BLE_ERROR_TYPE err) {
    static_assert(sizeof(dllBLEError) == 0x48);
    BLEModule::Instance()->DidReceiveError(pDllStruct->_a2.c_str(), pDllStruct->_a3.c_str(), err, 0);
}
/*
BLEDevice_HasCharacteristic(std::string const &, std::string const &)
BLEDevice_IsCurrentlyRecoveringLostDevices(void)
BLEDevice_IsCurrentlyScanning(void)
BLEDevice_IsSmartTrainer(std::string const &)
BLEDevice_Pair(BLEDevice const &)
BLEDevice_ProcessAdvertisedManufacturerData(zwift::protobuf::BLEAdvertisement const &, std::string const &, BLE_SOURCE)
BLEDevice_ProcessAdvertisedServiceUUIDs(zwift::protobuf::BLEAdvertisement const &, std::string const &, zwift::protobuf::BLEAdvertisementDataSection_Type, BLE_SOURCE)
BLEDevice_SendValueToDevice(zwift::protobuf::BLEPeripheralRequest const &, BLE_SOURCE)
BLEDevice_SetBleInitFinishedFlag(bool)
BLEDevice_SetDeviceConnectedFlag(zwift::protobuf::BLEPeripheralResponse const &, bool)
BLEDevice_SetGrantedFlag(bool)
BLEDevice_SetScanningFlag(bool)
BLEDevice_StartSearch(zwift::protobuf::BLEPeripheralRequest const &)
BLEDevice_StartSearchForLostDevices(void)
BLEDevice_StopSearch(void)
BLEDevice_StopSearchForLostDevices(void)
BLEDevice_Unpair(BLEDevice const &)
BLEDevice_UpdateConnectionState(std::string const &, std::string const &, int)*/
bool g_windowsBleSupported, g_windowsBleDllLoaded, g_windowsBleDllFunctionsOK;
uint32_t g_bleDllLoadingError;
typedef void (*fptr_void_void)();
typedef void (*fptr_void_ptr)(void *);
typedef bool (*unpairFromDeviceFunc_t)(uint64_t id);
typedef bool (*pairToDeviceFunc_t)(uint64_t id);
typedef bool (*writeToDeviceFunc_t)(uint64_t id, const std::string &chrId, const std::string &val);
#pragma comment(lib, "Version.lib")
struct BLEDeviceManager { //and BLEDeviceManagerWindows
    fptr_void_ptr m_startScanningFunc;
    fptr_void_void m_stopScanningFunc;
    fptr_void_void m_purgeDeviceListFunc;
    pairToDeviceFunc_t m_pairToDeviceFunc;
    unpairFromDeviceFunc_t m_unpairFromDeviceFunc;
    writeToDeviceFunc_t m_writeToDeviceFunc;
    fptr_void_void m_initFlagsFunc;
    std::vector<std::string> m_lostDevs;
    enum DeviceState { BLE_DEVICE_STATE_UNK, BLE_DEVICE_STATE_IDLE, BLE_DEVICE_STATE_SCANNING, BLE_DEVICE_STATE_RECOVERING } m_deviceState = BLE_DEVICE_STATE_UNK;
    bool m_HasBLE, m_AutoConnectPairingMode, m_bleAvalilable, m_initFlagCalled;
    ~BLEDeviceManager() { //vptr[0]
        if (m_HasBLE) {
            if (m_stopScanningFunc) {
                if (m_deviceState == BLE_DEVICE_STATE_SCANNING) {
                    m_stopScanningFunc();
                    FitnessDeviceManager::m_lastBLESearchTime = timeGetTime();
                }
            } else {
                zassert(!"BLE - Calling StopDeviceSearch when there is no stop device search function");
            }
            m_deviceState = BLE_DEVICE_STATE_IDLE;
        }
    }
    HMODULE doLoadBleDll(const char *dllName, uint16_t winBuild, uint32_t *pErr) {
        auto ret = LoadLibraryA(dllName);
        if (ret) {
            LogTyped(LOG_BLE, "Loaded DLL %s", dllName);
        } else {
            auto LastError = GetLastError();
            if (pErr)
                *pErr = LastError;
            //OMIT dumpBadWindowsForBLE("calling LoadLibrary", dllName, winBuild, LastError);
            Log("BLEDeviceManager::doLoadBleDll: calling LoadLibrary(%s) err: %d", dllName, LastError);
        }
        return ret;
    }
    void bleLoadDLL(uint16_t winBuild) {
        static bool st_bNewBleDll = Experimentation::Instance()->IsEnabled(FID_BLEDLLV);
        auto bleName = "BleWin10Lib_V2.dll";
        if (!st_bNewBleDll)
            bleName = "BleWin10Lib.dll";
        uint32_t err = 0;
        auto BleDll = doLoadBleDll(bleName, winBuild, &err);
        if (!BleDll && !st_bNewBleDll) {
            LogTyped(LOG_WARNING, "Falling back to default BLE DLL: BleWin10Lib.dll");
            bleName = "BleWin10Lib.dll";
            BleDll = doLoadBleDll(bleName, winBuild, &err);
            if (!BleDll) {
                g_bleDllLoadingError = err;
                Log("Could not load the Windows BLE DLL %s due to the following error %d", bleName, err);
                return;
            }
        }
        g_windowsBleDllLoaded = true;
        m_HasBLE = true;
        auto startScanningFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLEStartScanning");
        auto stopScanningFuncBLEStopScanning = (fptr_void_void)GetProcAddress(BleDll, "BLEStopScanning");
        auto purgeDeviceListFunc = (fptr_void_void)GetProcAddress(BleDll, "BLEPurgeDeviceList");
        auto pairToDeviceFunc = (pairToDeviceFunc_t)GetProcAddress(BleDll, "BLEPairToDevice");
        auto unpairFromDeviceFunc = (unpairFromDeviceFunc_t)GetProcAddress(BleDll, "BLEUnpairFromDevice");
        auto writeToDeviceFunc = (writeToDeviceFunc_t)GetProcAddress(BleDll, "BLEWriteToDevice");
        auto initFlagsFunc = (fptr_void_void)GetProcAddress(BleDll, "BLEInitBLEFlags");
        auto BLESetProcessBLEResponse = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetProcessBLEResponse");
        auto BLESetPeripheralDiscoveryFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetPeripheralDiscoveryFunc");
        auto BLESetOnPairCB = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetOnPairCB");
        auto BLESetAssertFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetAssertFunc");
        auto BLESetLogFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetLogFunc");
        auto BLESetConnectionStatusCBFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetConnectionStatusCBFunc");
        auto BLESetEnableSupportsBLEFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetEnableSupportsBLEFunc");
        auto BLESetEnableBLEIsAvailableFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetEnableBLEIsAvailableFunc");
        auto BLESetBLEErrorFunc = (fptr_void_ptr)GetProcAddress(BleDll, "BLESetBLEErrorFunc");
        bool hasAllFuncs = startScanningFunc && stopScanningFuncBLEStopScanning && purgeDeviceListFunc && pairToDeviceFunc && unpairFromDeviceFunc && writeToDeviceFunc &&
            initFlagsFunc && BLESetProcessBLEResponse && BLESetPeripheralDiscoveryFunc && BLESetOnPairCB && BLESetAssertFunc && BLESetLogFunc && BLESetConnectionStatusCBFunc &&
            BLESetEnableSupportsBLEFunc && BLESetEnableBLEIsAvailableFunc && BLESetBLEErrorFunc;
        if (hasAllFuncs) {
            m_startScanningFunc = startScanningFunc;
            m_stopScanningFunc = stopScanningFuncBLEStopScanning;
            m_purgeDeviceListFunc = purgeDeviceListFunc;
            m_pairToDeviceFunc = pairToDeviceFunc;
            m_unpairFromDeviceFunc = unpairFromDeviceFunc;
            m_writeToDeviceFunc = writeToDeviceFunc;
            m_initFlagsFunc = initFlagsFunc;
            BLESetProcessBLEResponse(cbProcessBLEResponse);
            BLESetPeripheralDiscoveryFunc(cbPeripheralDiscoveryFunc);
            BLESetOnPairCB(cbOnPairCB);
            BLESetAssertFunc(cbAssertFunc);
            BLESetLogFunc(cbLogFunc);
            BLESetConnectionStatusCBFunc(cbConnectionStatusCBFunc);
            BLESetEnableSupportsBLEFunc(cbEnableSupportsBLEFunc);
            BLESetEnableBLEIsAvailableFunc(cbBLEIsAvailableFunc);
            BLESetBLEErrorFunc(cbBLEErrorFunc);
            g_windowsBleDllFunctionsOK = true;
            return;
        }
        zassert(hasAllFuncs);
        g_windowsBleDllFunctionsOK = false;
        //OMIT dumpBadWindowsForBLE("missing functions", v4, winBuild, 0);
        Log("BLEDeviceManager::bleLoadDLL: missing functions");
    }
    void Init() { //vptr[1]
        if (m_deviceState == BLE_DEVICE_STATE_UNK) {
            g_windowsBleSupported = g_windowsBleDllLoaded = g_windowsBleDllFunctionsOK = false;
            g_bleDllLoadingError = 0;
            m_HasBLE = false;
            DWORD dwHandle;
            auto FileVersionInfoSizeW = GetFileVersionInfoSizeW(L"kernel32.dll", &dwHandle);
            if (FileVersionInfoSizeW) {
                auto v7 = calloc(FileVersionInfoSizeW, sizeof(wchar_t));
                if (GetFileVersionInfoW(L"kernel32.dll", 0, FileVersionInfoSizeW, v7)) {
                    VS_FIXEDFILEINFO *vi = nullptr;
                    UINT len = 0;
                    VerQueryValueW(v7, L"\\", (LPVOID *)&vi, &len);
                    uint16_t winBuild = HIWORD(vi->dwProductVersionLS);
                    if (HIWORD(vi->dwFileVersionMS) != 10 || LOWORD(vi->dwFileVersionMS) || winBuild < 15063) {
                        g_windowsBleSupported = false;
                        //OMIT dumpBadWindowsForBLE("bad windows build", L"N/A", winBuild, 0);
                        Log("BLEDeviceManager::Init: bad windows build: %d", winBuild);
                    } else {
                        g_windowsBleSupported = true;
                        bleLoadDLL(winBuild);
                    }
                    free(v7);
                } else {
                    free(v7);
                    Log("Windows Version: GetFileVersionInfo failed");
                }
            } else {
                Log("Windows Version: Cannot locate the following DLL for version checking: kernel32.dll");
            }
            zassert(m_deviceState == BLE_DEVICE_STATE_UNK);
            m_deviceState = BLE_DEVICE_STATE_IDLE;
            if (m_HasBLE) {
                zassert(m_startScanningFunc);
                zassert(m_stopScanningFunc);
                zassert(m_purgeDeviceListFunc);
                zassert(m_pairToDeviceFunc);
                zassert(m_unpairFromDeviceFunc);
                zassert(m_writeToDeviceFunc);
                if (!m_initFlagCalled) {
                    m_initFlagCalled = true;
                    m_initFlagsFunc();
                }
            }
        }
    }
    void StartScan(const protobuf::BLEPeripheralRequest &req) { //vptr[2]
        static_assert(sizeof(dllBLEPeripheralRequest) == 56);
        if (this->m_deviceState != BLE_DEVICE_STATE_SCANNING) {
            zassert(m_HasBLE);
            zassert(m_deviceState == BLE_DEVICE_STATE_IDLE);
            m_deviceState = BLE_DEVICE_STATE_SCANNING;
            if (m_startScanningFunc) {
                g_dllBLEPeripheralRequests.resize(req.servs_size());
                int i = 0;
                for (auto &serv : req.servs()) {
                    auto &dst = g_dllBLEPeripheralRequests[i++];
                    dst.m_id = serv.id();
                    dst.m_chars.resize(serv.chars_size());
                    int j = 0;
                    for (auto &chr : serv.chars())
                        dst.m_chars[j++] = chr.id();
                }
                m_startScanningFunc(&g_dllBLEPeripheralRequests);
            } else {
                zassert(!"BLE - Calling StartDeviceSearch when there is no start scanning function");
            }
        }
    }
    void StopScan() { //vptr[3]
        if (m_HasBLE) {
            if (m_stopScanningFunc) {
                if (m_deviceState == BLE_DEVICE_STATE_SCANNING) {
                    m_stopScanningFunc();
                    FitnessDeviceManager::m_lastBLESearchTime = timeGetTime();
                }
            } else {
                zassert(!"BLE - Calling StopDeviceSearch when there is no stop device search function");
            }
            m_deviceState = BLE_DEVICE_STATE_IDLE;
        }
    }
    void PurgeDeviceList() { //vptr[4]
        if (m_purgeDeviceListFunc)
            m_purgeDeviceListFunc();
    }
    void SendValueToDevice(const protobuf::BLEPeripheralRequest &rq) { //vptr[5]
        if (m_writeToDeviceFunc) {
            if (rq.type() == protobuf::WRITE_CHARACTERISTIC_VALUE && rq.has_per()) {
                auto devId = std::strtoull(rq.per().device_id().c_str(), nullptr, 10);
                for (auto &serv : rq.servs())
                    for (auto &chr : serv.chars())
                        if (chr.has_value())
                            m_writeToDeviceFunc(devId, chr.id(), chr.value());
            }
        } else {
            zassert("BLE - Calling SendValueToDevice when there is no write to device function");
        }
    }
    void StartSearchForLostDevices() { //vptr[6]
        BLEDevice_StartSearchForLostDevices();
        m_deviceState = BLE_DEVICE_STATE_RECOVERING;
    }
    void StopSearchForLostDevices() { //vptr[7]
        if (m_HasBLE) {
            if (m_stopScanningFunc) {
                if (m_deviceState == BLE_DEVICE_STATE_RECOVERING) {
                    m_stopScanningFunc();
                    m_lostDevs.clear();
                }
            } else {
                zassert(!"BLE - Calling StopDeviceSearch when there is no stop device search function");
            }
            m_deviceState = BLE_DEVICE_STATE_IDLE;
        }
    }
    void PairDevice(const BLEDevice &dev) { //vptr[8]
        if (!dev.IsPaired()) {
            if (m_pairToDeviceFunc) {
                auto v9 = std::strtoull(dev.m_devId.c_str(), nullptr, 10);
                if (!m_pairToDeviceFunc(v9))
                    Log("BLE: Pairing failed for %s", dev.m_devId.c_str());
            } else {
                zassert(!"BLE - Calling PairDevice when there is no pair to device function");
            }
        }
    }
    void UnpairDevice(const BLEDevice &dev) { //vptr[9]
        if (m_unpairFromDeviceFunc) {
            auto v9 = std::strtoull(dev.m_devId.c_str(), nullptr, 10);
            if (!m_unpairFromDeviceFunc(v9))
                Log("BLE: Unpairing failed for %s", dev.m_devId.c_str());
        } else {
            zassert(!"BLE - Calling UnPairDevice when there is no unpair from device function");
        }
    }
} g_BLEDeviceManager;
void InitializeBLESearchParameters(protobuf::BLEPeripheralRequest *rq) {
    rq->set_type(protobuf::BEGIN_PERIPHERAL_DISCOVERY);
    auto v4 = rq->add_servs();
    v4->set_id("0x1818"s);
    auto v12 = v4->add_chars();
    v12->set_id("2A63"s);
    //if (IsNewBLEMiddlewareEnabled()) { //QUEST: not sure what is the difference
          v4->add_chars()->set_id("A026E005-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    //} else {
    //    v4->add_chars()->set_id("A026E005-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    //}
    if (Experimentation::Instance()->IsEnabled(FID_FTMS)) {
        auto v33 = rq->add_servs();
        v33->set_id("0x181C"s);
        auto v36 = v33->add_chars();
        v36->set_id("2A98"s);
    }
    auto v39 = rq->add_servs();
    v39->set_id("0x1826"s);
    v39->add_chars()->set_id("2ACC"s);
    v39->add_chars()->set_id("2AD2"s);
    v39->add_chars()->set_id("2AD9"s);
    v39->add_chars()->set_id("2ADA"s);
    v39->add_chars()->set_id("2ACD"s);
    auto v51 = rq->add_servs();
    v51->set_id("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E"s);
    v51->add_chars()->set_id("6E40FEC3-B5A3-F393-E0A9-E50E24DCCA9E"s);
    auto v55 = rq->add_servs();
    v55->set_id("B4CC1223-BC02-4CAE-ADB9-1217AD2860D1"s);
    v55->add_chars()->set_id("B4CC1224-BC02-4CAE-ADB9-1217AD2860D1"s);
    v55->add_chars()->set_id("B4CC1225-BC02-4CAE-ADB9-1217AD2860D1"s);
    auto v61 = rq->add_servs();
    v61->set_id("babf1723-cedb-444c-88c3-c672c7a59806"s);
    v61->add_chars()->set_id("babf1724-cedb-444c-88c3-c672c7a59806"s);
    auto v65 = rq->add_servs();
    v65->set_id("a913bfc0-929e-11e5-b928-0002a5d5c51b"s);
    v65->add_chars()->set_id("58094966-498C-470D-8051-37E617A13895"s);
    auto v69 = rq->add_servs();
    v69->set_id("E9410200-B434-446B-B5CC-36592FC4C724"s);
    v69->add_chars()->set_id("E9410203-B434-446B-B5CC-36592FC4C724"s);
    v69->add_chars()->set_id("E9410201-B434-446B-B5CC-36592FC4C724"s);
    auto v75 = rq->add_servs();
    v75->set_id("E9410100-B434-446B-B5CC-36592FC4C724"s);
    v75->add_chars()->set_id("E9410101-B434-446B-B5CC-36592FC4C724"s);
    if (JetBlackSteeringComponent::IsFeatureFlagEnabled()) {
        auto v79 = rq->add_servs();
        v79->set_id("26D4A3EC-2E24-4364-A1BB-883ADDFD86BC"s);
        v79->add_chars()->set_id("26D42A4D-2E24-4364-A1BB-883ADDFD86BC"s);
    }
    auto v83 = rq->add_servs();
    v83->set_id("0x1816"s);
    v83->add_chars()->set_id("2A5B"s);
    auto v87 = rq->add_servs();
    v87->set_id("347B0001-7635-408B-8918-8FF3949CE592"s);
    v87->add_chars()->set_id("347B0010-7635-408B-8918-8FF3949CE592"s);
    v87->add_chars()->set_id("347B0030-7635-408B-8918-8FF3949CE592"s);
    v87->add_chars()->set_id("347B0031-7635-408B-8918-8FF3949CE592"s);
    v87->add_chars()->set_id("347B0032-7635-408B-8918-8FF3949CE592"s);
    auto v97 = rq->add_servs();
    v97->set_id("0x180A"s);
    v97->add_chars()->set_id("2A23"s);
    v97->add_chars()->set_id("2A24"s);
    v97->add_chars()->set_id("2A25"s);
    v97->add_chars()->set_id("2A26"s);
    v97->add_chars()->set_id("2A27"s);
    v97->add_chars()->set_id("2A28"s);
    v97->add_chars()->set_id("2A29"s);
    auto v113 = rq->add_servs();
    v113->set_id("c0f4013a-a837-4165-bab9-654ef70747c6"s);
    v113->add_chars()->set_id("ca31a533-a858-4dc7-a650-fdeb6dad4c14"s);
    auto v117 = rq->add_servs();
    v117->set_id("0x1814"s);
    v117->add_chars()->set_id("2A53"s);
    auto v121 = rq->add_servs();
    v121->set_id("A026EE07-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    v121->add_chars()->set_id("A026E01F-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    v121->add_chars()->set_id("A026E01D-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    auto v127 = rq->add_servs();
    v127->set_id("A026EE0D-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    v127->add_chars()->set_id("A026E03C-0A7D-4AB3-97FA-F1500F9FEB8B"s);
    auto v131 = rq->add_servs();
    v131->set_id("C4630001-003F-4CEC-8994-E489B04D857E"s);
    v131->add_chars()->set_id("C4632B01-003F-4CEC-8994-E489B04D857E"s);
    v131->add_chars()->set_id("C4632B02-003F-4CEC-8994-E489B04D857E"s);
    auto v137 = rq->add_servs();
    v137->set_id("EDFF9E80-CAD7-11E5-AB63-0002A5D5C51B"s);
    v137->add_chars()->set_id("E3F9AF20-2674-11E3-879E-0002A5D5C51B"s);
    v137->add_chars()->set_id("4E349C00-999E-11E3-B341-0002A5D5C51B"s);
    v137->add_chars()->set_id("1717B3C0-9803-11E3-90E1-0002A5D5C51B"s);
    v137->add_chars()->set_id("6BE8F580-9803-11E3-AB03-0002A5D5C51B"s);
    v137->add_chars()->set_id("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"s);
    v137->add_chars()->set_id("b8066ec0-9803-11e3-8346-0002a5d5c51b"s);
    v137->add_chars()->set_id("d57cda20-9803-11e3-8426-0002a5d5c51b"s);
    auto v153 = rq->add_servs();
    v153->set_id("b5c78780-cad7-11e5-b9f8-0002a5d5c51b"s);
    v153->add_chars()->set_id("E3F9AF20-2674-11E3-879E-0002A5D5C51B"s);
    v153->add_chars()->set_id("4E349C00-999E-11E3-B341-0002A5D5C51B"s);
    v153->add_chars()->set_id("1717B3C0-9803-11E3-90E1-0002A5D5C51B"s);
    v153->add_chars()->set_id("6BE8F580-9803-11E3-AB03-0002A5D5C51B"s);
    v153->add_chars()->set_id("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"s);
    v153->add_chars()->set_id("b8066ec0-9803-11e3-8346-0002a5d5c51b"s);
    v153->add_chars()->set_id("d57cda20-9803-11e3-8426-0002a5d5c51b");
    auto v169 = rq->add_servs();
    v169->set_id("0C46BE5F-9C22-48FF-AE0E-C6EAE1A2F4E5"s);
    v169->add_chars()->set_id("0C46BE60-9C22-48FF-AE0E-C6EAE1A2F4E5"s);
    v169->add_chars()->set_id("0C46BE61-9C22-48FF-AE0E-C6EAE1A2F4E5"s);
    auto v175 = rq->add_servs();
    v175->set_id("0x180D"s);
    v175->add_chars()->set_id("2A37"s);
    auto v179 = rq->add_servs();
    v179->set_id("0xFFF0"s);
    v179->add_chars()->set_id("FFF1"s);
    v179->add_chars()->set_id("FFF2"s);
    v179->add_chars()->set_id("FFF5"s);
    v179->add_chars()->set_id("FFB2"s);
    auto v189 = rq->add_servs();
    v189->set_id("0x180F"s);
    v189->add_chars()->set_id("2A19"s);
}
void cbProcessBLEResponse(dllBLEPeripheralResponse *resp) {
    if (!resp->m_deviceName.empty()) {
        protobuf::BLEPeripheralResponse pbresp;
        parseDllBLEPeripheralResponse(*resp, &pbresp);
        auto chr = pbresp.mutable_chr();
        auto &per = pbresp.per(); (void)per;
        for (auto &i56 : resp->m_servs) {
            for (auto &j48 : i56.m_chrValues) {
                chr->set_id(j48.m_id);
                if (pbresp.type() == protobuf::BL_TY_2) {
                    zassert(j48.m_buffer);
                    zassert(j48.m_bufferSize);
                    chr->set_value(std::string(j48.m_buffer, j48.m_bufferSize));
                }
                BLEModule::Instance()->ProcessBLEResponse(pbresp, BLES_BUILTIN);
            }
        }
        if (resp->m_type == protobuf::BL_TY_3) {
            for (auto &i56 : resp->m_servs) {
                if (i56.m_id == "6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E"s) { //FEC_BRAKE_SERVICE (TACX)
                    zassert(i56.m_chrValues.size() == 1);
                    cbProcessBLEResponseChr(i56.m_chrValues[0], *resp);
                } else {
                    for (auto &j48 : i56.m_chrValues) {
                        if (j48.m_id == "a2ad9"s //FTMS control point chr
                            || j48.m_id == "A026E005-0A7D-4AB3-97FA-F1500F9FEB8B"s    // Wahoo
                            || j48.m_id == "E9410201-B434-446B-B5CC-36592FC4C724"s    //???
                            || j48.m_id == "347B0010-7635-408B-8918-8FF3949CE592"s)  //???
                            cbProcessBLEResponseChr(j48, *resp);
                    }
                }
            }
            BLEModule::Instance()->DidConnect(resp->m_deviceName.c_str(), resp->m_deviceId.c_str());
        } else if (resp->m_type == protobuf::BL_TY_5) {
            auto v22 = BLEDevice::CreateUniqueID(resp->m_deviceId);
            for (auto &i56 : resp->m_servs) {
                if (i56.m_id == "A026EE07-0A7D-4AB3-97FA-F1500F9FEB8B"s) { //Wahoo Fitness Equipment Service
                    (void)v22;
                    auto Device = (BLEDevice *)FitnessDeviceManager::FindDevice(v22); //QUEST: why not dynamic_cast
                    if (Device)
                        g_BLEDeviceManager.PairDevice(*Device);
                    else
                        zassert(!"Trying to auto-connect to a device that does not exist");
                }
            }
        }
    } else {
        LogTyped(LOG_BLE, "Empty device names should not get passed to BLEDevice_ProcessBLEResponse");
    }
}
void BLEModule::Initialize(Experimentation *exp) {
    g_BLEModule.reset(new BLEModule(exp));
}
void BLEModule::HandleEvent(EVENT_ID, va_list) { /*TODO*/ }
BLEModule::BLEModule(Experimentation *exp) : EventObject(exp->m_eventSystem) {
    exp->IsEnabled(FID_LOG_BLE, [this](ExpVariant val) {
        if (val == EXP_ENABLED) {
            Log("\nBLE Packet Logging is enabled.\n\n\n");
            g_BLE_LoggingON = true;
        }
    });
}
void BLEModule::Shutdown() {
    g_BLEModule->StopScan();
    g_BLEModule.reset();
}
void BLEModule::LegacyBLEImpl::DidConnect(const char *devName, const char *devId) {
    if (devName && devId)
        FitnessDeviceManager::HandleBLEConnect(BLEDevice::CreateUniqueID(devId));
}
void BLEModule::LegacyBLEImpl::DidReceiveError(const char *devName, const char *devId, BLE_ERROR_TYPE err, uint32_t a5) {
    if (devName && devId)
        FitnessDeviceManager::HandleBLEError(BLEDevice::CreateUniqueID(devId), err, a5);
}
void BLEModule::LegacyBLEImpl::DidRecover(const char *devName, const char *devId) {
    if (devName && devId)
        FitnessDeviceManager::HandleBLEReconnect(BLEDevice::CreateUniqueID(devId));
}
bool g_DoHardwarePromptLogOnce;
void BLEModule::LegacyBLEImpl::DoHardwarePrompt() {
    if (!g_DoHardwarePromptLogOnce) {
        g_DoHardwarePromptLogOnce = true;
        LogTyped(LOG_WARNING, "BLEDevice_DoHardwarePrompt not implemented on Windows");
    }
}
void BLEModule::LegacyBLEImpl::EnableDeviceDiscovery(bool en) {
    g_EnableDeviceDiscovery = en;
}
void BLEModule::LegacyBLEImpl::GetRSSI(const char *) {
    //empty
}
bool BLEModule::LegacyBLEImpl::HasBLE() {
    return g_BLEDeviceManager.m_HasBLE;
}
void BLEModule::LegacyBLEImpl::IgnoreReceivedBluetoothPackets(bool ign) {
    g_IgnoreReceivedBluetoothPackets = ign;
}
void BLEModule::LegacyBLEImpl::InitializeBLE() {
    g_BLEDeviceManager.Init();
}
bool BLEModule::LegacyBLEImpl::IsAnyDeviceConnecting() {
    return false;
}
bool BLEModule::LegacyBLEImpl::IsAutoConnectPairingOn() {
    return g_BLEDeviceManager.m_AutoConnectPairingMode;
}
bool BLEModule::LegacyBLEImpl::IsBLEAvailable() {
    return g_BLEDeviceManager.m_bleAvalilable;
}
bool BLEModule::LegacyBLEImpl::IsRecoveringLostDevices() {
    return g_BLEDeviceManager.m_deviceState == BLEDeviceManager::BLE_DEVICE_STATE_RECOVERING;
}
bool BLEModule::LegacyBLEImpl::IsScanning() {
    return g_BLEDeviceManager.m_deviceState == BLEDeviceManager::BLE_DEVICE_STATE_SCANNING;
}
void BLEModule::LegacyBLEImpl::PairDevice(const BLEDevice &dev) {
    g_BLEDeviceManager.PairDevice(dev);
}
void BLEModule::LegacyBLEImpl::ProcessAdvertisedServiceUUIDs(const protobuf::BLEAdvertisement &, const std::string &, protobuf::BLEAdvertisementDataSection_Type, BLE_SOURCE) {
    //empty
}
void BLEModule::LegacyBLEImpl::ProcessAdvertisementManufacturerData(const protobuf::BLEAdvertisement &adv, const std::string &s, BLE_SOURCE src) {
    if (s.length() > 5 && s[0] == 0x4a && s[1] == 0x9 && s[2] == 6)
        BLEDevice_CreateTrainerST3(adv, 6u, s[4] | (s[3] << 8), src);
}
void BLEModule::LegacyBLEImpl::ProcessBLEResponse(const protobuf::BLEPeripheralResponse &resp, BLE_SOURCE src) {
    BLEDevice_ProcessBLEResponse(resp, src);
}
void BLEModule::LegacyBLEImpl::ProcessDiscovery(const protobuf::BLEAdvertisement &adv, BLE_SOURCE src) { //BLEDevice_ProcessBLEDiscovery
    if (g_EnableDeviceDiscovery && adv.sects_size()) {
        auto v8 = BLEDevice::CreateUniqueID(adv.per().device_id());
        for (auto &ds : adv.sects())
            if (protobuf::BLEAdvertisementDataSection_Type_IsValid(ds.type()) && ds.has_data())
                switch (ds.type()) {
                case protobuf::INCOMPLETE_UUIDS_16_BIT:
                case protobuf::COMPLETE_UUIDS_16_BIT:
                case protobuf::INCOMPLETE_UUIDS_32_BIT:
                case protobuf::COMPLETE_UUIDS_32_BIT:
                case protobuf::INCOMPLETE_UUIDS_128_BIT:
                case protobuf::COMPLETE_UUIDS_128_BIT:
                case protobuf::SOLICITATION_UUIDS_16_BIT:
                case protobuf::SOLICITATION_UUIDS_128_BIT:
                case protobuf::SOLICITATION_UUIDS_32_BIT:
                    BLEModule::Instance()->ProcessAdvertisedServiceUUIDs(adv, ds.data(), ds.type(), src);
                    break;
                case protobuf::MANUFACTURER_DATA:
                    BLEModule::Instance()->ProcessAdvertisementManufacturerData(adv, ds.data(), src);
                    break;
                default:
                    continue;
                }
        auto Device = FitnessDeviceManager::FindDevice(v8);
        if (Device)
            Device->m_last_time_ms = timeGetTime();
    }
}
void BLEModule::LegacyBLEImpl::PurgeDeviceList() {
    g_BLEDeviceManager.PurgeDeviceList();
}
void BLEModule::LegacyBLEImpl::ReceivedRSSI(int rssi, const char *devName, const char *devId) {
    if (devName && devId) {
        auto v11 = BLEDevice::CreateUniqueID(devId);
        auto Device = FitnessDeviceManager::FindDevice(v11);
        if (Device && rssi) {
            Device->m_rssiTime = timeGetTime();
            Device->m_rssi = rssi;
        }
    }
}
void BLEModule::LegacyBLEImpl::SendValueToDevice(const protobuf::BLEPeripheralRequest &req, BLE_SOURCE src) {
    switch (src) {
    case BLES_BUILTIN:
        g_BLEDeviceManager.SendValueToDevice(req);
        break;
    case BLES_ZCA:
        zwift_network::send_ble_peripheral_request(req);
        break;
    case BLES_ZH:
        break;
    case BLES_WFTN:
        WFTNPDeviceManager::WriteCharacteristic(req);
        break;
    default:
        zassert(!"BLEDevice_SendValueToDevice() not implemented for this BLE source.");
    }
}
void BLEModule::LegacyBLEImpl::SetAutoConnectPairingMode(bool ac) {
    g_BLEDeviceManager.m_AutoConnectPairingMode = ac;
}
uint32_t BLEDevice::CreateUniqueID(uint32_t hf1/*, uint32_t hf2*/) {
    return (hf1 & 0xFFFFFFF) | 0x10000000u;
}
uint32_t BLEDevice::CreateUniqueID(const std::string &f1/*, const std::string &f2*/) {
    //(void)f2;
    return CreateUniqueID(SIG_CalcCaseInsensitiveSignature(f1.c_str())/*, 0 not used too*/);
}
void BLEModule::LegacyBLEImpl::SetDeviceConnectedFlag(const protobuf::BLEPeripheralResponse &resp, bool flag) {
    if (resp.has_err_msg()) {
        Log("BLE Native Response error %d : %s", resp.err_kind(), resp.err_msg().c_str());
        g_lastBLEError = resp.err_kind();
    } else {
        g_lastBLEError = protobuf::BL_ERR_UNK;
        auto uid = BLEDevice::CreateUniqueID(resp.type() /*, 0not used resp.err_code()*/);
        auto Device = (BLEDevice *)FitnessDeviceManager::FindDevice(uid); //QUEST: why reinterpret, not dynamic_cast
        if (Device)
            Device->SetPaired(flag);
    }
}
void BLEModule::LegacyBLEImpl::StartBackgroundBLECommunication() {
    //empty
}
void BLEModule::LegacyBLEImpl::StartScan(const protobuf::BLEPeripheralRequest &req) {
    g_BLEDeviceManager.StartScan(req);
}
void BLEModule::LegacyBLEImpl::StartSearchForLostDevices() {
    g_BLEDeviceManager.StartSearchForLostDevices();
}
void BLEModule::LegacyBLEImpl::StopBackgroundBLECommunication() {
    //empty
}
void BLEModule::LegacyBLEImpl::StopScan() {
    g_BLEDeviceManager.StopScan();
}
void BLEModule::LegacyBLEImpl::StopSearchForLostDevices() {
    g_BLEDeviceManager.StopSearchForLostDevices();
}
void BLEModule::LegacyBLEImpl::UnpairDevice(const BLEDevice &dev) {
    g_BLEDeviceManager.UnpairDevice(dev);
}
void BLEModule::StopScan() {
    callVoidImplMethodOrLogIfUninitialized([this]() {
        this->m_bleImpl->StopScan();
    }, "BLEModule::StopScan");
}
void BLEModule::ProcessDiscovery(const protobuf::BLEAdvertisement &adv, BLE_SOURCE src) {
    callVoidImplMethodOrLogIfUninitialized([this, adv, src]() {
        this->m_bleImpl->ProcessDiscovery(adv, src);
    }, "BLEModule::ProcessDiscovery");
}
void BLEModule::ProcessBLEResponse(const protobuf::BLEPeripheralResponse &resp, BLE_SOURCE src) {
    callVoidImplMethodOrLogIfUninitialized([this, resp, src]() {
        this->m_bleImpl->ProcessBLEResponse(resp, src);
    }, "BLEModule::ProcessBLEResponse");
}
void BLEModule::callVoidImplMethodOrLogIfUninitialized(const std::function<void(void)> &func, const char *method) {
    if (g_BLEModule) {
        if (m_bleImpl)
            func();
        else
            Log("BLEModule: Called function '%s' before BLE has been initialized", method);
    }
}
bool BLEModule::callBoolImplMethodOrLogIfUninitialized(const std::function<bool(void)> &func, const char *method) {
    if (g_BLEModule) {
        if (m_bleImpl)
            return func();
        else
            Log("BLEModule: Called function '%s' before BLE has been initialized", method);
    }
    return false;
}
void BLEModule::PairDevice(const BLEDevice &dev) {
    callVoidImplMethodOrLogIfUninitialized([this, &dev]() {
        this->m_bleImpl->PairDevice(dev);
    }, "BLEModule::PairDevice");
}
void BLEModule::DidRecover(const char *a2, const char *a3) {
    callVoidImplMethodOrLogIfUninitialized([this, a2, a3]() {
        this->m_bleImpl->DidRecover(a2, a3);
    }, "BLEModule::DidRecover");
}
void BLEModule::DidConnect(const char *a2, const char *a3) {
    callVoidImplMethodOrLogIfUninitialized([this, a2, a3]() {
        this->m_bleImpl->DidConnect(a2, a3);
     }, "BLEModule::DidConnect");
}
void BLEModule::ReceivedRSSI(int a2, const char *a3, const char *a4) {
    callVoidImplMethodOrLogIfUninitialized([this, a2, a3, a4]() {
        this->m_bleImpl->ReceivedRSSI(a2, a3, a4);
    }, "BLEModule::ReceivedRSSI");
}
void BLEModule::DidReceiveError(const char *a2, const char *a3, BLE_ERROR_TYPE err, uint32_t a5) {
    callVoidImplMethodOrLogIfUninitialized([this, a2, a3, err, a5]() {
        this->m_bleImpl->DidReceiveError(a2, a3, err, a5);
    }, "BLEModule::DidReceiveError");
}
void BLEModule::SetDeviceConnectedFlag(const protobuf::BLEPeripheralResponse &resp, bool flag) {
    callVoidImplMethodOrLogIfUninitialized([this, resp, flag]() {
        this->m_bleImpl->SetDeviceConnectedFlag(resp, flag);
    }, "BLEModule::SetDeviceConnectedFlag");
}
void BLEModule::StartScan(const protobuf::BLEPeripheralRequest &req) {
    callVoidImplMethodOrLogIfUninitialized([this, req]() {
        this->m_bleImpl->StartScan(req);
    }, "BLEModule::StartScan");
}
bool BLEModule::HasBLE() {
    return callBoolImplMethodOrLogIfUninitialized([this]() {
        return this->m_bleImpl->HasBLE();
    }, "BLEModule::HasBLE");
}
bool BLEModule::IsBLEAvailable() {
    return callBoolImplMethodOrLogIfUninitialized([this]() {
        return this->m_bleImpl->IsBLEAvailable();
    }, "BLEModule::IsBLEAvailable");
}
void BLEModule::ProcessAdvertisementManufacturerData(const protobuf::BLEAdvertisement &adv, const std::string &a3, BLE_SOURCE src) {
    callVoidImplMethodOrLogIfUninitialized([this, adv, a3, src]() {
        this->m_bleImpl->ProcessAdvertisementManufacturerData(adv, a3, src);
    }, "BLEModule::ProcessAdvertisementManufacturerData");
}
void BLEModule::EnableDeviceDiscovery(bool en) {
    callVoidImplMethodOrLogIfUninitialized([this, en]() {
        this->m_bleImpl->EnableDeviceDiscovery(en);
    }, "BLEModule::EnableDeviceDiscovery");
}
void BLEModule::StartSearchForLostDevices() {
    callVoidImplMethodOrLogIfUninitialized([this]() {
        this->m_bleImpl->StartSearchForLostDevices();
    }, "BLEModule::StartSearchForLostDevices");
}
void BLEModule::SendValueToDevice(const protobuf::BLEPeripheralRequest &req, BLE_SOURCE src) {
    callVoidImplMethodOrLogIfUninitialized([this, req, src]() {
        this->m_bleImpl->SendValueToDevice(req, src);
    }, "BLEModule::SendValueToDevice");
}
void BLEModule::PurgeDeviceList() {
    callVoidImplMethodOrLogIfUninitialized([this]() {
        this->m_bleImpl->PurgeDeviceList();
    }, "BLEModule::PurgeDeviceList");
}
void BLEModule::DoHardwarePrompt() {
    callVoidImplMethodOrLogIfUninitialized([this]() {
        this->m_bleImpl->DoHardwarePrompt();
    }, "BLEModule::DoHardwarePrompt");
}
void BLEModule::IgnoreReceivedBluetoothPackets(bool ign) {
    callVoidImplMethodOrLogIfUninitialized([this, ign]() {
        this->m_bleImpl->IgnoreReceivedBluetoothPackets(ign);
    }, "BLEModule::IgnoreReceivedBluetoothPackets");
}
void BLEModule::ProcessAdvertisedServiceUUIDs(const protobuf::BLEAdvertisement &adv, const std::string &a2, protobuf::BLEAdvertisementDataSection_Type dst, BLE_SOURCE src) {
    callVoidImplMethodOrLogIfUninitialized([this, adv, a2, dst, src]() {
        this->m_bleImpl->ProcessAdvertisedServiceUUIDs(adv, a2, dst, src);
    }, "BLEModule::ProcessAdvertisedServiceUUIDs");
}
bool BLEModule::IsScanning() {
    return callBoolImplMethodOrLogIfUninitialized([this]() {
        return this->m_bleImpl->IsScanning();
    }, "BLEModule::IsScanning");
}
bool BLEModule::IsRecoveringLostDevices() {
    return callBoolImplMethodOrLogIfUninitialized([this]() {
        return this->m_bleImpl->IsRecoveringLostDevices();
    }, "BLEModule::IsRecoveringLostDevices");
}
bool BLEModule::IsAutoConnectPairingOn() {
    return callBoolImplMethodOrLogIfUninitialized([this]() {
        return this->m_bleImpl->IsAutoConnectPairingOn();
    }, "BLEModule::IsAutoConnectPairingOn");
}
void BLEModule::StopSearchForLostDevices() {
    callVoidImplMethodOrLogIfUninitialized([this]() {
        this->m_bleImpl->StopSearchForLostDevices();
    }, "BLEModule::StopSearchForLostDevices");
}
void BLEModule::UnpairDevice(const BLEDevice &dev) {
    callVoidImplMethodOrLogIfUninitialized([this, &dev]() {
        this->m_bleImpl->UnpairDevice(dev);
    }, "BLEModule::UnpairDevice");
}
void BLEModule::InitializeBLE() {
    if (!m_bleImpl) {
        m_bleImpl = new BLEModule::LegacyBLEImpl();
        m_bleImpl->InitializeBLE();
    }
}
void cbEnableSupportsBLEFunc(bool en) {
    g_BLEDeviceManager.m_HasBLE = en;
}
void cbBLEIsAvailableFunc(bool yes) {
    g_BLEDeviceManager.m_bleAvalilable = yes;
}
BLEModule::~BLEModule() {
    delete m_bleImpl;
}
void BLEDevice::SetPaired(bool p) {
    m_isPaired = p;
    m_field_2CC = false;
}
void BLEDevice::ProcessBLEData(const protobuf::BLEPeripheralResponse &resp) {
    //TODO
}
void BLEDevice::Pair(bool) {
    switch (m_bleSrc) {
    case BLES_BUILTIN:
        BLEModule::Instance()->PairDevice(*this);
        m_field_2CC = true;
        break;
    case BLES_ZCA: {
        protobuf::BLEPeripheralRequest rq;
        //CommonPair(&rq, m_devId, m_charId):
        rq.set_type(protobuf::CONNECT_PERIPHERAL);
        rq.mutable_per()->set_device_id(m_devId);
        auto serv = rq.add_servs();
        auto chr = serv->add_chars();
        char serv_id[1024], chr_id[1024];
        chr_id[0] = 0;
        serv_id[0] = '0'; serv_id[1] = 0;
        switch (m_charId) {
        case 0xA026E01D: case 0xA026E01F:
            strcpy(serv_id, "A026EE07-0A7D-4AB3-97FA-F1500F9FEB8B");
            break;
        case 0xE9410101:
            strcpy(serv_id, "E9410100-B434-446B-B5CC-36592FC4C724");
            strcpy(chr_id, "E9410101-B434-446B-B5CC-36592FC4C724");
            break;
        case 0xE9410201:
            strcpy(serv_id, "E9410200-B434-446B-B5CC-36592FC4C724");
            break;
        case 0x347B0030:
            strcpy(serv_id, "347B0001-7635-408B-8918-8FF3949CE592");
            strcpy(chr_id, "347B0030-7635-408B-8918-8FF3949CE592");
            break;
        case 0x2A37:
            strcpy(serv_id, "180d");
            break;
        case 0x2A5B:
            strcpy(serv_id, "1816");
            break;
        case 0x2A63:
            strcpy(serv_id, "1818");
            break;
        case 0x2A53:
            strcpy(serv_id, "1814");
            break;
        default:
            Log("CommonPair() default: %d", m_charId);
        }
        serv->set_id(serv_id);
        if (!chr_id[0])
            sprintf(chr_id, "%x", m_charId);
        chr->set_id(chr_id);
        m_field_2CC = true;
        zwift_network::send_ble_peripheral_request(rq);
        auto v20 = (TrainerControlComponent *)ExerciseDevice::FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (v20)
            v20->OnPaired();
        break; }
    case BLES_ZH:
        zassert(!"Pair not implented for BLE source");
        break;
    case BLES_WFTN: {
        Log("WFTNPDeviceManager::Pairing device \"%s\"", m_name);
        /* later IDBySerial = WFTNPDeviceManager::GetIDBySerial(m_devId);
        *&this->field_11C[4] = *IDBySerial;
        *(IDBySerial + 121) = 1;
        if (!*(IDBySerial + 120)) {
            Log("WFTNPDeviceManager: connecting to LAN device \"%s\"", (IDBySerial + 8).c_str());
            icu_72::Transliterator::_getAvailableTarget(*&this->field_11C[4], 0i64, 0i64);
            if (!IsWdcErrorDialogs())
                SetPaired(true);
        }
        if (IsWdcErrorDialogs())
            SetPaired(true);*/
        m_field_2CC = true;
        auto cot = (TrainerControlComponent *)ExerciseDevice::FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (cot) {
            LogTyped(LOG_BLE, "Controllable found.. calling OnPaired!");
            cot->OnPaired();
        }}
        break;
    }
}
bool BLEDevice::IsActivelyPaired() {
    return m_isPaired;
}
bool BLEDevice::IsPaired() const  {
    return m_isPaired || m_field_2CC;
}
uint32_t BLEDevice::GetPrefsID() {
    return (uint16_t)SIG_CalcCaseInsensitiveSignature(m_devId.c_str()) | ((SIG_CalcCaseInsensitiveSignature(m_nameId.c_str()) & 0xFFF | 0x1000) << 16);
}
void CommonUnpair(protobuf::BLEPeripheralRequest *rq, const std::string &devId, uint32_t charId) {
    rq->set_type(protobuf::DISCONNECT_PERIPHERAL);
    rq->mutable_per()->set_device_id(devId);
    auto serv = rq->add_servs();
    auto chr = serv->add_chars();
    char serv_id[1024], chr_id[1024];
    chr_id[0] = 0;
    serv_id[0] = '0'; serv_id[1] = 0;
    switch (charId) {
    case 0xE9410101:
        strcpy(serv_id, "E9410100-B434-446B-B5CC-36592FC4C724");
        strcpy(chr_id, "E9410101-B434-446B-B5CC-36592FC4C724");
        break;
    case 0xE9410201:
        strcpy(serv_id, "E9410200-B434-446B-B5CC-36592FC4C724");
        break;
    case 0x347B0030:
        strcpy(serv_id, "347B0001-7635-408B-8918-8FF3949CE592");
        strcpy(chr_id, "347B0030-7635-408B-8918-8FF3949CE592");
        break;
    case 0x2A37:
        strcpy(serv_id, "180d");
        break;
    case 0x2A5B:
        strcpy(serv_id, "1816");
        break;
    case 0x2A63:
        strcpy(serv_id, "1818");
        break;
    }
    serv->set_id(serv_id);
    if (!chr_id[0])
        sprintf(chr_id, "%x", charId);
    chr->set_id(chr_id);
}
void BLEDevice::UnPair() {
    switch(m_bleSrc) {
    case BLES_BUILTIN:
        BLEModule::Instance()->UnpairDevice(*this);
        SetPaired(false);
        break;
    case BLES_ZCA:
        if (m_isPaired) {
            protobuf::BLEPeripheralRequest rq;
            CommonUnpair(&rq, m_devId, m_charId);
            m_isPaired = false;
            zwift_network::send_ble_peripheral_request(rq);
        }
        break;
    case BLES_ZH:
        zassert(!"UnPair not implented for BLE source");
        break;
    case BLES_WFTN:
        WFTNPDeviceManager::UnPair(this);
        SetPaired(false);
        break;
    }
    FitnessDeviceManager::RemoveDevice(this, false);
}
bool g_bLogBlePackets;
void BLEDevice::LogBleRxPacket(const protobuf::BLEPeripheralResponse &resp) {
    if (g_bLogBlePackets) {
        char Buffer[256];
        auto chVal = resp.chr().value().c_str();
        auto v7 = resp.per().device_name().c_str();
        auto v9 = sprintf_s(Buffer, "%d RX '%s' %s [%d]: ", GFX_GetFrameCount(), v7, resp.chr().id().c_str(), (int)resp.chr().value().length());
        if (v9 <= 0 || v9 >= _countof(Buffer) - 8) {
            if (v7)
                LogTyped(LOG_BLE, "Error logging RX packet for '%s'", v7);
        } else {
            for (int i = 0; i < resp.chr().value().length(); ++chVal, ++i) {
                auto v12 = sprintf(Buffer + v9, "%02X ", *chVal);
                if (v12 <= 0 || v12 >= 8)
                    break;
                v9 += v12;
                if (v9 >= 0xF8)
                    break;
            }
            LogTyped(LOG_BLE, "%s", Buffer);
        }
    }
}
void BLEDevice::LogBleTxPacket(char const *funcName, char const *devName, protobuf::BLEPeripheralRequest &req) {
    if (g_bLogBlePackets) {
        char Buffer[256];
        auto fr = GFX_GetFrameCount();
        for (auto &s : req.servs()) {
            for (auto &c : s.chars()) {
                auto v9 = sprintf_s(Buffer, "%d TX '%s' %s {%s} %s [%02d]: ", fr, devName, funcName, s.id().c_str(), c.id().c_str(), (int)c.value().length());
                if (v9 <= 0 || v9 >= sizeof(Buffer) - 8) {
                    if (devName)
                        LogTyped(LOG_BLE, "Error logging TX packet for '%s'", devName);
                } else {
                    auto chVal = c.value().c_str();
                    for (int i = 0; i < c.value().length(); ++chVal, ++i) {
                        auto v12 = sprintf(Buffer + v9, "%02X ", *chVal);
                        if (v12 <= 0 || v12 >= 8)
                            break;
                        v9 += v12;
                        if (v9 >= 0xF8)
                            break;
                    }
                    LogTyped(LOG_BLE, "%s", Buffer);
                }
            }
        }
    }
}
int g_RunSensorSecondsSincePacket;
ExpVariant g_expFID_HWEXPER = EXP_NONE;
bool expFID_HWEXPER() {
    if (g_expFID_HWEXPER != EXP_NONE)
        return g_expFID_HWEXPER == EXP_ENABLED;
    g_expFID_HWEXPER = Experimentation::Instance()->IsEnabled(FID_HWEXPER, EXP_UNASSIGNED);
    return g_expFID_HWEXPER == EXP_ENABLED;
}
void BLEDevice::Update(float f) {
    auto cad = (SensorValueComponent *)FindComponentOfType(DeviceComponent::CPT_CAD);
    auto t = timeGetTime();
    if (cad && (t - m_lastCadTs) * 0.001f > 3.0f)
        cad->m_val = 0.0f;
    auto spd = (SensorValueComponent *)FindComponentOfType(DeviceComponent::CPT_SPD);
    if (spd && (t - m_lastSpdTs) * 0.001f > 3.0f) {
        spd->m_val = 0.0f;
        spd->m_bInitState = false;
    }
    auto run_spd = (SensorValueComponent *)FindComponentOfType(DeviceComponent::CPT_RUN_SPD);
    if (run_spd) {
        auto since = (t - m_lastSpdTs) * 0.001f;
        if (this == FitnessDeviceManager::m_pSelectedRunSpeedDevice)
            g_RunSensorSecondsSincePacket = int(since);
        if (since > 30.0f) {
            if (this == FitnessDeviceManager::m_pSelectedRunSpeedDevice && run_spd->m_val != 0.0f)
                Log("Timeout on run speed data. Timing out and setting speed to 0");
            run_spd->m_val = 0.0f;
            run_spd->m_bInitState = false;
            if (m_field_29D && !m_field_29C && m_isPaired) {
                auto run_spd_b = (Bowflex_BLE_ControlComponent *)run_spd;
                run_spd_b->InitStreaming();
                m_field_29C = true;
            }
        }
    }
    if (Experimentation::IsEnabledCached<FID_FTMSBIK>()) {
        auto ct = (TrainerControlComponent *)FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (ct && ct->m_protocolType == TrainerControlComponent::FTMS_V3)
            ct->Update(f);
    } else if (expFID_HWEXPER()) {
        auto ct = (TrainerControlComponent *)FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (ct && ct->m_protocolType == TrainerControlComponent::P_2)
            ct->Update(f);
    }
}
BLEDevice::BLEDevice(const std::string &devId, const std::string &devName, uint32_t charId, uint32_t hash, BLE_SOURCE src) {
    m_hash = hash;
    m_protocol = DP_BLE;
    m_prefsID = hash & 0xFFFFFFF | 0x10000000;
    m_charId = charId;
    m_devId = devId;
    m_bleSrc = src;
    m_nameId = devName;
    char buffer[64];
    auto v18 = std::strtoull(devId.c_str(), nullptr, 10);
    sprintf_s(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
        uint8_t(v18 >> 40),
        uint8_t(v18 >> 32),
        uint8_t(v18 >> 24),
        uint8_t(v18 >> 16),
        uint8_t(v18 >> 8),
        uint8_t(v18));
    m_address = buffer;
    switch(charId) {
    default: Log("Unknown Native BLE component attached. Device name: %s", devName.c_str());
    case 0x347B0010: case 0x26D42A4D: break;
    case 0xE9410101:
        AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
        AddComponent(new SensorValueComponent(DeviceComponent::CPT_CAD));
        break;
    case 0x4E349C00: case 0xE3F9AF20:
        AddComponent(new Bowflex_BLE_ControlComponent());
        break;
    case 0xC4632B01: case 0xE9410201: case 0xFF01: case 0x2A63:
        AddComponent(new SensorValueComponent(DeviceComponent::CPT_PM));
        break;
    case 0x2A37:
        AddComponent(new SensorValueComponent(DeviceComponent::CPT_HR));
        break;
    case 0x2A5B:
        AddComponent(new SensorValueComponent(DeviceComponent::CPT_SPD));
        AddComponent(new SensorValueComponent(DeviceComponent::CPT_CAD));
        AddComponent(new Component_7(DeviceComponent::CPT_7));
        break;
    }
    if (isEliteSteering(devName))
        AddComponent(new EliteSteeringComponent(this));
    if (JetBlackSteeringComponent::IsFeatureFlagEnabled() && isJetBlackSteering(devName))
        AddComponent(new JetBlackSteeringComponent(this));
    //later: experimentals
}

TEST(SmokeTestBLE, Init_V1) {
    auto     evSysInst = EventSystem::GetInst();
    Experimentation::Initialize(evSysInst);
    auto exp = Experimentation::Instance();
    exp->m_fsms[FID_BLEDLLV].m_enableStatus = EXP_DISABLED;
    BLEModule::Initialize(exp);
    EXPECT_TRUE(BLEModule::IsInitialized());
    BLEModule::Instance()->InitializeBLE();
    auto until = GetTickCount64() + 1000;
    while (GetTickCount64() < until) {
        Sleep(100);
        if (g_BLEDeviceManager.m_HasBLE && g_BLEDeviceManager.m_bleAvalilable)
            break;
    }
    BLEModule::Shutdown();
    Experimentation::Shutdown();
    EventSystem::Destroy();
}
TEST(SmokeTestBLE, Init_V2) {
    auto     evSysInst = EventSystem::GetInst();
    Experimentation::Initialize(evSysInst);
    auto exp = Experimentation::Instance();
    exp->m_fsms[FID_BLEDLLV].m_enableStatus = EXP_ENABLED;
    BLEModule::Initialize(exp);
    BLEModule::Instance()->PurgeDeviceList(); //neg branch
    EXPECT_FALSE(BLEModule::Instance()->IsScanning()); //neg branch
    EXPECT_TRUE(BLEModule::IsInitialized());
    BLEModule::Instance()->InitializeBLE();
    auto until = GetTickCount64() + 1000;
    while (GetTickCount64() < until) {
        Sleep(100);
        if (BLEModule::Instance()->HasBLE() && BLEModule::Instance()->IsBLEAvailable())
            break;
    }
    BLEModule::Instance()->PurgeDeviceList(); //pos branch
    EXPECT_FALSE(BLEModule::Instance()->IsScanning());
    EXPECT_FALSE(BLEModule::Instance()->IsRecoveringLostDevices());
    EXPECT_FALSE(BLEModule::Instance()->IsAutoConnectPairingOn());
    BLEModule::Instance()->StopSearchForLostDevices();
    BLEModule::Shutdown();
    Experimentation::Shutdown();
    EventSystem::Destroy();
}
TEST(SmokeTestBLE, Scan) {
    auto     evSysInst = EventSystem::GetInst();
    Experimentation::Initialize(evSysInst);
    auto exp = Experimentation::Instance();
    exp->m_fsms[FID_BLEDLLV].m_enableStatus = EXP_ENABLED;
    BLEModule::Initialize(exp);
    BLEModule::Instance()->PurgeDeviceList(); //neg branch
    EXPECT_FALSE(BLEModule::Instance()->IsScanning()); //neg branch
    EXPECT_TRUE(BLEModule::IsInitialized());
    BLEModule::Instance()->InitializeBLE();
    auto until = GetTickCount64() + 1000;
    while (GetTickCount64() < until) {
        Sleep(100);
        if (BLEModule::Instance()->HasBLE() && BLEModule::Instance()->IsBLEAvailable())
            break;
    }
    protobuf::BLEPeripheralRequest rq;
    auto s = rq.add_servs();
    //s->set_id("1816"); //SPC
    s->set_id("1816"); //CP
    s->add_chars()->set_id("2A55");
    BLEModule::Instance()->StartScan(rq);
    Sleep(10000);
    BLEModule::Shutdown();
    Experimentation::Shutdown();
    EventSystem::Destroy();
}