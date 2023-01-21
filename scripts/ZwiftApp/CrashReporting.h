#pragma once

class EventSystem;
enum EVENT_ID;

class CrashReporting { //432 bytes, not implemented
public:
	CrashReporting(EventSystem *ptr);
	void HandleEvent(EVENT_ID e, va_list va);
	~CrashReporting();
	static void Initialize(EventSystem *ptr);
};

extern std::unique_ptr<CrashReporting> g_sCrashReportingUPtr;
