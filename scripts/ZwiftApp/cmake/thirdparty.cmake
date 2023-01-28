cmake_path(SET CMAKE_NOESIS_DIR NORMALIZE "$ENV{NOESIS_DIR}")
cmake_path(SET CMAKE_WWISE_DIR NORMALIZE "$ENV{WWISE_DIR}")

#gui
if(NOT EXISTS ${CMAKE_NOESIS_DIR})
  file(ARCHIVE_EXTRACT INPUT c:/$ENV{NOESIS_ID}.zip DESTINATION ${CMAKE_NOESIS_DIR}/..)
endif()

#audio
if(NOT EXISTS ${CMAKE_WWISE_DIR})
  file(ARCHIVE_EXTRACT INPUT c:/$ENV{WWISE_ID}.zip DESTINATION ${CMAKE_WWISE_DIR}/..)
endif()

#protobuf
set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)

#boost (ZLIB::ZLIB inside?)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(Boost_INCLUDE_DIR ${_VCPKG_INSTALLED_DIR}/x64-windows-static/include)
find_package(Boost 1.51 REQUIRED filesystem system)

#opengl wrapper
include(ExternalProject)
ExternalProject_Add(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        1861cf4975b7f2a43f7a342127daf3ea1d26fce7
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/glfw
                 -DGLFW_BUILD_EXAMPLES:BOOL=OFF
                 -DGLFW_BUILD_TESTS:BOOL=OFF
                 -DGLFW_BUILD_DOCS:BOOL=OFF
                 -DUSE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF
                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/glfw/lib/glfw3.lib
)

#decNumber (not sure it is used here)
include(FetchContent)
FetchContent_Declare(decNumber
  GIT_REPOSITORY https://github.com/dnotq/decNumber.git
  SOURCE_DIR     ${CMAKE_CURRENT_BINARY_DIR}/decNumber
  GIT_TAG        8852ef5b4339d611f493e94a791f3543b6ee81cd
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(decNumber)

find_package(CURL CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc dt in io)
find_package(jsoncpp CONFIG REQUIRED) #./vcpkg install jsoncpp:x64-windows-static --editable

add_library(decNumber STATIC
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decimal64.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decimal64.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decimal32.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decimal32.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decimal128.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decimal128.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decSingle.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decSingle.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decQuad.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decQuad.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decPacked.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decPacked.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decNumberLocal.h
  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decNumber.h
  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decNumber.c
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decDouble.h
#  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decDouble.c
  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decDPD.h
  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decContext.h
  ${CMAKE_CURRENT_BINARY_DIR}/decNumber/decNumber-icu-368/decContext.c
)

target_compile_definitions(decNumber PUBLIC DECNUMDIGITS=34 DECPMAX=34 DECSTRING=45)
set_property(TARGET decNumber PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")