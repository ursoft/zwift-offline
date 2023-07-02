//UT Coverage: 50%, 249/502, NEED_MORE
#include "ZwiftApp.h"
void BLEDevice_CreateTrainerST3(const protobuf::BLEAdvertisement &adv, uint8_t a2, uint16_t a3, BLE_SOURCE src) {
    //TODO
}
bool IsWahooDirectConnectEnabled() {
    static bool g_bWahooDirectConnectEnabled = Experimentation::Instance()->IsEnabled(FID_WAHOOD);
    return g_bWahooDirectConnectEnabled;
}
void BLEDevice_ProcessBLEResponse(const protobuf::BLEPeripheralResponse &resp, BLE_SOURCE src) {
    //TODO
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
    uint16_t m_f3;
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
    per->set_f3(src.m_f3); //TACX 1816/2A55: FFB6; TACX 1818/2A55:FFB3
    per->set_device_id(src.m_deviceId); //TACX 1816/2A55(any): 274523460006625
    per->set_device_name(src.m_deviceName);
}
void parseDllBLEAdvertisement(const dllBLEAdvertisement &src, protobuf::BLEAdvertisement *dest) {
    auto per = dest->mutable_per();
    per->set_f3(src.m_per_f3);
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
    per->set_f3(resp.m_f3);
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
typedef bool (*fptr_bool_ptr)(void *);
typedef bool (*fptr_bool_ptr3)(void *, void *, void *);
#pragma comment(lib, "Version.lib")
struct BLEDeviceManager { //and BLEDeviceManagerWindows
    fptr_void_ptr m_startScanningFunc;
    fptr_void_void m_stopScanningFunc;
    fptr_void_void m_purgeDeviceListFunc;
    fptr_bool_ptr m_pairToDeviceFunc;
    fptr_bool_ptr m_unpairFromDeviceFunc;
    fptr_bool_ptr3 m_writeToDeviceFunc;
    fptr_void_void m_initFlagsFunc;
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
        auto pairToDeviceFunc = (fptr_bool_ptr)GetProcAddress(BleDll, "BLEPairToDevice");
        auto unpairFromDeviceFunc = (fptr_bool_ptr)GetProcAddress(BleDll, "BLEUnpairFromDevice");
        auto writeToDeviceFunc = (fptr_bool_ptr3)GetProcAddress(BleDll, "BLEWriteToDevice");
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
        //TODO
    }
    void PurgeDeviceList() { //vptr[4]
        //TODO
    }
    void SendValueToDevice(const protobuf::BLEPeripheralRequest &req) { //vptr[5]
        //TODO
    }
    void StartSearchForLostDevices() { //vptr[6]
        //TODO
    }
    void StopSearchForLostDevices() { //vptr[7]
        //TODO
    }
    void PairDevice(const std::string &) {
        //TODO
    }
    void PairDevice(const BLEDevice &dev) { //vptr[8]
        //TODO
    }
    void UnpairDevice(const BLEDevice &dev) { //vptr[9]
        //TODO
    }
} g_BLEDeviceManager;
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
bool g_EnableDeviceDiscovery = true;
void BLEModule::LegacyBLEImpl::EnableDeviceDiscovery(bool en) {
    g_EnableDeviceDiscovery = en;
}
void BLEModule::LegacyBLEImpl::GetRSSI(const char *) {
    //empty
}
bool BLEModule::LegacyBLEImpl::HasBLE() {
    return g_BLEDeviceManager.m_HasBLE;
}
bool g_IgnoreReceivedBluetoothPackets;
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
void BLEModule::LegacyBLEImpl::PairDevice(const BLEDevice &) {
    //empty
}
void BLEModule::LegacyBLEImpl::PairDevice(const std::string &s) {
    g_BLEDeviceManager.PairDevice(s);
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
    case BLES_2:
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
protobuf::BLEPeripheralResponse_Error g_lastBLEError = protobuf::BL_ERR_UNK;
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
void BLEModule::PairDevice(const std::string &s) {
    callVoidImplMethodOrLogIfUninitialized([this, s]() {
        this->m_bleImpl->PairDevice(s);
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
void BLEDevice::ProcessBLEData(const protobuf::BLEPeripheralResponse &) {
    //TODO
}
void BLEDevice::Pair(bool) {
    //TODO
}
bool BLEDevice::IsActivelyPaired() {
    return m_isPaired;
}
bool BLEDevice::IsPaired() {
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
    switch(m_bleSubType) {
    case BLEST_GENERIC:
        BLEModule::Instance()->UnpairDevice(*this);
        SetPaired(false);
        break;
    case BLEST_ZC:
        if (m_isPaired) {
            protobuf::BLEPeripheralRequest rq;
            CommonUnpair(&rq, m_devId, m_charId);
            m_isPaired = false;
            zwift_network::send_ble_peripheral_request(rq);
        }
        break;
    case BLEST_ZH:
        zassert(!"UnPair not implented for BLE source");
        break;
    case BLEST_LAN:
        WFTNPDeviceManager::UnPair(this);
        SetPaired(false);
        break;
    }
    FitnessDeviceManager::RemoveDevice(this, false);
}
void BLEDevice::Update(float) {
    //TODO
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