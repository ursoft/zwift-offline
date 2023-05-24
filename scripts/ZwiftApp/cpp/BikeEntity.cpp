#include "ZwiftApp.h"
BikeEntity::BikeEntity() {
    //TODO
    m_bc = new VirtualBikeComputer(); //delme
}
void BikeEntity::SaveProfile(bool, bool) {
    //TODO
}
int64_t g_GroupEventsActive_CurrentEventId;
int64_t BikeEntity::GetEventID() {
    if (m_field_C98) {
        if (g_currentPrivateEvent)
            return g_currentPrivateEvent->m_eventId;
        return g_GroupEventsActive_CurrentEventId;
    }
    return m_curEventId;
}
