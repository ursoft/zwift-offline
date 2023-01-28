@echo off
setlocal enabledelayedexpansion

set CFG=Debug
set LIB_CFG=Debug
if "%1"=="Release" (
  set CFG=Release
  set LIB_CFG=Release
) else (
  if "%1"=="RelWithDebInfo" (
    set CFG=RelWithDebInfo
    set LIB_CFG=Release
  ) else (
    if "%1" neq "" (
      echo build.cmd ERROR: cfg '%1' is not supported yet (use Debug, Release or RelWithDebInfo)
      exit /b 1
    )
  )
)

set SRC_FOLDER=%~dp0
call %SRC_FOLDER%cmn_libs.cmd

rem c:\ZwiftApp is docker-container-only folder!!!
if exist c:\ZwiftApp (
  git config --global --add safe.directory C:/zwiftapp_build/%CFG%/glfw-prefix/src/glfw
  git config --global --add safe.directory C:/zwiftapp_build/%CFG%/decNumber
  set ENV=docker
  set BUILD_FOLDER=c:\ZwiftApp_build\%CFG%
  set VCPKG_ROOT=c:\vcpkg
) else (
  set ENV=host
  set "BUILD_FOLDER=%SRC_FOLDER%..\..\..\ZwiftApp_build\%CFG%-host"
  set VCPKG_ROOT_FB=c:\Users\builder\projects\ALL\vcpkg
)

for %%i in ("!BUILD_FOLDER!") do SET "BUILD_FOLDER=%%~fi"
set "BUILD_FOLDER_CMN_LIBS=!BUILD_FOLDER!\.."
for %%i in ("!BUILD_FOLDER_CMN_LIBS!") do SET "BUILD_FOLDER_CMN_LIBS=%%~fi"
set NOESIS_DIR=!BUILD_FOLDER_CMN_LIBS!\!NOESIS_ID!
set WWISE_DIR=!BUILD_FOLDER_CMN_LIBS!\!WWISE_ID!

if "%ENV%"=="host" (
  if not exist !NOESIS_DIR! (
    echo build.cmd ERROR: You need to provide !NOESIS_DIR! folder ^(see cmake\thirdparty.cmake and Dockerfile^)
    exit /b 1
  )
  if not exist !WWISE_DIR! (
    echo build.cmd ERROR: You need to provide !WWISE_DIR! folder ^(see cmake\thirdparty.cmake and Dockerfile^)
    exit /b 1
  )
)

if "%VCPKG_ROOT%"=="" (
  echo build.cmd Warning: VCPKG_ROOT is empty, using fallback: VCPKG_ROOT=%VCPKG_ROOT_FB%
  set VCPKG_ROOT=%VCPKG_ROOT_FB%
)
if "%VSCMD_VER%"=="" (
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)
if "%VSCMD_VER%"=="" (
  echo build.cmd ERROR: You need to install Visual Studio 2022 and call VsDevCmd x64 before launching build.cmd
  exit /b 1
)

@echo on

     cmake      -B %BUILD_FOLDER% -S %SRC_FOLDER% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_BUILD_TYPE=%CFG% -DBoost_NO_WARN_NEW_VERSIONS=1 -GNinja ^
  && cmake --build %BUILD_FOLDER%
