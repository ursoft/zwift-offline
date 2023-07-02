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
const char *g_devBleSubtype[BLEST_CNT] = { "BLE", "BLE (ZC)", "BLE (ZH)", "BLE (LAN)" };
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
        auto v8 = ((BLEDevice *)this)->m_bleSubType;
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
void WFTNPDeviceManager::WriteCharacteristic(const protobuf::BLEPeripheralRequest &rq) {
    //TODO
}
void WFTNPDeviceManager::UnPair(BLEDevice *dev) {
    //TODO
}
