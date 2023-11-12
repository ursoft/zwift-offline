#include "ZwiftApp.h"
void WindowSizeCallback(GLFWwindow *wnd, int w, int h) {
    if (!w) w = 1;
    if (!h) h = 1;
    int rx_w, rx_h;
    glfwGetWindowSize(wnd, &rx_w, &rx_h);
    if (!rx_w) rx_w = 1;
    if (!rx_h) rx_h = 1;
    g_UI_WindowWidth = (float)rx_w;
    g_UI_WindowHeight = (float)rx_h;
    g_kwidth = g_UI_WindowWidth / (float)w;
    g_kheight = g_UI_WindowHeight / (float)h;
    Log("resize: %d x %d", w, h);
    g_WIN32_WindowWidth = rx_w;
    g_WIN32_WindowHeight = rx_h;
    g_UI_WindowOffsetX = 0.0f;
    g_UI_WindowOffsetY = 0.0f;
    if (g_UI_WindowWidth / g_UI_WindowHeight > 1.78) {
        g_UI_WindowWidth = g_UI_AspectRatio * g_UI_WindowHeight;
        g_UI_WindowOffsetX = ((float)g_WIN32_WindowWidth - g_UI_WindowWidth) * 0.5f;
    }
    if (auto pNoesisGUI = g_pNoesisGUI.lock())
        pNoesisGUI->OnResize(g_WIN32_WindowWidth, g_WIN32_WindowHeight);
}
void WindowFocusCallback(GLFWwindow *, int f) {
    //looks like empty lock
}
void FramebufferSizeCallback(GLFWwindow *wnd, int w, int h) {
    Log("FB resize: %d x %d\n", w, h);
    if (w == 0) w++;
    if (h == 0) h++;
    int iw, ih;
    glfwGetWindowSize(wnd, &iw, &ih);
    if (iw == 0) iw = 1;
    if (ih == 0) ih = 1;
    g_WIN32_WindowWidth = w;
    g_WIN32_WindowHeight = h;
    g_kwidth = w / (float)iw;
    g_kheight = h / (float)ih;
    auto fw = (float)w, fh = (float)h, ox = 0.0f;
    if (fw / fh > 1.78f) {
        fw = VRAM_GetUIAspectRatio() * fh;
        ox = (g_WIN32_WindowWidth - fw) * 0.5f;
    }
    g_UI_WindowWidth = fw;
    g_UI_WindowHeight = fh;
    g_UI_WindowOffsetX = ox;
    g_UI_WindowOffsetY = 0.0f;
}
void CharModsCallback(GLFWwindow *, uint32_t codePoint, int keyModifiers) {
    //not used if (auto pNoesisGUI = g_pNoesisGUI.lock())
    //GLFW_MOD_SHIFT           0x0001, GLFW_MOD_CONTROL         0x0002, GLFW_MOD_ALT             0x0004, GLFW_MOD_SUPER           0x0008
    CONSOLE_KeyFilter(codePoint, keyModifiers | 0x2000);
}
struct MouseState {
    VEC2 m_posLastCB;
    bool m_butStates[4];
    VEC2 m_butDowns[4];
} g_MouseState;
VEC2 g_MousePos;
DWORD g_lastMousePress;
LARGE_INTEGER g_usTimeLastMenuChange;
void MouseButtonCallback(GLFWwindow *, int button, int down, int /*mods*/) {
    button &= 3;
    g_MouseState.m_posLastCB = g_MousePos;
    if (down == 0 || down == 1) {
        if (auto pNoesisGUI = g_pNoesisGUI.lock()) {
            if (down == 1) {
                if (pNoesisGUI->OnMouseButtonDown(g_MousePos))
                    return;
            } else {
                if (pNoesisGUI->OnMouseButtonUp(g_MousePos))
                    return;
            }
        }
    }
//LABEL_26:
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    if (down == 1) {
        g_MouseState.m_butDowns[button] = g_MousePos;
        g_MouseState.m_butStates[button] = true;
    } else {
        g_MouseState.m_butStates[button] = false;
        if (button == 0) {
            if (pc.QuadPart - g_usTimeLastMenuChange.QuadPart > 500'000) {
                auto newt = timeGetTime();
                if (newt - g_lastMousePress <= 250)
                    GUI_MouseDoubleClick();
                g_lastMousePress = newt;
            }
        }
    }
    int v48{};
    if (pc.QuadPart - g_usTimeLastMenuChange.QuadPart > 500'000) {
        auto x = (g_MousePos.m_data[0] - g_UI_WindowOffsetX) * 1280.0f / g_UI_WindowWidth;
        auto y = 1280.0f / VRAM_GetUIAspectRatio() * (g_MousePos.m_data[1] - g_UI_WindowOffsetY) / g_UI_WindowHeight;
        auto w = g_WideUISpace_Width * (g_MousePos.m_data[0] - GFX_UI_GetLeftEdgePad()) / (g_WIN32_WindowWidth - 2 * GFX_UI_GetLeftEdgePad());
        auto h = g_WideUISpace_Height * g_MousePos.m_data[1] / g_WIN32_WindowHeight;
        GUI_MouseClick(button, down, x, y, w, h, &v48);
    }
}
void KeyCallback(GLFWwindow *, int key, int scanCode, int action, int mods) {
    //URSOFT FIX (anti-bouncing algo)
    static int lastTime = timeGetTime(), lastMod = -1, lastCodePoint = -1, waiting = 300;
    auto       nowTime = timeGetTime();
    if (nowTime - lastTime > 300)
        waiting = 300;
    if (lastCodePoint == key && lastMod == mods && int(nowTime - lastTime) < waiting)
        return;
    if (lastCodePoint == key && lastMod == mods) {
        if (waiting == 300)
            waiting = 100;
    } else {
        waiting = 300;
    }
    lastCodePoint = key;
    lastMod = mods;
    lastTime = nowTime;
    bool noesisHandled = false;
    switch (action) {
    case GLFW_REPEAT:
        break;
    case GLFW_RELEASE:
        lastCodePoint = -1;
        if (auto pNoesisGUI = g_pNoesisGUI.lock()) {
#if 0 //LATER
            v17 = *(_QWORD *)(qword_7FF60C88A370 + 8);
            v18 = qword_7FF60C88A370;
            while (!*(_BYTE *)(v17 + 25)) {
                if (*(_DWORD *)(v17 + 28) >= key) {
                    v18 = v17;
                    v17 = *(_QWORD *)v17;
                } else {
                    v17 = *(_QWORD *)(v17 + 16);
                }
            }
            if (*(_BYTE *)(v18 + 25) || key < *(_DWORD *)(v18 + 28))
                v18 = qword_7FF60C88A370;
            if (v18 != qword_7FF60C88A370) {
                noesisHandled = (*(__int64(__fastcall **)(void *, _QWORD, _QWORD))(*(_QWORD *)v69._Ptr[1].vptr + 280i64))(
                    v69._Ptr[1].vptr,
                    *(unsigned int *)(v18 + 32),
                    *(_QWORD *)&scanCode);
                if (!noesisHandled)
                    sub_7FF60B323940((__int64)v69._Ptr, *(_DWORD *)(v18 + 32), qword_7FF60C854E50, 0);
            }
#endif
        }
        for (int v19 = 0; v19 < 4; ++v19) {
            if (key == g_keys4[v19]) {
                g_keys4[v19] = 0;
                g_mods[v19] = 0;
                break;
            }
        }
        return;
    case GLFW_PRESS:
        if (auto pNoesisGUI = g_pNoesisGUI.lock()) {
#if 0 //LATER
            v11 = *(_QWORD *)(qword_7FF60C88A370 + 8);
            v12 = qword_7FF60C88A370;
            while (!*(_BYTE *)(v11 + 25))
            {
                if (*(_DWORD *)(v11 + 28) >= key)
                {
                    v12 = v11;
                    v11 = *(_QWORD *)v11;
                } else
                {
                    v11 = *(_QWORD *)(v11 + 16);
                }
            }
            if (*(_BYTE *)(v12 + 25) || key < *(_DWORD *)(v12 + 28))
                v12 = qword_7FF60C88A370;
            if (v12 != qword_7FF60C88A370)
            {
                noesisHandled = (*(__int64(__fastcall **)(void *, _QWORD, _QWORD))(*(_QWORD *)v65._Ptr[1].vptr + 272i64))(
                    v65._Ptr[1].vptr,
                    *(unsigned int *)(v12 + 32),
                    *(_QWORD *)&scanCode);
                if (!noesisHandled)
                    sub_7FF60B323940((__int64)v65._Ptr, *(_DWORD *)(v12 + 32), qword_7FF60C854E48, 1);
            }
#endif
        }
        for (int v13 = 0; v13 < 4; ++v13) {
            if (0 == g_keys4[v13]) {
                g_keys4[v13] = key;
                g_mods[v13] = mods;
                break;
            }
        }
        if (noesisHandled)
            return;
        break;
    }
    if ((mods & GLFW_MOD_SHIFT) == 0) {
//LABEL_126:
        switch (key) {
        case GLFW_KEY_RIGHT: case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS && !GUI_Key(key, mods))
                CONSOLE_KeyFilter(key, mods);
            break;
        case GLFW_KEY_F1:
            TriggerLocalPlayerAction(BikeEntity::UA_ELBOW);
            break;
        case GLFW_KEY_F2:
            TriggerLocalPlayerAction(BikeEntity::UA_WAVE);
            break;
        case GLFW_KEY_F3:
            TriggerLocalPlayerAction(BikeEntity::UA_RIDEON);
            break;
        case GLFW_KEY_F4:
            TriggerLocalPlayerAction(BikeEntity::UA_HAMMERTIME);
            break;
        case GLFW_KEY_F5:
            TriggerLocalPlayerAction(BikeEntity::UA_NICE);
            break;
        case GLFW_KEY_F6:
            TriggerLocalPlayerAction(BikeEntity::UA_BRINGIT);
            break;
        case GLFW_KEY_F7:
            TriggerLocalPlayerAction(BikeEntity::UA_TOAST);
            break;
        case GLFW_KEY_F8:
            TriggerLocalPlayerAction(BikeEntity::UA_BELL);
            break;
        case GLFW_KEY_F9:
            return;
        case GLFW_KEY_F10:
            GAME_QueueScreenshot(GAME_ScreenshotParams(SCS_USER_TRIGGERED));
            break;
        default:
            if (key > 255 || (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) == GLFW_MOD_CONTROL)
                CONSOLE_KeyFilter(key, mods);
            break;
        }
    }
