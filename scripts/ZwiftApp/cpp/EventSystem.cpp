//#include "ZwiftApp.h" //READY for testing
#include "EventSystem.h"
EventSystem::EventSystem() {}
EventSystem::~EventSystem() {}
EventSystem *EventSystem::GetInst() {
    if (!g_eventSystem)
        g_eventSystem = new EventSystem();
    return g_eventSystem;
}
void EventSystem::TriggerEvent(EVENT_ID e, int arg, ...) {
    va_list va;
    va_start(va, arg);
    if (g_eventSystem) {
        if(auto si = m_fsubs.find(e); si != m_fsubs.end()) for (auto eo : si->second) {
            eo(e, va);
        }
        if (auto si = m_osubs.find(e); si != m_osubs.end()) for (auto eo : si->second) {
            eo->HandleEvent(e, va);
        }
    }
}
void EventSystem::Subscribe(EVENT_ID e, EventObject *ptr) { m_osubs[e].push_back(ptr); }
void EventSystem::Subscribe(EVENT_ID e, EventCallback cb) { m_fsubs[e].push_back(cb); }
void EventSystem::Unsubscribe(EVENT_ID e, EventObject *ptr) {
    if (auto si = m_osubs.find(e); si != m_osubs.end())
        si->second.erase(std::remove(si->second.begin(), si->second.end(), ptr), si->second.end());
}
void EventSystem::Unsubscribe(EVENT_ID e, EventCallback cb) {
    if (auto si = m_fsubs.find(e); si != m_fsubs.end())
        si->second.erase(std::remove(si->second.begin(), si->second.end(), cb), si->second.end());
}
void EventSystem::Destroy() { 
    auto es = g_eventSystem;
    es->m_fsubs.clear();
    es->m_osubs.clear();
    g_eventSystem = nullptr;
    delete es;
}
