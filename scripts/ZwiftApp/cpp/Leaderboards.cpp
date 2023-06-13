#include "ZwiftApp.h"
void Leaderboards::SaveUserSegmentPR(int64_t playerId, int64_t segmentId, double wtSec, float elapsedSec, float avg_power, bool isMale, protobuf::PowerType powTy, uint32_t weightGrams, const std::string &firstName, const std::string &lastName, uint64_t eventId) {
    //TODO
}
void Leaderboards::FetchJerseyLeadersForAllSegments() {
    ZNet::FetchSegmentJerseyLeaders([](const protobuf::SegmentResults &proto) {
        for (auto &i : proto.segment_results()) {
            g_Leaderboards.SaveUserSegmentPR(
                i.player_id(),
                i.segment_id(),
                i.world_time() * 0.001,
                i.elapsed_ms() * 0.001f,
                i.avg_power(),
                !i.has_is_male() || i.is_male(),
                i.power_source_model(),
                i.weight_in_grams(),
                i.first_name(),
                i.last_name(),
                i.event_subgroup_id());
        }
    }, [](ZNet::Error e) {
        Log("Leaderboards fetch all jeserey leaders ERROR: %s", e.m_msg.data());
    });
}
std::unordered_map<int64_t, std::list<protobuf::SegmentResult>::iterator> m_map;
void Leaderboards::UserLeftWorld(int64_t playerId) {
    for (auto &i : m_srwList) {
        if (i.m_srList2.size()) {
            auto f = m_map.find(playerId);
            if (f != m_map.end()) {
                ZML_SendRemoveSegmentLeaderboardResultLive(*f->second);
                m_map.erase(f);
                i.m_srList2.erase(f->second);
            }
        }
    }
}
void Leaderboards::FlagSandbagger(int64_t id, double, float) {
    //TODO
}
void Leaderboards::FlagCheater(int64_t id, double, float) {
    //TODO
}
void Leaderboards::SetPlayerSegmentResult(protobuf::SegmentResult *sr, int64_t playerIdTx, int64_t segmentId, double wtSec, float durSec, float avgPower, bool isMale, protobuf::PowerType pty,
    uint32_t grams, const char *, const char *, uint64_t eventId, float avgHr, int64_t actId) {
    //TODO
}
