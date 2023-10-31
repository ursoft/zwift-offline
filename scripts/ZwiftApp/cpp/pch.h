#pragma once
#define _ITERATOR_DEBUG_LEVEL 0 //otherwise sizoef(std::string) == 40, ble dll does not like this
#define _DISABLE_VECTOR_ANNOTATION
#define _DISABLE_STRING_ANNOTATION
#define _CRT_SECURE_NO_WARNINGS
#define OPENSSL_API_COMPAT 0x10100000L
#define NOMINMAX
//#define AK_USE_UWP_API
#ifdef NDEBUG
#define AK_OPTIMIZED
#endif //NDEBUG
#include "framework.h"
#include <Winhttp.h>
#include <ShellScalingApi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <timeapi.h>
#include <Shlobj.h>
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <string>
#include <sstream> 
#include <vector> 
#include <chrono>
#include <thread>
#include <mutex>
#include <map>
#include <unordered_map>
#include <functional>
#include <cassert> 
#include <ctime>
#include <cmath>
#include <algorithm>
#include <deque>
#include <queue>
#include <filesystem>
#include <fstream>
#include <random>
#include <numbers>
#include <bitset>
#include <shellapi.h>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <NoesisPCH.h>
#include <zlib.h>
#include <tinyxml2.h>
#include <curl/curl.h>
extern "C" {
//#include <decNumber.h>
void __libm_sse2_sincosf_(float, float *, float *);
}
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>
#include <unicode/uloc.h>
#include <unicode/uchar.h>
#include <unicode/uclean.h>

#include <json/json.h>
#include <gtest/gtest.h>

using std::string_literals::operator""s;
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define OVR_OS_WIN32
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include "GL/glew.h"
#include "glfw/include/GLFW/glfw3.h"
#include "glfw/include/GLFW/glfw3native.h"
#include "squish.h"