#if 0 //TODO
    if (key != GLFW_KEY_F1) {
        wnd = (GLFWwindow *)(unsigned int)(key - 291);
        if (key != 291)
        {
            if (key == 292)
            {
                SaveGame_0 = (__int64 **)GetSaveGame_0(BikeManager::g_pBikeManager->m_mainBike);
                v21 = SIG_CalcCaseInsensitiveSignature("ENTITLED_ACCOUNT_FOR_RESPAWNING");
                if (!sub_7FF60B3A1C90(SaveGame_0, v21))
                    return;
                m_mainBike = BikeManager::g_pBikeManager->m_mainBike;
                if (WorldID(g_pGameWorld) == WID_WATOPIA)
                {
                    BikeEntity::Respawn_((__int64)m_mainBike, 35, 0.76, 0, 1);
                LABEL_77:
                    v28 = (__int64)g_entityMgr;
                    if (!g_entityMgr)
                    {
                        v66 = operator new(0x5F8ui64);
                        v28 = EntityManager_ctr((__int64)v66);
                        g_entityMgr = (EntityManager *)v28;
                    }
                    v29 = 0;
                    v30 = *(_QWORD *)(v28 + 464);
                    if (!((*(_QWORD *)(v28 + 472) - v30) >> 3))
                        return;
                    v31 = 0i64;
                    do
                    {
                        v32 = *(_QWORD *)(v31 + v30);
                        if (v32)
                        {
                            *(_DWORD *)(v32 + 4160) = 0;
                            *(_DWORD *)(v32 + 1040) = 0;
                            *(_QWORD *)(v32 + 1048) = 0i64;
                            *(_WORD *)(v32 + 1056) = 0;
                            *(_WORD *)(v32 + 1086) = 0;
                            *(_OWORD *)(v32 + 1116) = xmmword_7FF60C1FE860;
                            *(_BYTE *)(v32 + 1628) = 0;
                            *(_DWORD *)(v32 + 1632) = 0;
                            *(_QWORD *)(v32 + 1640) = 0i64;
                            *(_QWORD *)(v32 + 1648) = 0i64;
                            *(_QWORD *)(v32 + 1656) = 0i64;
                            *(_QWORD *)(v32 + 1664) = 0i64;
                            *(_QWORD *)(v32 + 1672) = 0i64;
                            *(_QWORD *)(v32 + 1680) = 0i64;
                            *(_DWORD *)(v32 + 1688) = 0;
                            *(_BYTE *)(v32 + 1692) = 0;
                            *(_DWORD *)(v32 + 1696) = 0;
                            *(_BYTE *)(v32 + 1700) = 0;
                            *(_QWORD *)(v32 + 1704) = 0i64;
                            *(_QWORD *)(v32 + 1720) = *(_QWORD *)(v32 + 1712);
                            *(_QWORD *)(v32 + 1744) = *(_QWORD *)(v32 + 1736);
                            *(_DWORD *)(v32 + 1760) = 0;
                            *(_QWORD *)(v32 + 1768) = 0i64;
                            *(_WORD *)(v32 + 1776) = 0;
                            *(_WORD *)(v32 + 1806) = 0;
                            *(_OWORD *)(v32 + 1836) = xmmword_7FF60C1FE860;
                            *(_BYTE *)(v32 + 2348) = 0;
                            *(_DWORD *)(v32 + 2352) = 0;
                            *(_QWORD *)(v32 + 2360) = 0i64;
                            *(_QWORD *)(v32 + 2368) = 0i64;
                            *(_QWORD *)(v32 + 2376) = 0i64;
                            *(_QWORD *)(v32 + 2384) = 0i64;
                            *(_QWORD *)(v32 + 2392) = 0i64;
                            *(_QWORD *)(v32 + 2400) = 0i64;
                            *(_DWORD *)(v32 + 2408) = 0;
                            *(_BYTE *)(v32 + 2412) = 0;
                            *(_DWORD *)(v32 + 2416) = 0;
                            *(_BYTE *)(v32 + 2420) = 0;
                            *(_QWORD *)(v32 + 2424) = 0i64;
                            *(_QWORD *)(v32 + 2440) = *(_QWORD *)(v32 + 2432);
                            *(_QWORD *)(v32 + 2464) = *(_QWORD *)(v32 + 2456);
                            *(_WORD *)(v32 + 952) = 0;
                            *(_DWORD *)(v32 + 4164) = 0;
                            *(_DWORD *)(v32 + 4168) = -1082130432;
                            *(_DWORD *)(v32 + 4172) = -1082130432;
                            v30 = *(_QWORD *)(v28 + 464);
                        }
                        ++v29;
                        v31 += 8i64;
                    } while (v29 < (unsigned __int64)((*(_QWORD *)(v28 + 472) - v30) >> 3));
                    return;
                }
                if (WorldID(g_pGameWorld) == WID_INNSBRUCK)
                {
                    v23 = 214013 * g_seed + 2531011;
                    g_seed = 214013 * v23 + 2531011;
                    BikeEntity::Respawn_(
                        (__int64)m_mainBike,
                        7,
                        (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.000000018311107) - 0.0000000091555536)
                            * (float)(HIWORD(v23) & 0x7FFF))
                            + 0.55650002),
                        1u,
                        1);
                    v24 = 1083353771;
                } else if (WorldID(g_pGameWorld) == WID_RICHMOND)
                {
                    v25 = 214013 * g_seed + 2531011;
                    g_seed = 214013 * v25 + 2531011;
                    BikeEntity::Respawn_(
                        (__int64)m_mainBike,
                        0,
                        (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.00000012207404) - 0.000000061037021)
                            * (float)(HIWORD(v25) & 0x7FFF))
                            + 0.1717),
                        1u,
                        1);
                    v24 = 1083528533;
                } else
                {
                    if (WorldID(g_pGameWorld) == WID_LONDON)
                    {
                        v26 = 214013 * g_seed + 2531011;
                        g_seed = 214013 * v26 + 2531011;
                        v27 = (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.00000018311106) - 0.000000091555528)
                            * (float)(HIWORD(v26) & 0x7FFF))
                            + 0.77700001);
                    } else
                    {
                        if (WorldID(g_pGameWorld) != WID_YORKSHIRE)
                            goto LABEL_77;
                        v27 = 0.5971000194549561;
                    }
                    BikeEntity::Respawn_((__int64)m_mainBike, 0, v27, 0, 1);
                    v24 = 1077936128;
                }
                LODWORD(g_WorldTime) = v24;
                goto LABEL_77;
            }
            goto LABEL_126;
        }
        v33 = (__int64 **)GetSaveGame_0(BikeManager::g_pBikeManager->m_mainBike);
        v34 = SIG_CalcCaseInsensitiveSignature("ENTITLED_ACCOUNT_FOR_RESPAWNING");
        if (!sub_7FF60B3A1C90(v33, v34))
            return;
        v35 = BikeManager::g_pBikeManager->m_mainBike;
        if (WorldID(g_pGameWorld) == WID_WATOPIA)
        {
            v36 = 214013 * g_seed + 2531011;
            g_seed = 214013 * v36 + 2531011;
            BikeEntity::Respawn_(
                (__int64)v35,
                0,
                (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.000000018311107) - 0.0000000091555536)
                    * (float)(HIWORD(v36) & 0x7FFF))
                    + 0.65509999),
                0,
                1);
            v37 = 1082479957;
        } else if (WorldID(g_pGameWorld) == WID_RICHMOND)
        {
            v38 = 214013 * g_seed + 2531011;
            g_seed = 214013 * v38 + 2531011;
            BikeEntity::Respawn_(
                (__int64)v35,
                0,
                (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.00000012207404) - 0.000000061037021)
                    * (float)(HIWORD(v38) & 0x7FFF))
                    + 0.1717),
                1u,
                1);
            v37 = 1083528533;
        } else if (WorldID(g_pGameWorld) == WID_LONDON)
        {
            v39 = 214013 * g_seed + 2531011;
            g_seed = 214013 * v39 + 2531011;
            BikeEntity::Respawn_(
                (__int64)v35,
                0,
                (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.00000018311106) - 0.000000091555528)
                    * (float)(HIWORD(v39) & 0x7FFF))
                    + 0.77700001),
                0,
                1);
            v37 = 1077936128;
        } else
        {
            if (WorldID(g_pGameWorld) == WID_NYC)
            {
                v40 = 214013 * g_seed + 2531011;
                g_seed = 214013 * v40 + 2531011;
                v41 = (float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.000000018311107) - 0.0000000091555536)
                    * (float)(HIWORD(v40) & 0x7FFF))
                    + 0.89749998;
                v42 = 4;
            } else
            {
                if (WorldID(g_pGameWorld) != WID_INNSBRUCK)
                {
                LABEL_98:
                    v44 = (__int64)g_entityMgr;
                    if (!g_entityMgr)
                    {
                        v67 = operator new(0x5F8ui64);
                        v44 = EntityManager_ctr((__int64)v67);
                        g_entityMgr = (EntityManager *)v44;
                    }
                    v45 = 0;
                    v46 = *(_QWORD *)(v44 + 464);
                    if (!((*(_QWORD *)(v44 + 472) - v46) >> 3))
                        return;
                    v47 = 0i64;
                    do
                    {
                        v48 = *(_QWORD *)(v47 + v46);
                        if (v48)
                        {
                            *(_DWORD *)(v48 + 4160) = 0;
                            *(_DWORD *)(v48 + 1040) = 0;
                            *(_QWORD *)(v48 + 1048) = 0i64;
                            *(_WORD *)(v48 + 1056) = 0;
                            *(_WORD *)(v48 + 1086) = 0;
                            *(_OWORD *)(v48 + 1116) = xmmword_7FF60C1FE860;
                            *(_BYTE *)(v48 + 1628) = 0;
                            *(_DWORD *)(v48 + 1632) = 0;
                            *(_QWORD *)(v48 + 1640) = 0i64;
                            *(_QWORD *)(v48 + 1648) = 0i64;
                            *(_QWORD *)(v48 + 1656) = 0i64;
                            *(_QWORD *)(v48 + 1664) = 0i64;
                            *(_QWORD *)(v48 + 1672) = 0i64;
                            *(_QWORD *)(v48 + 1680) = 0i64;
                            *(_DWORD *)(v48 + 1688) = 0;
                            *(_BYTE *)(v48 + 1692) = 0;
                            *(_DWORD *)(v48 + 1696) = 0;
                            *(_BYTE *)(v48 + 1700) = 0;
                            *(_QWORD *)(v48 + 1704) = 0i64;
                            *(_QWORD *)(v48 + 1720) = *(_QWORD *)(v48 + 1712);
                            *(_QWORD *)(v48 + 1744) = *(_QWORD *)(v48 + 1736);
                            *(_DWORD *)(v48 + 1760) = 0;
                            *(_QWORD *)(v48 + 1768) = 0i64;
                            *(_WORD *)(v48 + 1776) = 0;
                            *(_WORD *)(v48 + 1806) = 0;
                            *(_OWORD *)(v48 + 1836) = xmmword_7FF60C1FE860;
                            *(_BYTE *)(v48 + 2348) = 0;
                            *(_DWORD *)(v48 + 2352) = 0;
                            *(_QWORD *)(v48 + 2360) = 0i64;
                            *(_QWORD *)(v48 + 2368) = 0i64;
                            *(_QWORD *)(v48 + 2376) = 0i64;
                            *(_QWORD *)(v48 + 2384) = 0i64;
                            *(_QWORD *)(v48 + 2392) = 0i64;
                            *(_QWORD *)(v48 + 2400) = 0i64;
                            *(_DWORD *)(v48 + 2408) = 0;
                            *(_BYTE *)(v48 + 2412) = 0;
                            *(_DWORD *)(v48 + 2416) = 0;
                            *(_BYTE *)(v48 + 2420) = 0;
                            *(_QWORD *)(v48 + 2424) = 0i64;
                            *(_QWORD *)(v48 + 2440) = *(_QWORD *)(v48 + 2432);
                            *(_QWORD *)(v48 + 2464) = *(_QWORD *)(v48 + 2456);
                            *(_WORD *)(v48 + 952) = 0;
                            *(_DWORD *)(v48 + 4164) = 0;
                            *(_DWORD *)(v48 + 4168) = -1082130432;
                            *(_DWORD *)(v48 + 4172) = -1082130432;
                            v46 = *(_QWORD *)(v44 + 464);
                        }
                        ++v45;
                        v47 += 8i64;
                    } while (v45 < (unsigned __int64)((*(_QWORD *)(v44 + 472) - v46) >> 3));
                    return;
                }
                v43 = 214013 * g_seed + 2531011;
                g_seed = 214013 * v43 + 2531011;
                v41 = (float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.000000018311107) - 0.0000000091555536)
                    * (float)(HIWORD(v43) & 0x7FFF))
                    + 0.054099999;
                v42 = 7;
            }
            BikeEntity::Respawn_((__int64)v35, v42, v41, 1u, 1);
            v37 = 1082829483;
        }
        LODWORD(g_WorldTime) = v37;
        goto LABEL_98;
    }
    v49 = (__int64 **)GetSaveGame_0(BikeManager::g_pBikeManager->m_mainBike);
    v50 = SIG_CalcCaseInsensitiveSignature("ENTITLED_ACCOUNT_FOR_RESPAWNING");
    if (!sub_7FF60B3A1C90(v49, v50))
        return;
    v51 = BikeManager::g_pBikeManager->m_mainBike;
    if (WorldID(g_pGameWorld) == WID_WATOPIA)
    {
        v52 = 214013 * g_seed + 2531011;
        g_seed = 214013 * v52 + 2531011;
        BikeEntity::Respawn_(
            (__int64)v51,
            0,
            (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.000000018311107) - 0.0000000091555536)
                * (float)(HIWORD(v52) & 0x7FFF))
                + 0.076499999),
            1u,
            1);
        v53 = 1082479957;
    LABEL_117:
        LODWORD(g_WorldTime) = v53;
        goto LABEL_118;
    }
    if (WorldID(g_pGameWorld) == WID_RICHMOND)
    {
        v54 = 214013 * g_seed + 2531011;
        g_seed = 214013 * v54 + 2531011;
        BikeEntity::Respawn_(
            (__int64)v51,
            0,
            (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.00000012207404) - 0.000000061037021)
                * (float)(HIWORD(v54) & 0x7FFF))
                + 0.67680001),
            1u,
            1);
        v53 = 1076538027;
        goto LABEL_117;
    }
    if (WorldID(g_pGameWorld) == WID_LONDON)
    {
        v55 = 214013 * g_seed + 2531011;
        g_seed = 214013 * v55 + 2531011;
        BikeEntity::Respawn_(
            (__int64)v51,
            7,
            (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.00000012207404) - 0.000000061037021)
                * (float)(HIWORD(v55) & 0x7FFF))
                + 0.37970001),
            0,
            1);
        v53 = 1077936128;
        goto LABEL_117;
    }
    if (WorldID(g_pGameWorld) == WID_NYC)
    {
        v56 = 214013 * g_seed + 2531011;
        g_seed = 214013 * v56 + 2531011;
        BikeEntity::Respawn_(
            (__int64)v51,
            24,
            (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.0000000061037015) - 0.0000000030518508)
                * (float)(HIWORD(v56) & 0x7FFF))
                + 0.069499999),
            1u,
            1);
        v53 = 1077237077;
        goto LABEL_117;
    }
    if (WorldID(g_pGameWorld) == WID_INNSBRUCK)
    {
        v57 = 214013 * g_seed + 2531011;
        g_seed = 214013 * v57 + 2531011;
        BikeEntity::Respawn_(
            (__int64)v51,
            7,
            (float)((float)((float)((float)((float)(HIWORD(g_seed) & 1) * 0.000000012207403) - 0.0000000061037015)
                * (float)(HIWORD(v57) & 0x7FFF))
                + 0.97000003),
            1u,
            1);
        v53 = 1084227584;
        goto LABEL_117;
    }
