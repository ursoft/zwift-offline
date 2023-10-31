#include "ZwiftApp.h"
BikeManager::BikeManager() {
    //TODO
    m_mainBike = new BikeEntity(nullptr, false, true); //delme
    m_mainBike->m_entityType = Entity::ET_BIKE; //delme
}
void BikeManager::Initialize(Experimentation *exp) {
    //TODO
}
BikeEntity *BikeManager::FindBikeWithNetworkID(int64_t id, bool a3) {
    if (!id || id == m_mainBike->m_playerIdTx)
        return m_mainBike;
    for (auto i : m_allBikes)
        if (i->m_playerIdTx == id)
            return i;
    if (a3)
        for (auto i : m_field_9CF0)
            if (i->m_playerIdTx == id)
                return i;
    for (auto i : m_field_9CD8)
        if (i->m_playerIdTx == id)
            return i;
    return nullptr;
}
void BikeManager::ProcessPlayerState(zwiftUpdateContext *, const protobuf::PlayerState &pst) {
    //TODO
}
BikeManager::~BikeManager() {
    delete m_mainBike; //TODO, temporary
}