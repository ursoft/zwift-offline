#include "ZwiftApp.h"

#define MAX_LOADSTRING 100
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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
        printf /*sub_1400DCA30*/("CreateProcess failed (%d).\n", LastError);
        lpCaption = "ERROR ZA1";
    }
    MessageBoxA(nullptr, "Could not run Update process. Zwift may not be up to date.", lpCaption, MB_ICONERROR);
    CloseHandle(Toolhelp32Snapshot);
}
GLFWwindow *g_mainWindow;
bool InitApplicationOK = true;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
    CheckEnvironment();
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ZWIFTAPP, szWindowClass, MAX_LOADSTRING);
    if (hInstance) MyRegisterClass(hInstance);

    int64_t v6 = 0;
    size_t cmdLen = strlen(lpCmdLine);
    std::string cmdLine(lpCmdLine, cmdLen);

#if 0
    v9 = v56;
    p_wCmdLine = (__int128*)&wCmdLine;
    v11 = wCmdLine;
    v12 = v55;
    if (v56 >= 0x10)
        p_wCmdLine = (__int128*)wCmdLine;
    v44 = 0i64;
    *(_OWORD*)Block = 0i64;
    if (v55 >= 0x10)
    {
        v13 = v55 | 0xF;
        if ((v55 | 0xF) > 0x7FFFFFFFFFFFFFFFi64)
            v13 = 0x7FFFFFFFFFFFFFFFi64;
        *(_QWORD*)&v60 = sub_1400C2FF0(v13 + 1);
        v14 = (__int128*)v60;
        memmove((void*)v60, p_wCmdLine, v12 + 1);
    }
    else
    {
        v13 = 15i64;
        v60 = *p_wCmdLine;
        v14 = (__int128*)v60;
    }
    v15 = 0;
    v62 = v13;
    v61 = v12;
    v57[0] = 0i64;
    *(_QWORD*)v46 = &unk_141259E18;
    v58 = 0i64;
    v48 = (int*)&unk_1411F7940;
    v59 = 15i64;
    v51[0] = (__int64)&std::istream::`vftable';
        v51[1] = 0i64;
    v51[6] = 0i64;
    v52 = 0i64;
    v50 = 128;
    v47 = 0i64;
    sub_1400CFD90((struct std::ios_base*)v51);
    *(__int64*)((char*)&v49[-1] + v48[1]) = (__int64)&std::ostream::`vftable';
        * (_DWORD*)((char*)&v47 + v48[1] + 4) = v48[1] - 16;
    *(_QWORD*)((char*)v46 + *(int*)(*(_QWORD*)v46 + 4i64)) = &std::iostream::`vftable';
        * (_DWORD*)((char*)&v45.hIconSm + *(int*)(*(_QWORD*)v46 + 4i64) + 4) = *(_DWORD*)(*(_QWORD*)v46 + 4i64) - 32;
    *(_QWORD*)((char*)v46 + *(int*)(*(_QWORD*)v46 + 4i64)) = &std::basic_stringstream<char, std::char_traits<char>, std::allocator<char>>::`vftable';
        * (_DWORD*)((char*)&v45.hIconSm + *(int*)(*(_QWORD*)v46 + 4i64) + 4) = *(_DWORD*)(*(_QWORD*)v46 + 4i64) - 152;
    sub_1400CFA50(v49);
    v16 = &v60;
    v49[0] = (__int64)&std::stringbuf::`vftable';
        if (v13 >= 0x10)
            v16 = v14;
    sub_14010F6B0(v49, v16, v12, 0i64);
    v54 = 23586;
    wCmdLine = v57;
    v19 = sub_1406B4B10((int)v46, (int)&wCmdLine, v17, v18, v40, SBYTE8(v40), (int)v41, v42);
    v20 = (void**)Block[1];
    if ((*(_BYTE*)(*(int*)(*(_QWORD*)v19 + 4i64) + v19 + 16) & 6) == 0)
    {
        do
        {
            if (v20 == (void**)v6)
            {
                sub_1400C62E0(Block, v20, v57);
                v6 = v44;
                v20 = (void**)Block[1];
            }
            else
            {
                sub_1400C5620(v20, v57);
                v20 += 4;
                Block[1] = v20;
            }
            v54 = 23586;
            wCmdLine = v57;
            v23 = sub_1406B4B10((int)v46, (int)&wCmdLine, v21, v22, v40, SBYTE8(v40), (int)v41, v42);
        } while ((*(_BYTE*)(*(int*)(*(_QWORD*)v23 + 4i64) + v23 + 16) & 6) == 0);
    }
    v41 = 0i64;
    v40 = 0i64;
    v24 = *(_QWORD*)sub_140EA84B8();
    sub_1400C7DD0(&v40, 0i64, v24);
    v25 = (_QWORD*)*((_QWORD*)&v40 + 1);
    v26 = (void**)Block[0];
    for (i = (void**)Block[0]; i != v20; i += 4)
    {
        v28 = i;
        if ((unsigned __int64)i[3] >= 0x10)
            v28 = *i;
        wCmdLine = v28;
        if (v25 == v41)
        {
            sub_1400C7DD0(&v40, v25, &wCmdLine);
            v25 = (_QWORD*)*((_QWORD*)&v40 + 1);
        }
        else
        {
            *v25 = v28;
            v25 = (_QWORD*)(*((_QWORD*)&v40 + 1) + 8i64);
            *((_QWORD*)&v40 + 1) += 8i64;
        }
    }
#endif
    _set_FMA3_enable(0);
    InitApplication(__argc, __argv);
    //memset(&v45, 0, 56);
    int iteration = 0;
    HWND hMainWindow = nullptr;
    float fcounter = 0;
    while (!glfwWindowShouldClose(g_mainWindow)) {
        if (InitApplicationOK) {
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
        //doFrameWorldID(&v45);
        if (++iteration == 1) {
            int w, h;
            glfwGetWindowSize(g_mainWindow, &w, &h);
            resize(g_mainWindow, w, h);
        }
    }
#if 0
    LOBYTE(v31) = 1;
    byte_141857556 = 1;
    sub_1407251D0(v31);
    v33 = (void*)v40;
    if ((_QWORD)v40)
    {
        if (((unsigned __int64)((unsigned __int64)v41 - v40) & 0xFFFFFFFFFFFFFFF8ui64) >= 0x1000)
        {
            v33 = *(void**)(v40 - 8);
            if ((unsigned __int64)(v40 - (_QWORD)v33 - 8) > 0x1F)
                goto LABEL_57;
        }
        j_j_free(v33);
        v41 = 0i64;
        v40 = 0i64;
    }
    sub_1400CBF50(v46);
    if (v59 >= 0x10)
    {
        v34 = (void*)v57[0];
        if (v59 + 1 >= 0x1000)
        {
            v34 = *(void**)(v57[0] - 8);
            if ((unsigned __int64)(v57[0] - (_QWORD)v34 - 8) > 0x1F)
                goto LABEL_57;
        }
        j_j_free(v34);
    }
    v58 = 0i64;
    v59 = 15i64;
    LOBYTE(v57[0]) = 0;
    if (v62 >= 0x10)
    {
        v35 = (void*)v60;
        if (v62 + 1 >= 0x1000)
        {
            v35 = *(void**)(v60 - 8);
            if ((unsigned __int64)(v60 - (_QWORD)v35 - 8) > 0x1F)
                goto LABEL_57;
        }
        j_j_free(v35);
    }
    if (v26)
    {
        for (j = v26; j != v20; j += 4)
            sub_1400C2C00(j);
        v37 = v26;
        if (((v6 - (_QWORD)v26) & 0xFFFFFFFFFFFFFFE0ui64) >= 0x1000)
        {
            v26 = (void**)*(v26 - 1);
            if ((unsigned __int64)((char*)v37 - (char*)v26 - 8) > 0x1F)
                goto LABEL_57;
        }
        j_j_free(v26);
    }
    if (v9 >= 0x10)
    {
        v38 = v11;
        if (v9 + 1 < 0x1000 || (v11 = (_BYTE*)*((_QWORD*)v11 - 1), (unsigned __int64)(v38 - v11 - 8) <= 0x1F))
        {
            j_j_free(v11);
            return 0;
        }
    LABEL_57:
        invalid_parameter_noinfo_noreturn();
    }
#endif
    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
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
