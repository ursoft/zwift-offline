#pragma once //READY for testing
struct BikeManager;
struct contextForwarder;
struct CameraManager;
struct GameHolidayManager;
struct RoadManager;
struct RouteManager;
struct zwiftUpdateContext { //0x38 bytes
    BikeManager *m_bikeOver = nullptr;
    CameraManager *m_camOver = nullptr;
    contextForwarder *m_fwOver = nullptr;
    GameHolidayManager *m_holOver = nullptr;
    RoadManager *m_roadmOver = nullptr;
    RouteManager *m_routemOver = nullptr;
    void *m_field_20 = nullptr; //this[4] = ?, not found
    BikeManager *GetBikeManager();
    CameraManager *GetCameraManager();
    contextForwarder *GetForwarder();
    GameHolidayManager *GetGameHolidayManager();
    RoadManager *GetRoadManager();
    RouteManager *GetRouteManager();
    void SetBikeManagerOverride(BikeManager *ptr) { m_bikeOver = ptr; }
    void SetCameraManagerOverride(CameraManager *ptr) { m_camOver = ptr; }
    void SetForwarderOverride(contextForwarder *ptr) { m_fwOver = ptr; }
    void SetGameHolidayManagerOverride(GameHolidayManager *ptr) { m_holOver = ptr; }
    void SetRoadManagerOverride(RoadManager *ptr) { m_roadmOver = ptr; }
    void SetRouteManagerOverride(RouteManager *ptr) { m_routemOver = ptr; }
};
inline GLFWwindow *g_mainWindow;
void SetIcon();
//non-zwift: console redirection (useful for debugging and unit testing)
namespace non_zwift {
    struct ConsoleHandler {
        bool m_releaseNeed;
    public:
        ConsoleHandler(int16_t minLength);
        bool LaunchUnitTests(int argc, char **argv);
        ~ConsoleHandler();
    };
}
enum ZwiftStartupFlow {
    ZSF_LOGIN = 0x0, ZSF_1 = 0x1, ZSF_EULA = 0x2, ZSF_3 = 0x3, ZSF_4 = 0x4, ZSF_SEL_SPORT = 0x5, ZSF_BOARDING_CUST = 0x6, ZSF_PROFILE = 0x7, ZSF_8 = 0x8,
    ZSF_9 = 0x9, ZSF_PAIRING = 0xA, ZSF_b = 0xB, ZSF_c = 0xC, ZSF_SEL_CHALLENGE = 0xD, ZSF_GENDER = 0xE, ZSF_HEIGHT_WEIGHT = 0xF, ZSF_BC_CONF = 0x10, ZSF_11 = 0x11, ZSF_TRAINING_PLAN = 0x12,
    ZSF_CNT
};
enum AssetCategory : uint32_t { AC_UNK, AC_1, AC_2, AC_CNT };
inline ZwiftStartupFlow g_gameStartupFlowState;
inline const void *g_startupFlowStateParam;
inline bool g_MaintainFullscreenForBroadcast = true, g_removeFanviewHints, g_bShutdown, g_WorkoutDistortion, g_openglFail;
inline uint32_t g_MainThread;
inline float    g_vegetationWind[4];