LABEL_118:
    v58 = (__int64)g_entityMgr;
    if (!g_entityMgr)
    {
        v68 = operator new(0x5F8ui64);
        v58 = EntityManager_ctr((__int64)v68);
        g_entityMgr = (EntityManager *)v58;
    }
    v59 = 0;
    v60 = *(_QWORD *)(v58 + 464);
    if ((*(_QWORD *)(v58 + 472) - v60) >> 3)
    {
        v61 = 0i64;
        do
        {
            v62 = *(_QWORD *)(v61 + v60);
            if (v62)
            {
                *(_DWORD *)(v62 + 4160) = 0;
                *(_DWORD *)(v62 + 1040) = 0;
                *(_QWORD *)(v62 + 1048) = 0i64;
                *(_WORD *)(v62 + 1056) = 0;
                *(_WORD *)(v62 + 1086) = 0;
                *(_OWORD *)(v62 + 1116) = xmmword_7FF60C1FE860;
                *(_BYTE *)(v62 + 1628) = 0;
                *(_DWORD *)(v62 + 1632) = 0;
                *(_QWORD *)(v62 + 1640) = 0i64;
                *(_QWORD *)(v62 + 1648) = 0i64;
                *(_QWORD *)(v62 + 1656) = 0i64;
                *(_QWORD *)(v62 + 1664) = 0i64;
                *(_QWORD *)(v62 + 1672) = 0i64;
                *(_QWORD *)(v62 + 1680) = 0i64;
                *(_DWORD *)(v62 + 1688) = 0;
                *(_BYTE *)(v62 + 1692) = 0;
                *(_DWORD *)(v62 + 1696) = 0;
                *(_BYTE *)(v62 + 1700) = 0;
                *(_QWORD *)(v62 + 1704) = 0i64;
                *(_QWORD *)(v62 + 1720) = *(_QWORD *)(v62 + 1712);
                *(_QWORD *)(v62 + 1744) = *(_QWORD *)(v62 + 1736);
                *(_DWORD *)(v62 + 1760) = 0;
                *(_QWORD *)(v62 + 1768) = 0i64;
                *(_WORD *)(v62 + 1776) = 0;
                *(_WORD *)(v62 + 1806) = 0;
                *(_OWORD *)(v62 + 1836) = xmmword_7FF60C1FE860;
                *(_BYTE *)(v62 + 2348) = 0;
                *(_DWORD *)(v62 + 2352) = 0;
                *(_QWORD *)(v62 + 2360) = 0i64;
                *(_QWORD *)(v62 + 2368) = 0i64;
                *(_QWORD *)(v62 + 2376) = 0i64;
                *(_QWORD *)(v62 + 2384) = 0i64;
                *(_QWORD *)(v62 + 2392) = 0i64;
                *(_QWORD *)(v62 + 2400) = 0i64;
                *(_DWORD *)(v62 + 2408) = 0;
                *(_BYTE *)(v62 + 2412) = 0;
                *(_DWORD *)(v62 + 2416) = 0;
                *(_BYTE *)(v62 + 2420) = 0;
                *(_QWORD *)(v62 + 2424) = 0i64;
                *(_QWORD *)(v62 + 2440) = *(_QWORD *)(v62 + 2432);
                *(_QWORD *)(v62 + 2464) = *(_QWORD *)(v62 + 2456);
                *(_WORD *)(v62 + 952) = 0;
                *(_DWORD *)(v62 + 4164) = 0;
                *(_DWORD *)(v62 + 4168) = -1082130432;
                *(_DWORD *)(v62 + 4172) = -1082130432;
                v60 = *(_QWORD *)(v58 + 464);
            }
            ++v59;
            v61 += 8i64;
        } while (v59 < (unsigned __int64)((*(_QWORD *)(v58 + 472) - v60) >> 3));
    }
#endif
    /* was simplified: if (action == GLFW_PRESS || (action == GLFW_REPEAT && (key == GLFW_KEY_BACKSPACE || key == GLFW_KEY_PAGE_UP || key == GLFW_KEY_PAGE_DOWN || key == GLFW_KEY_UP || key == GLFW_KEY_DOWN || key == GLFW_KEY_HOME || key == GLFW_KEY_END)))
        if (key > 255 || (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) == GLFW_MOD_CONTROL)
            CONSOLE_KeyFilter(key, mods);*/
}
bool g_ResetLastSaveTime, g_onceEndSession = true;
void Zwift_EndSession(bool bShutdown) {
    g_ResetLastSaveTime = true;
    GAME_ResetScreenshotsForActivity();
    auto mainBike = BikeManager::Instance()->m_mainBike;
    static auto orgJersey = SIG_CalcCaseInsensitiveSignature("Humans/Accessories/CyclingJerseys/Originals_Zwift_02.xml");
    if (bShutdown && mainBike && Experimentation::Instance()->IsEnabled(FID_TDFFEM)
        && mainBike->m_yellowJersey && mainBike->m_profile.ride_jersey() == orgJersey) {
        mainBike->m_profile.set_ride_jersey(mainBike->m_yellowJersey);
        mainBike->m_yellowJersey = 0;
        EndGameSession(false);
    } else {
        EndGameSession(false);
        if (!bShutdown)
            return;
    }
    if (g_onceEndSession) {
        g_onceEndSession = false;
        glfwSetWindowShouldClose(g_mainWindow, 1);
    }
}
void WindowCloseCallback(GLFWwindow *) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (mainBike) {
        if (mainBike->m_bc->m_distance <= 0.01f) {
            g_ResetLastSaveTime = true;
            GAME_ResetScreenshotsForActivity();
            EndGameSession(0);
        } else {
            UI_CreateDialog(UID_QUIT, OnQuit, nullptr);
            glfwSetWindowShouldClose(g_mainWindow, 0);
        }
        //return; TODO: removeme
    }
    ZwiftExit(0); //URSOFT FIX TODO: removeme (now I see Audio is not destroyed without this call)
}
void ScrollCallback(GLFWwindow *wnd, double a2, double dir) { //mouse scrolling
    auto v3 = -1;
    if (dir > 0.0)
        v3 = 1;
    ScrollLog(v3);
    if (auto pNoesisGUI = g_pNoesisGUI.lock()) {
        /*LATER NoesisGUI_vtbl v7 = (unsigned int)(int)g_MousePos.m_data[1];
        v8 = (int)g_MousePos.m_data[0];
        (*(void(__fastcall **)(void *, _QWORD, __int64, _QWORD))(*(_QWORD *)v10._Ptr[1].vptr + 208i64))(
            v10._Ptr[1].vptr,
            v8,
            v7,
            (unsigned int)(int)(a2 * 100.0));
        (*(void(__fastcall **)(void *, _QWORD, _QWORD, _QWORD))(*(_QWORD *)v10._Ptr[1].vptr + 200i64))(
            v10._Ptr[1].vptr,
            v8,
            (unsigned int)v7,
            (unsigned int)(int)(dir * 100.0));*/
    }
    return GUI_MouseWheel(v3);
}
DWORD g_cursorPosLastTime;
bool g_ShowCursor = true;
void CursorPosCallback(GLFWwindow *wnd, double x, double y) {
    g_cursorPosLastTime = timeGetTime();
    if (!g_ShowCursor && g_tweakArray[TWI_3DTVENABLED].IntValue() == 0) {
        g_ShowCursor = true;
        ShowCursor(1);
    }
    double x_ = x * g_kwidth, y_ = y * g_kheight;
    if (x_ > 32768.0)
        x_ -= 65536.0;
    if (y_ > 32768.0)
        y_ -= 65536.0;
    g_MousePos.m_data[0] = x_;
    g_MousePos.m_data[1] = y_;
    g_MouseState.m_posLastCB = g_MousePos;
    auto TopmostDialog = GUI_GetTopmostDialog();
    bool hasActiveChild = false;
    if (auto pNoesisGUI = g_pNoesisGUI.lock()) {
        double _x, _y;
        if (y_ >= 0.0)
            _y = y_ + 0.5;
        else
            _y = y_ - 0.5;
        if (x_ >= 0.0)
            _x = x_ + 0.5;
        else
            _x = x_ - 0.5;
        pNoesisGUI->OnCursorMove(_x, _y);
        if (!TopmostDialog)
            hasActiveChild = pNoesisGUI->HasActiveChild();
    }
    if (!hasActiveChild) {
        auto fx = (g_MousePos.m_data[0] - g_UI_WindowOffsetX) / g_UI_WindowWidth * 1280.0f;
        auto fy = 1280.0f * (g_MousePos.m_data[1] - g_UI_WindowOffsetY) / g_UI_WindowHeight / VRAM_GetUIAspectRatio();
        auto w = GFX_UI_GetWideSpaceWidth() * (g_MousePos.m_data[0] - GFX_UI_GetLeftEdgePad()) / (g_WIN32_WindowWidth - GFX_UI_GetLeftEdgePad() - GFX_UI_GetLeftEdgePad());
        auto h = GFX_UI_GetWideSpaceHeight() * g_MousePos.m_data[1] / g_WIN32_WindowHeight;
        GUI_MouseMove(fx, fy, w, h);
    }
    /* OMIT g_PROFILER_CursorPosition = (float)((float)(g_MousePos.m_data[0] / (float)g_WIN32_WindowWidth) * 16.700001)
        + (float)((float)(g_MousePos.m_data[0] / (float)g_WIN32_WindowWidth) * 16.700001);*/
}
#include "optionparser.h"
enum optionIndex { UNKNOWN, LAUNCHER, TOKEN };
const option::Descriptor        g_countOptsMetadata[] = {
    { UNKNOWN, 0, "", "", option::Arg::None, "USAGE: ZwiftAdmin [options]\n\nOptions:" },
    { LAUNCHER, 0, "l", "launcher_version", option::Arg::Optional, "  --launcher_version=1.0.8     The version of the Launcher" },
    { TOKEN, 0, "t", "token", option::Arg::Optional, "  --token=jsonToken     Login Token" },
    { UNKNOWN, 0, "", "", option::Arg::None, "\nExamples:\n  example --launcher_version=1.0.8 --token=jsonToken\n" }, {} };
