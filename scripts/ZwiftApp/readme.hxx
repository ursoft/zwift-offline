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

Coverage
~~~~~~~~
C:\Users\builder\AppData\Roaming\OpenCppCoverage\config.json
{"BasicSettingController":{"Data":{"ProgramToRun":"C:\\Users\\builder\\source\\repos\\zwift-offline\\scripts\\ZwiftApp\\1.32.1_106405-Debug\\ZwiftApp.exe","OptionalWorkingDirectory":"C:\\Users\\builder\\source\\repos\\zwift-offline\\scripts\\ZwiftApp\\1.32.1_106405-Debug","Arguments":"--gtest_also_run_disabled_tests --gtest_filter=*SmokeTestNet*","CompileBeforeRunning":false,"OptimizedBuild":false},"IsSelectedByProjectPath":{}},"FilterSettingController":{"AdditionalSourcePatterns":[{"Value":"*ZNet*"}],"AdditionalModulePatterns":[{"Value":"*ZwiftApp*"}],"ExcludedSourcePatterns":[],"ExcludedModulePatterns":[],"UnifiedDiffs":[]},"ImportExportSettingController":{"Exports":[],"InputCoverages":[],"CoverChildrenProcesses":false,"AggregateByFile":true},"MiscellaneousSettingController":{"OptionalConfigFile":null,"LogTypeValue":0,"ContinueAfterCppExceptions":true}}

current decompilation stats
~~~~~~~~~~~~~~~~~~~~~~~~~~~
future/lib/total code(kb): 9352/07012/16364, badNames=39764; libs/total: 55%: 34618/62119
future/lib/total code(kb): 9173/07191/16364, badNames=38464; libs/total: 56%: 35017/62119
future/lib/total code(kb): 9149/07214/16364, badNames=38269; libs/total: 56%: 35168/62119 - protobuf constructors
future/lib/total code(kb): 9149/07214/16364, badNames=37623; libs/total: 56%: 35168/62119 - protobuf vtable names
future/lib/total code(kb): 9110/07253/16364, badNames=37250; libs/total: 56%: 35246/62119 - 1:1 (1st iter)
future/lib/total code(kb): 9094/07270/16364, badNames=37214; libs/total: 56%: 35346/62119 - done with TcpClient
future/lib/total code(kb): 9068/07296/16364, badNames=37155; libs/total: 57%: 35419/62119 - done with AuxiliaryController
future/lib/total code(kb): 8997/07367/16364, badNames=37100; libs/total: 57%: 35708/62119 - NetworkClientImpl 30%
future/lib/total code(kb): 8959/07404/16364, badNames=37062; libs/total: 57%: 35846/62119 - NetworkClientImpl 70%
future/lib/total code(kb): 8932/07432/16364, badNames=36989; libs/total: 57%: 35913/62119 - NetworkClientImpl 100%, NETWORK_* and DataRecorder 10%
future/lib/total code(kb): 8888/07477/16365, badNames=36921; libs/total: 57%: 36025/62129 - NetworkService & RPCs: 100%
future/lib/total code(kb): 8818/07546/16365, badNames=36895; libs/total: 58%: 36253/62129 - ZNet: 100%
future/lib/total code(kb): 8810/07555/16365, badNames=36895; libs/total: 58%: 36265/62129 - DataRecorder 95%
future/lib/total code(kb): 8776/07588/16364, badNames=36770; libs/total: 58%: 36345/62107 - ZNETWORK_INTERNAL_ProcessReceivedWorldAttribute
future/lib/total code(kb): 8756/07608/16364, badNames=36640; libs/total: 58%: 36395/62107 - ZNETWORK_INTERNAL_ProcessPhoneInput 50%
future/lib/total code(kb): 8740/07624/16364, badNames=36609; libs/total: 58%: 36420/62107 - ZNETWORK_*: 100%, CNL unit tests 19%
future/lib/total code(kb): 8735/07629/16364, badNames=36586; libs/total: 58%: 36443/62107 - XMLDoc unit tests 99%, CNL unit tests 38%
future/lib/total code(kb): 8677/07687/16364, badNames=36412; libs/total: 59%: 36683/62107 - Started BLE
future/lib/total code(kb): 5850/10514/16364, badNames=36389; libs/total: 66%: 41378/62107 - OMITted all BLE devices except FTMS, Elite steering and TACX
future/lib/total code(kb): 5839/10523/16363, badNames=35604; libs/total: 66%: 41412/62107 - names from vtbls
future/lib/total code(kb): 5697/10665/16363, badNames=35381; libs/total: 67%: 41825/62107 - Camera & math
future/lib/total code(kb): 5656/10708/16364, badNames=35193; libs/total: 67%: 41873/62108 - GameWorld, CFont2D

NEXT: CNL unit tests, BLE & ANT, GFX