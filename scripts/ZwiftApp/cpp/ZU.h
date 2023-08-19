#pragma once
struct zwiftUpdateContext { //0x38 bytes
    //TODO
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
bool check_float(float f);
enum ZwiftStartupFlow {
    ZSF_LOGIN = 0x0, ZSF_1 = 0x1, ZSF_EULA = 0x2, ZSF_3 = 0x3, ZSF_4 = 0x4, ZSF_SEL_SPORT = 0x5, ZSF_BOARDING_CUST = 0x6, ZSF_PROFILE = 0x7, ZSF_8 = 0x8,
    ZSF_9 = 0x9, ZSF_PAIRING = 0xA, ZSF_b = 0xB, ZSF_c = 0xC, ZSF_SEL_CHALLENGE = 0xD, ZSF_GENDER = 0xE, ZSF_HEIGHT_WEIGHT = 0xF, ZSF_BC_CONF = 0x10, ZSF_11 = 0x11, ZSF_TRAINING_PLAN = 0x12,
    ZSF_CNT
};
inline ZwiftStartupFlow g_gameStartupFlowState;
inline const void *g_startupFlowStateParam;
inline bool g_MaintainFullscreenForBroadcast = true, g_removeFanviewHints, g_bShutdown, g_WorkoutDistortion, g_openglFail;
inline uint32_t         g_MainThread;
inline float            g_vegetationWind[4];

void ZwiftInitialize(const std::vector<std::string> &argv);
void ZSF_SwitchState(ZwiftStartupFlow, const void *);
void ZSF_FinishedState(ZwiftStartupFlow, uint32_t);
const char *GAMEPATH(const char *path);
void WindowSizeCallback(GLFWwindow *wnd, int w, int h);
void WindowFocusCallback(GLFWwindow *, int);
void FramebufferSizeCallback(GLFWwindow *, int, int);
void CharModsCallback(GLFWwindow *, uint32_t, int);
void MouseButtonCallback(GLFWwindow *, int, int, int);
void KeyCallback(GLFWwindow *, int, int, int, int);
void WindowCloseCallback(GLFWwindow *);
void ScrollCallback(GLFWwindow *, double, double);
void CursorPosCallback(GLFWwindow *, double, double);

void EndGameSession(bool bShutDown);
void ShutdownSingletons();
void ZwiftExit(int code);
/*ZwiftFrame_Draw(float, float, float, float, dialogBoxStyle, uint, float, bool, bool)
ZwiftFrame_Draw(float, float, float, float, dialogBoxStyle, uint, uint, float, bool)*/
void ZWIFT_UpdateLoading(const UChar *, bool);
void LoadingRender(float time, const UChar *text);
void ZWIFT_UpdateCaretPosition(int, int);
void ZWIFT_ShowDialog(int);
void ZWIFT_SetupUIViewport();
void ZWIFT_SetupDeviceViewport();
void ZWIFT_SetJoystickValues(float, float, float, float, float, float, bool, bool, bool, bool, bool, bool);
void MsgBoxAndExit(const char *lpText);
enum AssetCategory : uint32_t { AC_UNK, AC_1, AC_2, AC_CNT };

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
