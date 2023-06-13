#include "ZwiftApp.h"
#define MAX_LOADSTRING 100
HINSTANCE        hInst;                         // current instance
WCHAR            szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR            szWindowClass[MAX_LOADSTRING]; // the main window class name
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CheckEnvironment() {
    PROCESSENTRY32W pe = {};
    pe.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE          Toolhelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    const char      *lpCaption = "ERROR ZA2";
    if (Process32FirstW(Toolhelp32Snapshot, &pe)) {
        while (true) {
            if (!_wcsicmp(pe.szExeFile, L"zwiftapp.exe")) {
                HANDLE  hProcess = OpenProcess(0x410u, 0, pe.th32ParentProcessID);
                wchar_t moduleName[MAX_PATH + 4] = {};
                if (hProcess) {
                    GetModuleFileNameExW(hProcess, 0i64, moduleName, 260i64);
                    CloseHandle(hProcess);
                }
                if (!wcsstr(moduleName, L"ZwiftLauncher.exe") && !wcsstr(moduleName, L"devenv.exe") && !wcsstr(moduleName, L"ZwiftHelper.exe"))
                    break;
            }
            if (!Process32NextW(Toolhelp32Snapshot, &pe)) {
                CloseHandle(Toolhelp32Snapshot);
                return;
            }
        }
        STARTUPINFOW        StartupInfo = {};
        StartupInfo.cb = sizeof(StartupInfo);
        WCHAR               cmdLauncher[] = L"ZwiftLauncher.exe";
        PROCESS_INFORMATION ProcessInformation = {};
        if (CreateProcessW(nullptr, cmdLauncher, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo, &ProcessInformation)) {
            CloseHandle(Toolhelp32Snapshot);
            ExitProcess(0);
        }
        auto LastError = GetLastError();
        printf("CreateProcess failed (%d).\n", LastError);
        lpCaption = "ERROR ZA1";
    }
    MessageBoxA(nullptr, "Could not run Update process. Zwift may not be up to date.", lpCaption, MB_ICONERROR);
    CloseHandle(Toolhelp32Snapshot);
}
void doFrameWorldID(zwiftUpdateContext *ptr);
//TODO: __declspec(thread) - see tls0_dtr (GameAssertHandler::PushContext), TlsCallbackDtr
//TODO: global variables ctrs/dtrs: _initterm, some_global_ctr
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
    non_zwift::ConsoleHandler nz_ch(1024);
    bool testsResult = true;
    bool testsOnly = strstr(lpCmdLine, "--gtest_also_run_disabled_tests") != nullptr;
    if (auto ut = getenv("ZWIFT_UT"); testsOnly || (ut && *ut == '1'))
        testsResult = nz_ch.LaunchUnitTests(__argc, __argv);
    if (auto zg = getenv("ZWIFT_GAME"); testsOnly || (zg != nullptr && *zg == '0'))
        return testsResult ? 0 : 1;
    CheckEnvironment();
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ZWIFTAPP, szWindowClass, MAX_LOADSTRING);
    if (hInstance) MyRegisterClass(hInstance);
    //--launcher_version=1.1.4 --token={"access_token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ._kPfXO8MdM7j0meG4MVzprSa-3pdQqKyzYMHm4d494w","expires_in":1000021600,"id_token":"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y","not-before-policy":1408478984,"refresh_expires_in":611975560,"refresh_token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ.5e1X1imPlVfXfhDHE_OGmG9CNGvz7hpPYPXcNkPJ5lw","scope":"","session_state":"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3","token_type":"bearer"}
    OutputDebugStringA(lpCmdLine);
    std::vector<std::string> argv;
    argv.push_back(__argv[0]);
    std::istringstream       cmd_parser(lpCmdLine);
    while (cmd_parser.good()) {
        std::string param;
        int         ch = cmd_parser.peek();
        if (ch != '"') {
            cmd_parser >> param;
        } else while (cmd_parser.good()) {
                ch = cmd_parser.peek();
                cmd_parser.ignore();
                if (ch == -1)
                    break;
                else if (ch == '\\') {
                    ch = cmd_parser.peek();
                    cmd_parser.ignore();
                    if (ch == -1)
                        break;
                } else if (ch == '"') {
                    if (param.length())
                        break;
                    else
                        continue;
                }
                param += (char)ch;
            }
        if (param.length()) argv.emplace_back(param);
    }
    _set_FMA3_enable(0);    
    DWORD t1 = GetTickCount();
    ZwiftInitialize(argv);
    t1 = GetTickCount() - t1;
    printf("ZwiftInitialize took: %dms\n", t1);
    int   iteration = 0;
    HWND  hMainWindow = nullptr;
    float fcounter = 0;
    zwiftUpdateContext zuc = {};
    while (!glfwWindowShouldClose(g_mainWindow)) {
        if (g_MaintainFullscreenForBroadcast) {
            hMainWindow = glfwGetWin32Window(g_mainWindow);
            if (GetActiveWindow() != hMainWindow) {
                fcounter += 1 / 60.0f;
                if (fcounter > 30.0f) {
                    fcounter = 0;
                    ShowWindow(hMainWindow, SW_SHOWMAXIMIZED);
                    SetFocus(hMainWindow);
                    SetActiveWindow(hMainWindow);
                }
            }
        }
        doFrameWorldID(&zuc);
        if (++iteration == 1) {
            int w, h;
            glfwGetWindowSize(g_mainWindow, &w, &h);
            WindowSizeCallback(g_mainWindow, w, h);
        }
    }
    g_bShutdown = true;
    EndGameSession(true);
    return 0;
}
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ZWIFTAPP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ZWIFTAPP));
    return RegisterClassExW(&wcex);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_ABOUT:
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About, 0);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
void ZwiftExit(int code) {
    GameCritical::AbortJobs();
    //using namespace std::chrono_literals;
    //std::this_thread::sleep_for(5000ms); //бред!
    AUDIO_Shutdown();
    ShutdownSingletons();
    exit(code);
}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
bool g_bDisableConsoleRendering;
void Render(double tim) {
    //TODO
    if (!g_bDisableConsoleRendering /*read-only*/ && g_bShowConsole)
        CONSOLE_Draw(0.0f, 0.03333333333f);
    //TODO
}
void LoadingRender(float time, const UChar *text) {
    if (time <= 1.0) {
        GFX_UploadShaderVEC4(GSR_24, g_Vec4White, 0);
        GFX_SetScissorTestEnable(false);
        static auto g_seeThroughHandle = GFX_CreateTextureFromTGAFile("UI/z_logo_see_through.tga", -1, true);
        int         width = 1280, height = 720;
        glfwGetWindowSize(g_mainWindow, &width, &height);
        glViewport(0, 0, width, height);
        width = 1280;
        height = 720;
        GFX_SetDepthTestEnable(false);
        GFX_SetDepthWrite(false);
        GFX_SetBlendFunc(GBO_FUNC_ADD, GB_SRC_ALPHA, GB_ONE_MINUS_SRC_ALPHA);
        GFX_SetAlphaBlendEnable(true);
        GFX_SetCullMode(GFC_NONE);
        GFX_SetShader(g_DrawTexturedShaderHandle);
        GFX_ActivateTexture(g_seeThroughHandle, 0xFFFFFFFF, nullptr, TWM_REPEAT);
        GFX_Draw2DQuad(0.0f, 0.0f, width, height, ((uint8_t)((1.0f - time) * 255.0) << 24) | 0xFFFFFF, true);
        auto v4 = powf(time, 0.7f);
        auto v5 = (v4 * 2.70158f - 1.70158f) * time * v4 * v4;
        if (v5 > 0.0f)
            v5 = v5 * (v5 * 4.5f * v5 + 1.0f);
        auto      v6 = v5 * 4096.0f + 512.0f;
        auto      l = (width - v6) * 0.5f;
        auto      t = (height - v6) * 0.5f;
        const int loaderColor = 0xFFCC9211;
        GFX_Draw2DQuad_720p(l, t, v6, v6, 0.0, 0.0, 1.0, 1.0, loaderColor, 0.0, -1, 0);
        auto      v9 = l + v6;
        auto      v10 = t + v6;
        if (l >= 0.0f)
            GFX_Draw2DQuad_UI(0.0f, 0.0f, l + 1.0f, (float)height, loaderColor);
        if (v9 <= (float)width)
            GFX_Draw2DQuad_UI(v9 - 1.0f, 0.0f, width - v9 + 2.0f, (float)height, loaderColor);
        if (t >= 0.0f)
            GFX_Draw2DQuad_UI(l, 0.0f, v6, t + 1.0f, loaderColor);
        if (v10 <= (float)height) {
            GFX_Draw2DQuad_UI(l, v10 - 1.0f, v6, height - v10 + 2.0f, loaderColor);
        }
        if (text)
            g_GiantFontW.RenderWString(width * 0.5f, height * 0.5f + 170.0f, text,
                                       ((int)(std::clamp(1.0f - time - time, 0.0f, 1.0f) * 255.0f) << 24) | 0xFFFFFF,
                                       1, 0.6666f, false, false, true);
        GFX_SetDepthTestEnable(true);
        GFX_SetDepthWrite(true);
    }
}
void doFrameWorldID(zwiftUpdateContext *ptr) {
    //TODO fake1 {
    GFX_Begin();
    VRAM_EndRenderTo(0);
    GFX_Clear(60);
    glClearColor(0.0f, 0.0f, 0.9f, 1.0f);
    int width, height;
    glfwGetWindowSize(g_mainWindow, &width, &height);
    glViewport(0, 0, width, height);
    GFX_SetDepthTestEnable(false);
    GFX_SetDepthWrite(false);
    //TODO fake1 }

    Render(timeGetTime() / 1000.0f);

    //TODO fake2 {
    GFX_SetDepthTestEnable(true);
    GFX_SetDepthWrite(true);
    GFX_Present();
    GFX_EndFrame();
    //TODO fake2 }
}
struct UpdateLoadingItem {
    const UChar *m_msg;
    bool        m_bShown;
};
void ZWIFT_UpdateLoading(const UChar *text, bool last) {
    g_mDownloader.Update();
    static UpdateLoadingItem g_UpdateLoadingDB[] = {
        { GetTextW("LOC_LOADING_QUIP_0") }, { GetTextW("LOC_LOADING_QUIP_1") }, { GetTextW("LOC_LOADING_QUIP_2") }, { GetTextW("LOC_LOADING_QUIP_3") },
        { GetTextW("LOC_LOADING_QUIP_4") }, { GetTextW("LOC_LOADING_QUIP_5") }, { GetTextW("LOC_LOADING_QUIP_6") }, { GetTextW("LOC_LOADING_QUIP_7") },
        { GetTextW("LOC_LOADING_QUIP_8") }, { GetTextW("LOC_LOADING_QUIP_9") }, { GetTextW("LOC_LOADING_QUIP_10") }, { GetTextW("LOC_LOADING_QUIP_11") },
        { GetTextW("LOC_LOADING_QUIP_12") }, { GetTextW("LOC_LOADING_QUIP_13") }, { GetTextW("LOC_LOADING_QUIP_14") }, { GetTextW("LOC_LOADING_QUIP_15") },
        { GetTextW("LOC_LOADING_QUIP_16") }, { GetTextW("LOC_LOADING_QUIP_17") }, { GetTextW("LOC_LOADING_QUIP_18") }, { GetTextW("LOC_LOADING_QUIP_19") },
        { GetTextW("LOC_LOADING_QUIP_20") }, { GetTextW("LOC_LOADING_QUIP_21") }, { GetTextW("LOC_LOADING_QUIP_22") }, { GetTextW("LOC_LOADING_QUIP_23") },
        { GetTextW("LOC_LOADING_QUIP_24") }, { GetTextW("LOC_LOADING_QUIP_25") }, { GetTextW("LOC_LOADING_QUIP_26") }, { GetTextW("LOC_LOADING_QUIP_27") },
        { GetTextW("LOC_LOADING_QUIP_28") }
    };
    static uint32_t          g_lastTime, g_txtChanges;
    uint32_t now = timeGetTime();
    if (now - g_lastTime >= 500.0 || last) {
        ++g_txtChanges;
        if (!text) {
            auto dbi = timeGetTime() % _countof(g_UpdateLoadingDB);
            for (auto i = 0; i < 4; ++i) {
                if (!g_UpdateLoadingDB[dbi].m_bShown)
                    break;
                dbi = (dbi + 1) % _countof(g_UpdateLoadingDB);
            }
            text = g_UpdateLoadingDB[dbi].m_msg;
            g_UpdateLoadingDB[dbi].m_bShown = true;
        }
        GFX_Begin();
        VRAM_EndRenderTo(0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        GFX_Clear(60);
        LoadingRender(0.0f, text);
        //OMIT v6 = 0;
        //WDT_Tick(&v6);
        GFX_Present();
        GFX_EndFrame();
        g_lastTime = now;
    }
}
void MsgBoxAndExit(const char *lpText) {
    MessageBoxA(nullptr, lpText, "ERROR", MB_ICONSTOP);
    exit(-1);
}

//Unit Tests
TEST(SmokeTest, Linkage) {                                        //testing if libs are linked properly
    AK::MemoryMgr::GetDefaultSettings(g_memSettings);             //Wwize, not debuggable
    Noesis::GUI::SetLicense("NS_LICENSE_NAME", "NS_LICENSE_KEY"); //NOESIS, not debuggable
    EXPECT_TRUE(g_memSettings.pfAllocVM != nullptr) << "AK::MemoryMgr";

    protobuf::FeatureRequest fr; //Google protobuf
    fr.set_str_player_id("123");
    auto bs = fr.ByteSizeLong();
    EXPECT_EQ(5, bs) << "protobuf::ByteSize";

    boost::asio::io_context        io_context; //boost ASIO, openssl
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ssl::context      ctx(boost::asio::ssl::context::sslv23);
    auto iocr = io_context.run(); //nothing to do
    EXPECT_EQ(0, iocr) << "io_context.run";

    z_stream strm{}; //zlib
    auto     di = deflateInit(&strm, 6);
    EXPECT_EQ(0, di) << "deflateInit";

    tinyxml2::XMLDocument doc; //tinyxml2
    auto err_xml = doc.LoadFile("");
    EXPECT_EQ(tinyxml2::XML_ERROR_FILE_NOT_FOUND, err_xml) << "doc.LoadFile";

    auto curl = curl_easy_init(); //curl
    EXPECT_TRUE(curl != nullptr) << "curl_easy_init";
    curl_easy_cleanup(curl);

    //auto dec = decContextTestEndian(0); //decNumber
    //EXPECT_EQ(0, dec) << "decContextTestEndian";

    UErrorCode uc_err = U_AMBIGUOUS_ALIAS_WARNING; //ICU
    auto       conv = ucnv_open("utf-8", &uc_err);
    EXPECT_EQ(U_AMBIGUOUS_ALIAS_WARNING, uc_err) << "ucnv_open err";
    EXPECT_TRUE(conv != nullptr) << "ucnv_open";
    ucnv_close(conv);

    Json::Value json(123); //jsoncpp
    std::string jss(json.toStyledString());
    EXPECT_STREQ("123\n", jss.c_str()) << "json.toStyledString";

    auto hMainWindow = glfwGetWin32Window(g_mainWindow); //glfw
    EXPECT_TRUE(hMainWindow == nullptr) << "glfwGetWin32Window";

    char openssl_err[128];
    ERR_error_string_n(SSL_ERROR_WANT_READ, openssl_err, sizeof(openssl_err));
    EXPECT_STREQ("error:00000002:lib(0)::reason(2)", openssl_err) << "SSL_ERROR_WANT_READ";
}
