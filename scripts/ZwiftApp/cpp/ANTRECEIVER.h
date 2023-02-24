#pragma once
void ANTRECEIVER_Connect();
void ANTRECEIVER_Disconnect();
void ANTRECEIVER_GetDongleIDString();
void ANTRECEIVER_Initialize();
bool ANTRECEIVER_IsConnected();
void ANTRECEIVER_PairDevice(uint8_t, uint16_t, uint32_t);
void ANTRECEIVER_PostConnect();
void ANTRECEIVER_SetUSBDevicePollTime(float);
void ANTRECEIVER_Shutdown();
void ANTRECEIVER_UnpairChannel(uint8_t);
void ANTRECEIVER_Update(float);
void AntReceiver_Update(float);