#include "ZwiftApp.h"
#include "Experimentation.h"
std::unique_ptr<Experimentation> g_sExperimentationUPtr;
ZNetAdapter g_znetAdapter;
void Experimentation::Initialize(EventSystem *ev) {
	assert(g_sExperimentationUPtr.get() == nullptr);
	g_sExperimentationUPtr.reset(new Experimentation(&g_znetAdapter, ev));
	assert(g_sExperimentationUPtr.get() != nullptr);
}
const FeatureMetadata g_featureMetadata[FID_CNT] = { //FillFeatureMetadata
    { FID_PPDATA,  "game_1_31_pp_data", 0, 0, 6, 6, 7 },
    { FID_PPDATA,  "game_1_31_rlog", 0, 0, 6, 6, 9 },
    { FID_FQUERY,  "game_1_31_fquery", 0, 0, 6, 6, 7 },
    { FID_VID_CAP, "game_VideoCapture", 0, 0, 7, 8, 9 },
    { FID_SERV_PC, "game_1_32_server_power_curves", 0, 1, 7, 7, 9 },
    { FID_HIDE_HW, "game_1_28_enable_workout_hud_hiding", 0, 0, 1, 2, 7 },
    { FID_HIDE_HD, "game_1_28_display_pr_hud_in_dynamic_workouts", 0, 0, 2, 2, 7 },
    { FID_FIXLEAD, "game_1_28_fix_leaderboard_hud_starting_display_road_time", 0, 0, 2, 2, 7 },
    { FID_REND_P,  "game_1_28_render_profile_screen_drop_down_box_title_text", 0, 0, 2, 2, 7 },
    { FID_AVOIDAB, "game_1_28_avoid_aborting_current_timing_sector_when_minimizing", 0, 0, 2, 2, 7 },
    { FID_INTEGRA, "game_1_29_integrate_road_and_bike_type_into_braking_performance", 0, 0, 3, 3, 8 },
    { FID_USERELA, "game_1_29_use_relay_spline_time_in_meetup_position_late_joiner", 0, 0, 3, 3, 8 },
    { FID_IGNOREP, "game_1_29_ignore_private_meetups_in_group_event_processing_logic", 0, 0, 3, 3, 8 },
    { FID_CLEARPO, "game_1_30_clear_powerups_on_event_start", 0, 0, 5, 5, 9 },
    { FID_ENABLEA, "game_1_32_enable_autobraking_in_watopia", 0, 0, 7, 7, 9 },
    { FID_WNS,     "game_1_31_workout_network_service", 0, 1, 6, 6, 9 },
    { FID_ENABLEJ, "game_1_31_enable_join_zwifter", 0, 0, 6, 6, 9 },
    { FID_MRB_V2,  "game_1_32_meetups_rubberbanding_v2", 0, 0, 7, 7, 9 },
    { FID_NINETYD, "game_1_26_ninety_day_prs" },
    { FID_DRAFTLO, "game_1_14_draftlock_fix" },
    { FID_PPV,     "xplatform_partner_connection_vitality" },
    { FID_ASSERTD, "game_1_15_assert_disable_abort" },
    { FID_NOESISE, "game_1_17_noesis_enabled" },
    { FID_SETTING, "game_1_14_settings_refactor" },
    { FID_HOMESCR, "game_1_20_home_screen" },
    { FID_PERFANA, "game_1_21_perf_analytics" },
    { FID_FORYOUC, "game_1_30_for_you_carousel" },
    { FID_SYSTEMA, "game_1_19_system_alerts" },
    { FID_BLEALT,  "game_1_16_2_ble_alternate_unpair_all_paired_devices" },
    { FID_TDFFEM,  "game_1_17_1_tdf_femmes_yellow_jersey" },
    { FID_BLEDISA, "game_1_17_ble_disable_component_sport_filter" },
    { FID_NEWWELC, "game_1_18_new_welcome_ride" },
    { FID_HOLIDAY, "game_1_18_holiday_mode" },
    { FID_PACKDY,  "game_1_18_0_pack_dynamics_2_5_collision_push_back_removal" },
    { FID_OSXMON,  "game_1_18_0_osx_monterey_bluetooth_uart_fix" },
    { FID_DEFAUL,  "game_1_19_0_default_rubberbanding" },
    { FID_USETABB, "game_1_19_use_tabbed_settings" },
    { FID_HWEXPER, "game_1_20_hw_experiment_1" },
    { FID_PAIREDD, "game_1_19_paired_devices_alerts" },
    { FID_REALTIM, "game_1_19_real_time_unlocks" },
    { FID_BLEDATA, "game_1_21_ble_data_guard_v2" },
    { FID_MIX,     "game_1_20_disable_high_volume_send_mixpanel" },
    { FID_ADDDEVI, "game_1_21_add_device_jet_black_swb" },
    { FID_CLICKAB, "game_1_20_clickable_telemetry_box" },
    { FID_ENABLE,  "game_1_20_0_enable_stages_steering" },
    { FID_HUDHIG,  "game_1_21_0_hud_highlighter" },
    { FID_GPUDEP,  "game_1_21_0_gpu_deprecation_warning_message" },
    { FID_FTMS,    "game_1_21_ftms_set_rider_weight" },
    { FID_BLEDLLV, "game_1_27_ble_dll_v2" },
    { FID_ALLOWUT, "game_1_22_allow_uturns_at_low_speed" },
    { FID_FTMSSP,  "game_1_21_0_ftms_sport_filter" },
    { FID_FTMSBIK, "game_1_21_ftms_bike_trainer_v3" },
    { FID_LOG_BLE, "log_ble_packets" },
    { FID_WAHOOD,  "game_1_23_3_wahoo_direct_connect_trainer" },
    { FID_WAP,     "game_1_23_workout_auto_pause" },
    { FID_WBE,     "game_1_24_workout_blocks_extendable" },
    { FID_UNIVERS, "game_1_23_universal_start_line_for_time_based_event" },
    { FID_BLE_EXC, "game_1_30_ble_cycling_speed_pairing_exception" },
    { FID_DWOW,    "game_1_24_display_workout_of_the_week_card" },
    { FID_DRWC,    "game_1_24_display_recommended_workout_cards" },
    { FID_PART,    "game_training_partners" },
    { FID_URR,     "game_upload_route_results_v2" },
    { FID_WDCERRO, "game_1_25_wdc_error_dialogs" },
    { FID_EXTENDE, "game_1_25_extended_pedal_asssit" },
    { FID_FLATGRA, "game_1_25_flat_grade_while_in_ui" },
    { FID_ANDROID, "game_1_25_android_ant_force_fec", 1, 0, 0, 0, 0 },
    { FID_WD,      "game_1_28_watchdog_timer" },
    { FID_CLUBATT, "game_1_26_club_attribution" },
    { FID_FIXTRAI, "game_1_27_fix_training_plan_summary_no_stats" },
    { FID_EVENTSU, "game_1_26_event_survey", 1, 0, 0, 0, 0 },
    { FID_BLEMIDD, "game_1_29_ble_middleware" },
    { FID_MEMORAB, "game_1_26_memorable_screenshots" },
    { FID_ATVREMO, "game_1_27_atv_remote_gestures" },
    { FID_HARDWAR, "game_1_27_hardware_restriction" },
    { FID_GOALNOT, "game_1_27_goal_notable_moment" },
    { FID_HUBFIRM, "game_1_28_hub_firmware_update" },
    { FID_FSF,     "game_ftms_spindown_fix" },
    { FID_DYNAMIC, "game_1_27_dynamic_campaigns_filter" },
    { FID_BOUT,    "bike_override_using_triggers" },
    { FID_KOUT,    "kit_override_using_triggers" },
    { FID_GETSTAR, "game_1_29_get_started_workouts" },
    { FID_EVENTCO, "game_1_28_event_core_feed_service" },
    { FID_FTMSROA, "game_1_28_ftms_road_grade_lookahead_time_exposed" },
    { FID_WT2,     "workout_training_v2_release" },
    { FID_TRAININ, "game_1_29_training_partners_survey", 1, 0, 0, 0, 0 },
    { FID_HANDCYC, "game_1_29_hand_cycle" },
    { FID_EVENTCA, "game_1_29_event_carousel" },
    { FID_SETTHRE, "game_1_29_set_thread_names" },
    { FID_EVENTQU, "game_1_29_event_query_interval" },
    { FID_BLECYCL, "game_1_30_ble_cycling_speed_pairing_opt" },
    { FID_NEWLOCA, "game_1_29_new_local_trainer_profile" },
    { FID_FIXSTUC, "game_1_29_fix_stuck_zpower" },
    { FID_CLIENTR, "game_1_29_client_recommendations" },
    { FID_GWP,     "game_workout_partners" },
    { FID_LEVEL60, "game_1_29_level_60_cap_raise" },
    { FID_PLAYERH, "game_1_29_player_hightlight" },
    { FID_POS_G,   "game_1_30_position_accuracy_over_network_global" },
    { FID_POS_L,   "game_1_30_position_accuracy_over_network_london" },
    { FID_LO_WM,   "game_1_29_logout_for_windows_and_mac" },
    { FID_PACKDYN, "game_1_30_pack_dynamics_v4" },
    { FID_CAMPAIG, "game_1_30_campaign_fullscreen_modal" },
    { FID_ENABLES, "game_1_30_enable_scale_setting_for_mobile" },
    { FID_SETDEFA, "game_1_30_set_default_mobile_scale_to_small" },
    { FID_PD4,     "game_1_32_pack_dynamics_v4_global" }
};
const char *Feature::c_str() { return c_str(m_id); }
const char *Feature::c_str(FeatureID id) {
    static thread_local std::unordered_map<FeatureID, const char *> g_tlsFeatureMetadataCont; //неэффективно
    if (g_tlsFeatureMetadataCont.size() == 0) {
        for (auto i : g_featureMetadata)
            g_tlsFeatureMetadataCont[i.m_id] = i.m_name;
    }
    return g_tlsFeatureMetadataCont[id];
}
Experimentation::Experimentation(ZNetAdapter *na, EventSystem *ev) : m_pNA(na), m_event_system(ev){
    m_userAttributes.m_somePointer = nullptr;
	ev->Subscribe(EV_RESET, this);
	ev->Subscribe(EV_28, this);
}
bool Experimentation::IsEnabled(FeatureID id) { return m_fsms[id].m_enabled == 1; }
bool Experimentation::IsEnabled(FeatureID id, bool overrideIfNot /*Experiment::Variant*/) { return overrideIfNot ? overrideIfNot : IsEnabled(id); }
ExpIsEnabledResult Experimentation::IsEnabled(FeatureID id, std::function<void(bool)> func) {
    FeatureStateMachine *m = m_fsms + id;
    FeaRequestResult res = m->OnRequest(func);
    ExpIsEnabledResult ret{ id, res.m_unk };
    if (res.m_succ) {
        if (!m_userAttributes.m_somePointer) {
            m_ids.push_back(id);
        } else {
            RequestFeatureData(id);
        }
    }
    return ret;
}
void Experimentation::HandleEvent(EVENT_ID e, va_list args) {
    if (e == EV_RESET) {
        static thread_local std::vector<FeatureID> tlsFeatureMetafataIDs;
        assert(nullptr == m_userAttributes.m_somePointer);
        m_userAttributes.m_somePointer = va_arg(args, void *);
        { //Experiment::Feature::BulkRequestFeatures
            for (const auto &i : g_featureMetadata) {                
                if (/*m_turnedOff ???*/ i.m_unk[0] == 0) tlsFeatureMetafataIDs.push_back(i.m_id);
            }
        }
        BulkRequestFeatureData(tlsFeatureMetafataIDs);
        assert(nullptr != m_userAttributes.m_somePointer);
    } else if (e == EV_28) {
        for (auto &s : m_userAttributes.m_str) {
            s = va_arg(args, std::string);
        }
    }
}
inline ZwiftDispatcher::ZwiftDispatcher() {
}
inline void ZwiftDispatcher::Assert(bool bPredicate) {
	assert(bPredicate);
}
inline void ZwiftDispatcher::Assert(bool bPredicate, const char *errMsg) {
	_ASSERT_EXPR(bPredicate, errMsg);
}
FeaRequestResult FeatureStateMachine::OnRequest(std::function<void(bool)> func) {
    //TODO
    return {};
}
void Experimentation::SetEventTypeAttribute(const std::string_view &src) {
    m_userAttributes.m_eventTypeAttr.m_filled = true;
    m_userAttributes.m_eventTypeAttr.m_val = src;
}
void Experimentation::RequestFeatureData(FeatureID id) {
    //TODO
}
void Experimentation::BulkRequestFeatureData(const std::vector<FeatureID> &ids) {
    std::vector<std::string> rqs;
    std::vector<FeatureID> rqf;
    for (auto id : ids) {
        auto &fsm = m_fsms[id];
        if (0 == fsm.m_field20) {
            fsm.m_field20 = 1;
            rqs.push_back(Feature::c_str(id));
            rqf.push_back(id);
            continue;
        }
        auto dec = fsm.m_field20 - 1;
        if (dec) {
            if (dec == 1 && fsm.m_enabled)
                continue;
            assert(false);
        }
        assert(fsm.m_enabled == 0);
    }
    //from assert(fsm.m_enabled == 0); to label40 - before break
    //TODO:from label40 here (line220)
    // (line 479) ZNetAdapter::GetFeatureVariants(m_pNA, &m_userAttributes, &rqs, f1, f2)
    //dtrs
}