set CFG=Debug
if "%1"=="Release" (
  set CFG=Release
)

git config --global --add safe.directory C:/zwiftapp_build/%CFG%/glfw-prefix/src/glfw
git config --global --add safe.directory C:/zwiftapp_build/%CFG%/decNumber

cmake -B c:\ZwiftApp_build\%CFG% -S c:\ZwiftApp -DCMAKE_TOOLCHAIN_FILE=c:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_BUILD_TYPE=%CFG% -DBoost_NO_WARN_NEW_VERSIONS=1 -GNinja && cmake --build c:\ZwiftApp_build\%CFG%
