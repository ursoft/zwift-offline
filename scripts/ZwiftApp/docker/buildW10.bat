set DOCKER_BUILDKIT=0
set SRC_FOLDER=%~dp0
docker build -f DockerfileW10host -t vc_buildtools:win10 --build-arg MYBASE=mcr.microsoft.com/windows/server:win10-21h1-preview -m 10GB %SRC_FOLDER%