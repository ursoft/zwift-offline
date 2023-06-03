#pragma once
enum EVENT_ID {
    EV_SLIPPING_ON = 0x0,
    EV_FLIER = 0x1,
    EV_SANDBAGGER = 0x2,
    EV_SLIPPING_OFF = 0x3,
    EV_SENS_DATA = 0x4,
    EVENT_ID_5 = 0x5,
    EV_BC_PROMPT = 0x6,
    EV_ROUTE_COMPLETE = 0x7,
    EV_SESSION_END = 0x8,
    EV_9 = 0x9,
    EV_PHONE_1A = 0xA,
    EV_WRK_CH = 0xB,
    EV_WRK_ABANDON = 0xC,
    EV_FINISH_GE = 0xD,
    EV_QUIT_GE = 0xE,
    EV_16 = 0x10,
    EV_LOGGEDOUT = 0x11,
    EV_SESSION_START = 0x12,
    EV_PROFILE_LD_END = 0x13,
    EV_SPORT_CH = 0x14,
    EV_FTP_CH = 0x15,
    EV_PROFILE_CH = 0x16,
    EV_FRAME_CH = 0x17,
    EV_RESET = 0x18,
    EV_ACTIVITY_CH = 0x19,
    EV_SENS_DISCONN = 0x1A,
    EV_SENS_RECONN = 0x1B,
    EV_ROUTE_CH = 0x1C,
    EV_1d = 0x1D,
    EV_1e = 0x1E,
    EV_1f = 0x1F,
    EV_20 = 0x20,
    EV_21 = 0x21,
    EV_22 = 0x22,
    EV_23 = 0x23,
    EV_24 = 0x24,
    EV_25 = 0x25,
    EV_26 = 0x26,
    EV_27 = 0x27,
    EV_28 = 0x28,
    EV_29 = 0x29,
    EV_2A = 0x2A,
    EV_2B = 0x2B,
    EV_2C = 0x2C,
    EV_2D = 0x2D,
    EV_2E = 0x2E,
    EV_2F = 0x2F,
    EV_30 = 0x30,
    EV_31 = 0x31,
    EV_32 = 0x32,
    EV_33 = 0x33,
};
typedef void (*EventCallback)(EVENT_ID, va_list);
struct EventSystem;
struct EventObject {
    EventSystem *m_eventSystem;
    EventObject(EventSystem *e) : m_eventSystem(e) {}
    virtual void HandleEvent(EVENT_ID, va_list) = 0;
    virtual ~EventObject() {}
};
struct EventSystem { //136 bytes
    static inline EventSystem *g_eventSystem;
    std::unordered_map<EVENT_ID, std::vector<EventCallback> > m_fsubs;
    std::unordered_map<EVENT_ID, std::vector<EventObject *> > m_osubs;
    EventSystem();
    static EventSystem *GetInst();
    void TriggerEvent(EVENT_ID e, int, ...);
    void Subscribe(EVENT_ID e, EventObject *ptr);
    void Subscribe(EVENT_ID e, EventCallback cb);
    void Unsubscribe(EVENT_ID e, EventObject *ptr);
    void Unsubscribe(EVENT_ID e, EventCallback cb);
    static void Destroy();
    static bool IsInitialized() { return g_eventSystem != nullptr; }
    ~EventSystem();
};
