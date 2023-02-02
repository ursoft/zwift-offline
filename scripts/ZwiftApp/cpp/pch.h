#pragma once
#define NOMINMAX
#include "framework.h"
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

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <NoesisPCH.h>
#include <zlib.h>
#include <tinyxml2.h>
#include <curl/curl.h>
extern "C" {
#include <decNumber.h>
}
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>
#include <unicode/uloc.h>
#include <unicode/uchar.h>

#include <json/json.h>
#include <gtest/gtest.h>
