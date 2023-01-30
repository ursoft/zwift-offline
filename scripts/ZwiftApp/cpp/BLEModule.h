#pragma once

class BLEModule : public EventObject {
public:
	BLEModule(Experimentation *exp);
	static void Initialize(Experimentation *exp);
	void HandleEvent(EVENT_ID, va_list) override;
	inline static std::unique_ptr<BLEModule> g_sBLEModule;
};
