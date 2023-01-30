#include "ZwiftApp.h"
ConnectionManager::ConnectionManager() {
    //TODO
}
ConnectionManager::~ConnectionManager() {
    //TODO
}
void ConnectionManager::DestroyInstance() {
    g_ConnectionManagerUPtr.reset();
}
void ConnectionManager::Unsubscribe(SaveActivityService *serv) {
    //TODO
}