void ScrollCallback(GLFWwindow *, double, double);
void CursorPosCallback(GLFWwindow *, double, double);
void WindowCloseCallback(GLFWwindow *);
void KeyCallback(GLFWwindow *, int, int, int, int);
void MouseButtonCallback(GLFWwindow *, int, int, int);
void CharModsCallback(GLFWwindow *, uint32_t, int);
void FramebufferSizeCallback(GLFWwindow *, int, int);
void WindowFocusCallback(GLFWwindow *, int);
void WindowSizeCallback(GLFWwindow *wnd, int w, int h);
void ZSF_FinishedState(ZwiftStartupFlow, uint32_t);
void ZSF_SwitchState(ZwiftStartupFlow, const void *);
void ZwiftInitialize(const std::vector<std::string> &argv);
const char *GAMEPATH(const char *path);
bool check_float(float f);
void EndGameSession(bool bShutDown);
void ShutdownSingletons();
void ZwiftExit(int code);
void Zwift_EndSession(bool bShutdown);
void ZWIFT_UpdateLoading(const UChar *, bool);
void LoadingRender(float time, const UChar *text);
//NOT FOUND void ZWIFT_UpdateCaretPosition(int, int);
//NOT FOUND void ZWIFT_ShowDialog(int);
void ZWIFT_SetupUIViewport();
void ZWIFT_SetupDeviceViewport();
//NOT FOUND void ZWIFT_SetJoystickValues(float, float, float, float, float, float, bool, bool, bool, bool, bool, bool);
void MsgBoxAndExit(const char *lpText);
/*NOT FOUND ZWIFT_BackPressed(void)
Zwift_OpenUpdatePage(void)
ZwiftMOBILE_TriggerAppRatingDialog(void)
ZwiftIOS_BindBackbuffer(void) //inlined
ZwiftIOS_CacheCredentials(char const *, char const *)
ZwiftIOS_CachePassword(char const *, char const *)
ZwiftIOS_CheckForUpdate(void)
ZwiftIOS_GetAdvertisingID(void)
ZwiftIOS_GetCachedCredentials(char const *)
ZwiftIOS_GetCachedPassword(char const *)
ZwiftIOS_GetCurrentSport(void)
ZwiftIOS_GetDeviceID(void)
ZwiftIOS_GetLogs(void *, void *)
ZwiftIOS_GetScreenDPI(GFX_DPI *) //inlined
ZwiftIOS_HandleUpdateUI(void)
ZwiftIOS_HideKeyboard(void)
ZwiftIOS_HowManyDaysLeftUntilUpdate(char const *, char const *)
ZwiftIOS_IsFacebookAvailable(void)
ZwiftIOS_IsKeyboardUp(void)
ZwiftIOS_IsMandatoryUpdateRequired(char const *)
ZwiftIOS_IsUpdateAvailable(char const *)
ZwiftIOS_MouseDown(float, float)
ZwiftIOS_MouseMove(float, float)
ZwiftIOS_MouseUp(float, float)
ZwiftIOS_OnEscape(void)
ZwiftIOS_OnKey(uint, uint)
ZwiftIOS_PostToFacebook(char const *, char *)
ZwiftIOS_SaveJPEGToCameraRoll(char const *)
ZwiftIOS_SaveVitals(float, float, float, bool, bool, long) //empty
ZwiftIOS_SendLogs(ulong long, ulong long, char const *)
ZwiftIOS_SetSpindownSpeed(float, bool, bool)
ZwiftIOS_ShowKeyboard(char const *, int, float, float, float, float, float, float, bool, float, int)
ZwiftIOS_Swipe(int, float, float, float, float, float, float)
ZwiftIOS_UpdateFramerateCap(float)
ZwiftIOS_UserOnAppleTV(void)
andr: empty ZwiftLogSpam_TEST(void)
ZwiftIOS_sendMixpanelPreLoginData(char const **, char const *)
ZwiftIOS_UpdateCaretPosForDevice(int, int)
ZwiftIOS_StartSaveVitals(void)
ZwiftIOS_AddWatchDeviceIfExists(void)
ZwiftAppEventsFrameEnded_TEST(void)
*/
//NOT FOUND void ZU_ConvertScreenPosToRay(const VEC2 &, VEC3 &);
//NOT FOUND void ZU_ConvertScreenPosToWorldPos(const VEC2 &, VEC3 &);
//NOT FOUND void ZU_ConvertWorldPosToScreenPos(const VEC3 &, VEC3 &);
//NOT FOUND void ZU_GetTimeStampVars(int *, int *, int *, int *, int *, int *);
//NOT FOUND bool ZU_IsDateInPast(uint64_t, uint64_t, uint64_t);
bool ZU_IsInPreReleaseRestrictedMode(int courseId);
bool ZU_IsInPreReleaseRestrictedMode();
//NOT FOUND void ZU_RayTriangleIntersection(const VEC3 &, const VEC3 &, const VEC3 &, const VEC3 &, const VEC3 &, VEC3 &);
template<class T> void FreeAndNil(T &ptr) {
    auto copy = ptr;
    ptr = nullptr;
    delete copy;
}
class NoCopy {
    NoCopy(const NoCopy &); // non construction-copyable
    NoCopy &operator=(const NoCopy &); // non copyable
};