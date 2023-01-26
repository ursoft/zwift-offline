# escape=`

# Use the latest Windows Server Core 2022 image.
FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN `
    # Download the Build Tools bootstrapper.
    curl -SL --output vs_buildtools.exe https://aka.ms/vs/17/release/vs_buildtools.exe `
    `
    && (start /w vs_buildtools.exe --quiet --wait --norestart --includeRecommended --nocache `
        --installPath "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools" `
        --add Microsoft.VisualStudio.Component.Windows10SDK `
        --add Microsoft.VisualStudio.Component.Windows10SDK.20348 `
        --add Microsoft.VisualStudio.Component.Windows11SDK.22000 `
        --add Microsoft.Component.VC.Runtime.UCRTSDK `
        --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        --add Microsoft.VisualStudio.Component.VC.CMake.Project `
        --add Microsoft.VisualStudio.Component.VC.ASAN `
        --add Microsoft.VisualStudio.Component.VC.CoreBuildTools `
        --add Microsoft.VisualStudio.Component.VC.CoreIde `
        --add Microsoft.VisualStudio.Component.VC.Redist.14.Latest `
        --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.10240 `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.10586 `
        --remove Microsoft.VisualStudio.Component.Windows10SDK.14393 `
        --remove Microsoft.VisualStudio.Component.Windows81SDK `
        || IF "%ERRORLEVEL%"=="3010" EXIT 0)

#git layer
RUN curl -SL -o install_git.exe https://github.com/git-for-windows/git/releases/download/v2.39.1.windows.1/Git-2.39.1-64-bit.exe && install_git.exe /VERYSILENT /NORESTART && set path=%path%;c:\Program Files\Git\bin

#vcpkg layer + build static boost x64
RUN git clone https://github.com/Microsoft/vcpkg.git `
    && .\vcpkg\bootstrap-vcpkg.bat  -disableMetrics `
    && C:\vcpkg\vcpkg.exe install boost:x64-windows-static
    
#protobuf layer
RUN C:\vcpkg\vcpkg.exe install protobuf:x64-windows-static

ENTRYPOINT ["cmd.exe", "/k", "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\Common7\\Tools\\VsDevCmd.bat", "-arch=amd64", "-host_arch=amd64" ]