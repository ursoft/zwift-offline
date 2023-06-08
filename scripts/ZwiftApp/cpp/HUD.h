#pragma once
inline int32_t HUD_flagIcons;
void HUD_UpdateChatFont();
//void HUD_CheckCulling(zwift::protobuf::EventCulling, uint64_t, uint64_t, uint64_t, uint64_t);
void HUD_ClearPowerups();
void HUD_DismissBranchSelector();
void HUD_DontRender();
//void HUD_DrawActivityTarget(float, float, float, DAILY_TARGET_TYPE, int, float, bool, bool, bool);
void HUD_DrawArchIcon(float, float, float, int);
void HUD_DrawHalloweenUI(float, float, float, float, void *);
void HUD_DrawMissionUI(float);
//void HUD_DrawPaceTime(float, UNIT_TYPE, VEC2, float, uint32_t);
void HUD_DrawPacerBotDropMeter(float);
void HUD_DrawPowerupDisplay(float, float, float, float);
void HUD_DrawRouteUI(float);
void HUD_DrawSubgroupIcon(uint32_t, VEC2, VEC2, bool, uint32_t);
void HUD_EndMiniHUD();
void HUD_EndStandardHUD();
void HUD_EndTTHUD();
void HUD_GeneratePlayerIcon(BikeEntity *, float, float, float, uint32_t, uint32_t, int);
void HUD_GetCurrentNotification();
int HUD_GetFlagTextureFromISO3166(uint32_t countryCode, bool);
void HUD_GetPlayerHRZoneColor(float);
void HUD_GetPowerHRCadenceDimensionValues();
void HUD_GetPowerHRCadencePositionValues();
//void HUD_GetPowerupTexture(POWERUP_TYPE);
void HUD_GetSizeMultiplier();
//void HUD_GetTexture(HUD_TEXTURES);
void HUD_GiveDrops(uint32_t);
//void HUD_GivePlayerPowerup(POWERUP_TYPE);
void HUD_GivePlayerRandPowerup();
void HUD_GiveRideOn(long long, char const *, char const *, uint32_t);
void HUD_GiveWorkoutRideOn(long long, char const *, uint32_t);
void HUD_GiveXP(uint32_t);
//void HUD_Highlighter::DisableHighlighter();
//void HUD_Highlighter::ElementTagToEnum(std::string const &);
//void HUD_Highlighter::EnableHighlighter(HUD_ELEMENTS, float);
//void HUD_Highlighter::EnableHighlighter(std::string const &, float);
//void HUD_Highlighter::GetElementIndex(HUD_ELEMENTS);
//void HUD_Highlighter::void HUD_Highlighter();
//void HUD_Highlighter::Render();
//void HUD_Highlighter::StartNextEvent();
//void HUD_Highlighter::Update(float);
void HUD_Initialize();
void HUD_IsDisplayingIntersectionOption();
void HUD_LeaderboardWasUsed();
enum HUD_NotificationType { HNT_RIDEON_RX = 4 };
void HUD_Notify(const char *msg, float a2, HUD_NotificationType nt, int texture, uint32_t not_used1, float not_used2, int64_t playerId);
void HUD_OnEscape();
void HUD_PostUIRender();
void HUD_PrepareForPlayerIcons();
void HUD_PushSPA(const protobuf::SocialPlayerAction &, const VEC3 &, float = 500.0f);
void HUD_PushTextMessage(const ZNETWORK_TextMessage &msg);
void HUD_RefreshText();
//void HUD_RegisterTimingArchCallback(TimingArchEntity *, float);
//void HUD_RegisterTopBarCallback(void (*);(float, float, float, float, void *);, float, void *);
void HUD_Render(float);
void HUD_RenderAllPlayerIcons();
void HUD_RenderCurrentActivityIcon(float, float, float, float);
void HUD_RenderCurrentMessage(float, float, float, float);
void HUD_RenderDropsUI(float, float, float, float &, float);
void HUD_RenderFillRing(float, float, float, uint32_t, uint32_t, float);
void HUD_RenderGearDisplay();
void HUD_RenderGlobalStuff(float);
void HUD_RenderRegularLevel(float, float, float, float, float, uint32_t);
//void HUD_RenderReturnToMe(GUI_Button *);
void HUD_RenderStandardHUD(float, bool);
void HUD_RenderSteeringUI(float, float, float, float, float);
void HUD_RenderTopSystemBar();
void HUD_ResetSessionData();
void HUD_ResetTopBarCallback();
void HUD_RidersNearbyWasUsed();
void HUD_SetupZCTurnButtonScheme();
void HUD_ShouldDrawIntersectionUI(BikeEntity *);
void HUD_SpawnWorkoutFloaty(float, float, char const *, float, uint32_t);
void HUD_StartMiniHUD();
void HUD_StartStandardHUD();
void HUD_StartTTHUD();
void HUD_StopStandardHUD();
void HUD_Swiped(int, VEC2, VEC2, float, float, int);
void HUD_ToggleMobileActionKeysDial(int);
void HUD_Update(float);
void HUD_UpdateAndDrawFloaties(float, float, float);
void HUD_UpdateAndDrawIntersectionSelection();
void HUD_UpdateChatFont();
void HUD_UpdateDropsUI(float, float &);
void HUD_UpdateNotifications(float);
void HUD_UpdateRouteUI(float, bool);
void HUD_cbDrawPacerBotDropMeterUI(float, float, float, float, void *);
void HUD_cbDrawRouteCompletionUI(float, float, float, float, void *);
struct HUDToggle {
    static void ShowHUD(bool, bool);
    /*HUDToggle::HandleMenuButton(void)
HUDToggle::IsEnabled(void)
HUDToggle::IsHintShowing(void)
HUDToggle::IsSuspended(void)
HUDToggle::RenderHintText(float,char const*)
HUDToggle::Reset(void)
HUDToggle::SetSkipIntro(long long)
HUDToggle::ShouldSkipIntro(long long)
HUDToggle::ShowHint(bool)
HUDToggle::ShowIntro(bool)
HUDToggle::ToggleHUD(void)
HUDToggle::Update(float)
HUDToggle::UpdateKeyPress(float,float &)
HUDToggle::~HUDToggle()*/
};