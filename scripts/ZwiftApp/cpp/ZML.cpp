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
bool g_bSkipThisWorkoutSection;
enum ZML_ButtonActions {
    ZBA_0, ZBA_1, ZBA_2, ZBA_3, ZBA_RESUME_WKO, ZBA_PAUSE_WKO
};
void INTERNAL_ZML_ActivateButton(int, ZML_ButtonActions) {
    //TODO
}
void IncreaseWorkoutIntensityClicked() {
    //TODO
}
void DecreaseWorkoutIntensityClicked() {
    //TODO
}
void ZML_ReceivedWorkoutAction(protobuf::WorkoutActionRequest_Type wart) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    switch (wart) {
    case protobuf::PAUSE:
        if (mainBike)
            mainBike->m_bc->m_workoutPaused = true;
        Log("PAUSE WORKOUT REQUEST");
        INTERNAL_ZML_ActivateButton(0, ZBA_PAUSE_WKO);
        break;
    case protobuf::RESUME:
        if (mainBike)
            mainBike->m_bc->m_workoutPaused = false;
        Log("RESUME WORKOUT REQUEST");
        INTERNAL_ZML_ActivateButton(0, ZBA_RESUME_WKO);
        break;
    case protobuf::SKIP:
        g_bSkipThisWorkoutSection = true;
        Log("CANCEL WORKOUT SECTION REQUEST");
        break;
    case protobuf::FTP_BIAS_INCREASE:
        IncreaseWorkoutIntensityClicked();
        break;
    case protobuf::FTP_BIAS_DECREASE:
        DecreaseWorkoutIntensityClicked();
        break;
    case protobuf::HARDER:
        GAME_IncreaseFlatRoadTrainerResistance();
        break;
    case protobuf::EASIER:
        GAME_DecreaseFlatRoadTrainerResistance();
        break;
    case protobuf::TOGGLE_ERG_MODE:
        GAME_SetUseErgModeInWorkouts(mainBike && mainBike->m_bc->m_sport != protobuf::CYCLING ? true : !g_UseErgModeInWorkouts);
        break;
    default:
        return;
    }
}
void SelectBranch(uint32_t a1, bool a2, bool a3_notused, bool a4, bool a5) {
    //TODO
}
void CreateCenterYourBarsPopup(float, void (*f)(/*UI_TwoButtonsDialog::DIALOG_RESULTS*/)) {
    //TODO
}
void ZML_HandleCustomButton(uint32_t but) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (mainBike) {
        switch (but) {
        case 1000:
            mainBike->m_bc->m_workoutPaused = true;
            Log("PAUSE WORKOUT REQUEST");
            INTERNAL_ZML_ActivateButton(0, ZBA_PAUSE_WKO);
            break;
        case 1001:
            mainBike->m_bc->m_workoutPaused = false;
            Log("RESUME WORKOUT REQUEST");
            INTERNAL_ZML_ActivateButton(0, ZBA_RESUME_WKO);
            break;
        case 1002:
            g_bSkipThisWorkoutSection = true;
            Log("CANCEL WORKOUT SECTION REQUEST");
            break;
        case 1003:
            GAME_IncreaseFlatRoadTrainerResistance();
            break;
        case 1004:
            GAME_DecreaseFlatRoadTrainerResistance();
            break;
        case 1005:
            IncreaseWorkoutIntensityClicked();
            break;
        case 1006:
            DecreaseWorkoutIntensityClicked();
            break;
        case 1010:
            SelectBranch(263, true, false, false, true);
            break;
        case 1011:
            SelectBranch(265, true, false, false, true);
            break;
        case 1012:
            SelectBranch(262, true, false, false, true);
            break;
        case 1020:
            if (mainBike && mainBike->m_eboost && /* TODO mainBike->m_eboost->field_8 && *mainBike->m_eboost->field_8 &&*/ (mainBike->m_eboost->m_stateBits & 2) == 0)
                mainBike->m_eboost->ActivateBoost(mainBike);
            break;
        case 1021:
            if (mainBike && mainBike->m_eboost && /* TODO mainBike->m_eboost->field_8 && *mainBike->m_eboost->field_8 &&*/ (mainBike->m_eboost->m_stateBits & 1) == 0)
                mainBike->m_eboost->ActivateCharge(mainBike);
            break;
        case 1022:
        case 1023:
            return;
        case 1030:
        case 1033:
        case 1034:
        case 1035:
        case 1036:
        case 1037:
        case 1038:
            mainBike->ClearPowerups();
            break;
        case 1050:
            CreateCenterYourBarsPopup(but, FitnessDeviceManager::SetInitialRotationZC);
            break;
        case 1060:
            g_ShowGraph = !g_ShowGraph;
            g_UserConfigDoc.SetBool("ZWIFT\\CONFIG\\SHOW_GRAPH", g_ShowGraph, true);
            break;
        case 1070:
            if (mainBike && mainBike->m_grFenceComponent)
                mainBike->m_grFenceComponent->ToggleFenceLeaderUI();
            break;
        case 1080:
            HUDToggle::ShowHUD(true, false);
            break;
        case 1081:
            HUDToggle::ShowHUD(false, false);
            break;
        default:
            zassert(!but);
            break;
        }
    }
}
void ZML_ReceivedClientAction(const protobuf::ClientAction &) {
    //TODO
}
void ZML_DetonateRideOnBomb() {
    //TODO
}