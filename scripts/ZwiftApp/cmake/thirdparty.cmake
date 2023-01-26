#gui
include(FetchContent)
FetchContent_Declare( #3.0.12
  noesis
  URL https://www.googleapis.com/drive/v3/files/1V4u2MFtPojK-U0PaV1Ot_OaEfb2GTm2y?alt=media&key=AIzaSyCe8S1RwtJK0aPavBFJzaM_or6L_dENsRo
  DOWNLOAD_NAME Noesis-3.0.12.zip
)
FetchContent_Populate(noesis)

#audio
FetchContent_Declare( #2021.1.11
  wwise
  URL https://www.googleapis.com/drive/v3/files/1Q5lhYKAZ6gLuLfBsNJ77EUVkuLehiaB-?alt=media&key=AIzaSyCe8S1RwtJK0aPavBFJzaM_or6L_dENsRo
  DOWNLOAD_NAME Wwise_v2021.1.11.zip
)
FetchContent_Populate(wwise)

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
