#gui
set (noesis_DIR ${CMAKE_CURRENT_BINARY_DIR}/../Noesis-3.0.12)
if(NOT EXISTS ${noesis_DIR})
  file(ARCHIVE_EXTRACT INPUT c:/Noesis-3.0.12.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/..)
endif()

#audio
set (wwise_DIR ${CMAKE_CURRENT_BINARY_DIR}/../Wwise_v2021.1.11)
if(NOT EXISTS ${wwise_DIR})
  file(ARCHIVE_EXTRACT INPUT c:/Wwise_v2021.1.11.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/..)
endif()

#protobuf
set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)

#boost
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

#decNumber
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