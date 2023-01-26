cmake_minimum_required(VERSION 3.24)
project(ZwiftApp)

include(cmake/thirdparty.cmake)

add_executable(ZwiftApp WIN32)
add_dependencies(ZwiftApp glfw)

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
  AkSoundEngine.lib AkMemoryMgr.lib AkMusicEngine.lib AkStreamMgr.lib
  ${CMAKE_CURRENT_BINARY_DIR}/glfw/lib/glfw3.lib
)