#include "ZwiftApp.h"
const char *g_dialogNames[UI_DIALOGS_CNT] = {"UID_NONE",
"UID_LOG_IN",
"UID_CREATE_USER",
"UID_EULA_VIEW",
"UID_CUSTOMIZE_BIKE_AND_RIDER",
"UID_DETECTED_DEVICE_LIST",
"UID_ONBOARDINGCUSTOMIZE",
"UID_ONBOARDINGCUSTOMIZATION_PLAYER",
"UID_DEVICE_PAIRING",
"UID_DEVICE_PAIRING_WIDGET",
"UID_CALIBRATE_RUN_SENSOR_CONTROLLER",
"UID_CALIBRATE_SPIN_DOWN_SENSOR_CONTROLLER",
"UID_CALIBRATE_STEERING_SENSOR_CONTROLLER",
"UID_AUTHORITATIVE_DEVICE_DATA_VIEW",
"UID_DROP_IN",
"UID_PAUSED",
"UID_QUIT",
"UID_SELECT_MODE",
"UID_ACHIEVEMENT_NOTIFICATION",
"UID_TRAINER_SELECT",
"UID_SEND_AREA_TEXT",
"UID_EXTEND_WORKOUT_CONFIRM",
"UID_WORKOUT_SELECT",
"UID_UNLOCKABLE_NOTIFICATION",
"UID_CONFIG_SCREEN",
"UID_APPLY_PROMO_CODE",
"UID_PROFILE_SCREEN",
"UID_CHALLENGE_SCREEN",
"UID_TOKEN_LOGIN",
"UID_FLAGGED_INVALID_PERFORMANCE_NOTIFICATION",
"UID_SLIPPING_NOTIFICATION",
"UID_ACHIEVEMENTS_LIST",
"UID_TUTORIAL_SCREEN",
"UID_TRIAL_NAG_SCREEN",
"UID_TRIAL_END_SCREEN",
"UID_BONUS_NOTIFICATION_POPUP",
"UID_WORKOUT_CREATOR",
"UID_RACE_RESULTS",
"UID_POSTRIDE_RESULTS",
"UID_HEIGHTWEIGHT_SCREEN",
"UID_GENDER_SCREEN",
"UID_INTRO_SLIDESHOW",
"UID_SOCIAL_PREVIEW",
"UID_DAILY_TARGET_CONFIGURE",
"UID_CUSTOM_TARGET_CONFIGURE",
"UID_SELECT_ROUTE",
"UID_PERFORMANCE_INCREASE_NOTIFICATION",
"UID_SPORT_SELECT_CONTROLLER",
"UID_ZML_ADVERT_VIEW",
"UID_TRAININGPLAN_VIEW",
"UID_EVENT_FINISHED",
"UID_SKILL_SELECT",
"UID_RATING_SUBMISSION_CONTROLLER",
"UID_RATING_POST_EVENT",
"UID_RATING_HOLO_REPLAY",
"UID_MISSION_MESSAGE",
"UID_RPE_RATING_CONTROLLER",
"UID_ACCELERATED_LEVELING_SELECTOR",
"UID_FLAGGED_OUT_OF_CATEGORY_PERFORMANCE_NOTIFICATION",
"UID_PACERBOT",
"UID_RETURN_TO_WORLD",
"UID_CONNECTION_NOTIFICATIONS",
"UID_DELETE_ACCOUNT",
"UID_BLOCKER",
"UID_NEXT_UP_TRAINING_PLAN" };
GUI_Obj *UI_DialogPointer(UI_DIALOGS d) {
    auto ret = g_pDialogs[d];
    if (d == UID_DROP_IN)
        ret = dynamic_cast<UI_DropInDialog *>(ret); //QUEST: why?
    return ret;
}
bool CMD_ShowUI(const char *par) {
    int v1 = UID_NONE;
    int v4 = 0;
    char v5[512];
    sscanf_s(par, "%s %d", v5, (unsigned)sizeof(v5), &v4);
    for (auto i : g_dialogNames) {
        if (0 == strcmp(v5, i)) {
            if (v1)
                UI_CreateDialog((UI_DIALOGS)v1, nullptr, (void *)(uint64_t)v4);
            break;
        }
        ++v1;
    }
    return true;
}
void UI_CreateDialog(UI_DIALOGS, void *, void *) {
    //TODO
}
void UI_CloseDialog(UI_DIALOGS d) {
    //TODO
}
void UI_DropInDialog::RefreshGroupList() {
    //TODO
}
void UI_QuitDialog::FinalizeDontSaveAndQuit(/*void **/) {
    //TODO
}
void UI_IOS_QuitScreen::OnSavePressed(void *quitDialog) {
    //TODO
}
void OnQuit(UI_QuitDialog::DIALOG_RESULTS a1) {
    UI_CloseDialog(UID_QUIT);
    if (a1) {
        //OMIT crashReport
    } else {
        Zwift_EndSession(g_bShutdown);
        if (!g_bShutdown)
            ZSF_SwitchState(ZSF_c, nullptr);
    }
}
