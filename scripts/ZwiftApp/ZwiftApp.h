#pragma once
#include "framework.h"
#include "tlhelp32.h"
#include "psapi.h"
#include "stdio.h"
#include <cstdint>
#include <string>
#include <sstream> 
#include <vector> 

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define OVR_OS_WIN32
#include "glfw/include/GLFW/glfw3.h"
#include "glfw/include/GLFW/glfw3native.h"

#include "resource.h"

void resize(GLFWwindow *wnd, int w, int h);
void InitApplication(const std::vector<std::string> &argv);
void EndGameSession(bool bShutDown);
extern bool InitApplicationOK, byte_7FF6D5F638EB /*about app finish?*/;
