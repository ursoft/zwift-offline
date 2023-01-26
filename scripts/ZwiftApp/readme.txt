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
