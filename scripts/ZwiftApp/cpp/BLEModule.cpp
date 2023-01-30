#include "ZwiftApp.h"
#include "BLEModule.h"

void BLEModule::Initialize(Experimentation *exp) {
    g_BLEModule.reset(new BLEModule(exp));
}

void BLEModule::HandleEvent(EVENT_ID, va_list) { /*TODO*/ }

BLEModule::BLEModule(Experimentation *exp) : EventObject(exp->m_eventSystem) {
    //TODO
    //exp->IsEnabled(FID_LOG_BLE, [](bool) {});
}

void BLEModule::Shutdown() {
    g_BLEModule->StopScan();
    g_BLEModule.reset();
}

void BLEModule::StopScan() {
    //TODO
}