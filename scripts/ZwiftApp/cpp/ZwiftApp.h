#pragma once
#include "pch.h"

#include "FeatureRequest.pb.h"
#include "Achievements.pb.h"
#include "Profile.pb.h"
#include "Goal.pb.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define OVR_OS_WIN32
struct VEC4 { float m_data[4]; };
struct VEC3 { float m_data[3]; };
struct VEC2 { float m_data[2]; };
struct MATRIX44 { VEC4 m_data[4]; };
#include "GL/glew.h"
#include "glfw/include/GLFW/glfw3.h"
#include "glfw/include/GLFW/glfw3native.h"
#include "../res/resource.h"

#include "Logging.h"
#include "GameCritical.h"
#include "ZMutex.h"
#include "ZNoesis.h"
#include "UI_Dialogs.h"
#include "Console.h"
#include "RenderTarget.h"
#include "Audio.h"
#include "GameWorld.h"
#include "EventSystem.h"
#include "CrashReporting.h"
#include "Experimentation.h"
#include "BLEModule.h"
#include "Thread.h"
#include "XMLDoc.h"
#include "ZStringUtil.h"
#include "CRC.h"
#include "ZNet.h"
#include "PlayerAchievementService.h"
#include "NoesisPerfAnalytics.h"
#include "ClientTelemetry.h"
#include "GroupEvents.h"
#include "VideoCapture.h"
#include "Powerups.h"
#include "IoCPP.h"
#include "GoalsManager.h"
#include "UnitTypeManager.h"
#include "SaveActivityService.h"
#include "ConnectionManager.h"
#include "Localization.h"
#include "HoloReplayManager.h"
#include "DataRecorder.h"
#include "BikeManager.h"
#include "BikeEntity.h"
#include "PlayerProfileCache.h"
#include "GFX.h"
#include "Downloader.h"
#include "OS.h"
#include "GAME.h"
#include "tHigFile.h"
#include "ICU.h"
#include "CameraManager.h"
#include "NotableMomentsManager.h"
#include "WADManager.h"
#include "ProfanityFilter.h"
#include "FitnessDeviceManager.h"
#include "RaceDictionary.h"
#include "ANIM.h"
#include "JM.h"
#include "LanExerciseDeviceManager.h"
#include "VRAM.h"
#include "GDE.h"
#include "MATERIAL.h"
#include "CFont2D.h"
#include "GameHolidayManager.h"
#include "ANTRECEIVER.h"
#include "GUI.h"
#include "GNScene.h"
#include "LOADER.h"
#include "GNView.h"
#include "PostFX.h"
#include "Sky.h"
#include "Weather.h"
#include "ParticulateManager.h"
#include "AccessoryManager.h"
#include "SteeringModule.h"
#include "HUD.h"
#include "INSTANCING.h"
#include "Bib.h"
#include "ZwiftPowers.h"
#include "Databases.h"

enum ZwiftStartupFlow { ZSF_LOGIN = 0x0, ZSF_1 = 0x1, ZSF_EULA = 0x2, ZSF_3 = 0x3, ZSF_4 = 0x4, ZSF_SEL_SPORT = 0x5, ZSF_BOARDING_CUST = 0x6, ZSF_PROFILE = 0x7, ZSF_8 = 0x8,
    ZSF_9 = 0x9, ZSF_PAIRING = 0xA, ZSF_b = 0xB, ZSF_c = 0xC, ZSF_SEL_CHALLENGE = 0xD, ZSF_GENDER = 0xE, ZSF_HEIGHT_WEIGHT = 0xF, ZSF_BC_CONF = 0x10, ZSF_11 = 0x11, ZSF_TRAINING_PLAN = 0x12,
    ZSF_CNT
};
inline ZwiftStartupFlow g_gameStartupFlowState;
inline const void *g_startupFlowStateParam;

inline uint32_t g_MainThread;
inline float g_vegetationWind[4];
struct IKeyProcessor {
    virtual bool ProcessKey(int, int) = 0;
};
struct GUIKeyProcessor : public IKeyProcessor {
    bool ProcessKey(int, int) override;
};
struct GoKeyProcessor : public IKeyProcessor {
    bool ProcessKey(int, int) override;
};
class KeyProcessorStack {
    std::vector<IKeyProcessor *> m_data;
public:
    //~KeyProcessorStack();
    //void RemoveAllKeyProcessors();
    //void Remove(IKeyProcessor *);
    void Push(IKeyProcessor *);
    bool ProcessKey(int, int);
    //void Pop();
    //void Find(IKeyProcessor const*,std::vector<IKeyProcessor*> &)
};
class ZwiftAppKeyProcessorManager {
    GUIKeyProcessor m_guiKP;
    GoKeyProcessor m_goKP;
    KeyProcessorStack m_stack;
public:
    //ZwiftAppKeyProcessorManager(); - 0's all
    static ZwiftAppKeyProcessorManager *Instance();
    void Init();
};

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

void ZwiftInitialize(const std::vector<std::string> &argv);
void EndGameSession(bool bShutDown);
void ShutdownSingletons();
void ZwiftExit(int code);
void ZWIFT_UpdateLoading(const wchar_t *, bool);
void ZWIFT_UpdateCaretPosition(int, int);
void ZWIFT_ShowDialog(int);
void ZWIFT_SetupUIViewport();
void ZWIFT_SetupDeviceViewport();
void ZWIFT_SetJoystickValues(float, float, float, float, float, float, bool, bool, bool, bool, bool, bool);
void MsgBoxAndExit(const char *lpText);
