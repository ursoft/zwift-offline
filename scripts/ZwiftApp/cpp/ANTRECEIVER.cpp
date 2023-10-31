#include "ZwiftApp.h" //READY for testing
int g_ANT_Mutex = -1;
typedef BOOL(*fANT_CloseChannel)(uint8_t ch);
fANT_CloseChannel ANT_CloseChannel;
typedef BOOL(*fANT_SetChannelSearchTimeout)(uint8_t ch, uint8_t to);
fANT_SetChannelSearchTimeout ANT_SetChannelSearchTimeout;
typedef BOOL(*fANT_AssignChanelExt)(uint8_t ch, uint8_t ty, uint8_t net, uint8_t flags);
fANT_AssignChanelExt ANT_AssignChanelExt;
typedef BOOL(*fANT_Init)(uint8_t ucUSBDeviceNum, uint64_t ulBaudrate, uint8_t ucPortType, uint8_t ucSerialFrameType);
fANT_Init ANT_Init;
typedef BOOL(*fANT_SetLowPriorityChannelSearchTimeout)(uint8_t ucANTChannel, uint8_t ucSearchTimeout);
fANT_SetLowPriorityChannelSearchTimeout ANT_SetLowPriorityChannelSearchTimeout;
typedef BOOL(*fANT_SetTransmitPower)(uint8_t ucTransmitPower);
fANT_SetTransmitPower ANT_SetTransmitPower;
typedef void(*fANT_Close)();
fANT_Close ANT_Close;
typedef BOOL(*fANT_SetChannelId)(uint8_t ucANTChannel, uint16_t usDeviceNumber, uint8_t ucDeviceType, uint8_t ucTransmissionType);
fANT_SetChannelId ANT_SetChannelId;
typedef BOOL(*fANT_OpenChannel)(uint8_t ucANTChannel);
fANT_OpenChannel ANT_OpenChannel;
typedef BOOL(*fANT_UnAssignChannel)(uint8_t ucANTChannel);
fANT_UnAssignChannel ANT_UnAssignChannel;
typedef BOOL(*fANT_SendAcknowledgedData)(uint8_t ucANTChannel, uint8_t *pucData);
fANT_SendAcknowledgedData ANT_SendAcknowledgedData;
typedef BOOL(*fANT_SetNetworkKey)(uint8_t ucNetNumber, uint8_t *pucKey);
fANT_SetNetworkKey ANT_SetNetworkKey;
typedef BOOL(*fANT_GetDeviceUSBPID)(uint16_t *pusPID);
fANT_GetDeviceUSBPID ANT_GetDeviceUSBPID;
typedef BOOL(*fANT_SendBurstTransfer)(uint8_t ucANTChannel, uint8_t *pucData, uint16_t usNumDataPackets);
fANT_SendBurstTransfer ANT_SendBurstTransfer;
typedef BOOL(*RESPONSE_FUNC)(uint8_t ucANTChannel, uint8_t ucResponseMsgID);
typedef BOOL(*CHANNEL_EVENT_FUNC)(uint8_t ucANTChannel, uint8_t ucEvent);
typedef BOOL(*fANT_AssignResponseFunction)(RESPONSE_FUNC pfResponse, uint8_t *pucResponseBuffer);
fANT_AssignResponseFunction ANT_AssignResponseFunction;
typedef BOOL(*fANT_SetChannelPeriod)(uint8_t ucANTChannel, uint16_t usMesgPeriod);
fANT_SetChannelPeriod ANT_SetChannelPeriod;
typedef BOOL(*fANT_LibConfigCustom)(uint8_t ucLibConfigFlags_, uint64_t ulResponseTime);
fANT_LibConfigCustom ANT_LibConfigCustom;
typedef BOOL(*fANT_SetChannelRFFreq)(uint8_t ucANTChannel, uint8_t ucRFFreq);
fANT_SetChannelRFFreq ANT_SetChannelRFFreq;
typedef BOOL(*fANT_Nap)(uint64_t ulMilliseconds);
fANT_Nap ANT_Nap;
typedef BOOL(*fANT_AssignChannel)(uint8_t ucANTChannel, uint8_t ucChanType, uint8_t ucNetNumber);
fANT_AssignChannel ANT_AssignChannel;
typedef BOOL(*fANT_RequestMessage)(uint8_t ucANTChannel, uint8_t ucMessageID);
fANT_RequestMessage ANT_RequestMessage;
typedef void(*fANT_AssignChannelEventFunction)(uint8_t ucANTChannel, CHANNEL_EVENT_FUNC pfChannelEvent, uint8_t *pucRxBuffer);
fANT_AssignChannelEventFunction ANT_AssignChannelEventFunction;
typedef BOOL(*fANT_GetDeviceUSBInfo)(uint8_t ucUSBDeviceNum, uint8_t *pucProductString, uint8_t *pucSerialString);
fANT_GetDeviceUSBInfo ANT_GetDeviceUSBInfo;
typedef BOOL(*fANT_ResetSystem)(void);
fANT_ResetSystem ANT_ResetSystem;
typedef BOOL(*fANT_SetChannelTxPower)(uint8_t ucANTChannel, uint8_t ucTransmitPower);
fANT_SetChannelTxPower ANT_SetChannelTxPower;
typedef BOOL(*fANT_SetCWTestMode)(uint8_t ucTransmitPower, uint8_t ucRFChannel);
fANT_SetCWTestMode ANT_SetCWTestMode;
typedef BOOL(*fANT_SendExtAcknowledgedData)(uint8_t ucANTChannel, uint8_t *pucData);
fANT_SendExtAcknowledgedData ANT_SendExtAcknowledgedData;
typedef BOOL(*fANT_AddChannelID)(uint8_t ucANTChannel, uint16_t usDeviceNumber, uint8_t ucDeviceType, uint8_t ucTranmissionType, uint8_t ucIndex);
fANT_AddChannelID ANT_AddChannelID;
typedef BOOL(*fANT_OpenRxScanMode)(void);
fANT_OpenRxScanMode ANT_OpenRxScanMode;
typedef BOOL(*fANT_SetProximitySearch)(uint8_t ucANTChannel, uint8_t ucSearchThreshold);
fANT_SetProximitySearch ANT_SetProximitySearch;
typedef BOOL(*fANT_SendExtBurstTransferPacket)(uint8_t ucANTChannelSeq, uint8_t *pucData);
fANT_SendExtBurstTransferPacket ANT_SendExtBurstTransferPacket;
typedef BOOL(*fANT_EnableLED)(uint8_t ucEnable);
fANT_EnableLED ANT_EnableLED;
typedef BOOL(*fANT_ConfigList)(uint8_t ucANTChannel, uint8_t ucListSize, uint8_t ucExclude);
fANT_ConfigList ANT_ConfigList;
typedef BOOL(*fANT_InitCWTestMode)(void);
fANT_InitCWTestMode ANT_InitCWTestMode;
typedef BOOL(*fANT_SetSerialNumChannelId)(uint8_t ucANTChannel, uint8_t ucDeviceType, uint8_t ucTransmissionType);
fANT_SetSerialNumChannelId ANT_SetSerialNumChannelId;
typedef BOOL(*fANT_ConfigFrequencyAgility)(uint8_t ucANTChannel, uint8_t ucFreq1, uint8_t ucFreq2, uint8_t ucFreq3);
fANT_ConfigFrequencyAgility ANT_ConfigFrequencyAgility;
typedef BOOL(*fANT_SleepMessage)(void);
fANT_SleepMessage ANT_SleepMessage;
typedef uint16_t(*fANT_SendExtBurstTransfer)(uint8_t ucANTChannel, uint8_t *pucData, uint16_t usNumDataPackets);
fANT_SendExtBurstTransfer ANT_SendExtBurstTransfer;
typedef BOOL(*fANT_CrystalEnable)(void);
fANT_CrystalEnable ANT_CrystalEnable;
typedef BOOL(*fANT_RxExtMesgsEnable)(uint8_t ucEnable);
fANT_RxExtMesgsEnable ANT_RxExtMesgsEnable;
typedef BOOL(*fANT_SendExtBroadcastData)(uint8_t ucANTChannel, uint8_t *pucData);
fANT_SendExtBroadcastData ANT_SendExtBroadcastData;
typedef BOOL(*fANT_SendBroadcastData)(uint8_t ucANTChannel, uint8_t *pucData);
fANT_SendBroadcastData ANT_SendBroadcastData;
typedef BOOL(*fANT_AssignChannelExt)(uint8_t ch, uint8_t ty, uint8_t net, uint8_t flags);
fANT_AssignChannelExt ANT_AssignChannelExt;
struct ANT_INFO { //12 bytes
    bool m_opened = false;
    uint8_t m_periodCode = 0, m_antNet = 0xFF;
    char field_3 = 0;
    int m_mfgNet = 4;
    uint16_t m_deviceID = 0;
    char field_A = 0;
    char field_B = 0;
} g_antInfo[8];
struct ANT_EXT_INFO {
    int m_antNet, m_rfFreq;
    uint8_t m_networkKey[8];
} g_ANT_ExtInfo[] = { 
    { 0, 57, { 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45 } },
    { 1, 53, { 0xE8, 0xE4, 0x32, 0x68, 0x1D, 0x56, 0x47, 0xc5 } },
    { 2, 71 },
    { 1, 57, { 0xA9, 0xAD, 0x32, 0x68, 0x3d, 0x76, 0xC7, 0x4D } }
};
uint8_t g_ANTDongleProductName[0x400];
time_t g_ANT_ConnectTime;
uint8_t g_ANT_ResponseBuffer[44], g_ANT_RxBuffer[_countof(g_antInfo)][41], g_ANTDeviceNum = 0xFF;
int g_ANT_NetStates[3] = { 4, 4, 4 };
const uint16_t g_ANT_PeriodCodes[] = { 120, 121, 122, 123, 11, 124, 24, 8, 19, 17, 1, 0 };
const uint16_t g_ANT_Periods[] = { 8070u, 8086u, 8102u, 8118u, 8182u, 8134u, 8182u, 10923u, 31814u, 8192u, 8198u, 0u };
BOOL ANT_Response(uint8_t ucANTChannel, uint8_t ucResponseMsgID) {
    ZMUTEX_Lock(g_ANT_Mutex);
    if (ucANTChannel == 1)
        LogTyped(LOG_ANT, "AUC message id=%d", ucResponseMsgID);
    switch (ucResponseMsgID) {
    case 62u:
        LogTyped(LOG_ANT, "VERSION: %s", g_ANT_ResponseBuffer);
        break;
    case 64u:
        switch (g_ANT_ResponseBuffer[2]) {
        case 0u:
            switch (g_ANT_ResponseBuffer[1]) {
            case 65u:
                LogTyped(LOG_ANT, "Unassigned Channel");
                g_antInfo[ucANTChannel] = ANT_INFO();
                break;
            case 66u:
                LogTyped(LOG_ANT, "Channel Assigned");
                LogTyped(LOG_ANT, "Setting Channel ID for chan %d (device %d%d)...", ucANTChannel, g_antInfo[ucANTChannel].m_deviceID, g_antInfo[ucANTChannel].m_periodCode);
                if (ucANTChannel)
                    ANT_SetChannelId(ucANTChannel, g_antInfo[ucANTChannel].m_deviceID, g_antInfo[ucANTChannel].m_periodCode, 0);
                else
                    ANT_SetChannelId(0, 0, 0, 0);
                break;
            case 67u:
                break;
            case 69u:
                ANT_LibConfigCustom(0xE0u, 0); //QUEST: WAS v5 instead of 0
                LogTyped(LOG_ANT, "Radio Frequency set");
                LogTyped(LOG_ANT, "Opening channel %d", ucANTChannel);
                if (ANT_OpenChannel(ucANTChannel))
                    g_antInfo[ucANTChannel].m_opened = true;
                else
                    LogTyped(LOG_ANT, "Opening Failed");
                break;
            case 70u:
                LogTyped(LOG_ANT, "Network Key Set");
                break;
            case 71u:
                LogTyped(LOG_ANT, "TX POWER ID");
                break;
            case 75u:
                LogTyped(LOG_ANT, "Channel opened");
                break;
            case 76u:
                LogTyped(LOG_ANT, "Closed channel %d", ucANTChannel);
                break;
            case 81u:
                LogTyped(LOG_ANT, "Channel ID Set");
                ANT_SetChannelRFFreq(ucANTChannel, g_ANT_ExtInfo[g_antInfo[ucANTChannel].m_mfgNet].m_rfFreq);
                if (ucANTChannel) {
                    BOOL bSuccess = FALSE;
                    for (auto i = 0; i < 11; ++i) {
                        if (g_antInfo[ucANTChannel].m_periodCode == g_ANT_PeriodCodes[i]) {
                            bSuccess = ANT_SetChannelPeriod(ucANTChannel, g_ANT_Periods[i]);
                            if (!bSuccess)
                                LogTyped(LOG_ANT, "Unable to set channel period on channel %d", ucANTChannel);
                        }
                    }
                    zassert(bSuccess);
                }
                break;
            case 102u:
                LogTyped(LOG_ANT, "Extended messages enabled");
                break;
            default:
                LogTyped(LOG_ANT, "Unknown response");
                break;
            }
            break;
        case 0x15u:
            LogTyped(LOG_ANT, "Channel %d in wrong state", ucANTChannel);
            break;
        case 0x16u:
            LogTyped(LOG_ANT, "Channel %d not opened", ucANTChannel);
            break;
        case 0x18u:
            LogTyped(LOG_ANT, "Channel ID not set");
            break;
        case 0x19u:
            LogTyped(LOG_ANT, "Close all channels");
            break;
        case 0x1Fu:
            LogTyped(LOG_ANT, "Transfer in progress");
            break;
        case 0x20u:
            LogTyped(LOG_ANT, "Transfer sequence number error");
            break;
        case 0x21u:
            LogTyped(LOG_ANT, "Transfer in error");
            break;
        case 0x28u:
            LogTyped(LOG_ANT, "Invalid message");
            break;
        case 0x29u:
            LogTyped(LOG_ANT, "Invalid network number");
            break;
        case 0x30u:
            LogTyped(LOG_ANT, "Invalid list ID");
            break;
        case 0x31u:
            LogTyped(LOG_ANT, "Invalid Scanning transmit channel");
            break;
        case 0x33u:
            LogTyped(LOG_ANT, "Invalid parameter provided");
            break;
        case 0x35u:
            LogTyped(LOG_ANT, "Queue overflow");
            break;
        default:
            LogTyped(LOG_ANT, "Unknown Error Code %d to message %d", g_ANT_ResponseBuffer[2], g_ANT_ResponseBuffer[1]);
            break;
        }
        break;
    case 81u:
        LogTyped(LOG_ANT, "CHANNEL ID: (%d/%d/%d)", (uint16_t)g_ANT_ResponseBuffer[1] | (uint16_t(g_ANT_ResponseBuffer[2]) << 8), g_ANT_ResponseBuffer[3], g_ANT_ResponseBuffer[4]);
        break;
    case 82u:
        switch (g_ANT_ResponseBuffer[1] & 3) {
        case 0: LogTyped(LOG_ANT, "Got Status: UNASSIGNED_CHANNEL"); break;
        case 1: LogTyped(LOG_ANT, "Got Status: ASSIGNED_CHANNEL"); break;
        case 2: LogTyped(LOG_ANT, "Got Status: SEARCHING_CHANNEL"); break;
        case 3: LogTyped(LOG_ANT, "Got Status: TRACKING_CHANNEL"); break;
        }
        break;
    case 84u:
        LogTyped(LOG_ANT, "CAPABILITIES:");
        LogTyped(LOG_ANT, "   Max ANT Channels: %d", g_ANT_ResponseBuffer[0]);
        LogTyped(LOG_ANT, "   Max ANT Networks: %d", g_ANT_ResponseBuffer[1]);
        LogTyped(LOG_ANT, "Standard Options:");
        if (g_ANT_ResponseBuffer[2] & 1)
            LogTyped(LOG_ANT, "CAPABILITIES_NO_RX_CHANNELS");
        if (g_ANT_ResponseBuffer[2] & 2)
            LogTyped(LOG_ANT, "CAPABILITIES_NO_TX_CHANNELS");
        if (g_ANT_ResponseBuffer[2] & 4)
            LogTyped(LOG_ANT, "CAPABILITIES_NO_RX_MESSAGES");
        if (g_ANT_ResponseBuffer[2] & 8)
            LogTyped(LOG_ANT, "CAPABILITIES_NO_TX_MESSAGES");
        if (g_ANT_ResponseBuffer[2] & 0x10)
            LogTyped(LOG_ANT, "CAPABILITIES_NO_ACKD_MESSAGES");
        if (g_ANT_ResponseBuffer[2] & 0x20)
            LogTyped(LOG_ANT, "CAPABILITIES_NO_BURST_TRANSFER");
        LogTyped(LOG_ANT, "Advanced Options:");
        if (g_ANT_ResponseBuffer[3] & 1)
            LogTyped(LOG_ANT, "CAPABILITIES_OVERUN_UNDERRUN");
        if (g_ANT_ResponseBuffer[3] & 2)
            LogTyped(LOG_ANT, "CAPABILITIES_NETWORK_ENABLED");
        if (g_ANT_ResponseBuffer[3] & 4)
            LogTyped(LOG_ANT, "CAPABILITIES_AP1_VERSION_2");
        if (g_ANT_ResponseBuffer[3] & 8)
            LogTyped(LOG_ANT, "CAPABILITIES_SERIAL_NUMBER_ENABLED");
        if (g_ANT_ResponseBuffer[3] & 0x10)
            LogTyped(LOG_ANT, "CAPABILITIES_PER_CHANNEL_TX_POWER_ENABLED");
        if (g_ANT_ResponseBuffer[3] & 0x20)
            LogTyped(LOG_ANT, "CAPABILITIES_LOW_PRIORITY_SEARCH_ENABLED");
        if (g_ANT_ResponseBuffer[3] & 0x40)
            LogTyped(LOG_ANT, "CAPABILITIES_SCRIPT_ENABLED");
        if (g_ANT_ResponseBuffer[3] & 0x80)
            LogTyped(LOG_ANT, "CAPABILITIES_SEARCH_LIST_ENABLED");
        LogTyped(LOG_ANT, "Advanced 2 Options 1:");
        if (g_ANT_ResponseBuffer[4] & 1)
            LogTyped(LOG_ANT, "CAPABILITIES_LED_ENABLED");
        if (g_ANT_ResponseBuffer[4] & 2)
            LogTyped(LOG_ANT, "CAPABILITIES_EXT_MESSAGE_ENABLED");
        if (g_ANT_ResponseBuffer[4] & 4)
            LogTyped(LOG_ANT, "CAPABILITIES_SCAN_MODE_ENABLED");
        if (g_ANT_ResponseBuffer[4] & 8)
            LogTyped(LOG_ANT, "CAPABILITIES_RESERVED");
        if (g_ANT_ResponseBuffer[4] & 0x10)
            LogTyped(LOG_ANT, "CAPABILITIES_PROX_SEARCH_ENABLED");
        if (g_ANT_ResponseBuffer[4] & 0x20)
            LogTyped(LOG_ANT, "CAPABILITIES_EXT_ASSIGN_ENABLED");
        if (g_ANT_ResponseBuffer[4] & 0x80)
            LogTyped(LOG_ANT, "CAPABILITIES_FIT1_ENABLED");
        break;
    case 111u:
        LogTyped(LOG_ANT, "RESET Complete, reason: ");
        if (g_ANT_ResponseBuffer[0] == 0)
            LogTyped(LOG_ANT, "RESET_POR");
        if (g_ANT_ResponseBuffer[0] & 0x80u)
            LogTyped(LOG_ANT, "RESET_SUSPEND ");
        if (g_ANT_ResponseBuffer[0] & 0x40)
            LogTyped(LOG_ANT, "RESET_SYNC ");
        if (g_ANT_ResponseBuffer[0] & 0x20)
            LogTyped(LOG_ANT, "RESET_CMD ");
        if (g_ANT_ResponseBuffer[0] & 2)
            LogTyped(LOG_ANT, "RESET_WDT ");
        if (g_ANT_ResponseBuffer[0] & 1)
            LogTyped(LOG_ANT, "RESET_RST ");
        break;
    default:
        break;
    }
    ZMUTEX_Unlock(g_ANT_Mutex);
    return TRUE;
}
ANTDevice *FindRxDevice(uint8_t ucANTChannel) {
    auto v31 = (uint32_t)g_ANT_RxBuffer[ucANTChannel][11], v32 = (uint32_t)g_ANT_RxBuffer[ucANTChannel][10], v33 = (uint32_t)g_ANT_RxBuffer[ucANTChannel][12] & 0x7F;
    return (ANTDevice *)FitnessDeviceManager::FindDevice(v32 | ((v31 | (v33 << 8)) << 8));
}
void ANT_Decypher(uint8_t a1lo, uint8_t a1hi, uint8_t *a2) {
    uint8_t v9;
    for(int i = 1; i < 8; i++) {
        if (a2[i - 1] & 2) {
            if (i == 1) {
                v9 = 0xEE ^ a2[i];
            } else if (i == 2) {
                v9 = 0xE9 ^ a2[i];
            } else {
                v9 = (i == 3) ? (0x87 ^ a2[i]) : (a1hi ^ a2[i]);
            }
        } else {
            v9 = ((a1hi ^ a2[i]) >> 4) + 16 * (a1hi ^ a2[i]);
        }
        a2[i] = a1lo ^ v9 ^ a2[0];
    }
}
char g_ANT_NameId[256];
bool g_ANT_KickrNew, g_ANT_KickrBurst = true;
float g_ANT_KickrNewGrade, g_ANT_KickrNewErg;
int g_ANT_KickrChnl = -1, g_ANT_KickrCnt;
uint16_t g_ANT_KickrErgBuf[4], g_ANT_KickrSimBuf[12], g_ANT_KickrSimBufBurst[12];
uint8_t g_ANT_KickrErgBufBirst[18];
BOOL ANT_ChannelEvent(uint8_t ucANTChannel, uint8_t ucEvent) {
    if (g_ANT_KickrNew) {
        if (g_ANT_KickrNewGrade <= -900.0) {
            LogTyped(LOG_ANT, "KICKR changing ERG to %3.2f", g_ANT_KickrNewErg);
            if (g_ANT_KickrBurst) {
                g_ANT_KickrErgBufBirst[0] = 0x48;
                g_ANT_KickrErgBufBirst[1] = 0x42;
                g_ANT_KickrErgBufBirst[2] = g_ANT_KickrCnt++;
                g_ANT_KickrErgBufBirst[3] = 0x03;
                g_ANT_KickrErgBufBirst[4] = 0xFF;
                g_ANT_KickrErgBufBirst[5] = 0xFF;
                g_ANT_KickrErgBufBirst[6] = 0xFF;
                g_ANT_KickrErgBufBirst[7] = 0xFF;
                g_ANT_KickrErgBufBirst[8] = 0x20;
                g_ANT_KickrErgBufBirst[9] = 0x00;
                g_ANT_KickrErgBufBirst[10] = 0xAA;
                g_ANT_KickrErgBufBirst[11] = 0xAA;
                g_ANT_KickrErgBufBirst[12] = 0x95;
                g_ANT_KickrErgBufBirst[13] = 0x59;
                g_ANT_KickrErgBufBirst[14] = 0xC8;
                g_ANT_KickrErgBufBirst[15] = 0x00;
                auto v11 = (int)g_ANT_KickrNewErg;
                g_ANT_KickrErgBufBirst[16] = v11 & 0xFF;
                g_ANT_KickrErgBufBirst[17] = (v11 >> 8) & 0xFF;
                LogTyped(LOG_ANT, "Setting ERG mode to %d watts", v11);
                ZMUTEX_Lock(g_ANT_Mutex);
                ANT_SendBurstTransfer(g_ANT_KickrChnl, g_ANT_KickrErgBufBirst, 3u);
                ZMUTEX_Unlock(g_ANT_Mutex);
                g_ANT_KickrCnt %= 8;
            } else {
                g_ANT_KickrErgBuf[0] = 0x42F1;
                g_ANT_KickrErgBuf[1] = g_antInfo[g_ANT_KickrChnl].m_deviceID;
                g_ANT_KickrErgBuf[2] = (uint16_t)g_ANT_KickrNewErg;
                g_ANT_KickrErgBuf[3] = 0xFF00;
                LogTyped(LOG_ANT, "Setting ERG mode to %d watts", g_ANT_KickrErgBuf[2]);
                ZMUTEX_Lock(g_ANT_Mutex);
                ANT_SendAcknowledgedData(g_ANT_KickrChnl, (uint8_t *)g_ANT_KickrErgBuf);
                ZMUTEX_Unlock(g_ANT_Mutex);
            }
        } else {
            LogTyped(LOG_ANT, "KICKR changing grade to %3.2f", g_ANT_KickrNewGrade);
            auto v5 = (uint16_t)((fmaxf(fminf(g_ANT_KickrNewGrade, 1.0f), -1.0f) + 1.0f) * 32768.0f);
            if (!g_ANT_KickrBurst) {
                g_ANT_KickrSimBuf[0] = 0x46F1;
                g_ANT_KickrSimBuf[1] = g_antInfo[g_ANT_KickrChnl].m_deviceID;
                g_ANT_KickrSimBuf[2] = v5;
                g_ANT_KickrSimBuf[3] = 0xFFFF;
                LogTyped(LOG_ANT, "Setting Sim Grade %d ", v5);
                ZMUTEX_Lock(g_ANT_Mutex);
                ANT_SendBroadcastData(g_ANT_KickrChnl, (uint8_t *)&g_ANT_KickrSimBuf);
                ZMUTEX_Unlock(g_ANT_Mutex);
            } else {
                g_ANT_KickrSimBufBurst[0] = 0x4648;
                g_ANT_KickrSimBufBurst[1] = (g_ANT_KickrCnt++) | 0x0300;
                g_ANT_KickrSimBufBurst[2] = 0xFFFF;
                g_ANT_KickrSimBufBurst[3] = 0xFFFF;
                g_ANT_KickrSimBufBurst[4] = 0x0020;
                g_ANT_KickrSimBufBurst[5] = 0xAAAA;
                g_ANT_KickrSimBufBurst[6] = 0x5995;
                g_ANT_KickrSimBufBurst[7] = 0x00C8;
                g_ANT_KickrSimBufBurst[8] = v5;
                g_ANT_KickrSimBufBurst[9] = 0;
                g_ANT_KickrSimBufBurst[10] = 0;
                g_ANT_KickrSimBufBurst[11] = 0;
                LogTyped(LOG_ANT, "Setting Sim Grade %d (burst)", v5);
                ZMUTEX_Lock(g_ANT_Mutex);
                ANT_SendBurstTransfer(g_ANT_KickrChnl, (uint8_t *)g_ANT_KickrSimBufBurst, 3u);
                ZMUTEX_Unlock(g_ANT_Mutex);
                g_ANT_KickrCnt %= 8;
            }
        }
        g_ANT_KickrNew = 0;
        g_ANT_KickrChnl = -1;
        g_ANT_KickrNewErg = 0.0f;
        g_ANT_KickrNewGrade = 0.0f;
    }
    switch (ucEvent) {
    case 1u:
        LogTyped(LOG_ANT, "Search Timeout on channel %d", ucANTChannel);
        break;
    case 2u:
        LogTyped(LOG_ANT, "Rx Fail on channel %d", ucANTChannel);
        break;
    case 3u:
        //v16 = byte_7FF6E3FF9A70++; //QUEST: where used
        //byte_7FF6E3FF48E0 = v16;
        break;
    case 4u:
        LogTyped(LOG_ANT, "Burst recieve has failed");
        break;
    case 5u:
        LogTyped(LOG_ANT, "Transfer Completed.");
        break;
    case 6u:
        LogTyped(LOG_ANT, "Transfer Failed.");
        break;
    case 7u:
        LogTyped(LOG_ANT_IMPORTANT, "Channel %d Closed", ucANTChannel);
        LogTyped(LOG_ANT_IMPORTANT, "Unassign Channel");
        ANT_UnAssignChannel(ucANTChannel);
        break;
    case 8u:
        LogTyped(LOG_ANT, "Goto Search.");
        break;
    case 9u:
        break;
    case 0xAu:
        LogTyped(LOG_ANT, "Burst Started");
        break;
    case 0x9Au: case 0x9Bu: case 0x9Cu: case 0x9Du: case 0x9Eu: case 0x9Fu: case 0xA3u: case 0xA4u: case 0xA5u:
        if (ucANTChannel) {
            auto Device = FindRxDevice(ucANTChannel);
            if (Device) {
                if (g_antInfo[ucANTChannel].m_mfgNet == 1)
                    ANT_Decypher(g_ANT_RxBuffer[ucANTChannel][10], g_ANT_RxBuffer[ucANTChannel][11], &g_ANT_RxBuffer[ucANTChannel][1]);
                Device->ProcessANTBroadcastData(g_ANT_RxBuffer[ucANTChannel]);
                Device->m_last_time_ms = timeGetTime();
            }
        } else if (ucEvent == 0xA3) {
            if (g_ANT_RxBuffer[0][9] & 0x80u) {
                auto v17 = *(uint16_t *)&g_ANT_RxBuffer[0][10];
                auto v18 = g_ANT_RxBuffer[0][12] & 0x7F;
                auto v19 = g_ANT_RxBuffer[0][13];
                auto v22 = FindRxDevice(0);
                if (v18 != 11 || v19 != 0xA5 || BikeManager::g_pBikeManager->m_mainBike->m_playerIdTx - 1000 <= 199000) {
                    if (!v22) {
                        v22 = new ANTDevice(v18, v17, v19);
                        switch (v18) {
                        case 1:
                            sprintf_s(g_ANT_NameId, "Di2 DFly %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "Di2 DFly");
                            break;
                        case 8:
                            sprintf_s(g_ANT_NameId, "CycleOps Bike %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "CycleOps Bike");
                            break;
                        case 11:
                            sprintf_s(g_ANT_NameId, (v19 == 0xA5) ? "WAHOO KICKR %d" : "Powermeter %d", v17);
                            strcpy_s(v22->m_nameIdBuf, (v19 == 0xA5) ? "WAHOO KICKR" : "Powermeter");
                            break;
                        case 17:
                            sprintf_s(g_ANT_NameId, "FE-C %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "FE-C");
                            break;
                        case 19:
                            sprintf_s(g_ANT_NameId, "Elite RealTrainer %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "Elite RealTrainer");
                            break;
                        case 24:
                            sprintf_s(g_ANT_NameId, "CycleOps %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "CycleOps");
                            break;
                        case 120:
                            sprintf_s(g_ANT_NameId, "HR Strap %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "HR Strap");
                            break;
                        case 121:
                            sprintf_s(g_ANT_NameId, "Speed/Cadence %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "Speed/Cadence");
                            break;
                        case 122:
                            sprintf_s(g_ANT_NameId, "Cadence %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "Cadence");
                            break;
                        case 123:
                            sprintf_s(g_ANT_NameId, "Speed %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "Speed");
                            break;
                        case 124:
                            sprintf_s(g_ANT_NameId, "Footpod %d", v17);
                            strcpy_s(v22->m_nameIdBuf, "Footpod");
                            break;
                        default:
                            strcpy_s(v22->m_nameIdBuf, "UnknownDevice");
                            strcpy_s(g_ANT_NameId, "Unknown device");
                            break;
                        }
                        FitnessDeviceManager::AddDevice(v22, g_ANT_NameId);
                        if (v18 == 1 && !FitnessDeviceManager::m_pSelectedDi2Device) {
                            v22->Pair(true);
                            FitnessDeviceManager::m_pSelectedDi2Device = v22;
                        }
                    }
                    v22->m_last_time_ms = timeGetTime();
                    if (g_antInfo[0].m_mfgNet == 1)
                        ANT_Decypher(g_ANT_RxBuffer[0][10], g_ANT_RxBuffer[0][11], &g_ANT_RxBuffer[0][1]);
                    v22->ProcessANTBroadcastData(g_ANT_RxBuffer[0]);
                }
            }
        } else if (ucEvent == 0x9A) {
            LogTyped(LOG_ANT, "Non flagged broadcast received");
        } else {
            LogTyped(LOG_ANT, "UNKNOWN Search channel data received");
        }
        return TRUE;
    default:
        LogTyped(LOG_ANT, "Unknown Channel(%d) Event %d", ucANTChannel, ucEvent);
        break;
    }
    if (!ucANTChannel)
        return TRUE;
    if (g_ANT_RxBuffer[ucANTChannel][14] == 32) {
        Log("RSSI %d", g_ANT_RxBuffer[ucANTChannel][15]);
        zassert(0);
    }
    auto v34 = FindRxDevice(ucANTChannel);
    if (!v34)
        return TRUE;
    v34->m_last_time_ms = timeGetTime();
    v34->ProcessANTEvent(ucEvent);
    return true;
}
void ANTRECEIVER_Connect() {
    //uint16_t v5[8];
    uint8_t v6[1024];
    if (!g_bANTConnected && g_bANTInitialized != g_bANTConnected) {
        for (g_ANTDeviceNum = 0; g_ANTDeviceNum < 4; g_ANTDeviceNum++) {
            g_bANTConnected = ANT_Init(g_ANTDeviceNum, 57600u, 0, 0) > 0;
            if (g_bANTConnected) {
                memset(g_ANTDongleProductName, 0, sizeof(g_ANTDongleProductName));
                ANT_GetDeviceUSBInfo(0, g_ANTDongleProductName, v6);
                //not used? ANT_GetDeviceUSBPID(v5);
                ANT_SetTransmitPower(0xFFu);
                ANT_AssignResponseFunction(ANT_Response, g_ANT_ResponseBuffer);
                for (int v2 = 0; v2 < _countof(g_antInfo); ++v2) {
                    g_antInfo[v2] = ANT_INFO();
                    ANT_AssignChannelEventFunction(v2, ANT_ChannelEvent, g_ANT_RxBuffer[v2]);
                }
                ANT_ResetSystem();
                g_ANT_ConnectTime = timeGetTime();
                for (int i = 0; i < _countof(g_ANT_NetStates); i++) {
                    LogTyped(LOG_ANT, "Clearing network key on network %d.", i);
                    g_ANT_NetStates[i] = 4;
                }
                ANT_RequestMessage(0, 0x61u);
                ANT_RequestMessage(0, 0xC6u);
                return;
            }
            ANT_Close();
        }
    }
}
const char *ANTRECEIVER_GetDongleIDString() {
    return (const char *)g_ANTDongleProductName;
}
bool g_ANT_LOAD;
HMODULE g_AntDLL;
bool ANT_Load() {
    if (g_ANT_LOAD)
        return true;
    g_AntDLL = LoadLibraryA("ANT_DLL.dll");
    if (!g_AntDLL)
        return false;
    ANT_Init = (fANT_Init)GetProcAddress(g_AntDLL, "ANT_Init");
    ANT_GetDeviceUSBInfo = (fANT_GetDeviceUSBInfo)GetProcAddress(g_AntDLL, "ANT_GetDeviceUSBInfo");
    ANT_GetDeviceUSBPID = (fANT_GetDeviceUSBPID)GetProcAddress(g_AntDLL, "ANT_GetDeviceUSBPID");
    //GetProcAddress(g_AntDLL, "ANT_GetDeviceUSBVID");
    ANT_Close = (fANT_Close)GetProcAddress(g_AntDLL, "ANT_Close");
    ANT_AssignResponseFunction = (fANT_AssignResponseFunction)GetProcAddress(g_AntDLL, "ANT_AssignResponseFunction");
    ANT_AssignChannelEventFunction = (fANT_AssignChannelEventFunction)GetProcAddress(g_AntDLL, "ANT_AssignChannelEventFunction");
    ANT_UnAssignChannel = (fANT_UnAssignChannel)GetProcAddress(g_AntDLL, "ANT_UnAssignChannel");
    ANT_AssignChannel = (fANT_AssignChannel)GetProcAddress(g_AntDLL, "ANT_AssignChannel");
    ANT_AssignChannelExt = (fANT_AssignChannelExt)GetProcAddress(g_AntDLL, "ANT_AssignChannelExt");
    ANT_SetChannelId = (fANT_SetChannelId)GetProcAddress(g_AntDLL, "ANT_SetChannelId");
    ANT_SetChannelPeriod = (fANT_SetChannelPeriod)GetProcAddress(g_AntDLL, "ANT_SetChannelPeriod");
    ANT_SetChannelSearchTimeout = (fANT_SetChannelSearchTimeout)GetProcAddress(g_AntDLL, "ANT_SetChannelSearchTimeout");
    ANT_SetChannelRFFreq = (fANT_SetChannelRFFreq)GetProcAddress(g_AntDLL, "ANT_SetChannelRFFreq");
    ANT_SetNetworkKey = (fANT_SetNetworkKey)GetProcAddress(g_AntDLL, "ANT_SetNetworkKey");
    ANT_SetTransmitPower = (fANT_SetTransmitPower)GetProcAddress(g_AntDLL, "ANT_SetTransmitPower");
    ANT_ResetSystem = (fANT_ResetSystem)GetProcAddress(g_AntDLL, "ANT_ResetSystem");
    ANT_OpenChannel = (fANT_OpenChannel)GetProcAddress(g_AntDLL, "ANT_OpenChannel");
    ANT_CloseChannel = (fANT_CloseChannel)GetProcAddress(g_AntDLL, "ANT_CloseChannel");
    ANT_RequestMessage = (fANT_RequestMessage)GetProcAddress(g_AntDLL, "ANT_RequestMessage");
    ANT_RxExtMesgsEnable = (fANT_RxExtMesgsEnable)GetProcAddress(g_AntDLL, "ANT_RxExtMesgsEnable");
    ANT_LibConfigCustom = (fANT_LibConfigCustom)GetProcAddress(g_AntDLL, "ANT_LibConfigCustom");
    ANT_SendBroadcastData = (fANT_SendBroadcastData)GetProcAddress(g_AntDLL, "ANT_SendBroadcastData");
    ANT_SendAcknowledgedData = (fANT_SendAcknowledgedData)GetProcAddress(g_AntDLL, "ANT_SendAcknowledgedData");
    ANT_SendBurstTransfer = (fANT_SendBurstTransfer)GetProcAddress(g_AntDLL, "ANT_SendBurstTransfer");
    ANT_AddChannelID = (fANT_AddChannelID)GetProcAddress(g_AntDLL, "ANT_AddChannelID");
    ANT_ConfigList = (fANT_ConfigList)GetProcAddress(g_AntDLL, "ANT_ConfigList");
    ANT_SetChannelTxPower = (fANT_SetChannelTxPower)GetProcAddress(g_AntDLL, "ANT_SetChannelTxPower");
    ANT_SetLowPriorityChannelSearchTimeout = (fANT_SetLowPriorityChannelSearchTimeout)GetProcAddress(g_AntDLL, "ANT_SetLowPriorityChannelSearchTimeout");
    ANT_SetSerialNumChannelId = (fANT_SetSerialNumChannelId)GetProcAddress(g_AntDLL, "ANT_SetSerialNumChannelId");
    ANT_EnableLED = (fANT_EnableLED)GetProcAddress(g_AntDLL, "ANT_EnableLED");
    ANT_CrystalEnable = (fANT_CrystalEnable)GetProcAddress(g_AntDLL, "ANT_CrystalEnable");
    ANT_ConfigFrequencyAgility = (fANT_ConfigFrequencyAgility)GetProcAddress(g_AntDLL, "ANT_ConfigFrequencyAgility");
    ANT_SetProximitySearch = (fANT_SetProximitySearch)GetProcAddress(g_AntDLL, "ANT_SetProximitySearch");
    ANT_OpenRxScanMode = (fANT_OpenRxScanMode)GetProcAddress(g_AntDLL, "ANT_OpenRxScanMode");
    ANT_SleepMessage = (fANT_SleepMessage)GetProcAddress(g_AntDLL, "ANT_SleepMessage");
    ANT_InitCWTestMode = (fANT_InitCWTestMode)GetProcAddress(g_AntDLL, "ANT_InitCWTestMode");
    ANT_SetCWTestMode = (fANT_SetCWTestMode)GetProcAddress(g_AntDLL, "ANT_SetCWTestMode");
    ANT_SendExtBroadcastData = (fANT_SendExtBroadcastData)GetProcAddress(g_AntDLL, "ANT_SendExtBroadcastData");
    ANT_SendExtAcknowledgedData = (fANT_SendExtAcknowledgedData)GetProcAddress(g_AntDLL, "ANT_SendExtAcknowledgedData");
    ANT_SendExtBurstTransferPacket = (fANT_SendExtBurstTransferPacket)GetProcAddress(g_AntDLL, "ANT_SendExtBurstTransferPacket");
    ANT_SendExtBurstTransfer = (fANT_SendExtBurstTransfer)GetProcAddress(g_AntDLL, "ANT_SendExtBurstTransfer");
    //GetProcAddress(g_AntDLL, "ANT_LibVersion");
    ANT_Nap = (fANT_Nap)GetProcAddress(g_AntDLL, "ANT_Nap");
    if (!ANT_Init || !ANT_Close || !ANT_AssignResponseFunction || !ANT_AssignChannelEventFunction || !ANT_UnAssignChannel || !ANT_AssignChannel
        || !ANT_AssignChannelExt || !ANT_SetChannelId || !ANT_SetChannelPeriod || !ANT_SetChannelSearchTimeout || !ANT_SetChannelRFFreq || !ANT_SetNetworkKey
        || !ANT_SetTransmitPower || !ANT_ResetSystem || !ANT_OpenChannel || !ANT_CloseChannel || !ANT_RequestMessage || !ANT_SendBroadcastData 
        || !ANT_SendAcknowledgedData || !ANT_SendBurstTransfer || !ANT_RxExtMesgsEnable || !ANT_AddChannelID || !ANT_ConfigList || !ANT_SetChannelTxPower
        || !ANT_SetLowPriorityChannelSearchTimeout || !ANT_SetSerialNumChannelId || !ANT_EnableLED || !ANT_CrystalEnable || !ANT_ConfigFrequencyAgility
        || !ANT_SetProximitySearch || !ANT_OpenRxScanMode || !ANT_SleepMessage || !ANT_InitCWTestMode|| !ANT_SetCWTestMode || !ANT_SendExtBroadcastData
        || !ANT_SendExtAcknowledgedData || !ANT_SendExtBurstTransferPacket || !ANT_SendExtBurstTransfer || !ANT_Nap)
        return false;
    g_ANT_LOAD = true;
    return true;
}
void ANTRECEIVER_Initialize() {
    g_bANTConnected = 0;
    g_ANTDeviceNum = 0;
    g_bANTInitialized = ANT_Load();
    g_ANT_Mutex = ZMUTEX_Create("ANT_MUTEX");
}
bool ANTRECEIVER_IsConnected() {
    return g_bANTConnected;
}
int ANTRECEIVER_PairDevice(uint8_t periodCode, uint16_t deviceID, uint32_t mfgNet) {
    for (int i = 1; i < _countof(g_antInfo); i++)
        if (g_antInfo[i].m_deviceID == deviceID && g_antInfo[i].m_periodCode == periodCode)
            return i;
    for (int i = 1; i < _countof(g_antInfo); i++)
        if (g_antInfo[i].m_deviceID == 0) {
            ZMUTEX_Lock(g_ANT_Mutex);
            g_antInfo[i].m_deviceID = deviceID;
            g_antInfo[i].m_periodCode = periodCode;
            g_antInfo[i].m_mfgNet = mfgNet;
            g_antInfo[i].m_antNet = g_ANT_ExtInfo[mfgNet].m_antNet;
            ANT_AssignChannel(i, 0, g_ANT_ExtInfo[mfgNet].m_antNet);
            ANT_SetChannelSearchTimeout(i, 0xFFu);
            ZMUTEX_Unlock(g_ANT_Mutex);
            LogTyped(LOG_ANT_IMPORTANT, "Pairing deviceID %d to channel %d, mfg network %d, ant network %d", deviceID, i, mfgNet, g_antInfo[i].m_antNet);
            return i;
        }
    return -1;
}
void ANTRECEIVER_PostConnect() {
    auto dt = timeGetTime() - g_ANT_ConnectTime;
    if (dt < 1000)
        ANT_Nap(1000 - dt);
    for (int i = 0; i < _countof(g_ANT_NetStates); i++) {
        if (g_ANT_NetStates[i] != i) {
            LogTyped(LOG_ANT, "Setting network key on network %d...", i);
            if (ANT_SetNetworkKey) {
                ANT_SetNetworkKey(i, g_ANT_ExtInfo[i].m_networkKey);
                ANT_Nap(500);
                g_ANT_NetStates[i] = i;
            } else {
                LogTyped(LOG_ANT, "ANT dll not found\n");
            }
        }
    }
    ANT_StopDeviceSearch();
}
void ANTRECEIVER_Shutdown() {
    if (g_bANTInitialized) {
        for (int i = 0; i < _countof(g_antInfo); i++) {
            if (g_antInfo[i].m_opened) {
                LogTyped(LOG_ANT, "SHUTDOWN: Closing ANT channel %d", i);
                if (ANT_CloseChannel) {
                    ANT_CloseChannel(i);
                    ANT_Nap(500);
                } else {
                    Log("ANT DLL MISSING");
                }
            }
        }
        if (g_bANTConnected && ANT_Close)
            ANT_Close();
        if (g_AntDLL) {
            FreeLibrary(g_AntDLL);
            g_AntDLL = nullptr;
        }
        g_ANT_LOAD = g_bANTInitialized = false;
    }
}
float g_ANT_AccTimeNoConn = 3.0f, g_USBDevicePollTime = 500.0f;
BOOL g_deviceUSBInfo = TRUE;
bool g_ANT_KillingGarmin;
GUI_Obj *g_ANT_KillDlg;
void RunZwiftHelper(MessageBoxResults r) {
    if (r == MBR_BUTTON0 && ::ShellExecuteA(nullptr, "runas", "ZwiftHelper.exe", nullptr, nullptr, 1) == (HINSTANCE)32)
        Log("zwifthelper: %d\n", 32);
    FreeAndNil(g_ANT_KillDlg);
}
void ANTRECEIVER_Update(float dt) {
    PROCESSENTRY32W pe;
    if (g_bANTConnected) {
        if (g_ANT_KillDlg) {
            Log("ANT DONGLE HAS BEEN CONNECTED");
            FreeAndNil(g_ANT_KillDlg);
            g_ANT_KillingGarmin = false;
        }
    } else {
        g_ANT_AccTimeNoConn += dt;
        if (!g_ANT_KillDlg && !g_ANT_KillingGarmin) {
            Log("ANT DONGLE NOT CONNECTED");
            pe.dwSize = sizeof(pe);
            auto ts = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            bool found = false;
            if (ts != INVALID_HANDLE_VALUE && Process32FirstW(ts, &pe) && Process32NextW(ts, &pe)) {
                found = true;
                while (_wcsicmp(pe.szExeFile, L"GarminService.exe")
                    && _wcsicmp(pe.szExeFile, L"expresstray.exe")
                    && _wcsicmp(pe.szExeFile, L"express.exe")) {
                    if (!Process32NextW(ts, &pe))
                        found = false;
                }
                if (found) {
                    bool wide = (LOC_GetLanguageIndex() & 0xFFFFFFFB) != 0;
                    g_ANT_KillDlg = GUI_CreateMessageBox(GetText("LOC_ERROR_NOANTDONGLE"), GetText("LOC_ERROR_NOANTDONGLE_GARMINEXPRESSKILL"),
                        GetText("LOC_YES"), GetText("LOC_NO"), RunZwiftHelper, wide ? 525.0f : 325.0f, wide ? 325.0f : 150.0f, false, 200.0f);
                }
            }
            if (!found)
                g_USBDevicePollTime = 5.0f;
            g_ANT_KillingGarmin = true;
        }
        EventSystem::GetInst()->TriggerEvent(EVENT_ID_5, 1, 5);
        if (g_ANT_AccTimeNoConn >= 3.0f) {
            g_ANT_AccTimeNoConn = 0.0f;
            ANTRECEIVER_Connect();
            ANTRECEIVER_PostConnect();
        }
    }
    if (g_bANTInitialized) {
        static float g_ANT_AccTimeInit = g_USBDevicePollTime;
        g_ANT_AccTimeInit += dt;
        if (g_ANT_AccTimeInit > g_USBDevicePollTime) {
            g_ANT_AccTimeInit = 0.0f;
            if (g_ANTDeviceNum >= 0) {
                memset(g_ANTDongleProductName, 0, sizeof(g_ANTDongleProductName));
                g_deviceUSBInfo = ANT_GetDeviceUSBInfo(g_ANTDeviceNum, g_ANTDongleProductName, (uint8_t *)&pe);
                if (!g_deviceUSBInfo) {
                    g_bANTConnected = false;
                    g_ANTDeviceNum = 0xFF;
                    memset(g_ANTDongleProductName, 0, sizeof(g_ANTDongleProductName));
                    ANT_Close();
                }
            }
        }
    }
}
void ANT_DeviceSearch(int mfgNet) {
    static_assert(sizeof(ANT_INFO) == 12);
    static_assert(sizeof(ANT_EXT_INFO) == 16);
    if (g_bANTConnected) {
        ZMUTEX_Lock(g_ANT_Mutex);
        if (g_antInfo[0].m_opened) {
            if (g_antInfo[0].m_mfgNet != mfgNet)
                ANT_StopDeviceSearch();
            if (g_antInfo[0].m_opened) {
                ZMUTEX_Unlock(g_ANT_Mutex);
                return;
            }
        }
        if (!ANT_AssignChannelExt) {
            Log("ANT DLL MISSING");
        } else {
            auto ei = &g_ANT_ExtInfo[mfgNet];
            g_antInfo[0].m_opened = ANT_AssignChannelExt(0, 0x40u, ei->m_antNet, 1) != 0;
            if (g_antInfo[0].m_opened) {
                LogTyped(LOG_ANT, "Starting ANT search (mfg network %d)", mfgNet);
                g_antInfo[0].m_antNet = ei->m_antNet;
                g_antInfo[0].m_mfgNet = mfgNet;
            } else {
                LogTyped(LOG_ANT, "Could not assign channel %d to network %d", 0, ei->m_antNet);
            }
            ANT_SetLowPriorityChannelSearchTimeout(0, 0);
            ANT_SetChannelSearchTimeout(0, 0xFF);
            g_antInfo[0].m_mfgNet = mfgNet;
        }
        ZMUTEX_Unlock(g_ANT_Mutex);
    }
}
bool ANT_IsInitialized() {
    return g_bANTInitialized;
}
bool ANT_IsSearchEnabled() {
    return UI_IsDialogOfTypeOpen(UID_DROP_IN);
}
void ANT_StopDeviceSearch() {
    if (g_antInfo[0].m_opened) {
        LogTyped(LOG_ANT, "Stopping ANT search (mfg network %d)", g_antInfo[0].m_mfgNet);
        if (ANT_CloseChannel)
            ANT_CloseChannel(0);
        else
            Log("ANT DLL MISSING");
    }
}
