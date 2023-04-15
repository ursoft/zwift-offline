set DOCKER_BUILDKIT=0
set SRC_FOLDER=%~dp0
docker build -t vc_buildtools:latest -m 10GB %SRC_FOLDER%