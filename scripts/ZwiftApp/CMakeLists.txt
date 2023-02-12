cmake_minimum_required(VERSION 3.24)
project(ZwiftApp)

add_compile_options("/nologo")
include(cmake/thirdparty.cmake)

add_executable(${PROJECT_NAME} WIN32)
add_dependencies(${PROJECT_NAME} glfw decNumber)

FILE(GLOB PB_SRCS protobuf/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
  ${PB_SRCS}
)

FILE(GLOB Z_SRCS cpp/*)
target_sources(${PROJECT_NAME} PRIVATE
  ${Z_SRCS}
  res/Resource.h
  res/${PROJECT_NAME}.rc
  ${PROTO_SRCS} ${PROTO_HDRS}
 "cpp/GDE.cpp")

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
set_property(TARGET ${PROJECT_NAME} PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_NOESIS_DIR}/Include
  ${CMAKE_WWISE_DIR}/SDK/include
  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368
)
target_link_directories(${PROJECT_NAME} PRIVATE 
  ${CMAKE_NOESIS_DIR}/Lib/windows_x86_64
  ${CMAKE_WWISE_DIR}/x64_vc170/$ENV{LIB_CFG}\(StaticCRT\)/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  ${Protobuf_LIBRARIES} #protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
  ${Boost_LIBRARIES}
  Noesis.lib #NoesisApp.lib
  Winmm.lib Shcore
  Opengl32.lib GLEW::GLEW
  AkSoundEngine.lib AkMemoryMgr.lib AkMusicEngine.lib AkStreamMgr.lib
  ${CMAKE_CURRENT_BINARY_DIR}/glfw/lib/glfw3.lib
  CURL::libcurl
  OpenSSL::SSL OpenSSL::Crypto
  tinyxml2::tinyxml2
  ICU::uc ICU::dt ICU::in ICU::uc ICU::io
  decNumber
  jsoncpp_object jsoncpp_static JsonCpp::JsonCpp
  GTest::gtest 
)

install(TARGETS ${PROJECT_NAME} DESTINATION ./)
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ./ OPTIONAL)
install(FILES ${CMAKE_NOESIS_DIR}/Bin/windows_x86_64/Noesis.dll DESTINATION ./)

target_precompile_headers(${PROJECT_NAME} PRIVATE cpp/pch.h)