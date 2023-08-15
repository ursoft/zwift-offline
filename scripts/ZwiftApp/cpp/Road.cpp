#include "ZwiftApp.h"
#include "Road.h"
void RouteManager::FindRoutesByRoadInfo(int segmentId, double not_used, bool, std::vector<Route *> *ret) {
    //TODO
}
RoadSegment *RoadManager::GetRoadSegment(int32_t segmId) {
    if (segmId < 0 || segmId >= m_segments.size())
        return nullptr;
    return m_segments[segmId];
}
int RoadManager::GetRoadCount() {
    //TODO
    return 0;
}
void RoadManager::FindClosestPointOnRoad(const VEC3 &, int32_t, int32_t *, uint64_t, bool, int32_t/*, bool, ERoadMeasureLocation*/) {
    //TODO
}
VEC3 RoadSegment::CalculateRoadPositionAtTime(double, bool) {
    //TODO
    return VEC3();
}
VEC3 RoadSegment::CalculateRoadPositionAtTimeF(float, bool) {
    //TODO
    return VEC3();
}
float RoadSegment::GetRoadWidth(double, bool, float *, float *, const RoadMarkerEntity **) {
    //TODO
    return 0.0f;
}
float RoadSegment::GetRiderBoundsRatio() {
    //TODO
    return 0.0f;
}
void RouteComputer::FlipRoute(BikeEntity *be) {
    //TODO
}
VEC3 RoadSegment::GetMaxExtents() {
    //TODO
    return VEC3{};
}
VEC3 RoadSegment::GetMinExtents() {
    //TODO
    return VEC3{};
}
void RoadManager::Load(XMLDoc *xroad, XMLDoc *xroadStyle) {
    //TODO
}
void RoadManager::Update(float dt) { //inlined into GameWorld::Update
    //TODO 
}
void RoadManager::Render() {
    //TODO
}
void RoadManager::Destroy() { //inlined into onGameWorldDtr
    //TODO
}
void RoadManager::ConstructNearbyRiderGroupsEmpty() { //TODO
}
void RoadManager::ConstructNearbyRiderGroupsLondon() { //TODO
}
void RoadManager::ConstructNearbyRiderGroupsNewYork() { //TODO
}
void RoadManager::ConstructNearbyRiderGroupsRichmond() { //TODO
}
void RoadManager::ConstructNearbyRiderGroupsWatopia() { //TODO
}
