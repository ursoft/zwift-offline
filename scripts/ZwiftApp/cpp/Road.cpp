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