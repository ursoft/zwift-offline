#include "ZwiftApp.h"
void FitnessDeviceManager::TrainerSetSimGrade(float v) { 
    /*TODO*/ 
}
void FitnessDeviceManager::Initialize() {
    /*TODO*/
}
void FitnessDeviceManager::SetInitialRotationZC() {
    /*TODO*/
}
void FitnessDeviceManager::HandleBLEReconnect(uint32_t hash) {
    //TODO
}
void FitnessDeviceManager::HandleBLEConnect(uint32_t hash) {
    //TODO
}
void FitnessDeviceManager::HandleBLEError(uint32_t hash, BLE_ERROR_TYPE err, uint32_t a5) {
    //TODO
}
ExerciseDevice *FitnessDeviceManager::FindDevice(uint32_t hash) {
    //TODO
    return nullptr;
}
const char *DeviceComponent::GetTypeName(ComponentType ct) {
    const char *loc = nullptr;
    switch (m_type) { // (0x15Fu >> ct) & 1)
    case CPT_SPD:     loc = "LOC_SPEED_SENSOR_TITLE"; break;
    case CPT_CAD:     loc = "LOC_CADENCE_TITLE"; break;
    case CPT_RUN_SPD: loc = "LOC_RUN_SPEED_TITLE"; break;
    case CPT_RUN_CAD: loc = "LOC_CADENCE_TITLE"; break;
    case CPT_HR:      loc = "LOC_HEART_RATE_TITLE"; break;
    case CPT_PM:      loc = "LOC_POWER_SOURCE_TITLE"; break;
    case CPT_CTRL:    loc = "LOC_CONTROLLABLE_TITLE"; break;
    }
    return loc ? GetText(loc) : "NULL";
}
ExerciseDevice::ExerciseDevice() {
    /*TODO:
    std::vector<12byte> m_cont12;

    *this->field_8 = 0i64;
    this->field_118 = -1;
    *this->field_11C = 0i64;
    *&this->field_11C[8] = -65536;
    *&this->field_11C[16] = 0i64;
    *&this->field_11C[24] = 0i64;
    this->field_13C = 1.0;
    *&this->field_11C[12] = 0;
    this->field_200 = 0;
    */
}
ExerciseDevice::SignalStrengthGroup ExerciseDevice::GetSignalStrengthGroup() {
    if (!m_rssi || timeGetTime() - m_rssiTime >= 10'000)
        return ExerciseDevice::LOW;
    if (m_rssi < -85)
        return ExerciseDevice::LOW_ONE_BAR;
    if (m_rssi >= -80)
        return (m_rssi >= -75) ? ExerciseDevice::GOOD_4_BARS : ExerciseDevice::NORM_3_BARS;
    return ExerciseDevice::LOW_TWO_BARS;
}
const char *g_devProtocol[DP_CNT] = {"ANT", "BLE", "SERIAL", "BLE_MOBILE", "WATCH", "WAHOO_ANT", "ZML", "USB", "WIFI", "UNKNOWN"};
const char *g_devBleSubtype[BLES_CNT] = { "BLE", "BLE (ZC)", "BLE (ZH)", "BLE (LAN)" };
void ExerciseDevice::AddComponent(DeviceComponent *devComp) {
    zassert(devComp);
    {
        std::lock_guard l(m_mutex);
        m_components.push_back(devComp);
    }
    devComp->m_owner = this;
    auto v6 = "UNKNOWN";
    if (m_protocol == DP_BLE) {
        v6 = "BLE (!)";
        auto v8 = ((BLEDevice *)this)->m_bleSrc;
        if (v8 >= 0 && v8 <= _countof(g_devBleSubtype))
            v6 = g_devBleSubtype[v8];
    } else if (m_protocol < DP_CNT) {
        v6 = g_devProtocol[m_protocol];
    }
    Log("\"%s\" adding component type: %d (%s)", m_name, devComp->m_type, v6);
    if (FitnessDeviceManager::m_PairingSport) {
        if (FitnessDeviceManager::m_PairingSport == protobuf::RUNNING) {
            if (devComp->m_type == DeviceComponent::CPT_RUN_SPD) {
                if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTRSDEVICE", (uint32_t)-1) == GetPrefsID())
                    FitnessDeviceManager::PairRunSpeedSensor(this);
            } else if (devComp->m_type == DeviceComponent::CPT_RUN_CAD) {
                if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTRUNCADENCEDEVICE", (uint32_t)-1) == GetPrefsID())
                    FitnessDeviceManager::PairRunCadenceSensor(this);
            }
        }
    } else switch (devComp->m_type) {
    case DeviceComponent::CPT_SPD:
        if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTSPEEDDEVICE", (uint32_t)-1) == GetPrefsID())
            FitnessDeviceManager::PairSpeedSensor(this);
        break;
    case DeviceComponent::CPT_CAD:
        if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTCADENCEDEVICE", (uint32_t)-1) == GetPrefsID())
            FitnessDeviceManager::PairCadenceSensor(this);
        break;
    case DeviceComponent::CPT_PM:
        if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTPOWERDEVICE", (uint32_t)-1) == GetPrefsID())
            FitnessDeviceManager::PairPowerSensor(this);
        break;
    case DeviceComponent::CPT_HR:
        if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTHRMDEVICE", (uint32_t)-1) == GetPrefsID())
            FitnessDeviceManager::PairHRSensor(this);
        break;
    case DeviceComponent::CPT_CTRL:
        if (m_protocol == DP_ANT && FitnessDeviceManager::FindMfgAndModel(GetPrefsID()) == 0x560017)
            m_is560017 = true;
        if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTCONTROLLABLETRAINER", (uint32_t)-1) == GetPrefsID()) {
            if (m_protocol == DP_ANT)
                g_UserConfigDoc.SetU32("ZWIFT\\DEVICES\\LASTANTDEVICE", ((AntDevice *)this)->m_prefsAntID);
            if (m_protocol != DP_ANT || !m_is560017 || ((AntDevice *)this)->m_may17 == 17)
                FitnessDeviceManager::PairControllableTrainer(this);
        } else if (m_protocol == DP_ANT) {
            if (m_is560017 && ((AntDevice *)this)->m_may17 == 17) {
                auto U32 = g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTANTDEVICE", (uint32_t)-1);
                if (((AntDevice *)this)->m_prefsAntID == U32 || U32 == (uint32_t)-1)
                    FitnessDeviceManager::PairControllableTrainer(this);
            }
        }
        break;
    case DeviceComponent::CPT_STEER:
        if (g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTSTEERINGDEVICE", (uint32_t)-1) == GetPrefsID())
            FitnessDeviceManager::PairSteeringSensor(this);
        break;
    }
}
uint32_t FitnessDeviceManager::FindMfgAndModel(uint32_t hash) {
    for (auto &i : m_DeviceDB)
        if (i->m_hash == hash)
            return i->m_mfgModel;
    return 0;
}
void FitnessDeviceManager::AddDeviceToKnownDatabase(DeviceProtocol prot, uint32_t hash, const char *nameId, uint32_t mfgModel) {
    for (auto i : m_DeviceDB)
        if (i->m_hash == hash) {
            free(i->m_name);
            i->m_name = _strdup(nameId);
            if (mfgModel)
                i->m_mfgModel = mfgModel;
            return;
        }
    auto v11 = new DeviceDbItem;
    v11->m_hash = hash;
    v11->m_protocol = prot;
    v11->m_mfgModel = mfgModel;
    v11->m_name = _strdup(nameId);
    m_DeviceDB.push_back(v11);
}
void FitnessDeviceManager::AddDevice(ExerciseDevice *dev, const char *nameId) {
    for (auto i : m_DeviceDB)
        if (i->m_hash == dev->m_prefsID) {
            strcpy_s(dev->m_name, i->m_name);
            std::lock_guard l(g_FDM_DeviceListMutex);
            m_DeviceList.push_back(dev);
            return;
        }
    FitnessDeviceManager::AddDeviceToKnownDatabase(dev->m_protocol, dev->m_prefsID, nameId ? nameId : "UNKNOWN DEVICE", 0);
    FitnessDeviceManager::AddDevice(dev, nameId);
}
bool FitnessDeviceManager::AreAnyBLEDevicesCurrentlyPaired() {
    return (m_pSelectedHRDevice && m_pSelectedHRDevice->m_protocol == DP_BLE)
        || (m_pSelectedDi2Device && m_pSelectedDi2Device->m_protocol == DP_BLE)
        || (m_pSelectedPowerDevice && m_pSelectedPowerDevice->m_protocol == DP_BLE)
        || (m_pSelectedSpeedDevice && m_pSelectedSpeedDevice->m_protocol == DP_BLE)
        || (m_pSelectedCadenceDevice && m_pSelectedCadenceDevice->m_protocol == DP_BLE)
        || (m_pSelectedRunSpeedDevice && m_pSelectedRunSpeedDevice->m_protocol == DP_BLE)
        || (m_pSelectedRunCadenceDevice && m_pSelectedRunCadenceDevice->m_protocol == DP_BLE)
        || (m_pSelectedControllableTrainerDevice && m_pSelectedControllableTrainerDevice->m_protocol == DP_BLE)
        || (m_pSelectedSteeringDevice && m_pSelectedSteeringDevice->m_protocol == DP_BLE)
        || (m_pSelectedBrakingDevice && m_pSelectedBrakingDevice->m_protocol == DP_BLE);
}
void FitnessDeviceManager::AddDevice(DeviceDbItem *dev, const char *nameId) {
    //TODO
}
std::string FitnessDeviceManager::GetEquipmentTypesString(const BLEDevice *dev) {
    std::string ret;
    ret.reserve(1024);
    if (dev == FitnessDeviceManager::m_pSelectedPowerDevice)
        ret += "Power"s;
    if (dev == FitnessDeviceManager::m_pSelectedSpeedDevice) {
        if (!ret.empty()) ret += '|';
        ret += "Speed"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedCadenceDevice) {
        if (!ret.empty()) ret += '|';
        ret += "Cadence"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedDi2Device) {
        if (!ret.empty()) ret += '|';
        ret += "Di2"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedControllableTrainerDevice) {
        if (!ret.empty()) ret += '|';
        ret += "Controllable"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedRunSpeedDevice) {
        if (!ret.empty()) ret += '|';
        ret += "RunSpeed"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedRunCadenceDevice) {
        if (!ret.empty()) ret += '|';
        ret += "RunCadence"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedHRDevice) {
        if (!ret.empty()) ret += '|';
        ret += "HR"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedAuthoritativeDevice) {
        if (!ret.empty()) ret += '|';
        ret += "Authoritative"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedSteeringDevice) {
        if (!ret.empty()) ret += '|';
        ret += "Steering"s;
    }
    if (dev == FitnessDeviceManager::m_pSelectedBrakingDevice) {
        if (!ret.empty()) ret += '|';
        ret += "Braking"s;
    }
    if (ret.empty())
        ret = "Unknown"s;
    return ret;
}
void FitnessDeviceManager::PairAuthoritativeSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairBrakingSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairCadenceSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairControllableTrainer(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairDi2Sensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairHRSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairPowerSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairRunCadenceSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairRunSpeedSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairSpeedSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::PairSteeringSensor(ExerciseDevice *dev) {
    //TODO
}
void FitnessDeviceManager::RemoveDevice(ExerciseDevice *, bool) {
    //TODO
}
int FitnessDeviceManager::GetUnpairedBLEDeviceCount() {
    int ret = 0;
    std::lock_guard l(g_FDM_DeviceListMutex);
    for (auto i : m_DeviceList)
        if (i && !i->m_hidden && i->m_protocol == DP_BLE && !IsThisDevicePaired(i->m_prefsID))
            ++ret;
    return ret;
}
int FitnessDeviceManager::GetSelectedBLEDeviceCount() {
    int ret = 0;
    std::lock_guard l(g_FDM_DeviceListMutex);
    for (auto i : m_DeviceList)
        if (i && i->m_protocol == DP_BLE && IsThisDevicePaired(i->m_prefsID))
            ++ret;
    return ret;
}
bool FitnessDeviceManager::IsThisDevicePaired(uint32_t prefsId) {
    return (m_pSelectedHRDevice && m_pSelectedHRDevice->m_prefsID == prefsId)
        || (m_pSelectedDi2Device && m_pSelectedDi2Device->m_prefsID == prefsId)
        || (m_pSelectedPowerDevice && m_pSelectedPowerDevice->m_prefsID == prefsId)
        || (m_pSelectedSpeedDevice && m_pSelectedSpeedDevice->m_prefsID == prefsId)
        || (m_pSelectedCadenceDevice && m_pSelectedCadenceDevice->m_prefsID == prefsId)
        || (m_pSelectedRunSpeedDevice && m_pSelectedRunSpeedDevice->m_prefsID == prefsId)
        || (m_pSelectedRunCadenceDevice && m_pSelectedRunCadenceDevice->m_prefsID == prefsId)
        || (m_pSelectedControllableTrainerDevice && m_pSelectedControllableTrainerDevice->m_prefsID == prefsId)
        || (m_pSelectedSteeringDevice && m_pSelectedSteeringDevice->m_prefsID == prefsId)
        || (m_pSelectedBrakingDevice && m_pSelectedBrakingDevice->m_prefsID == prefsId);
}
void WFTNPDeviceManager::WriteCharacteristic(const protobuf::BLEPeripheralRequest &rq) {
    //TODO
}
void WFTNPDeviceManager::UnPair(BLEDevice *dev) {
    //TODO
}
bool ZMLAUXDevice::IsPaired() const { return true; }
void ZMLAUXDevice::UnPair() {
    ZML_SendAuxPairingSelection(false, protobuf::HEART_RATE);
    ZML_SendAuxPairingSelection(false, protobuf::WALK_RUN_SPEED);
}
void ZMLAUXDevice::Pair(bool) {
    ZML_SendAuxPairingSelection(true, protobuf::HEART_RATE);
    ZML_SendAuxPairingSelection(true, protobuf::WALK_RUN_SPEED);
}
void ZMLAUXDevice::Update(float) {
    //TODO
}
void Bowflex_BLE_ControlComponent::Bowflex_ParseStream(uint8_t *, uint32_t, Bowflex_BLE_ControlComponent *, SensorValueComponent *) {
    //TODO
}
void Bowflex_BLE_ControlComponent::InitStreaming() {
    //TODO
}
bool TrainerControlComponent::IsFitTech() {
    if (m_owner)
        return m_owner->m_field_11C == 1; //not sure
    return false;
}

FTMS_ControlComponent_v3::FTMS_ControlComponent_v3(BLEDevice *dev) : m_bleDevice(dev) {
    m_protocolType = FTMS_V3;
    m_field_28 = 1;
    m_gradeLookAheadSecs = 1.5f;
    m_field_40 = 1.0f;
    m_FID_FSF = Experimentation::Instance()->IsEnabled(FID_FSF);
    m_field_B18 = 60;
    m_field_B38 = -1;
    if (m_bleDevice) {
        //OMIT m_devAnalytics stuff
        Log( "%s configured for new FTMS indoor bike trainer implementation (v3).", m_bleDevice->m_name);
    } else {
        Log("Device configured for new FTMS indoor bike trainer implementation (v3).");
    }
}
bool FitnessDeviceManager::TrainerSetGradeLookAheadSecs(float f) {
    if (m_pSelectedControllableTrainerDevice && timeGetTime() - m_pSelectedControllableTrainerDevice->m_last_time_ms >= 5000) {
        auto cot = (TrainerControlComponent *)m_pSelectedControllableTrainerDevice->FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (cot) {
            cot->SetGradeLookAheadSecs(f);
            return true;
        }
    }
    return false;
}
bool FitnessDeviceManager::TrainerSetWindSpeed(float f) {
    if (m_pSelectedControllableTrainerDevice && timeGetTime() - m_pSelectedControllableTrainerDevice->m_last_time_ms >= 5000) {
        auto cot = (TrainerControlComponent *)m_pSelectedControllableTrainerDevice->FindComponentOfType(DeviceComponent::CPT_CTRL);
        if (cot) {
            cot->SetWindSpeed(f);
            return true;
        }
    }
    return false;
}
bool TACX_BLE_ControlComponent::SupportsRoadTexture() {
    return m_parent
        && (strstr(m_parent->m_nameId.c_str(), "Tacx Neo")
            || strstr(m_parent->m_nameId.c_str(), "Tacx Smart")
            || strstr(m_parent->m_nameId.c_str(), "Garmin Neo")
            || strstr(m_parent->m_nameId.c_str(), "Garmin Flux"));
}
void TACX_BLE_ControlComponent::SetERGMode(int erg) {
    auto t = timeGetTime();
    static uint32_t g_lastTs = t - 1001;
    if (t - g_lastTs > 1000 && m_parent) {
        g_lastTs = t;
        if (m_erg != erg)
            Log("[BLE TACX] ERG (%dw)", erg);
        m_field_20 = 1;
        std::string val("\xA4\x09\x4E\x05\x31\xFF\xFF\xFF\xFF\xFF\x0\0\0"s);
        *(uint16_t *)&val[10] = 4 * erg;
        val[12] = val[10] - 'x'; //checksum?
        protobuf::BLEPeripheralRequest req;
        req.set_type(protobuf::WRITE_CHARACTERISTIC_VALUE);
        auto per = req.mutable_per();
        per->set_device_id(m_parent->m_devId);
        auto serv = req.add_servs();
        serv->set_id("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E"s);
        auto chr = serv->add_chars();
        chr->set_id(m_parent->m_scharId);
        chr->set_value(val);
        m_parent->LogBleTxPacket("TACX_BLE_ControlComponent::SetERGMode", m_parent->m_name, req);
        BLEModule::Instance()->SendValueToDevice(req, m_parent->m_bleSrc);
        m_erg = erg;
        m_lastTimeSec = timeGetTime() * 0.001;
    }
}
void TACX_BLE_ControlComponent::SetSimulationGrade(float grade) {
    auto t = timeGetTime();
    static uint32_t g_lastTs = t - 1001;
    if (t - g_lastTs > 1000 && m_parent) {
        g_lastTs = t;
        m_field_20 = 3;
        auto spd = BikeManager::Instance()->m_mainBike->m_bc->m_speed;
        if (!check_float(spd) || spd < 0.0f)
            grade = 0.0f;
        m_gradePercent = grade * 100.0f;
        int v10 = std::clamp(int((m_gradePercent * 100.0) + 20000.0), 0, 0xFFFF);
        std::string val("\xA4\x09\x4E\x05\x33\xFF\xFF\xFF\xFF\x0\x0\xFF\0"s);
        *(uint16_t *)&val[9] = v10;
        val[12] = val[9] + val[10] - 'u'; //checksum?
        protobuf::BLEPeripheralRequest req;
        req.set_type(protobuf::WRITE_CHARACTERISTIC_VALUE);
        auto per = req.mutable_per();
        per->set_device_id(m_parent->m_devId);
        auto serv = req.add_servs();
        serv->set_id("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E"s);
        auto chr = serv->add_chars();
        chr->set_id(m_parent->m_scharId);
        chr->set_value(val);
        m_parent->LogBleTxPacket("TACX_BLE_ControlComponent::SetSimulationGrade", m_parent->m_name, req);
        BLEModule::Instance()->SendValueToDevice(req, m_parent->m_bleSrc);
        m_lastTimeSec = timeGetTime() * 0.001;
    }
}
void TACX_BLE_ControlComponent::SetRoadTexture(RoadFeelType ty, float strength) {
    if (SupportsRoadTexture() && m_parent) {
        float coeff = 1.0f;
        enum TacxRoadFeel { TACX_NOP = 0, TACX_CONCRETE_PLATES = 1, TACX_CATTLE_GUARD = 2, TACX_COBBLESTONES_1 = 3, TACX_COBBLESTONES_2 = 4, TACX_BRICKS = 5, TACX_OFFROAD = 6, TACX_GRAVEL = 7, TACX_ICE = 8, TACX_WOOD = 9 }
        tacxTy = TACX_NOP; //no road feel
        switch (ty) {
        case RF_WOOD:
            tacxTy = TACX_WOOD;
            break;
        case RF_BRICKS_HARD:
            tacxTy = TACX_BRICKS;
            break;
        case RF_BRICKS_SOFT:
            tacxTy = TACX_BRICKS;
            coeff = 0.5f;
            break;
        case RF_GRAVEL1: case RF_GRAVEL2:
            tacxTy = TACX_GRAVEL;
            break;
        case RF_GRAVEL_SOFT:
            tacxTy = TACX_GRAVEL;
            coeff = 0.9f;
            break;
        }
        float outcome = std::clamp(coeff * strength, 0.0f, 1.0f) * std::clamp(100.0f - m_gradePercent * 20.0f, 0.0f, 100.0f);
        static int g_cnt = 10, g_lastTacxTy = -1;
        static uint32_t g_lastTs;
        if (m_field_20 == 1)
            tacxTy = TACX_NOP;
        if (g_lastTacxTy != tacxTy)
            g_cnt = 0;
        auto t = timeGetTime();
        if (t - g_lastTs > 5000 || (g_cnt < 3 && t - g_lastTs > 250)) {
            g_lastTs = t;
            LogTyped(LOG_ANT, "BLE: Tacx set road texture %d (time=%d)", tacxTy, t);
            std::string val("\xA4\x09\x4E\x05\xFC\0\0\x64\0\0\0\0\0"s);
            val[9] = (char)tacxTy;
            auto outcomew = uint16_t(outcome + 0.5f);
            *(uint16_t *)&val[10] = outcomew;
            val[12] = val[9] + val[10] - 'D'; //checksum?
            protobuf::BLEPeripheralRequest req;
            req.set_type(protobuf::WRITE_CHARACTERISTIC_VALUE);
            auto per = req.mutable_per();
            per->set_device_id(m_parent->m_devId);
            auto serv = req.add_servs();
            serv->set_id("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E"s);
            auto chr = serv->add_chars();
            chr->set_id(m_parent->m_scharId);
            chr->set_value(val);
            m_parent->LogBleTxPacket("TACX_BLE_ControlComponent::SetRoadTexture", m_parent->m_name, req);
            BLEModule::Instance()->SendValueToDevice(req, m_parent->m_bleSrc);
            g_lastTacxTy = tacxTy;
            ++g_cnt;
        }
    }
}
