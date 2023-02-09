#include "ZwiftApp.h"
#define MAX_LOADSTRING 100
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
GLFWwindow *g_mainWindow;
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void CheckEnvironment() {
    PROCESSENTRY32W pe = {};
    pe.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE Toolhelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    const char *lpCaption = "ERROR ZA2";
    if (Process32FirstW(Toolhelp32Snapshot, &pe)) {
        while (true) {
            if (!_wcsicmp(pe.szExeFile, L"zwiftapp.exe")) {
                HANDLE hProcess = OpenProcess(0x410u, 0, pe.th32ParentProcessID);
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
        STARTUPINFOW StartupInfo = {};
        StartupInfo.cb = sizeof(StartupInfo);
        WCHAR cmdLauncher[] = L"ZwiftLauncher.exe";
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
struct zwiftUpdateContext {};
void doFrameWorldID(zwiftUpdateContext *ptr);
//TODO: __declspec(thread) - see tls0_dtr (GameAssertHandler::PushContext), TlsCallbackDtr
//TODO: global variables ctrs/dtrs: _initterm, some_global_ctr
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
    non_zwift::ConsoleHandler nz_ch(1024);
    if (auto ut = getenv("ZWIFT_UT"); ut && *ut == '1')
        nz_ch.LaunchUnitTests(__argc, __argv);
    if (auto zg = getenv("ZWIFT_GAME"); zg != nullptr && *zg == '0')
        return 0;
    CheckEnvironment();
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ZWIFTAPP, szWindowClass, MAX_LOADSTRING);
    if (hInstance) MyRegisterClass(hInstance);
    //--launcher_version=1.1.4 --token={"access_token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ._kPfXO8MdM7j0meG4MVzprSa-3pdQqKyzYMHm4d494w","expires_in":1000021600,"id_token":"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y","not-before-policy":1408478984,"refresh_expires_in":611975560,"refresh_token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ.5e1X1imPlVfXfhDHE_OGmG9CNGvz7hpPYPXcNkPJ5lw","scope":"","session_state":"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3","token_type":"bearer"}
    //OutputDebugStringA(lpCmdLine);
    std::vector<std::string> argv;
    argv.push_back(__argv[0]);
    std::istringstream cmd_parser(lpCmdLine);
    while (cmd_parser.good()) {
        std::string param;
        int ch = cmd_parser.peek();
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
    ZwiftInitialize(argv);
    int iteration = 0;
    HWND hMainWindow = nullptr;
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
            resize(g_mainWindow, w, h);
        }
    }
    g_bShutdown = true;
    EndGameSession(true);
    return 0;
}
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ZWIFTAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ZWIFTAPP));

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
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5000ms); //бред!
    AUDIO_Shutdown();
    ShutdownSingletons();
    exit(code);
}
void GameCritical_AbortJobs() {
    //TODO
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
void doFrameWorldID(zwiftUpdateContext *ptr) {
    //TODO
}
void ZWIFT_UpdateLoading(const uint16_t *, bool) {
    //TODO
}
//Unit Tests
TEST(SmokeTest, Linkage) { //testing if libs are linked properly
    AK::MemoryMgr::GetDefaultSettings(g_memSettings); //Wwize, not debuggable
    Noesis::GUI::SetLicense("NS_LICENSE_NAME", "NS_LICENSE_KEY"); //NOESIS, not debuggable
    EXPECT_TRUE(g_memSettings.pfAllocVM != nullptr) << "AK::MemoryMgr";

    protobuf::FeatureRequest fr; //Google protobuf
    fr.set_userid(123);
    auto bs = fr.ByteSize();
    EXPECT_EQ(2, bs) << "protobuf::ByteSize";

    boost::asio::io_context io_context; //boost ASIO, openssl
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    auto iocr = io_context.run(); //nothing to do
    EXPECT_EQ(0, iocr) << "io_context.run";

    z_stream strm{}; //zlib
    auto di = deflateInit(&strm, 6);
    EXPECT_EQ(0, di) << "deflateInit";

    tinyxml2::XMLDocument doc; //tinyxml2
    auto err_xml = doc.LoadFile("");
    EXPECT_EQ(tinyxml2::XML_ERROR_FILE_NOT_FOUND, err_xml) << "doc.LoadFile";

    auto curl = curl_easy_init(); //curl
    EXPECT_TRUE(curl != nullptr) << "curl_easy_init";
    curl_easy_cleanup(curl);

    auto dec = decContextTestEndian(0); //decNumber
    EXPECT_EQ(0, dec) << "decContextTestEndian";

    UErrorCode uc_err = U_AMBIGUOUS_ALIAS_WARNING; //ICU
    auto conv = ucnv_open("utf-8", &uc_err);
    EXPECT_EQ(U_AMBIGUOUS_ALIAS_WARNING, uc_err) << "ucnv_open err";
    EXPECT_TRUE(conv != nullptr) << "ucnv_open";
    ucnv_close(conv);

    Json::Value json(123); //jsoncpp
    std::string jss(json.toStyledString());
    EXPECT_STREQ("123\n", jss.c_str()) << "json.toStyledString";

    auto hMainWindow = glfwGetWin32Window(g_mainWindow); //glfw
    EXPECT_TRUE(hMainWindow  == nullptr) << "glfwGetWin32Window";

    char openssl_err[128];
    ERR_error_string_n(SSL_ERROR_WANT_READ, openssl_err, sizeof(openssl_err));
    EXPECT_STREQ("error:00000002:lib(0)::reason(2)", openssl_err) << "SSL_ERROR_WANT_READ";
}
