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
void ConnectionManager::Initialize() {
    g_ConnectionManagerUPtr.reset(new ConnectionManager());
}
void ConnectionManager::Subscribe(SaveActivityService *serv) {
    //TODO
}
ConnectionManager *ConnectionManager::Instance() { zassert(IsInitialized()); return g_ConnectionManagerUPtr.get(); }
bool ConnectionManager::IsInitialized() { return g_ConnectionManagerUPtr.get() != nullptr; }
