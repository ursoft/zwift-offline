#include "ZwiftApp.h"
#include "BLEModule.h"

std::unique_ptr<BLEModule> sBLEModule;

void BLEModule::Initialize(Experimentation *exp) {
	sBLEModule.reset(new BLEModule(exp));
}

void BLEModule::HandleEvent(EVENT_ID, va_list) { /*TODO*/ }

BLEModule::BLEModule(Experimentation *exp) {
	//TODO
	//exp->IsEnabled(FID_LOG_BLE, [](bool) {});
}
