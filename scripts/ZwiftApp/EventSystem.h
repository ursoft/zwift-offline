#pragma once
enum EVENT_ID {
	EV_SLIPPING_ON = 0x0,
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
}; 
typedef void (*EventCallback)(EVENT_ID, va_list);

class EventObject {
public:
	EventCallback m_callback;
};

class EventSystem { //136 bytes
	static EventSystem *g_eventSystem;
	std::unordered_map<EVENT_ID, std::vector<EventCallback> > m_fsubs;
	std::unordered_map<EVENT_ID, std::vector<EventObject *> > m_osubs;
public:
	EventSystem();
	static EventSystem *GetInst();
	void TriggerEvent(EVENT_ID e, int, ...);
	void Subscribe(EVENT_ID e, EventObject *ptr);
	void Subscribe(EVENT_ID e, EventCallback cb);
	void Unsubscribe(EVENT_ID e, EventObject *ptr);
	void Unsubscribe(EVENT_ID e, EventCallback cb);
	void Destroy();
	~EventSystem();
};
