set SRC_FOLDER=%~dp0
docker run --rm -it -v %SRC_FOLDER%..:c:\ZwiftApp -v %SRC_FOLDER%..\..\..\..\ZwiftApp_buildW10:c:\ZwiftApp_build vc_buildtools:win10