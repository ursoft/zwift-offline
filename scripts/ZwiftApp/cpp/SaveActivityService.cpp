#include "ZwiftApp.h"
SaveActivityService::SaveActivityService(XMLDoc *doc, Experimentation *exp, EventSystem *ev) {
    //TODO
}
void SaveActivityService::Shutdown() {
    ConnectionManager::GetInstance()->Unsubscribe(g_SaveActivityServicePtr.get());
    g_SaveActivityServicePtr.reset();
}
void SaveActivityService::Initialize(XMLDoc *doc, Experimentation *exp, EventSystem *ev) {
    g_SaveActivityServicePtr.reset(new SaveActivityService(doc, exp, ev));
    g_ConnectionManagerUPtr->Subscribe(g_SaveActivityServicePtr.get());
}
void SaveActivityService::HandleNotification(ConnectionType, NotificationType, NotificationDisplayType) {
    //TODO
}
