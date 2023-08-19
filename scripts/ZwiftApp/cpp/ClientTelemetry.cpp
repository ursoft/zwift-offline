//#include "ZwiftApp.h" //OMITted
struct Experimentation;
struct EventSystem;
#include "ClientTelemetry.h"
ClientTelemetry::ClientTelemetry(Experimentation *exp, EventSystem *eventSystem) /* : EventObject(eventSystem)*/ {
        //OMIT
        //eventSystem->Subscribe(EV_28, this);
        //eventSystem->Subscribe(EV_RESET, this);
        //exp->IsEnabled(FID_RLOG, [this](ExpVariant val) {
            //OMIT doClientTelemetryFID_RLOG
        //});
}
ClientTelemetry::~ClientTelemetry() {
    //m_eventSystem->Unsubscribe(EV_28, this);
    //m_eventSystem->Unsubscribe(EV_RESET, this);
}
void ClientTelemetry::Initialize(Experimentation *exp, EventSystem *eventSystem) {
    g_sClientTelemetryUPtr.reset(new ClientTelemetry(exp, eventSystem));
}
/*void ClientTelemetry::HandleEvent(EVENT_ID e, va_list va) {
    switch (e) {
    case EV_28:
        //OMIT
        break;
    case EV_RESET:
        //OMIT
        break;
    //OMIT EV_28|0x2
    }
}*/