std::unique_ptr<CrashReporting> g_sCrashReportingUPtr;
void LauncherUpdate(const std::string &launcherVersion) {
    //OMIT (if launcherVersion < "1.1.1" -> restart and update)
}
void PopWheelSlippingMessage(EVENT_ID, va_list) {
    if (!UI_IsDialogOfTypeOpen(UID_SLIPPING_NOTIFICATION))
        UI_CreateDialog(UID_SLIPPING_NOTIFICATION, nullptr, nullptr);
}
void ClearWheelSlippingMessage(EVENT_ID, va_list) {
    if (UI_IsDialogOfTypeOpen(UID_SLIPPING_NOTIFICATION))
        UI_CloseDialog(UID_SLIPPING_NOTIFICATION);
}
struct ZNETWORK_WebEventStart {
    float field_0{};
    char m_url[256]{}, m_title[128]{}, m_msg1[256]{}, m_msg2[128]{};
    float m_timeout = -1.0;
};
bool g_BroadcastReceived;
std::string g_BroadcastEventURL;
void EventNotificationDialogCallback(UI_TwoButtonsDialog::DIALOG_RESULTS dr) {
    if (dr == UI_TwoButtonsDialog::DIALOG_RESULTS::OK)
        ShellExecuteA(nullptr, "open", g_BroadcastEventURL.c_str(), nullptr, nullptr, 1);
}
void BroadcastPrompt(EVENT_ID eid, va_list e) {
    //va_list e;
    //va_start(e, eid);
    auto broadcast_event = va_arg(e, ZNETWORK_WebEventStart *);
    zassert(broadcast_event && "ZwiftApp::BroadcastPrompt Asserted; No instance of ZNETWORK_WebEventStart passed in");
    auto btn1 = "OK";
    const char *btn2 = nullptr;
    if (broadcast_event->m_url[0]) {
        btn1 = "Listen In";
        btn2 = "Nope";
    }
    if (!g_BroadcastReceived) {
        g_BroadcastReceived = true;
        if (broadcast_event->m_url[0])
            g_BroadcastEventURL = broadcast_event->m_url;
        GUI_CreateTwoButtonsDialog(btn1, btn2, broadcast_event->m_title, broadcast_event->m_msg1, broadcast_event->m_msg2,
            broadcast_event->m_timeout, EventNotificationDialogCallback, 650.0f, 260.0f, false, 0.0f, false);
    }
}
void ZwiftInitialize(const std::vector<std::string> &argv) {
    uint32_t startTime = timeGetTime();
    auto     evSysInst = EventSystem::GetInst();
    zassert(g_sCrashReportingUPtr.get() == nullptr);
    CrashReporting::Initialize(evSysInst);
    Experimentation::Initialize(evSysInst);
    auto exp = Experimentation::Instance();
    BLEModule::Initialize(exp);
    GameAssertHandler::Initialize();
    PlayerAchievementService::Initialize(evSysInst);
    NoesisPerfAnalytics::Initialize(exp);
    ClientTelemetry::Initialize(exp, evSysInst);
    GroupEvents::Initialize(exp);
    UnitTypeManager::Initialize(evSysInst, false);
    PlayerProfileCache::Initialize(evSysInst);
    GoalsManager::Initialize(evSysInst);
    ZNet::NetworkService::Initialize();
    //OMIT: v16 = (ZNet::NetworkService *)Cloud::CloudSyncManager::Initialize(Inst, v15);
    DataRecorder::Initialize(exp, ZNet::NetworkService::Instance(), evSysInst);
    Thread::Initialize(exp);
    //exam: --launcher_version=1.1.4 --token={"access_token":"_a_t_","expires_in":1000021600,"id_token":"_id_t","not-before-policy":1408478984,"refresh_expires_in":611975560,"refresh_token":"_r_t_","scope":"","session_state":"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3","token_type":"bearer"}
    int useful_argc = __argc; char **myargv = __argv;
    useful_argc -= (useful_argc > 0); myargv += (useful_argc > 0); // skip program name argv[0] if present
    option::Stats stats(g_countOptsMetadata, useful_argc, myargv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(g_countOptsMetadata, useful_argc, myargv, &options[0], &buffer[0]);
    auto           launcherVer = options[LAUNCHER].arg;
    if (launcherVer)
        LauncherUpdate(launcherVer);
    GFX_SetLoadedAssetMode(true);
    auto userPath = OS_GetUserPath();
    if (userPath) {
        char downloadPath[MAX_PATH] = {};
        sprintf_s(downloadPath, "%s/Zwift/", userPath);
        Downloader::Instance()->SetLocalPath(downloadPath);
    }
    Downloader::Instance()->SetServerURLPath("https://cdn.zwift.com/gameassets/");
    Downloader::Instance()->DownloadFptr("MapSchedule_v2.xml", 0LL, Downloader::m_noFileTime, (uint32_t)-1, GAME_onFinishedDownloadingMapSchedule);
    //OMIT: check GFX driver if no "<data>\Zwift\olddriver.ok" exist
    Downloader::Instance()->Update();
    //moved up so logging early stages is available too
    //ZMUTEX_SystemInitialize();
    //LogInitialize();
    OS_Initialize();
    u_setDataDirectory("data");
    InitICUBase();
    //OMIT watchdog init
    Log("[ZWATCHDOG]: Initialized (not)");
    if (launcherVer) {
        Log("Launcher Version : %s", launcherVer);
        //OMIT g_launcherVer - it used only at PrepareAnalytics
    } else {
        Log("Legacy Launcher Used");
    }
    const float arrInitWorldTimes[] = { 4.f / 3.f, 16.f / 3.f, 11.f / 3.f };
    g_WorldTime = arrInitWorldTimes[timeGetTime() % _countof(arrInitWorldTimes)];
    SetupGameCameras();
    g_UserConfigDoc.UserLoad("prefs.xml");
    GAME_GetSuppressedLogTypes();
    GAME_SetUseErgModeInWorkouts(g_UserConfigDoc.GetBool("ZWIFT\\WORKOUTS\\USE_ERG", true, true));
    UnitTypeManager::Instance()->SetUseMetric(g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\METRIC", false, true));
    NotableMomentsManager::Init();
    evSysInst->TriggerEvent(EV_SENS_RECONN, 0);
    g_MaintainFullscreenForBroadcast = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\MAINTAIN_FOCUS_FOR_BROADCAST", false, true);
    g_removeFanviewHints = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\REMOVE_FANVIEW_HINTS", false, true);
    LOC_Init();
    g_ShowGraph = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\SHOW_GRAPH", false, true);
    FitnessDeviceManager::Initialize();
    g_WADManager.LoadWADFile("assets/global.wad");
    g_WADManager.LoadWADFile("assets/Worlds/worlds.wad");
    g_WADManager.LoadWADFile("assets/Environment/Ground/textures.wad");
    ProfanityFilter::LoadProfanityList();
    g_WADManager.LoadWADFile("assets/UI/WhiteOrangeTheme/WhiteOrangeTheme.wad");
    g_WADManager.LoadWADFile("assets/UI/minimap/minimap.wad");
    g_WADManager.LoadWADFile("assets/UI/WhiteOrangeTheme/Scotty/Scotty.wad");
    g_WADManager.LoadWADFile("assets/UI/layouts/layouts.wad");
    g_WADManager.LoadWADFile("assets/Environment/GrassTextures/GrassTextures.wad");
    GameWorld::LoadWorldsData();
    g_RaceDictionary = RaceDictionary::Init();
    if (!g_RaceDictionary->LoadFromData())
        LogTyped(LOG_ERROR, "[ZwiftApp] Failed to load Race Dictionary!");
    SetupConsoleCommands();
    LOC_SetLanguageChangeCallback(LanguageChangeCallback); // GameLocalization::Init
    auto wadhLoc = g_WADManager.GetWadFileHeaderByItemName(
        "Localization/Localization.xml", WAD_ASSET_TYPE::GLOBAL, nullptr);
    if (wadhLoc)
        LOC_Initialize(wadhLoc->FirstChar(), wadhLoc->m_fileLength, 0);
    LOC_LoadStringTable("data/Localization/Workouts/Loc_WOSelection.xml");
    /* not need, I think if (sub_7FF7196E5900(&_VegetationShadowmapShaderHandle, &crasher, &argv_) && _VegetationShadowmapShaderHandle < 0xA)
    {
        Text_0 = GetText_0("LOC_UPDATE_REQUIRED");
        v303 = GetText_0("LOC_UPDATE_REQUIRED_MESSAGE_WINDOWS");
        MessageBoxW(0i64, v303, Text_0, 0);
        ZwiftExit(-1);
    }*/
    JM_Initialize();
    ANIM_PostInit();
    AUDIO_Init();
#ifdef URSOFT_FIXES
    g_WADManager.LoadWADFile("assets/fonts/font.wad");
#endif // URSOFT_FIXES
    GFX_Initialize();
    ZNETWORK_Initialize();
    ConnectionManager::Initialize();
    SaveActivityService::Initialize(&g_UserConfigDoc, exp, evSysInst);
    HoloReplayManager::Initialize(evSysInst, &g_UserConfigDoc);
    LanExerciseDeviceManager::Initialize();
    Log("Suceeded initializing graphics");
    VRAM_CreateRenderTarget(&g_RT_PreviewWindow, 2048, 1024, true, false, true);
    VRAM_EndRenderTo(0);
    MATERIAL_Init();
    GFX_DrawInit();
#ifndef URSOFT_FIXES
    g_WADManager.LoadWADFile("assets/fonts/font.wad");
#endif // !URSOFT_FIXES
    g_ChatFontGW = &g_GiantFontW;
    g_ChatFontLW = &g_LargeFontW;
    g_GiantFontW.Load(FS_FONDO_BLACK);
    g_GiantFontW.SetScaleAndKerning(0.34108528f, 0.93f);
    ZWIFT_UpdateLoading(nullptr, false); //first screen
    switch (GFX_GetPerformanceGroup()) {
        case GPG_BASIC:
            Log("Using basic graphics profile");
            g_nSkipMipCount = 2;
            COMMAND_RunCommandsFromFile("basic");
            break;
        case GPG_MEDIUM:
            Log("Using medium graphics profile");
            g_nSkipMipCount = 1;
            COMMAND_RunCommandsFromFile("medium");
            break;
        case GPG_HIGH:
            Log("Using high graphics profile");
            g_nSkipMipCount = 0;
            COMMAND_RunCommandsFromFile("high");
            break;
        case GPG_ULTRA:
            Log("Using ultra graphics profile");
            g_nSkipMipCount = 0;
            COMMAND_RunCommandsFromFile("ultra");
            break;
    }
    if (GFX_GetPerformanceFlags() & 3) {
        GfxConfig::gLODBias = 1;
    }
    if (GFX_GetPerformanceFlags() & 4)
        g_nSkipMipCount = 1;
    if (GFX_GetPerformanceFlags() & 8)
        g_nSkipMipCount = 2;
    ZWIFT_UpdateLoading(nullptr, false);
    GFX_SetMaxFPSOnBattery(float(g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\BATTPREFS", 20, true)));
    auto       s_urp = g_UserConfigDoc.GetCStr("ZWIFT\\CONFIG\\USER_RESOLUTION_PREF", nullptr, true);
    char       cmdRes[32];
    const char *cmdRes_do = nullptr;
    if (s_urp) {
        sprintf_s(cmdRes, "res %s", s_urp);
        cmdRes_do = cmdRes;
    } else if (GFX_GetPerformanceFlags() & 0x80) {
        cmdRes_do = "res 1024x576";
    } else if (GFX_GetPerformanceFlags() & 0x100) {
        cmdRes_do = "res 1280x720";
    } else if (GFX_GetPerformanceFlags() & 0x800) {
        cmdRes_do = "res 960x720";
    } else if (GFX_GetPerformanceFlags() & 0x200) {
        cmdRes_do = "res 1334x750";
    } else if (GFX_GetPerformanceFlags() & 0x400) {
        cmdRes_do = "res 1536x864";
    } else if (GFX_GetPerformanceFlags() & 0x1000) {
        cmdRes_do = "res 1024x768";
    } else if (GFX_GetPerformanceFlags() & 0x2000) {
        cmdRes_do = "res 1280x960";
    } else if (GFX_GetPerformanceFlags() & 0x4000) {
        cmdRes_do = "res 1920x1080";
    } else if (GFX_GetPerformanceFlags() & 0x8000) {
        cmdRes_do = "res 1536x1152";
    } else if (GFX_GetPerformanceFlags() & 0x10000) {
        cmdRes_do = "res 1792x1344";
    } else if (GFX_GetPerformanceFlags() & 0x20000) {
        cmdRes_do = "res 2048x1536";
    } else if (GFX_GetPerformanceFlags() & 0x40000) {
        cmdRes_do = "res 3840x2160";
    }
    if (cmdRes_do) COMMAND_RunCommand(cmdRes_do);
    if (GFX_GetPerformanceFlags() & 0x10)
        COMMAND_RunCommand("sres 512x512");
    if (GFX_GetPerformanceFlags() & 0x20)
        COMMAND_RunCommand("sres 1024x1024");
    if (GFX_GetPerformanceFlags() & 0x40)
        COMMAND_RunCommand("sres 2048x2048");
    if (GFX_GetPerformanceFlags() & 0x100000)
        GFX_SetMaxFPSOnBattery(20.0);
    if (GFX_GetPerformanceFlags() & 0x80000) {
        GfxConfig::gFXAA = 0;
    }
    Log("%s %s initialized", GFX_GetVersion(), GFX_GetAPIName());
    auto v = GFX_GetVendorName();
    Log("Graphics Vendor: %s", v ? v : "UNKNOWN");
    auto r = GFX_GetRendererName();
    Log("Graphics Renderer: %s", r ? r : "UNKNOWN");
    if (GetPhysicallyInstalledSystemMemory(&g_TotalMemoryInKilobytes))
        Log("RAM: %dGB", g_TotalMemoryInKilobytes >> 20);
    int      CPUInfo[4] = { -1 };
    unsigned nExIds, i = 0;
    // Get the information associated with each extended ID.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    for (i = 0x80000000; i <= nExIds; ++i) {
        __cpuid(CPUInfo, i);
        // Interpret CPU brand string
        if (i == 0x80000002)
            memmove(g_strCPU, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memmove(g_strCPU + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memmove(g_strCPU + 32, CPUInfo, sizeof(CPUInfo));
    }
    //string includes manufacturer, model and clockspeed
    Log("CPU: %s", g_strCPU);
    if (g_TotalMemoryInKilobytes <= 0x500'000 /*5GB*/ && g_nSkipMipCount <= 1) {
        g_nSkipMipCount = 2;
        GFX_AddPerformanceFlags(GPF_BIG_PERF_PENALTY);
    } else if (g_TotalMemoryInKilobytes <= 0x700'000 /*7GB*/ && !g_nSkipMipCount) {
        g_nSkipMipCount = 1;
        GFX_AddPerformanceFlags(GPF_SMALL_PERF_PENALTY);
    }
#if 0 //OMIT Telemetry info EV_28 HardwareInfo ???
    v400._Bx._Ptr = 0i64;
    v400._Mysize = 0i64;
    v400._Myres = 15i64;
    v401._Ptr = 0i64;
    * &v402 = 0i64;
    *(&v402 + 1) = 15i64;
    Bx._Ptr = 0i64;
    * &v404 = 0i64;
    *(&v404 + 1) = 15i64;
    v405._Ptr = 0i64;
    * &v406 = 0i64;
    *(&v406 + 1) = 15i64;
    v407._Bx._Ptr = 0i64;
    v407._Mysize = 0i64;
    v407._Myres = 15i64;
    v408 = 0i64;
    v409._Bx._Ptr = 0i64;
    v409._Mysize = 0i64;
    v409._Myres = 15i64;
    v410._Bx._Ptr = 0i64;
    v410._Mysize = 0i64;
    v410._Myres = 15i64;
    v411._Bx._Ptr = 0i64;
    v411._Mysize = 0i64;
    v411._Myres = 15i64;
    v412._Bx._Ptr = 0i64;
    v412._Mysize = 0i64;
    v412._Myres = 15i64;
    v221 = -1i64;
    do
        ++v221;
    while (a1321[v221]);
    string_assign(&v400, "1.32.1", v221);
    v222 = GFX_GetAPIName();
    v223 = zu::ToUpper(&coa2, v222);
    if (&Bx != v223) {
        if (*(&v404 + 1) >= 0x10ui64) {
            v224 = Bx._Ptr;
            if ((*(&v404 + 1) + 1i64) >= 0x1000) {
                v224 = *(Bx._Ptr - 1);
                if ((Bx._Ptr - v224 - 8) > 0x1F)
                    invalid_parameter_noinfo_noreturn();
            }
            j_j_free(v224);
        }
        * &v404 = 0i64;
        *(&v404 + 1) = 15i64;
        Bx._Buf[0] = 0;
        Bx = v223->_Bx;
        v404 = *&v223->_Mysize;
        v223->_Mysize = 0i64;
        v223->_Myres = 15i64;
        v223->_Bx._Buf[0] = 0;
    }
    string_dtr(&coa2.vptr);
    v225 = GFX_GetRendererName();
    v226 = zu::ToUpper(&coa2, v225);
    if (&v401 != v226) {
        if (*(&v402 + 1) >= 0x10ui64) {
            v227 = v401._Ptr;
            if ((*(&v402 + 1) + 1i64) >= 0x1000) {
                v227 = *(v401._Ptr - 1);
                if ((v401._Ptr - v227 - 8) > 0x1F)
                    invalid_parameter_noinfo_noreturn();
            }
            j_j_free(v227);
        }
        * &v402 = 0i64;
        *(&v402 + 1) = 15i64;
        v401._Buf[0] = 0;
        v401 = v226->_Bx;
        v402 = *&v226->_Mysize;
        v226->_Mysize = 0i64;
        v226->_Myres = 15i64;
        v226->_Bx._Buf[0] = 0;
    }
    string_dtr(&coa2.vptr);
    v228 = GFX_GetVendorName();
    v229 = zu::ToUpper(&coa2, v228);
    if (&v405 != v229) {
        if (*(&v406 + 1) >= 0x10ui64) {
            v230 = v405._Ptr;
            if ((*(&v406 + 1) + 1i64) >= 0x1000) {
                v230 = *(v405._Ptr - 1);
                if ((v405._Ptr - v230 - 8) > 0x1F)
                    invalid_parameter_noinfo_noreturn();
            }
            j_j_free(v230);
        }
        * &v406 = 0i64;
        *(&v406 + 1) = 15i64;
        v405._Buf[0] = 0;
        v405 = v229->_Bx;
        v406 = *&v229->_Mysize;
        v229->_Mysize = 0i64;
        v229->_Myres = 15i64;
        v229->_Bx._Buf[0] = 0;
    }
    string_dtr(&coa2.vptr);
    v231 = GFX_GetVersion();
    v232 = zu::ToUpper(&coa2, v231);
    string_set(&v407, v232);
    string_dtr(&coa2.vptr);
    PerformanceGroupName = GFX_GetPerformanceGroupName();
    v234 = zu::ToUpper(&coa2, PerformanceGroupName);
    string_set(&v409, v234);
    string_dtr(&coa2.vptr);
    v408 = *GFX_GetMemory(&coa1) >> 20;
    v235 = zu::ToUpper(&path, g_strCPU);
    string_set(&v410, v235);
    string_dtr(&path);
    DeviceName = HardwareInfo::GetDeviceName(&v396);
    string_set(&v411, DeviceName);
    string_dtr(&v396);
    HardwareInfo::GetOSNameVersioned(&path);
    string_set(&v412, &path);
    string_dtr(&path);
    v237 = EventSystem::GetInst();
    EventSystem::TriggerEvent(v237, EV_28, 1, &v400);
#endif
    ZWIFT_UpdateLoading(nullptr, false);
    GameHolidayManager::Instance()->SetupCurrentHoliday(_time64(nullptr));
    ANTRECEIVER_Initialize();
    ANTRECEIVER_Connect();
    LogTyped(LOG_ANT_IMPORTANT, "ANT USB receiver %sfound", ANTRECEIVER_IsConnected() ? "" : "not ");
    ZWIFT_UpdateLoading(nullptr, false);
    GAME_Initialize();
    GAME_SetTrainerSlopeModifier(g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\TRAINER_EFFECT", 0.5f, true));
    auto mb = BikeManager::Instance()->m_mainBike;
    mb->m_bc->SetTireSize(g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\TIRE_CIRC", 2105));
    mb->m_fwGdeSignature = SIG_CalcCaseInsensitiveSignature("bikes/Wheels/Campagnolo_Bora_Ultra/Campagnolo_Bora_Ultra_Low_Front.gde");
    mb->m_rwGdeSignature = SIG_CalcCaseInsensitiveSignature("bikes/Wheels/Campagnolo_Bora_Ultra/Campagnolo_Bora_Ultra_Low_Rear.gde");
    //TODO mb->m_field_11F0 = nullptr;
    //TODO mb->m_field_804 = true;
    mb->m_bc->m_powerSmoothing = (g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\POWERSMOOTHING", 1, true) != 0);
    //g_GlobalMouseOverSID = "Play_SFX_UI_MOUSEOVER_1"; //etc moved to static data
    GUI_Initialize(/*make_sound,*/ !g_IsOnProductionServer);
    GUI_SetDefaultFont(&g_GiantFontW);
    ZWIFT_UpdateLoading(nullptr, false);
    g_vegetationWind[0] = 1.0f;
    g_vegetationWind[3] = 0.2f;
    GameShaders::LoadAll();
    g_SimpleShaderHandle = GFX_CreateShaderFromFile("SimpleShader", -1);
    g_WorldNoLightingHandle = GFX_CreateShaderFromFile("gde_nolighting", -1);
    g_ShadowmapShaderHandle = GFX_CreateShaderFromFile("shadowmap", -1);
    g_ShadowmapInstancedShaderHandle = GFX_CreateShaderFromFile("shadowmap_instanced", -1);
    g_ShadowmapHairShaderHandle = GFX_CreateShaderFromFile("shadowmapHair", -1);
    g_TestShaderHandle = GFX_CreateShaderFromFile("Glossy_shadowmapped", -1);
    g_RoadShader = GFX_CreateShaderFromFile("Road", -1);
    g_RoadAccessoryShader = GFX_CreateShaderFromFile("RoadAccessory", -1);
    g_RoadWetShader = GFX_CreateShaderFromFile("RoadWet", -1);
    g_RoadAccessoryWetShader = GFX_CreateShaderFromFile("RoadAccessoryWet", -1);
    g_HeatHazeShader = GFX_CreateShaderFromFile("HeatHaze", -1);
    g_CausticShader = GFX_CreateShaderFromFile("CausticPass", -1);
    g_CrepuscularHandle = GFX_CreateShaderFromFile("crepuscular", -1);
    //GFX_CreateShaderFromFile("beam", -1); // g_BeamShaderHandle
    //GFX_CreateShaderFromFile("beam_inside", -1);// g_InsideBeamShaderHandle
    g_WorldShaderHandle = GFX_CreateShaderFromFile("defaultWorld", -1);
    g_WorldAOShaderHandle = GFX_CreateShaderFromFile("defaultWorldAO", -1);
    g_WorldShaderBillboardedHandle = GFX_CreateShaderFromFile("defaultWorldBillboard", -1);
    g_WorldShaderShinyHandle = GFX_CreateShaderFromFile("defaultWorldPhong", -1);
    g_WorldShaderSimpleHandle = GFX_CreateShaderFromFile("defaultWorldSimple", -1);
    g_HologramShader = GFX_CreateShaderFromFile("Hologram", -1);
    //GFX_CreateShaderFromFile("RainbowHologram", -1);// g_RainbowHologramShader
    g_LockedItemShader = GFX_CreateShaderFromFile("Glossy_locked", -1);
    g_LondonTerrainShader = GFX_CreateShaderFromFile("londonTerrainShader", -1);
    g_LondonTerrainHeightMapShader = GFX_CreateShaderFromFile("londonTerrainShader_HeightMap", -1);
    g_FranceTerrainShader = GFX_CreateShaderFromFile("franceTerrainShader", -1);
    g_FranceTerrainHeightMapShader = GFX_CreateShaderFromFile("franceTerrainShader_HeightMap", -1);
    g_BasicTerrainShader = GFX_CreateShaderFromFile("BasicTerrain", -1);
    g_BasicTerrainHeightMapShader = GFX_CreateShaderFromFile("BasicTerrain_HeightMap", -1);
    g_BasicTerrainNoSnowShader = GFX_CreateShaderFromFile("BasicTerrainNoSnow", -1);
    g_BasicTerrainNoSnowHeightMapShader = GFX_CreateShaderFromFile("BasicTerrainNoSnow_HeightMap", -1);
    g_InnsbruckTerrainHeightMapShader = GFX_CreateShaderFromFile("innsbruckTerrainShader_HeightMap", -1);
    g_InnsbruckTerrainHandle = GFX_CreateShaderFromFile("innsbruckTerrainShader", -1);
    g_BolognaTerrainHeightMapShader = GFX_CreateShaderFromFile("bolognaTerrainShader_HeightMap", -1);
    g_BolognaTerrainShader = GFX_CreateShaderFromFile("bolognaTerrainShader", -1);
    g_YorkshireTerrainShader = GFX_CreateShaderFromFile("yorkshireTerrainShader", -1);
    g_YorkshireTerrainHeightMapShader = GFX_CreateShaderFromFile("yorkshireTerrainShader_HeightMap", -1);
    g_RichmondTerrainShader = GFX_CreateShaderFromFile("richmondTerrainShader", -1);
    g_RichmondTerrainHeightMapShader = GFX_CreateShaderFromFile("richmondTerrainShader_HeightMap", -1);
    g_WorkoutHologramShader = GFX_CreateShaderFromFile("WorkoutHologramPortal", -1);
    g_WorkoutHologramPrShader = GFX_CreateShaderFromFile("WorkoutHologramPortalRing", -1);
    g_FinalCopyShader = GFX_CreateShaderFromFile("FinalCopy", -1);
    g_WorldShaderTerrainHandle = GFX_CreateShaderFromFile("defaultWorld_Terrain", -1);
    g_WorldShaderTerrainHeightMapHandle = GFX_CreateShaderFromFile("defaultWorld_TerrainHeightMap", -1);
    g_ShadowShaderTerrainHeightMap = GFX_CreateShaderFromFile("shadowmap_TerrainHeightMap", -1);
    g_WatopiaSpecialTileShaderHeightmap = GFX_CreateShaderFromFile("WatopiaDesert_TerrainHeightMap", -1);
    g_WatopiaSpecialTileShader = -1; //FIX RTL GFX_CreateShaderFromFile("WatopiaDesert", -1);
    if (g_ShadowShaderTerrainHeightMap == -1 || g_WorldShaderTerrainHeightMapHandle == -1 || g_LondonTerrainHeightMapShader == -1) {
        Log("Disabling heightmap shaders");
        g_bUseTextureHeightmaps = false;
    }
    g_WorldShaderInstancedHandle = GFX_CreateShaderFromFile("defaultWorld_instanced", -1);
    //GFX_CreateShaderFromFile("defaultWorld_notShadowed_instanced", -1);// g_WorldShaderNoShadowsInstancedHandle
    g_WorldShaderInstancedTerrainConformingHandle = GFX_CreateShaderFromFile(
        "defaultWorld_instanced_TerrainHeightMap",
        -1);
    g_World2LayerShaderHandle = GFX_CreateShaderFromFile("defaultWorld2Layer", -1);
    //GFX_CreateShaderFromFile("defaultWorld2Layer_instanced", -1);// g_World2LayerShaderInstancedHandle
    g_VegetationShaderHandle = GFX_CreateShaderFromFile("vegetation", -1);
    g_VegetationShaderInstancedTerrainConformHandle = GFX_CreateShaderFromFile(
        "vegetationTerrainHeightmapInstance",
        -1);
    g_VegetationShaderInstancedHandle = GFX_CreateShaderFromFile("vegetationInstance", -1);
    g_VegetationShadowmapShaderHandle = GFX_CreateShaderFromFile("vegetationShadowmap", -1);
    g_VegetationShadowmapInstancedShaderHandle = GFX_CreateShaderFromFile("vegetationShadowmapInstance", -1);
    g_WireShaderHandle = GFX_CreateShaderFromFile("wire", -1);
    g_WireShadowShaderHandle = -1; //FIX RTL GFX_CreateShaderFromFile("wireShadow", -1);
    g_BikeShaderInstancedHandle = GFX_CreateShaderFromFile("accessory_instanced", -1);
    g_HairShaderHandle = GFX_CreateShaderFromFile("Hair", -1);
    //GFX_CreateShaderFromFile("defaultWorld_notShadowed", -1);// g_WorldShaderNoShadowHandle
    g_SkinShader = GFX_CreateShaderFromFile("skinShader", -1);
    g_ShadowmapSkinShader = GFX_CreateShaderFromFile("shadowmapSkin", -1);
    g_SkinShaderHologram = GFX_CreateShaderFromFile("skinShaderHologram", -1);
    g_grayScaleShader = GFX_CreateShaderFromFile("grayScale", -1);
    GNSceneCreateParams sceneParams{ 0, 0x100010, 0x100010 };
    if (GFX_GetTier()) {
        auto _WorldShaderTerrainHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNdefaultWorld_Terrain" });
        auto _WorldShaderTerrainHeightMapHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNdefaultWorld_Terrain", 1 });
        auto _JapanTerrainShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNJapanTerrain" });
        auto _JapanTerrainShaderHeightMap = GFX_CreateShader(GFX_CreateShaderParams{ "GNJapanTerrain", 1 });
        auto _InnsbruckTerrainHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNInnsbruckTerrain" });
        auto _InnsbruckTerrainHeightMapShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNInnsbruckTerrain", 1 });
        auto _LondonTerrainShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNLondonTerrain" });
        auto _LondonTerrainHeightMapShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNLondonTerrain", 1 });
        auto _BolognaTerrainShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNBolognaTerrain", 0 });
        auto _BolognaTerrainHeightMapShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNBolognaTerrain", 1 });
        auto _RichmondTerrainShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNRichmondTerrain" });
        auto _RichmondTerrainHeightMapShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNRichmondTerrain", 1 });
        auto _YorkshireTerrainShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNYorkshireTerrain" });
        auto _YorkshireTerrainHeightMapShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNYorkshireTerrain", 1 });
        auto _FranceTerrainShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNFranceTerrain" });
        auto _FranceTerrainHeightMapShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNFranceTerrain", 1 });
        auto _WatopiaSpecialTileShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNWatopiaDesert_Terrain" });
        auto _WatopiaSpecialTileShaderHeightmap = GFX_CreateShader(GFX_CreateShaderParams{ "GNWatopiaDesert_Terrain", 1 });
        auto _TerrainShadow = GFX_CreateShader(GFX_CreateShaderParams{ "GNTerrainShadow" });
        auto _ShadowShaderTerrainHeightMap = GFX_CreateShader(GFX_CreateShaderParams{ "GNTerrainShadow", 1 });
        auto _RoadShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNRoad" });
        auto _RoadAccessoryShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNRoadAccessory" });
        auto _RoadAccessoryShaderSSR = GFX_CreateShader(GFX_CreateShaderParams{ "GNRoadAccessorySSR" });
        auto _RoadWetShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNRoadWet" });
        auto _WorldShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNdefaultWorld" });
        auto _WorldShaderBillboardedHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNdefaultWorldBillboard" });
        auto _WorldShaderInstancedHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNdefaultWorld_instanced" });
        auto _WorldShaderInstancedTerrainConformingHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNdefaultWorld_instanced_TerrainHeightMap" });
        auto _BikeShaderInstancedHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNaccessory_instanced" });
        auto _SkinShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNSkin" });
        auto _ShadowmapSkinShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNshadowmapSkin" });
        auto _HairShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNHair" });
        auto _ShadowmapHairShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNshadowmapHair" });
        auto _TestShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNGlossy_shadowmapped" });
        auto _ShadowmapShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNshadowmap" });
        auto _ShadowmapInstancedShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNshadowmap_instanced" });
        auto _VegetationShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNvegetation" });
        auto _VegetationShaderInstancedHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNvegetation", 2, 2 });
        auto _VegetationShadowmapShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNvegetation", 0, 1 });
        auto _VegetationShadowmapInstancedShaderHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNvegetation", 2, 3 });
        auto _VegetationShaderInstancedTerrainConformHandle = GFX_CreateShader(GFX_CreateShaderParams{ "GNvegetation", 3, 2 });
        auto _WaterShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNWater" });
        auto _WaterShader2 = GFX_CreateShader(GFX_CreateShaderParams{ "GNWater", 0, 1 });
        auto _bibShader = GFX_CreateShader(GFX_CreateShaderParams{ "GNBib" });
        //GameShaders::LoadTier
        GameShaders::shGNLinearizeDepth = GFX_CreateShader(GFX_CreateShaderParams{ "GNLinearizeDepth" });
        GameShaders::shGNDownsampleLinearizeDepth = GFX_CreateShader(GFX_CreateShaderParams{ "GNLinearizeDepth", 0, 1 });
        GameShaders::shGNRoadSSR = GFX_CreateShader(GFX_CreateShaderParams{ "GNRoadSSR" });
        if (_WorldShaderTerrainHandle != -1 && _WorldShaderTerrainHeightMapHandle != -1 && _WatopiaSpecialTileShader != -1 && _WatopiaSpecialTileShaderHeightmap != -1
            && _JapanTerrainShader != -1 && _JapanTerrainShaderHeightMap != -1 && _InnsbruckTerrainHandle != -1 && _InnsbruckTerrainHeightMapShader != -1 && _LondonTerrainShader != -1
            && _LondonTerrainHeightMapShader != -1 && _BolognaTerrainShader != -1 && _BolognaTerrainHeightMapShader != -1 && _RichmondTerrainShader != -1 && _RichmondTerrainHeightMapShader != -1
            && _YorkshireTerrainShader != -1 && _YorkshireTerrainHeightMapShader != -1 && _FranceTerrainShader != -1 && _FranceTerrainHeightMapShader != -1
            && _TerrainShadow != -1 && _ShadowShaderTerrainHeightMap != -1 && _RoadShader != -1 && _RoadAccessoryShader != -1 && _RoadAccessoryShaderSSR != -1
            && _RoadWetShader != -1 && _WorldShaderHandle != -1 && _WorldShaderBillboardedHandle != -1 && _WorldShaderInstancedHandle != -1 && _WorldShaderInstancedTerrainConformingHandle != -1
            && _BikeShaderInstancedHandle != -1 && _SkinShader != -1 && _ShadowmapSkinShader != -1 && _HairShaderHandle != -1 && _ShadowmapHairShaderHandle != -1 && _TestShaderHandle != -1
            && _ShadowmapShaderHandle != -1 && _ShadowmapInstancedShaderHandle != -1 && _VegetationShaderHandle != -1 && _VegetationShaderInstancedHandle != -1 && _VegetationShadowmapShaderHandle != -1
            && _VegetationShadowmapInstancedShaderHandle != -1 && _VegetationShaderInstancedTerrainConformHandle != -1 && _WaterShader != -1 && _WaterShader2 != -1 && _bibShader != -1
            && GameShaders::shGNLinearizeDepth != -1 && GameShaders::shGNDownsampleLinearizeDepth != -1 && GameShaders::shGNRoadSSR != -1) {
            sceneParams.m_tier = 1;
            sceneParams.m_field_18 = -10.0f;
            sceneParams.m_field_1C = 1.0f;
            sceneParams.m_field_20 = 2.0f;
            sceneParams.m_field_24 = 0.0f;
            sceneParams.m_field_28 = 260.0f;
            sceneParams.m_field_2C = 50000.0f;
            LOADER_UseHWInstancing(1);
            g_SkinShader = _SkinShader;
            g_ShadowmapSkinShader = _ShadowmapSkinShader;
            g_WorldShaderHandle = _WorldShaderHandle;
            g_WorldShaderTerrainHandle = _WorldShaderTerrainHandle;
            g_WorldShaderTerrainHeightMapHandle = _WorldShaderTerrainHeightMapHandle;
            g_WatopiaSpecialTileShaderHeightmap = _WatopiaSpecialTileShaderHeightmap;
            g_WatopiaSpecialTileShader = _WatopiaSpecialTileShader;
            g_InnsbruckTerrainHandle = _InnsbruckTerrainHandle;
            g_InnsbruckTerrainHeightMapShader = _InnsbruckTerrainHeightMapShader;
            g_LondonTerrainShader = _LondonTerrainShader;
            g_LondonTerrainHeightMapShader = _LondonTerrainHeightMapShader;
            g_BolognaTerrainShader = _BolognaTerrainShader;
            g_BolognaTerrainHeightMapShader = _BolognaTerrainHeightMapShader;
            g_YorkshireTerrainShader = _YorkshireTerrainShader;
            g_YorkshireTerrainHeightMapShader = _YorkshireTerrainHeightMapShader;
            g_RichmondTerrainShader = _RichmondTerrainShader;
            g_RichmondTerrainHeightMapShader = _RichmondTerrainHeightMapShader;
            g_FranceTerrainShader = _FranceTerrainShader;
            g_FranceTerrainHeightMapShader = _FranceTerrainHeightMapShader;
            g_ShadowShaderTerrainHeightMap = _ShadowShaderTerrainHeightMap;
            g_WorldShaderInstancedHandle = _WorldShaderInstancedHandle;
            g_WorldShaderInstancedTerrainConformingHandle = _WorldShaderInstancedTerrainConformingHandle;
            g_BikeShaderInstancedHandle = _BikeShaderInstancedHandle;
            g_HairShaderHandle = _HairShaderHandle;
            g_ShadowmapHairShaderHandle = _ShadowmapHairShaderHandle;
            g_TestShaderHandle = _TestShaderHandle;
            g_ShadowmapShaderHandle = _ShadowmapShaderHandle;
            g_ShadowmapInstancedShaderHandle = _ShadowmapInstancedShaderHandle;
            g_RoadShader = _RoadShader;
            g_RoadAccessoryShader = _RoadAccessoryShader;
            g_RoadAccessoryShaderSSR = _RoadAccessoryShaderSSR;
            g_RoadWetShader = _RoadWetShader;
            g_WorldShaderBillboardedHandle = _WorldShaderBillboardedHandle;
            g_VegetationShaderHandle = _VegetationShaderHandle;
            g_VegetationShaderInstancedHandle = _VegetationShaderInstancedHandle;
            g_VegetationShaderInstancedTerrainConformHandle = _VegetationShaderInstancedTerrainConformHandle;
            g_VegetationShadowmapInstancedShaderHandle = _VegetationShadowmapInstancedShaderHandle;
            g_VegetationShadowmapShaderHandle = _VegetationShadowmapShaderHandle;
        }
    }
    Log("[ZWIFT]: GFX_Tier %d", sceneParams.m_tier);
    g_GNSceneSystem.Initialize(sceneParams);
    g_ctMainView.Initialize(GNViewInitializeParams{ &g_GNSceneSystem, 240, 135, 0x100 });
    g_ctReflView.Initialize(GNViewInitializeParams{ &g_GNSceneSystem, (g_RT_ReflectionMap.m_dw_width + 7) >> 3, (g_RT_ReflectionMap.m_dw_height + 7) >> 3, 0x80 });
    g_PreviewView.Initialize(GNViewInitializeParams{ &g_GNSceneSystem, (g_RT_PreviewWindow.m_dw_width + 15) >> 4, (g_RT_PreviewWindow.m_dw_height + 15) >> 4, 0x04 });
    if (g_WorldShaderHandle == -1) {
        MessageBoxA(nullptr, "Could not find required data files. Closing application now.", "Error", MB_OK);
        ZwiftExit(0);
    } else {
        PostFX_Initialize();
        Sky::Initialize();
        Weather::Initialize();
        ParticulateManager::Create();
        ParticulateManager::Init();
        AccessoryManager::InitAllAccessories();
        ZWIFT_UpdateLoading(nullptr, false);
        BikeManager::Instance()->Initialize(exp);
        SteeringModule::Self()->Init(mb);
        g_ScreenMeshHandle = LOADER_LoadGdeFile("data/screenbox.gde", false);
        g_TrainerMeshHandle = LOADER_LoadGdeFile("data/bikes/Trainers/Zwift/Trainer.gde", false);
        g_HandCycleTrainerMeshHandle = LOADER_LoadGdeFile("data/bikes/Trainers/Zwift/HandcycleTrainer.gde", false);
        g_TreadmillMeshHandle = LOADER_LoadGdeFile("data/Humans/Treadmill/Treadmill.gde", false);
        g_PaperMeshHandle = LOADER_LoadGdeFile("data/bikes/Frames/DefaultOrange/Paper.gde", false);
        ZWIFT_UpdateLoading(nullptr, false);
        g_SmallFont.Load(FS_SMALL);
        g_MediumFont.Load(FS_SANSERIF);
        g_LargeFontW.Load(FS_FONDO_MED);
        g_LargeFontW.SetHeadAndBaseLines(14.0, 20.0);
        HUD_UpdateChatFont();
        g_LargeFontW.SetScaleAndKerning(0.6f, 0.887f);
        g_LargeFontW.SetLanguageKerningScalar(CID_CHINESE, 1.3f);
        g_ButterflyTexture = GFX_CreateTextureFromTGAFile("blue_butterfly.tga", -1, true);
        g_RedButterflyTexture = GFX_CreateTextureFromTGAFile("white_butterfly.tga", -1, true);
        g_MonarchTexture = GFX_CreateTextureFromTGAFile("monarch.tga", -1, true);
        g_FireflyTexture = GFX_CreateTextureFromTGAFile("firefly.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("grid1meter.tga", -1, true);// g_GridTexture
        g_CausticTexture = GFX_CreateTextureFromTGAFile("Environment/Shared/Effects/Caustics/Caustics_00.tga", -1, true);
        GFX_SetAnimatedTextureFramerate(g_CausticTexture, 20.0);
        g_GrassTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/grass.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("Environment/Ground/WindsweptGrassD.tga", -1, true);// g_JapanGrassTexture
        g_GravelMtnGrassTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/DesertGroundD.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("Environment/Buildings/CentralLondon/Textures/Concrete02.tga", -1, true);// g_ConcreteTexture
        g_InnsbruckConcreteTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/InnsbruckConcrete.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("Environment/Ground/InnsbruckConcrete_NRM.tga", -1, true);// g_InnsbruckConcreteNRMTexture
        //GFX_CreateTextureFromTGAFile("Environment/Ground/RichmondConcreteD.tga", -1, true);// g_RichmondConcreteTexture
        g_ParisConcreteTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/FranceConcrete.tga", -1, true);
        g_DefaultNormalMapNoGloss = GFX_CreateTextureFromTGAFile("defaultNormalMap_nogloss.tga", -1, true);
        g_RoadDustTexture = GFX_CreateTextureFromTGAFile("FX/Particles/RoadDust_D.tga", -1, true);
        g_GravelDustTexture = GFX_CreateTextureFromTGAFile("FX/Particles/GravelDust_D.tga", -1, true);
        g_SandTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/sand.tga", -1, true);
        g_SandNormalTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/sand_NRM.tga", -1, true);
        g_RockTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/Rock.tga", -1, true);
        g_FranceRockTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/VentouxRockD.tga", -1, true);
        g_FranceRockNTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/VentouxRockN.tga", -1, true);
        g_RockNormalTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/Rock_NRM.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("shadow.tga", -1, true);// g_ShadowTexture
        //GFX_CreateTextureFromTGAFile("black.tga", -1, true);// g_BlackTexture
        g_ShowroomFloorTexture = GFX_CreateTextureFromTGAFile("showroomfloor.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("beams_bg.tga", -1, true);// g_LightBeamsTexture
        g_HeadlightTexture = GFX_CreateTextureFromTGAFile("proj_headlight.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("beam.tga", -1, true);// g_BeamTexture
        g_VignetteTexture = GFX_CreateTextureFromTGAFile("vignette_evenless.tga", -1, true);
        //GFX_CreateTextureFromTGAFile("particle2.tga", -1, true);// g_ParticleTexture
        g_GNSceneSystem.SetDefaultEnvironmentReflection();
        INSTANCING_CreateDitherTex();
        ArtConfig::LoadArtConfig();
        ZFeatureManager::Initialize();
        auto fm = ZFeatureManager::Instance();
        fm->AddFeature(&gPlayerHighlight);
        fm->AddFeature(&gHoloEffect);
        fm->Init();
        fm->LoadConfig();
        auto defPartShader = GFX_CreateShaderFromFile("defaultParticles", -1);
        g_pDustSystem = new ParticleSystem(1000, DustParticleUpdate, nullptr, nullptr, nullptr, nullptr);
        g_pDustSystem->m_texture = g_RoadDustTexture;
        //TODO*(uint32_t *)&pDustSystem->field_18 = 0;
        g_pDustSystem->m_shader = defPartShader;
        PARTICLESYS_Register(g_pDustSystem);
        g_pConfettiSystem = new ParticleSystem(3000, ConfettiParticleUpdate, nullptr, nullptr, nullptr, nullptr);
        g_pConfettiSystem->m_texture = GFX_CreateTextureFromTGAFile("FX/Particles/confetti03_sheet.tga", -1, true);
        //TODO *(_QWORD *)&g_pConfettiSystem->field_14 = 1i64;
        g_pConfettiSystem->m_shader = defPartShader;
        PARTICLESYS_Register(g_pConfettiSystem);
        Bib::InitOnce();
        ZWIFT_UpdateLoading(nullptr, false);
        HUD_Initialize();
        if (!UI_DialogPointer(UID_CONNECTION_NOTIFICATIONS))
            UI_CreateDialog(UID_CONNECTION_NOTIFICATIONS, nullptr, nullptr);
        ZWIFT_UpdateLoading(nullptr, false);
        auto     ltd = g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTTRAINERDEVICE", (uint32_t)-1);
        uint64_t Power;
        if (ltd != -1 && (Power = ZwiftPowers::GetInst()->GetPower(ltd)) != 0) {
            mb->m_bc->m_lastPower = Power;
        } else {
            g_UserConfigDoc.ClearPath("ZWIFT\\DEVICES\\LASTTRAINERDEVICE");
            g_UserConfigDoc.ClearPath("ZWIFT\\DEVICES\\LASTSPEEDDEVICE");
        }
        evSysInst->Subscribe(EV_SLIPPING_ON, PopWheelSlippingMessage);
        evSysInst->Subscribe(EV_SLIPPING_OFF, ClearWheelSlippingMessage);
        evSysInst->Subscribe(EV_BC_PROMPT, BroadcastPrompt);
        glClearColor(0.21176472f, 0.24313727f, 0.27843139f, 0.0f);
        glfwSetCharModsCallback(g_mainWindow, CharModsCallback);
        glfwSetKeyCallback(g_mainWindow, KeyCallback);
        glfwSetMouseButtonCallback(g_mainWindow, MouseButtonCallback);
        glfwSetCursorPosCallback(g_mainWindow, CursorPosCallback);
        glfwSetScrollCallback(g_mainWindow, ScrollCallback);
        glfwSetWindowCloseCallback(g_mainWindow, WindowCloseCallback);
        glfwSetWindowSizeCallback(g_mainWindow, WindowSizeCallback);
        glfwSetFramebufferSizeCallback(g_mainWindow, FramebufferSizeCallback);
        glfwSetWindowFocusCallback(g_mainWindow, WindowFocusCallback);
        ZwiftAppKeyProcessorManager::Instance()->Init();
        SetIcon();
        GAME_GetMapForTime(_time64(nullptr) - 14400); // MAP_SCHEDULE_GMT_4_OFFSET
        auto worldCfg = g_UserConfigDoc.GetU32("ZWIFT\\WORLD", 0);
        if (!GAME_IsWorldIDAvailableViaPrefsFile(worldCfg))
            worldCfg = 0;
        GFX_SetLoadedAssetMode(false);
        g_BillboardInfo.LoadBillboardInfoDatabase();
        g_ShrubHelperInfo.LoadShrubHelperInfoDatabase();
        RegionsDatabase::LoadRegionsDatabase();
        GAME_LoadLevel(worldCfg);
        AccessoryManager::CreateSegmentJerseys();
        DetermineNoesisFeatureFlags();
        if (options[TOKEN].arg) {
            Log("Got an access token");
            g_startupFlowStateParam = options[TOKEN].arg;
            g_gameStartupFlowState = ZSF_1;
        } else {
            Log("No username specified");
            g_startupFlowStateParam = nullptr;
            g_gameStartupFlowState = ZSF_LOGIN;
        }
        ZSF_SwitchState(g_gameStartupFlowState, g_startupFlowStateParam);
        ZWIFT_UpdateLoading(u"Ride On.", true);
        //delme {
        //ZWIFT_UpdateLoading(u"QWEАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЬЫЪЭЮЯ", true);
        //::Sleep(5000);
        //ZWIFT_UpdateLoading(u"qweабвгдеёжзийклмнопрстуфхцчшщьыъэюя", true);
        //::Sleep(5000);
        std::vector<UChar> alphabet;
        for (UChar c = 33; c < 128; c++)
            alphabet.push_back(c);
        for (UChar c = L'А'; c <= L'Я'; c++)
            alphabet.push_back(c);
        alphabet.push_back(L'Ё');
        for (UChar c = L'а'; c <= L'я'; c++)
            alphabet.push_back(c);
        alphabet.push_back(L'ё');
        std::wstring line;
        line.reserve(70);
        for (auto c1 : alphabet) {
            for (auto c2 : alphabet) {
                if (line.size() >= line.capacity() - 4) {
                    char buf[150]{};
                    WideCharToMultiByte(CP_UTF8, 0, line.c_str(), (int)line.length(), buf, 150, nullptr, nullptr);
                    Log("%s", buf);
                    line.erase(0, line.size() - 1);
                }
                line.push_back(c1);
                line.push_back(c2);
            }
        }{
            char buf[150]{};
            WideCharToMultiByte(CP_UTF8, 0, line.c_str(), (int)line.length(), buf, 150, nullptr, nullptr);
            Log("%s", buf);
        }
        //delme }
        ANTRECEIVER_PostConnect();
        auto TotalLoadTimeInSeconds = (timeGetTime() - startTime) * 0.001; (void)TotalLoadTimeInSeconds;
        if (parse.error()) {
            //GameDictionary::Create(); - what for???
            ZwiftExit(0);
        }
        VideoCapture::InitVideoCapture();
    }
}
void ZSF_SwitchState(ZwiftStartupFlow, const void *) {
    //TODO
}
void ZSF_FinishedState(ZwiftStartupFlow, uint32_t) {
    //TODO
}
void EndGameSession(bool bShutDown) {
    //TODO
    //UI_CloseDialog(UID_CONNECTION_NOTIFICATIONS);
    ShutdownSingletons();
    //TODO
    Log("ZwiftApp Gracefully Shutdown");
    //TODO
}
void ShutdownSingletons() {
    //zassert(g_sCrashReportingUPtr.get() != nullptr);
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down VideoCapture");
    VideoCapture::ShutdownVideoCapture();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down Powerups");
    Powerups::g_pPowerups.reset(); //DestroyPowerups
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down IoCpp");
    IoCPP::Set(nullptr);
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down NoesisPerfAnalytics");
    if (NoesisPerfAnalytics::IsInitialized())
        NoesisPerfAnalytics::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down GoalsManager");
    GoalsManager::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down UnitTypeManager");
    UnitTypeManager::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down ClientTelemetry");
    ClientTelemetry::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down SaveActivityService");
    if (SaveActivityService::IsInitialized())
        SaveActivityService::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down ConnectionManager");
    if (ConnectionManager::IsInitialized())
        ConnectionManager::DestroyInstance();
    //CrashReporting::AddBreadcrumb("Shutting down GameAssertHandler");
    GameAssertHandler::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down Localization");
    LOC_Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down GroupEvents");
    zassert("sExperimentationUPtr.get() != nullptr");
    GroupEvents::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down PlayerAchievementService");
    if (PlayerAchievementService::IsInitialized())
        PlayerAchievementService::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down HoloReplayManager");
    if (HoloReplayManager::IsInitialized())
        HoloReplayManager::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down BLEModule");
    if (BLEModule::IsInitialized())
        BLEModule::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down DataRecorder");
    if (DataRecorder::IsInitialized())
        DataRecorder::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down Experimentation");
    if (Experimentation::IsInitialized())
        Experimentation::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down ZFeatureManager");
    if (ZFeatureManager::IsInitialized())
        ZFeatureManager::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down ZNet::NetworkService");
    PlayerProfileCache::Shutdown(); //added by ursoft
    if (ZNet::NetworkService::IsInitialized())
        ZNet::NetworkService::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down zwift_network");
    ZNETWORK_Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down CrashReporting");
    CrashReporting::Shutdown();
    if (EventSystem::IsInitialized())
        EventSystem::Destroy();
}
BikeManager *zwiftUpdateContext::GetBikeManager() {
    return m_bikeOver ? m_bikeOver : BikeManager::Instance();
}
CameraManager *zwiftUpdateContext::GetCameraManager() {
    return m_camOver ? m_camOver : &g_CameraManager;
}
struct contextForwarder {
    GroupEvents::SubgroupState *GetCurrentEvent() {
        return FindSubgroupEventSharedPtr(g_GroupEventsActive_CurrentEventId);
    }
    time_t GetNetworkWorldTime() {
        return g_CachedWorldTime;
    }
    WORLD_ID GetWorldID() { return g_pGameWorld->WorldID(); }
    WORLD_ID GetWorldIDForWorldNetworkID(int /*WORLD_NETWORK_IDS*/ id) { return GameWorld::GetWorldIDForWorldNetworkID(id); }
    int GetWorldNetworkIDForWorldID(WORLD_ID id) { GameWorld::GetWorldNetworkIDForWorldID(id); }
    bool HasSubgroupStarted(int64_t id) { return GroupEvents::HasSubgroupStarted(id); }
    void PARTICLESYS_Unregister(ParticleSystem *ps) { PARTICLESYS_Unregister(ps); }
    void VRAM_ReleaseRenderTargetVRAM(RenderTarget *rt) { VRAM_ReleaseRenderTargetVRAM(rt); }
};
contextForwarder *g_ctxForwarder;
contextForwarder *zwiftUpdateContext::GetForwarder() {
    return m_fwOver ? m_fwOver : g_ctxForwarder;
}
GameHolidayManager *zwiftUpdateContext::GetGameHolidayManager() {
    return m_holOver ? m_holOver : &g_GameHolidayManager;
}
RoadManager *zwiftUpdateContext::GetRoadManager() {
    return m_roadmOver ? m_roadmOver : g_pRoadManager;
}
RouteManager *zwiftUpdateContext::GetRouteManager() {
    return m_routemOver ? m_routemOver : RouteManager::Instance();
}

//Unit Tests
TEST(SmokeTestNet, Linkage) { //testing if libs are linked properly
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
    deflateEnd(&strm);

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
