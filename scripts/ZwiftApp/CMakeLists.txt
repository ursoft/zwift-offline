cmake_minimum_required(VERSION 3.24)
project(ZwiftApp)

include(cmake/thirdparty.cmake)

add_executable(ZwiftApp WIN32)
add_dependencies(ZwiftApp glfw)

FILE(GLOB PB_SRCS protobuf/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
  ${PB_SRCS}
)

FILE(GLOB Z_SRCS cpp/*)
target_sources(ZwiftApp PRIVATE
  ${Z_SRCS}
  res/Resource.h
  res/ZwiftApp.rc
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
  $ENV{NOESIS_DIR}/Include
  $ENV{WWISE_DIR}/SDK/include
)
target_link_directories(ZwiftApp PRIVATE 
  $ENV{NOESIS_DIR}/Lib/windows_x86_64
  $ENV{WWISE_DIR}/x64_vc170/${CMAKE_BUILD_TYPE}\(StaticCRT\)/lib
)

target_link_libraries(ZwiftApp PRIVATE 
  ${Protobuf_LIBRARIES}
  ${Boost_LIBRARIES}
  Noesis.lib #NoesisApp.lib
  Winmm.lib
  Opengl32.lib
  AkSoundEngine.lib AkMemoryMgr.lib AkMusicEngine.lib AkStreamMgr.lib
  ${CMAKE_CURRENT_BINARY_DIR}/glfw/lib/glfw3.lib
  CURL::libcurl
  OpenSSL::SSL OpenSSL::Crypto
  tinyxml2::tinyxml2
  ICU::uc
)
