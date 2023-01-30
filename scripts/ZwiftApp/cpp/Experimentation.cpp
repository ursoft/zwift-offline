#include "ZwiftApp.h"
const char *g_expVarNames[EXP_CNT] = { "unassigned", "enabled", "disabled", "none", "unknown" };

std::unique_ptr<Experimentation> g_sExperimentationUPtr;
ZNetAdapter g_znetAdapter;
void Experimentation::Initialize(EventSystem *ev) {
    zassert(g_sExperimentationUPtr.get() == nullptr);
    g_sExperimentationUPtr.reset(new Experimentation(&g_znetAdapter, ev));
    zassert(g_sExperimentationUPtr.get() != nullptr);
}
const FeatureMetadata g_featureMetadata[FID_CNT] = { //FillFeatureMetadata
    //variants.txt[zwift_launcher_osx_metal] not used here (maybe because of windows)
    //variants.txt[game_1_26_2_data_encryption] not used here
    //variants.txt[game_1_27_0_disable_encryption_bypass] not used here
    { FID_PPDATA,  "game_1_31_pp_data", 0, 0, 6, 6, 7 }, //variants.txt simple bool.true
    { FID_PPDATA,  "game_1_31_rlog", 0, 0, 6, 6, 9 }, //variants.txt "values": { "fields": { "DISABLED_LEVELS": { "stringValue": "DEBUG,INFO,WARNING" }, "DISABLED_TYPES" : { "stringValue": "" } } }
    { FID_FQUERY,  "game_1_31_fquery", 0, 0, 6, 6, 7 }, //variants.txt simple bool.true
    { FID_VID_CAP, "game_VideoCapture", 0, 0, 7, 8, 9 }, //variants.txt simple bool.false
    { FID_SERV_PC, "game_1_32_server_power_curves", 0, 1, 7, 7, 9 }, //variants.txt "values": { "fields": { "view_enabled": { "boolValue": false } } } 
    { FID_HIDE_HW, "game_1_28_enable_workout_hud_hiding", 0, 0, 1, 2, 7 }, //variants.txt simple bool.true
    { FID_HIDE_HD, "game_1_28_display_pr_hud_in_dynamic_workouts", 0, 0, 2, 2, 7 }, //variants.txt simple bool.true
    { FID_FIXLEAD, "game_1_28_fix_leaderboard_hud_starting_display_road_time", 0, 0, 2, 2, 7 }, //variants.txt simple bool.true
    { FID_REND_P,  "game_1_28_render_profile_screen_drop_down_box_title_text", 0, 0, 2, 2, 7 }, //variants.txt simple bool.true
    { FID_AVOIDAB, "game_1_28_avoid_aborting_current_timing_sector_when_minimizing", 0, 0, 2, 2, 7 }, //variants.txt simple bool.true
    { FID_INTEGRA, "game_1_29_integrate_road_and_bike_type_into_braking_performance", 0, 0, 3, 3, 8 }, //variants.txt simple bool.false
    { FID_USERELA, "game_1_29_use_relay_spline_time_in_meetup_position_late_joiner", 0, 0, 3, 3, 8 }, //variants.txt simple bool.true
    { FID_IGNOREP, "game_1_29_ignore_private_meetups_in_group_event_processing_logic", 0, 0, 3, 3, 8 }, //variants.txt simple bool.true
    { FID_CLEARPO, "game_1_30_clear_powerups_on_event_start", 0, 0, 5, 5, 9 }, //variants.txt simple bool.true
    { FID_ENABLEA, "game_1_32_enable_autobraking_in_watopia", 0, 0, 7, 7, 9 }, //variants.txt simple bool.false
    { FID_WNS,     "game_1_31_workout_network_service", 0, 1, 6, 6, 9 }, //variants.txt simple bool.false
    { FID_ENABLEJ, "game_1_31_enable_join_zwifter", 0, 0, 6, 6, 9 }, //variants.txt simple bool.true
    { FID_MRB_V2,  "game_1_32_meetups_rubberbanding_v2", 0, 0, 7, 7, 9 }, //variants.txt simple bool.false
    { FID_NINETYD, "game_1_26_ninety_day_prs" }, //variants.txt simple bool.true
    { FID_DRAFTLO, "game_1_14_draftlock_fix" }, //variants.txt simple bool.false
    { FID_PPV,     "xplatform_partner_connection_vitality" }, //variants.txt simple bool.false
    { FID_ASSERTD, "game_1_15_assert_disable_abort" }, //variants.txt simple bool.true
    { FID_NOESISE, "game_1_17_noesis_enabled" }, //variants.txt "value": true, "values": { "fields": { "RenderDevice": { "varintValue": "0" } } }
    { FID_SETTING, "game_1_14_settings_refactor" }, //variants.txt simple bool.true
    { FID_HOMESCR, "game_1_20_home_screen" }, //variants.txt simple bool.true
    { FID_PERFANA, "game_1_21_perf_analytics" }, //variants.txt simple bool.true
    { FID_FORYOUC, "game_1_30_for_you_carousel" }, //variants.txt simple bool.false
    { FID_SYSTEMA, "game_1_19_system_alerts" }, //variants.txt simple bool.true
    { FID_BLEALT,  "game_1_16_2_ble_alternate_unpair_all_paired_devices" }, //variants.txt simple bool.true
    { FID_TDFFEM,  "game_1_17_1_tdf_femmes_yellow_jersey" }, //variants.txt simple bool.false
    { FID_BLEDISA, "game_1_17_ble_disable_component_sport_filter" }, //variants.txt simple bool.true
    { FID_NEWWELC, "game_1_18_new_welcome_ride" }, //variants.txt simple bool.false
    { FID_HOLIDAY, "game_1_18_holiday_mode" }, //variants.txt simple bool.true
    { FID_PACKDY,  "game_1_18_0_pack_dynamics_2_5_collision_push_back_removal" }, //variants.txt simple bool.true
    { FID_OSXMON,  "game_1_18_0_osx_monterey_bluetooth_uart_fix" }, //variants.txt has also values.boolValue:false
    { FID_DEFAUL,  "game_1_19_0_default_rubberbanding" }, //variants.txt simple bool.false
    { FID_USETABB, "game_1_19_use_tabbed_settings" }, //variants.txt has also values.boolValue:false
    { FID_HWEXPER, "game_1_20_hw_experiment_1" }, //variants.txt simple bool.true
    { FID_PAIREDD, "game_1_19_paired_devices_alerts" }, //variants.txt simple bool.true
    { FID_REALTIM, "game_1_19_real_time_unlocks" }, //variants.txt simple bool.true
    { FID_BLEDATA, "game_1_21_ble_data_guard_v2" }, //variants.txt simple bool.false
    { FID_MIX,     "game_1_20_disable_high_volume_send_mixpanel" }, //variants.txt simple bool.false
    { FID_ADDDEVI, "game_1_21_add_device_jet_black_swb" }, //variants.txt simple bool.true
    { FID_CLICKAB, "game_1_20_clickable_telemetry_box" }, //variants.txt simple bool.true
    { FID_ENABLE,  "game_1_20_0_enable_stages_steering" }, //variants.txt has also values.boolValue:false
    { FID_HUDHIG,  "game_1_21_0_hud_highlighter" }, //variants.txt simple bool.false
    { FID_GPUDEP,  "game_1_21_0_gpu_deprecation_warning_message" }, //variants.txt simple bool.true
    { FID_FTMS,    "game_1_21_ftms_set_rider_weight" }, //variants.txt simple bool.true
    { FID_BLEDLLV, "game_1_27_ble_dll_v2" }, //variants.txt simple bool.true
    { FID_ALLOWUT, "game_1_22_allow_uturns_at_low_speed" }, //variants.txt simple bool.false
    { FID_FTMSSP,  "game_1_21_0_ftms_sport_filter" }, //variants.txt simple bool.false
    { FID_FTMSBIK, "game_1_21_ftms_bike_trainer_v3" }, //variants.txt simple bool.true
    { FID_LOG_BLE, "log_ble_packets" }, //variants.txt simple bool.false
    { FID_WAHOOD,  "game_1_23_3_wahoo_direct_connect_trainer" }, //variants.txt simple bool.true
    { FID_WAP,     "game_1_23_workout_auto_pause" }, //variants.txt "values": { "fields": { "workoutAutoPauseTime": { "numberValue": 3.0 }, "rampUpTime" : { "numberValue": 10.0 }, "minERGModeDisableTime" : { "numberValue": 5.0 }, "reenableERGPowerThreshold" : { "numberValue": 10.0 } } }
    { FID_WBE,     "game_1_24_workout_blocks_extendable" }, //variants.txt simple bool.false
    { FID_UNIVERS, "game_1_23_universal_start_line_for_time_based_event" }, //variants.txt simple bool.true
    { FID_BLE_EXC, "game_1_30_ble_cycling_speed_pairing_exception" }, //variants.txt simple bool.true
    { FID_DWOW,    "game_1_24_display_workout_of_the_week_card" }, //variants.txt simple bool.true
    { FID_DRWC,    "game_1_24_display_recommended_workout_cards" }, //variants.txt simple bool.true
    { FID_PART,    "game_training_partners" }, //variants.txt "value": true, "values": { "fields": { "route_spawning_enabled": { "boolValue": false }, "screenshots_enabled" : { "boolValue": true }, "SOLO_WORKOUT_recording_disabled" : { "boolValue": false }, "spawn_enabled" : { "boolValue": true }, "route_recording_enabled" : { "boolValue": false }, "recording_enabled" : { "boolValue": true } } }
    { FID_URR,     "game_upload_route_results_v2" }, //variants.txt simple bool.true
    { FID_WDCERRO, "game_1_25_wdc_error_dialogs" }, //variants.txt simple bool.false
    { FID_EXTENDE, "game_1_25_extended_pedal_asssit" }, //variants.txt "value": true, "values": { "fields": { "initialDropInTime": { "numberValue": 10000.0 }, "extendedDropInTime" : { "numberValue": 60000.0 }, "initialDropInFrames" : { "numberValue": 1000.0 } }
    { FID_FLATGRA, "game_1_25_flat_grade_while_in_ui" }, //variants.txt simple bool.false
    { FID_ANDROID, "game_1_25_android_ant_force_fec", 1, 0, 0, 0, 0 }, //variants.txt: not found - deprecated
    { FID_WD,      "game_1_28_watchdog_timer" }, //variants.txt "values": { "fields": { "mainTimeoutInterval": { "numberValue": 30.0 }, "loadingTimeoutInterval" : { "numberValue": 60.0 }, "reportTimeoutsOnLoad" : { "boolValue": false }, "renderTimeoutInterval" : { "numberValue": 30.0 }, "abortEnabled" : { "boolValue": false } } }
    { FID_CLUBATT, "game_1_26_club_attribution" }, //variants.txt simple bool.true
    { FID_FIXTRAI, "game_1_27_fix_training_plan_summary_no_stats" }, //variants.txt simple bool.false
    { FID_EVENTSU, "game_1_26_event_survey", 1, 0, 0, 0, 0 }, //variants.txt: not found - deprecated
    { FID_BLEMIDD, "game_1_29_ble_middleware" }, //variants.txt simple bool.false
    { FID_MEMORAB, "game_1_26_memorable_screenshots" }, //variants.txt simple bool.true
    { FID_ATVREMO, "game_1_27_atv_remote_gestures" }, //variants.txt "value": true, "values": { "fields": { "ATVPopDelay": { "numberValue": 0.15 }, "ATVPopInitDelay" : { "numberValue": 0.22 }, "ATVPopInitDist" : { "numberValue": 250.0 }, "ATVMoveSensitivity" : { "varintValue": "5" }, "ATVFriction" : { "numberValue": 0.9 }, "ATVPopDist" : { "numberValue": 175.0 } }
    { FID_HARDWAR, "game_1_27_hardware_restriction" }, //variants.txt simple bool.true
    { FID_GOALNOT, "game_1_27_goal_notable_moment" }, //variants.txt simple bool.true
    { FID_HUBFIRM, "game_1_28_hub_firmware_update" }, //variants.txt simple bool.true
    { FID_FSF,     "game_ftms_spindown_fix" }, //variants.txt simple bool.true
    { FID_DYNAMIC, "game_1_27_dynamic_campaigns_filter" }, //variants.txt simple bool.false
    { FID_BOUT,    "bike_override_using_triggers" }, //variants.txt simple bool.true
    { FID_KOUT,    "kit_override_using_triggers" }, //variants.txt simple bool.false
    { FID_GETSTAR, "game_1_29_get_started_workouts" }, //variants.txt simple bool.false
    { FID_EVENTCO, "game_1_28_event_core_feed_service" }, //variants.txt simple bool.false
    { FID_FTMSROA, "game_1_28_ftms_road_grade_lookahead_time_exposed" }, //variants.txt simple bool.false
    { FID_WT2,     "workout_training_v2_release" }, //variants.txt simple bool.true
    { FID_TRAININ, "game_1_29_training_partners_survey", 1, 0, 0, 0, 0 }, //variants.txt: not found - deprecated
    { FID_HANDCYC, "game_1_29_hand_cycle" }, //variants.txt simple bool.true
    { FID_EVENTCA, "game_1_29_event_carousel" }, //variants.txt "value": true, "values": { "fields": { "NUM_CARDS": { "varintValue": "6" } } }
    { FID_SETTHRE, "game_1_29_set_thread_names" }, //variants.txt simple bool.false
    { FID_EVENTQU, "game_1_29_event_query_interval" }, //variants.txt "value": true, "values": { "fields": { "GROUP_EVENT_QUERY_RATE_SECONDS": { "numberValue": 15.0 } } }
    { FID_BLECYCL, "game_1_30_ble_cycling_speed_pairing_opt" }, //variants.txt simple bool.true
    { FID_NEWLOCA, "game_1_29_new_local_trainer_profile" }, //variants.txt simple bool.false
    { FID_FIXSTUC, "game_1_29_fix_stuck_zpower" }, //variants.txt simple bool.true
    { FID_CLIENTR, "game_1_29_client_recommendations" }, //variants.txt simple bool.true
    { FID_GWP,     "game_workout_partners" }, //variants.txt simple bool.false
    { FID_LEVEL60, "game_1_29_level_60_cap_raise" }, //variants.txt simple bool.true
    { FID_PLAYERH, "game_1_29_player_hightlight" }, //variants.txt simple bool.false
    { FID_POS_G,   "game_1_30_position_accuracy_over_network_global" }, //variants.txt simple bool.false
    { FID_POS_L,   "game_1_30_position_accuracy_over_network_london" }, //variants.txt simple bool.false
    { FID_LO_WM,   "game_1_29_logout_for_windows_and_mac" }, //variants.txt simple bool.true
    { FID_PACKDYN, "game_1_30_pack_dynamics_v4" }, //variants.txt "value": true, "values": { "fields": { "reducedPowerPercentage": { "numberValue": 98.0 }, "powerCeilingPercentage" : { "numberValue": 110.0 }, "speedDifferenceHigh" : { "numberValue": 3.0 }, "brakingAmount" : { "numberValue": 0.5 }, "cdaPenaltyMultiplier" : { "numberValue": 1.0 }, "downhillSlopeLimit" : { "numberValue": -200.0 }, "minDraftLimitFlat" : { "varintValue": "39" }, "minSpeedLimitDown" : { "numberValue": 45.0 }, "minSpeedLimitFlat" : { "numberValue": 26.0 }, "minSpeedLimitShallowDown" : { "numberValue": 38.0 }, "minDraftLimitDown" : { "varintValue": "95" }, "minDraftLimitShallowDown" : { "varintValue": "44" } } }
    { FID_CAMPAIG, "game_1_30_campaign_fullscreen_modal" }, //variants.txt "value": true, "values": { "fields": { "shortNames": { "stringValue": "tomi2022,tdz2023ride" } } }
    { FID_ENABLES, "game_1_30_enable_scale_setting_for_mobile" }, //variants.txt simple bool.false
    { FID_SETDEFA, "game_1_30_set_default_mobile_scale_to_small" }, //variants.txt simple bool.true
    { FID_PD4,     "game_1_32_pack_dynamics_v4_global" } // "value": true, "values": { "fields": { "watopia": { "boolValue": false }, "auto_braking_visuals" : { "boolValue": false }, "double_draft_global" : { "boolValue": false }, "global" : { "boolValue": false }, "auto_braking_visuals_events" : { "boolValue": true }, "makuri" : { "boolValue": false } } }
};
const char *Feature::c_str() { return c_str(m_id); }
const char *Feature::c_str(FeatureID id) {
    static std::unordered_map<FeatureID, const char *> g_tlsFeatureMetadataCont;
    if (g_tlsFeatureMetadataCont.size() == 0) {
        for (auto i : g_featureMetadata)
            g_tlsFeatureMetadataCont[i.m_id] = i.m_name;
    }
    return g_tlsFeatureMetadataCont[id];
}
Experimentation::Experimentation(ZNetAdapter *na, EventSystem *ev) : EventObject(ev), m_pNA(na) {
    m_userAttributes.m_somePointer = nullptr;
    ev->Subscribe(EV_RESET, this);
    ev->Subscribe(EV_28, this);
}
bool Experimentation::IsEnabled(FeatureID id) { return m_fsms[id].m_enableStatus == EXP_ENABLED; }
ExpVariant Experimentation::IsEnabled(FeatureID id, ExpVariant overrideIfUn) { return (overrideIfUn != EXP_UNASSIGNED) ? overrideIfUn : m_fsms[id].m_enableStatus; }
ExpIsEnabledResult Experimentation::IsEnabled(FeatureID id, const FeatureCallback &func) {
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
        static std::vector<FeatureID> tlsFeatureMetafataIDs;
        zassert(nullptr == m_userAttributes.m_somePointer);
        m_userAttributes.m_somePointer = va_arg(args, void *);
        { //Experiment::Feature::BulkRequestFeatures
            for (const auto &i : g_featureMetadata) {                
                if (i.m_deprecated == 0) tlsFeatureMetafataIDs.push_back(i.m_id);
            }
        }
        BulkRequestFeatureData(tlsFeatureMetafataIDs);
        zassert(nullptr != m_userAttributes.m_somePointer);
    } else if (e == EV_28) {
        for (auto &s : m_userAttributes.m_str) {
            s = va_arg(args, std::string);
        }
    }
}
inline ZwiftDispatcher::ZwiftDispatcher() {
}
inline void ZwiftDispatcher::Assert(bool bPredicate) {
    zassert(bPredicate);
}
inline void ZwiftDispatcher::Assert(bool bPredicate, const char *errMsg) {
    _ASSERT_EXPR(bPredicate, errMsg);
}
FeaRequestResult FeatureStateMachine::OnRequest(const FeatureCallback &func) {
    m_rqCounter++;
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
            if (dec == 1 && fsm.m_enableStatus != EXP_UNASSIGNED)
                continue;
            zassert(false);
        }
        zassert(fsm.m_enableStatus == EXP_UNASSIGNED);
    }
    //from assert(fsm.m_enableStatus == EXP_UNASSIGNED); to label40 - before break
    //TODO:from label40 here (line220)
    // (line 479) ZNetAdapter::GetFeatureVariants(m_pNA, &m_userAttributes, &rqs, f1, f2)
    //dtrs
}