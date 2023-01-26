cmake_minimum_required(VERSION 3.24)
project(ZwiftApp)

include(FetchContent)
FetchContent_Declare( #3.0.12
  noesis
  URL https://www.googleapis.com/drive/v3/files/1V4u2MFtPojK-U0PaV1Ot_OaEfb2GTm2y?alt=media&key=AIzaSyCe8S1RwtJK0aPavBFJzaM_or6L_dENsRo
  DOWNLOAD_NAME Noesis-3.0.12.zip
)
FetchContent_Declare( #2021.1.11
  wwise
  URL https://www.googleapis.com/drive/v3/files/1Q5lhYKAZ6gLuLfBsNJ77EUVkuLehiaB-?alt=media&key=AIzaSyCe8S1RwtJK0aPavBFJzaM_or6L_dENsRo
  DOWNLOAD_NAME Wwise_v2021.1.11.zip
)
FetchContent_Populate(noesis)
FetchContent_Populate(wwise)

set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(Boost_INCLUDE_DIR ${_VCPKG_INSTALLED_DIR}/x64-windows-static/include)
find_package(Boost 1.51 REQUIRED filesystem system)

add_executable(ZwiftApp)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
  protobuf/FeatureRequest.proto
)

target_sources(ZwiftApp PRIVATE
  Audio.cpp
  Audio.h
  BLEModule.cpp
  BLEModule.h
  Console.cpp
  Console.h
  CrashReporting.cpp
  CrashReporting.h
  EventSystem.cpp
  EventSystem.h
  Experimentation.cpp
  Experimentation.h
  FitnessDeviceManager.cpp
  FitnessDeviceManager.h
  framework.h
  GameCritical.cpp
  GameCritical.h
  GameWorld.cpp
  GameWorld.h
  InitApplication.cpp
  Logging.cpp
  Logging.h
  RenderTarget.cpp
  RenderTarget.h
  Resource.h
  targetver.h
  UI_Dialogs.cpp
  UI_Dialogs.h
  ZMutex.cpp
  ZMutex.h
  ZNoesis.cpp
  ZNoesis.h
  ZwiftApp.aps
  ZwiftApp.cpp
  ZwiftApp.h
  ZwiftApp.rc
  ${PROTO_SRCS} ${PROTO_HDRS}
)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
set_property(TARGET ZwiftApp PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  
set_property(TARGET ZwiftApp PROPERTY CXX_STANDARD 20)

target_include_directories(ZwiftApp PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${noesis_SOURCE_DIR}/Include
  ${wwise_SOURCE_DIR}/SDK/include
)
target_link_directories(ZwiftApp PRIVATE 
  ${noesis_SOURCE_DIR}/Lib/windows_x86_64
  ${wwise_SOURCE_DIR}/x64_vc170/${CMAKE_BUILD_TYPE}\(StaticCRT\)/lib
)

target_link_libraries(ZwiftApp PRIVATE 
  ${Protobuf_LIBRARIES}
  ${Boost_LIBRARIES}
  Noesis.lib #NoesisApp.lib
  Winmm.lib
  Opengl32.lib
  #glfw3.lib
  AkSoundEngine.lib AkMemoryMgr.lib AkMusicEngine.lib AkStreamMgr.lib
)