#pragma once
#include "framework.h"
#include "tlhelp32.h"
#include "psapi.h"
#include "stdio.h"
#include <cstdint>
#include <string>
#include <sstream> 
#include <vector> 
#include <chrono>
#include <thread>
#include <mutex>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define OVR_OS_WIN32
#include "glfw/include/GLFW/glfw3.h"
#include "glfw/include/GLFW/glfw3native.h"

#include "resource.h"

void resize(GLFWwindow *wnd, int w, int h);
void ZwiftInitialize(const std::vector<std::string> &argv);
void EndGameSession(bool bShutDown);
void ZwiftExit(int code);
void AUDIO_Shutdown();
void ShutdownSingletons();
#include "Logging.h"
#include "GameCritical.h"
#include "ZMutex.h"

extern bool InitApplicationOK, byte_7FF6D5F638EB /*about app finish?*/;
extern float g_kwidth, g_kheight, g_view_x, g_view_y, g_view_w, g_view_h;
extern int g_width, g_height;
const float g_UI_AspectRatio = 1.7777778f; //16x9

#include "NoesisPCH.h"
#include "ZNoesis.h"
#include "UI_Dialogs.h"
#include "Console.h"

extern DWORD g_MainThread;
const char *GAMEPATH(const char *path);

#include "RenderTarget.h"
#include "Audio.h"

//last:
#undef NDEBUG
#include <cassert> 
