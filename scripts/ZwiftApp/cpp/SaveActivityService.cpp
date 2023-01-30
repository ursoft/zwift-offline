#include "ZwiftApp.h"
SaveActivityService::SaveActivityService() {
    //TODO
}
void SaveActivityService::Shutdown() {
    ConnectionManager::Instance()->Unsubscribe(g_SaveActivityServicePtr.get());
    g_SaveActivityServicePtr.reset();
}
