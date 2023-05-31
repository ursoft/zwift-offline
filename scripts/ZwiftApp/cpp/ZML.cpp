#include "ZwiftApp.h"
#include "ZML.h"
void ZML_SendGameSessionInfo() {
    //TODO
}
void ZML_StartAuxPairing() {
    //TODO
}
void ZML_SendAuxPairingSelection(bool, protobuf::SportsDataSample_Type) {
    //TODO
}
void ZML_EndAuxPairing() {
    //TODO
}
void ZML_SendSegmentLeaderboardResult(protobuf::LeaderboardType ty, const protobuf::SegmentResult &) {
    //TODO
}
bool HasPairedToZML() {
    //TODO
    return true;
}
void ZML_SendRemoveSegmentLeaderboardResultLive(const protobuf::SegmentResult &sr) {
    if (zwift_network::is_paired_to_phone()) {
        protobuf::GamePacket v11;
        auto res = v11.mutable_segment_leaderboard_res();
        v11.set_type(protobuf::SEGMENT_RESULT_REMOVE);
        res->set_f1(sr.id());
        res->set_f5(sr.segment_id());
        res->set_f3(sr.player_id());
        res->set_f2(0);
        zwift_network::send_game_packet(v11.SerializeAsString(), false);
    }
}
void ZML_RequestEffectPlay(protobuf::EffectRequest_Effect ef, bool a2, bool a3) {
    protobuf::GamePacket v11;
    v11.set_type(protobuf::EFFECT_REQUEST);
    auto v6 = v11.mutable_effect_request();
    v6->set_eff(ef);
    v6->set_f2(a2);
    v6->set_f3(a3);
    zwift_network::send_game_packet(v11.SerializeAsString(), true);
}
