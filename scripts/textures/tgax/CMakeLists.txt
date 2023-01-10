add_executable (tgax "tgax.cpp" "tgax.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tgax PROPERTY CXX_STANDARD 20)
endif()

find_package(ZLIB REQUIRED)
target_link_libraries(tgax PRIVATE ZLIB::ZLIB)
# Linking setargv.obj to automatically enable globbing
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")