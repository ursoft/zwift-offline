#pragma once
enum UI_DIALOGS {
    UID_NONE,
    UID_LOG_IN,
    UID_CREATE_USER,
    UID_EULA_VIEW,
    UID_CUSTOMIZE_BIKE_AND_RIDER,
    UID_DETECTED_DEVICE_LIST,
    UID_ONBOARDINGCUSTOMIZE,
    UID_ONBOARDINGCUSTOMIZATION_PLAYER,
    UID_DEVICE_PAIRING,
    UID_DEVICE_PAIRING_WIDGET,
    UID_CALIBRATE_RUN_SENSOR_CONTROLLER,
    UID_CALIBRATE_SPIN_DOWN_SENSOR_CONTROLLER,
    UID_CALIBRATE_STEERING_SENSOR_CONTROLLER,
    UID_AUTHORITATIVE_DEVICE_DATA_VIEW,
    UID_DROP_IN,
    UID_PAUSED,
    UID_QUIT,
    UID_SELECT_MODE,
    UID_ACHIEVEMENT_NOTIFICATION,
    UID_TRAINER_SELECT,
    UID_SEND_AREA_TEXT,
    UID_EXTEND_WORKOUT_CONFIRM,
    UID_WORKOUT_SELECT,
    UID_UNLOCKABLE_NOTIFICATION,
    UID_CONFIG_SCREEN,
    UID_APPLY_PROMO_CODE,
    UID_PROFILE_SCREEN,
    UID_CHALLENGE_SCREEN,
    UID_TOKEN_LOGIN,
    UID_FLAGGED_INVALID_PERFORMANCE_NOTIFICATION,
    UID_SLIPPING_NOTIFICATION,
    UID_ACHIEVEMENTS_LIST,
    UID_TUTORIAL_SCREEN,
    UID_TRIAL_NAG_SCREEN,
    UID_TRIAL_END_SCREEN,
    UID_BONUS_NOTIFICATION_POPUP,
    UID_WORKOUT_CREATOR,
    UID_RACE_RESULTS,
    UID_POSTRIDE_RESULTS,
    UID_HEIGHTWEIGHT_SCREEN,
    UID_GENDER_SCREEN,
    UID_INTRO_SLIDESHOW,
    UID_SOCIAL_PREVIEW,
    UID_DAILY_TARGET_CONFIGURE,
    UID_CUSTOM_TARGET_CONFIGURE,
    UID_SELECT_ROUTE,
    UID_PERFORMANCE_INCREASE_NOTIFICATION,
    UID_SPORT_SELECT_CONTROLLER,
    UID_ZML_ADVERT_VIEW,
    UID_TRAININGPLAN_VIEW,
    UID_EVENT_FINISHED,
    UID_SKILL_SELECT,
    UID_RATING_SUBMISSION_CONTROLLER,
    UID_RATING_POST_EVENT,
    UID_RATING_HOLO_REPLAY,
    UID_MISSION_MESSAGE,
    UID_RPE_RATING_CONTROLLER,
    UID_ACCELERATED_LEVELING_SELECTOR,
    UID_FLAGGED_OUT_OF_CATEGORY_PERFORMANCE_NOTIFICATION,
    UID_PACERBOT,
    UID_RETURN_TO_WORLD,
    UID_CONNECTION_NOTIFICATIONS,
    UID_DELETE_ACCOUNT,
    UID_BLOCKER,
    UID_NEXT_UP_TRAINING_PLAN,
    UI_DIALOGS_CNT
};
struct UI_DropInDialog {
    void RefreshGroupList();
/*
    void BuildData(void);
    void CanSelectRoute(uint);
    void CanSelectWorld(int);
    void CheckBoxInFocus(GUI_Obj *);
    void ClearActiveWorkout(void);
    void ConfigurePlayersRouteSettings(void);
    void Destroy(void);
    void ForceCloseDialog(void *);
    void FriendModel::FriendModel(UI_DropInDialog::FriendModel const&);
    void GetDialogState(void);
    void GetTargetWorldFull(zwift::protobuf::DropInWorld const&);
    void InitDeviceStatusWidgets(void);
    void IsRouteCompatibleWithActiveWorkout(uint);
    void ListBoxInFocus(GUI_Obj *);
    void ListBoxOutFocus(GUI_Obj *);
    void MessageBoxCallback(MessageBoxResults);
    void OnButton1(void *);
    void OnButton2(void *);
    void OnButtonJoinZwift(void *);
    void OnButtonRestore(void *);
    void OnButtonUpdateMacDriver(void *);
    void OnChangeRideType(void *);
    void OnChangeRouteType(void *);
    void OnChooseGuestWorld(void *);
    void OnChooseMainWorld(void *);
    void OnChooseThirdWorld(void *);
    void OnEscape(void);
    void OnExitButton(void *);
    void OnKey(int,int);
    void OnMouseMove(float,float);
    void OnSelectRouteFromDialog(uint);
    void OnSelectWorkoutFromDialog(Workout *);
    void OnSignedUpForGroup(int);
    void OnViewPlan(void *);
    void PostRender(void);
    void RefreshRiderListForSelectedWorld(void);
    void Render(void);
    void RenderDeviceStatusInfo(float,float,float,float);
    void RenderGuestWorldButton(GUI_Button *);
    void RenderJoinZwifterInfo(float,float,float,float,char const*);
    void RenderMainWorldButton(GUI_Button *);
    void RenderRideTypeInfo(float,float,float,float);
    void RenderRouteInfo(float,float,float,float);
    void RenderThirdWorldButton(GUI_Button *);
    void RenderTrainingPlanInfo(float,float,float,float,TrainingPlan *,TrainingPlanEnrollmentInstance *);
    void RenderWorldButton(float,float,float,float,int,bool);
    void RenderWorldInfo(float,float,float,float);
    void RequestPacePartnerPlayerStates(long long,long long);
    void RequestPacePartnerProfiles(zwift::protobuf::DropInWorld const&);
    void SelectRoute(uint);
    void SelectWorld(int);
    void Selected(GUI_Obj *,uint,void *);
    void SetDisabled(bool);
    void SetDropInData(int);
    void UI_DropInDialog(float,float,float,float,char const*,char const*,void (*)(long long));
    void Update(float);
    void onPairingContainerPressed(void *);
    void ~UI_DropInDialog()*/;
};
UI_DropInDialog *UI_DialogPointer(UI_DIALOGS);
void UI_CreateDialog(UI_DIALOGS, void *, void *);
inline void *g_pDialogs[UI_DIALOGS_CNT];
inline bool UI_IsDialogOfTypeOpen(UI_DIALOGS d) { return g_pDialogs[d] != nullptr; }
void UI_CloseDialog(UI_DIALOGS d);