Docker
======
1. Install Docker Desktop (https://docs.docker.com/desktop/install/windows-install/). I used Hyper-V instead of WSL-2 (I suppose, windows containers should use it).
2. Switch to windows containers: 
HOST>"C:\Program Files\Docker\Docker\DockerCLI.exe" -SwitchDaemon
 (https://stackoverflow.com/questions/68984133/error-failed-to-solve-with-frontend-dockerfile-v0-failed-to-create-llb-defini#comment128300427_68984134)
3. Go with command processor to this folder and execute:
HOST>docker build -t vc_buildtools:latest -m 2GB .
(https://learn.microsoft.com/ru-ru/visualstudio/install/build-tools-container?view=vs-2022)
4. Check your vs_buildtools image with hello world program:
HOST>docker run --rm -it -v %cd%:c:\ZwiftApp -v %cd%\..\..\..\ZwiftApp_build:c:\ZwiftApp_build vc_buildtools
C:\>echo #include ^<iostream^>>main.cpp
C:\>echo int main() { std::cout ^<^< ^"Hello World!^"; return 0; }>>main.cpp
C:\>cl /EHsc main.cpp
C:\>main.exe
Hello World!
C:\>ZwiftApp\build.cmd

vcpkg notes
===========
Use flag --editable to debug in sources

Static variables
================
(NtCurrentTeb()->ThreadLocalStoragePointer + TlsIndex)[72]  - __declspec(thread) bool g_CurrentThreadNameSet
(NtCurrentTeb()->ThreadLocalStoragePointer + TlsIndex)[73]  - __declspec(thread) bool (??? around g_Task_cv_mutex lock@ZwiftAbort)
(NtCurrentTeb()->ThreadLocalStoragePointer + TlsIndex)[192] - static variables pOnce counter (__declspec(thread) int _Init_thread_epoch)
(NtCurrentTeb()->ThreadLocalStoragePointer + TlsIndex)[196] - __declspec(thread) bool (??? _dyn_tls_on_demand_init) also 136,144 etc@tls_stuff,tls_stuff2,3,4,5,6,7,8 TlsCallbackCtr
(NtCurrentTeb()->ThreadLocalStoragePointer + TlsIndex)[208] - __declspec(thread) bool (??? _tlregdtor) also 224

Other static libs
~~~~~~~~~~~~~~~~~
turned off https://github.com/getsentry/sentry-native

current decompilation stats
~~~~~~~~~~~~~~~~~~~~~~~~~~~
future/lib/total code(kb): 9352/7012/16364, badNames=39764; libs/total: 55%: 34618/62119
future/lib/total code(kb): 9173/7191/16364, badNames=38464; libs/total: 56%: 35017/62119
future/lib/total code(kb): 9149/7214/16364, badNames=38269; libs/total: 56%: 35168/62119 - protobuf constructors
future/lib/total code(kb): 9149/7214/16364, badNames=37623; libs/total: 56%: 35168/62119 - protobuf vtable names
future/lib/total code(kb): 9110/7253/16364, badNames=37250; libs/total: 56%: 35246/62119 - 1:1 (1st iter)
future/lib/total code(kb): 9094/7270/16364, badNames=37214; libs/total: 56%: 35346/62119 - done with TcpClient
future/lib/total code(kb): 9068/7296/16364, badNames=37155; libs/total: 57%: 35419/62119 - done with AuxiliaryController