#pragma once //READY for testing
void ANT_DeviceSearch(int mfgNet);
bool ANT_IsInitialized();
bool ANT_IsSearchEnabled();
bool ANT_Load();
void ANT_StopDeviceSearch();
void ANTRECEIVER_Connect();
//empty void ANTRECEIVER_Disconnect();
const char *ANTRECEIVER_GetDongleIDString();
void ANTRECEIVER_Initialize();
bool ANTRECEIVER_IsConnected();
int ANTRECEIVER_PairDevice(uint8_t, uint16_t, uint32_t);
void ANTRECEIVER_PostConnect();
//inlined void ANTRECEIVER_SetUSBDevicePollTime(float);
void ANTRECEIVER_Shutdown();
//inlined void ANTRECEIVER_UnpairChannel(uint8_t);
void ANTRECEIVER_Update(float);
//empty void AntReceiver_Update(float);
inline bool g_bANTInitialized, g_bANTConnected;
