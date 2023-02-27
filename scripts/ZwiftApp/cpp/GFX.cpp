#include "ZwiftApp.h"
void GFX_SetLoadedAssetMode(bool mode) {
    g_CurrentAssetCategory = mode ? AC_1 : AC_2;
}
bool GFX_Initialize() {
    g_MinimalUI = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\MINIMAL_UI", 0, true) == 0;
    g_WorkoutDistortion = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\WORKOUTDISTORTION", true, true);
    g_bFullScreen = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\FULLSCREEN", 0, true) != 0;
    auto WINWIDTH = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\WINWIDTH", 0, true);
    auto WINHEIGHT = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\WINHEIGHT", 0, true);
    auto PREFERRED_MONITOR = g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\PREFERRED_MONITOR", -1, true);
    auto VSYNC = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\VSYNC", 1, true);
    auto GPU = g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\GPU", -1, true);
    auto GFX_TIER = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\GFX_TIER", -1, true);
    Log("Initializing graphics window of size %d x %d", WINWIDTH, WINHEIGHT);
    GFX_InitializeParams gip{};
    gip.WINWIDTH = WINWIDTH;
    gip.WINHEIGHT = WINHEIGHT;
    gip.FullScreen = g_bFullScreen;
    gip.PREFERRED_MONITOR = PREFERRED_MONITOR;
    //TODO gip.field_38 = qword_7FF71A893CD0;
    gip.VSYNC = VSYNC;
    gip.GPU = GPU;
    gip.GFX_TIER = GFX_TIER;
    gip.field_44 = 266;
    if (!GFX_Initialize(gip)) {
        Log("Failed initializing graphics");
        auto msg = GetText("LOC_ERROR_NO_GRAPHICS");
        MsgBoxAndExit(msg);
    }
    //OMIT message "update your drivers for intel hd GFX"
    glfwGetWindowSize(g_mainWindow, &g_width, &g_height);
    zassert(g_width && g_height);
    g_view_w = g_width;
    if ((g_width / g_height) <= 1.78) {
        g_view_x = 0.0;
    } else {
        g_view_w = VRAM_GetUIAspectRatio() * g_height;
        g_view_x = (g_width - g_view_w) * 0.5;
    }
    g_view_h = g_height;
    g_view_y = 0.0;
    auto perf_flags = g_UserConfigDoc.GetU32("PERF", -1, true);
    if (perf_flags != -1)
        GFX_AddPerformanceFlags(perf_flags);
    return true;
}
HWND g_Hwnd;
void MainWndCorrectByFrame(GLFWmonitor *m) {
    int xpos, ypos, left, top, right, bottom;// , w, h;
    if (m) {
        glfwGetMonitorPos(m, &xpos, &ypos);
        //glfwGetVideoMode(m1);
        //glfwGetWindowSize(g_mainWindow, &w, &h);
        glfwGetWindowFrameSize(g_mainWindow, &left, &top, &right, &bottom);
        glfwSetWindowPos(g_mainWindow, xpos + left, ypos + top);
    }
}
bool GFXAPI_Initialize(const GFX_InitializeParams &gip) {
    glfwSetErrorCallback(glfwZwiftErrorCallback);
    if (!glfwInit()) {
        Log("Failed GLFW init. Returning");
        ZwiftExit(1);
    }
    auto ver = gip.GlContextVer;
    int prof;
    if (ver & 0xFFC00 | ((ver & 0x3FF) << 20) | (ver >> 20) & 0x3FF)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ver & 0x3FF);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (ver >> 10) & 0x3FF);
        if ((ver & 0xFFC00 | ((ver & 0x3FF) << 20) | (ver >> 20) & 0x3FF) < 0x300800)
            prof = 0;
        else
            prof = GLFW_OPENGL_COMPAT_PROFILE - gip.GlCoreProfile;
    } else
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        prof = GLFW_OPENGL_COMPAT_PROFILE - gip.GlCoreProfile;
    }
    glfwWindowHint(GLFW_OPENGL_PROFILE, prof);
    glfwWindowHint(GLFW_VISIBLE, 1);
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    int mons;
    auto pMons = glfwGetMonitors(&mons);
    auto pMon = glfwGetPrimaryMonitor();
    Log("Attached Monitors:\n");
    const char *fmt;
    for(int curMon = 0; curMon < mons; ++curMon, ++pMons) {
        int left, top;
        glfwGetMonitorPos(*pMons, &left, &top);
        if (((top + 0x8000) | ((left << 16) + 0x80000000)) == gip.PREFERRED_MONITOR) {
            pMon = *pMons;
            fmt = "preferred monitor found at: %d,%d\n";
        } else {
            fmt = "monitor found at: %d,%d\n";
        }
        Log(fmt, left, top);
    }
    SetProcessDPIAware();
    int cxs = GetSystemMetrics(SM_CXSCREEN), cys = GetSystemMetrics(SM_CYSCREEN);
    if (pMon != glfwGetPrimaryMonitor()) {
        auto m = glfwGetVideoMode(pMon);
        if (m) {
            cys = m->height;
            cxs = m->width;
        }
    }
    bool badWH = (!gip.WINWIDTH || !gip.WINHEIGHT);
    auto tryh = gip.WINHEIGHT, tryw = gip.WINWIDTH;
    if (badWH || gip.FullScreen) {
        tryh = cys;
        tryw = cxs;
    }
    auto selMon = gip.FullScreen ? pMon : nullptr;
    g_mainWindow = glfwCreateWindow(tryw, tryh, "Zwift", selMon, nullptr);
    if (!g_mainWindow) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, 0);
        g_mainWindow = glfwCreateWindow(gip.WINWIDTH, gip.WINHEIGHT, "Zwift", selMon, nullptr);
        if (!g_mainWindow) {
            g_openglFail = true;
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, 0);
            g_mainWindow = glfwCreateWindow(gip.WINWIDTH, gip.WINHEIGHT, "Zwift", selMon, nullptr);
            if (!g_mainWindow) {
                Log("Could not create graphics window of size %d x %d", gip.WINWIDTH, gip.WINHEIGHT);
                glfwTerminate();
                MessageBoxA(
                    nullptr,
                    "Zwift requires OpenGL 3.1 or higher to run.  Don't worry though, your computer probably supports it, you may j"
                    "ust need to update your graphics drivers. If you need help with this, contact support@zwift.com",
                    "Uh Oh!",
                    MB_OK);
                ZwiftExit(1);
            }
        }
    }
    if (pMon)
        MainWndCorrectByFrame(pMon);
    glfwMakeContextCurrent(g_mainWindow);
    g_Hwnd = glfwGetWin32Window(g_mainWindow);
    glfwSwapInterval(-glfwExtensionSupported("WGL_EXT_swap_control_tear"));
    if (badWH && !gip.FullScreen) {
        int left, top, right, bottom;
        glfwGetWindowFrameSize(g_mainWindow, &left, &top, &right, &bottom);
        glfwSetWindowSize(g_mainWindow, tryw - left - right, tryh - top - bottom);
    }
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        char buf[128];
        sprintf(buf, "glewInit returned: %d", (int)err);
        Log(buf);
        MsgBoxAndExit(buf);
    }
    g_GL_vendor = (const char *)glGetString(GL_VENDOR);
    g_GL_renderer = (const char *)glGetString(GL_RENDERER);
    g_GL_apiName = (const char *)glGetString(GL_VERSION);
    if (gip.m_vendorName)
        g_GL_vendor = gip.m_vendorName;
    if (gip.m_renderer)
        g_GL_renderer = gip.m_renderer;
    if (!g_GL_apiName) {
        Log("GLver == NULL!");
        MsgBoxAndExit(GetText("LOC_ERROR_NO_GRAPHICS"));
    }
    if (g_GL_vendor && g_GL_renderer) {
        Log("GFX_Initialize: GL_version = %s", g_GL_apiName);
        Log("                GL_vendor = %s", g_GL_vendor);
        Log("                GL_render = %s", g_GL_renderer);
    }
    auto glVerMajor = atoi(g_GL_apiName);
    auto glVerMinor = atoi(g_GL_apiName + 2);
    if (strstr(g_GL_renderer, "GDI") || glVerMajor == 1) {
        char buf[4096];
        sprintf(buf, "%s\n\nVersion: %s\nVendor: %s\nRenderer: %s", GetText("LOC_ERROR_UNSUPPORTED_GL"), g_GL_apiName, g_GL_vendor, g_GL_renderer);
        Log("GDI renderer found. Failed to get sufficient openGL implementation");
        MsgBoxAndExit(buf);
    }
    auto v49 = 1023;
    auto v50 = 1023;
    if (glVerMinor < 0x3FF)
        v49 = glVerMinor;
    auto v51 = (v49 & 0x3FF) << 10;
    if (glVerMajor < 0x3FF)
        v50 = glVerMajor;
    g_glVersion = v50 & 0x3FF | v51;
    Log("Checking Extensions");
    auto glv = g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | (g_glVersion >> 20) & 0x3FF;
    if (glv < 0x300000 && !GLEW_EXT_framebuffer_object) {
        auto err = "EXT_framebuffer_object OpenGL extension is required.  Try updating your video card drivers.";
        LogTyped(LOG_ERROR, err);
        MsgBoxAndExit(err);
    }
    if (glv < 0x300000 && !glDeleteFramebuffersEXT) {
        auto err = "glDeleteFramebuffersEXT OpenGL extension is required.  Try updating your video card drivers.";
        LogTyped(LOG_ERROR, err);
        MsgBoxAndExit(err);
    }
    if (glv < 0x300000 && !glDeleteRenderbuffersEXT) {
        auto err = "glDeleteRenderbuffersEXT OpenGL extension is required.  Try updating your video card drivers.";
        LogTyped(LOG_ERROR, err);
        MsgBoxAndExit(err);
    }
    if (glv < 0x200000 && !GLEW_ARB_shader_objects) {
        MsgBoxAndExit("ARB_shader_objects OpenGL extension is required.  Try updating your video card drivers.");
    }
    if (GFX_CheckExtensions()) {
        //auto _GLEW_EXT_debug_marker = GLEW_EXT_debug_marker;        //not used = 0
        //auto _GLEW_NVX_gpu_memory_info = GLEW_NVX_gpu_memory_info;  //not used = 1
        g_openglCore = glfwGetWindowAttrib(g_mainWindow, GLFW_OPENGL_PROFILE) == GLFW_OPENGL_CORE_PROFILE;
        Log("[GFX]: %s profile", g_openglCore ? "Core" : "Compatability");
        auto clamper = glClampColor;
        auto clamper_arg = GL_CLAMP_READ_COLOR;
        if (glClampColor) {
            if (!g_openglCore) {
                glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
                glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
            }
        } else {
            clamper = glClampColorARB;
            if (!g_openglCore) {
                glClampColorARB(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
                glClampColorARB(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
            }
            clamper_arg = GL_CLAMP_FRAGMENT_COLOR;
        }
        if (clamper) {
            clamper(clamper_arg, GL_FALSE);
        }
        if (!g_openglCore)
            glEnable(GL_POINT_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
        if (g_openglCore) {
            glGenVertexArrays(1, &g_CoreVA);
            glBindVertexArray(g_CoreVA);
        }
        g_gfxShaderModel = (glv < 0x400C00) ? 1 : 4;
        if (glv >= 0x300800 || GLEW_ARB_seamless_cube_map)
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        if (g_gfxCaps.uniform_buffer_object) {
            glGenBuffers(_countof(g_UBOs), g_UBOs);
            for (int i = 0; i < _countof(g_UBOs); ++i)
                glBindBufferBase(GL_UNIFORM_BUFFER, i, g_UBOs[i]);
        }
        g_gfxTier = 0;
        Log("[GFX]: GFX_Tier %d", g_gfxTier);
        int rb, gb, bb, ab;
        glGetIntegerv(GL_RED_BITS, &rb);
        glGetIntegerv(GL_GREEN_BITS, &gb);
        glGetIntegerv(GL_BLUE_BITS, &bb);
        glGetIntegerv(GL_ALPHA_BITS, &ab);
        if (rb != 8 || gb != 8 || bb != 8 || (g_colorChannels = 3, ab))
            g_colorChannels = 4;
        //g_floatConsts12 init statically
        GFX_SetFillMode(GFM_FILL);
        GFX_SetStencilFunc(false, GCF_ALWAYS, 0xFFu, 0xFFu, GSO_KEEP, GSO_KEEP, GSO_KEEP);
        GFX_SetStencilRef(0);
        for (int at = 0; at < g_gfxCaps.max_color_atchs; at++)
            GFX_SetColorMask(at, 15);
        return true;
    }
    return false;
}
void GFX_SetColorMask(uint64_t idx, uint8_t mask) {
    if (idx < _countof(g_pGFX_CurrentStates->m_colorMask) && g_pGFX_CurrentStates->m_colorMask[idx] != mask) {
        g_pGFX_CurrentStates->m_colorMask[idx] = mask;
        if (idx) {
            if (glColorMaskIndexedEXT) {
                glColorMaskIndexedEXT((uint32_t)idx, mask & 1, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0);
            }
        } else {
            glColorMask(mask & 1, (mask & 2) != 0, (mask & 4) != 0, (mask & 8) != 0);
        }
    }
}
const uint32_t g_stensilFuncs[] = {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};
const uint32_t g_stensilOps[] = {GL_FALSE, GL_KEEP, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT, GL_FALSE, GL_FALSE};
void GFX_SetStencilRef(uint8_t ref) {
    if (g_pGFX_CurrentStates->m_stensilRef != ref) {
        glStencilFunc(g_stensilFuncs[g_pGFX_CurrentStates->m_stensilFunc], ref, g_pGFX_CurrentStates->m_stensilFuncMask);
        g_pGFX_CurrentStates->m_stensilRef = ref;
    }
}
void GFX_SetStencilFunc(bool enableTest, GFX_COMPARE_FUNC compareFunc, uint8_t stensilFuncMask, uint8_t stensilMask, GFX_StencilOp sfail, GFX_StencilOp dpfail, GFX_StencilOp dppass) {
    if (g_pGFX_CurrentStates->m_enableStensilTest != enableTest) {
        if (enableTest)
            glEnable(GL_STENCIL_TEST);
        else
            glDisable(GL_STENCIL_TEST);
        g_pGFX_CurrentStates->m_enableStensilTest = enableTest;
    }
    if (g_pGFX_CurrentStates->m_stensilFunc != compareFunc || g_pGFX_CurrentStates->m_stensilFuncMask != stensilFuncMask) {
        glStencilFunc(g_stensilFuncs[compareFunc], g_pGFX_CurrentStates->m_stensilRef, stensilFuncMask);
        g_pGFX_CurrentStates->m_stensilFunc = compareFunc;
        g_pGFX_CurrentStates->m_stensilFuncMask = stensilFuncMask;
    }
    if (g_pGFX_CurrentStates->m_stensilMask != stensilMask) {
        glStencilMask(stensilMask);
        g_pGFX_CurrentStates->m_stensilMask = stensilMask;
    }
    if (g_pGFX_CurrentStates->m_sopsFail != sfail || g_pGFX_CurrentStates->m_sopdpFail != dpfail || g_pGFX_CurrentStates->m_sopdpPass != dppass) {
        glStencilOp( g_stensilOps[sfail], g_stensilOps[dpfail], g_stensilOps[dppass]);
        g_pGFX_CurrentStates->m_sopsFail = sfail;
        g_pGFX_CurrentStates->m_sopdpFail = dpfail;
        g_pGFX_CurrentStates->m_sopdpPass = dppass;
    }
}
void GetMonitorCaps(MonitorInfo *dest) {
    auto hmon = MonitorFromWindow(g_Hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfoA(hmon, &mi);
    dest->m_width = abs(mi.rcMonitor.left - mi.rcMonitor.right);
    dest->m_height = abs(mi.rcMonitor.top - mi.rcMonitor.bottom);
    GetDpiForMonitor(hmon, MDT_RAW_DPI, &dest->m_dpiX, &dest->m_dpiY);
    dest->m_dpiPhys = dest->m_dpiX;
    DEVICE_SCALE_FACTOR dsf;
    GetScaleFactorForMonitor(hmon, &dsf);
    dest->m_dsf = (float)dsf / 100.0;
}
bool GFX_Initialize(const GFX_InitializeParams &gip) {
    if (GFXAPI_Initialize(gip)) {
        MonitorInfo mi;
        GetMonitorCaps(&mi);
        Log("Screen reported DPI: %d physical DPI: %d Scale: %.2f", mi.m_dpiX, mi.m_dpiPhys, mi.m_dsf);
        g_bGFXINITIALIZED = true;
        GFX_MatrixStackInitialize();
        Log("Calculating Graphics Score");
        GFXAPI_CalculateGraphicsScore();
        if (g_GFX_PerformanceFlags & 0x200000)
            g_renderDetailed = DR_NO;
        else if (g_GFX_PerformanceFlags & 0x400000)
            g_renderDetailed = DR_MIDDLE;
        Log("Initializing Render Targets");
        VRAM_Initialize(gip.HasPickingBuf);
        Log("Initializing Texture Systems");
        GFX_TextureSys_Initialize();
        g_BlurShaderHandle = GFX_CreateShaderFromFile("Blur", -1);
        g_debugFont.Load(FS_0);
        GDEMESH_Initialize();
        return true;
    }
    return false;
}
void GFXAPI_CreateTextureFromRGBA(int idx, uint32_t w, uint32_t h, const void *data, bool genMipMap) { //GFXAPI_CreateTextureFromRGBA_idx
    auto id = &g_Textures[idx].m_glid;
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    g_VRAMBytes_Textures += 4 * h * w;
    if (genMipMap && glGenerateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
}
int GFXAPI_CreateTextureFromRGBA(uint32_t w, uint32_t h, const void *data, bool genMipMap) {
    if (g_nTexturesLoaded >= _countof(g_Textures) - 1)
        return -1;
    auto &t = g_Textures[g_nTexturesLoaded];
    t.m_loaded &= ~1;
    t.m_bestHeight = h;
    t.m_bestWidth = w;
    t.m_align= 1;
    t.m_field_20_5 = 5;
    GFXAPI_CreateTextureFromRGBA(g_nTexturesLoaded, w, h, data, genMipMap);
    return g_nTexturesLoaded++;
}
void GFX_TextureSys_Initialize() {
    //memset(g_Textures, 0, sizeof(g_Textures)); //� ��� ����� ����� ������
    for (auto &i : g_Textures)
        i.m_field_20_5 = -1; //������ �� id ???
    memset(g_WhiteTexture, 255, sizeof(g_WhiteTexture));
    g_WhiteHandle = GFXAPI_CreateTextureFromRGBA(32, 32, g_WhiteTexture, true);
}
int GFX_CreateShaderFromFile(const char *fileName, int handle) {
    GFX_CreateShaderParams s{ fileName };
    return GFX_CreateShaderFromFile(s, handle);
}
int GFX_Internal_GetNextShaderHandle() {
    auto ret = g_nShadersLoaded++;
    zassert(g_nShadersLoaded < MAX_SHADERS);
    if (g_nShadersLoaded >= MAX_SHADERS)
        return MAX_SHADERS - 1;
    return ret;
}
int GFX_CreateShaderFromFile(const GFX_CreateShaderParams &s, int handle) {
    char buf[272];
    sprintf_s(buf, sizeof(buf), "%s[%d][%d]", s.m_name, s.m_vertIdx, s.m_fragIdx);
    auto shaderId = SIG_CalcCaseInsensitiveSignature(buf);
    if (handle == -1) {
        auto res = g_ShaderMap.find(shaderId);
        if (res != g_ShaderMap.end())
            return res->second;
    }
    DWORD t = timeGetTime();
    if (handle == -1)
        handle = GFX_Internal_GetNextShaderHandle();
    int ret = GFXAPI_CreateShaderFromFile(handle, s);
    if (ret == -1)
        Log("GFX_CreateShaderFromFile(%s[%d][%d]) failed!", s.m_name, s.m_vertIdx, s.m_fragIdx);
    g_TotalShaderCreationTime += timeGetTime() - t;
    g_ShaderMap[shaderId] = ret;
    return ret;
}
struct GFX_ShaderMetadata { //32 bytes
    uint64_t m_modelIndex;
    const char *m_readableName, *m_vertexExt, *m_fragmentExt;
};
const GFX_ShaderMetadata g_shaderMetadata[] = {
  { 4, "GLSL/430", "zvsh", "zfsh" },
  { 2, "GLSL/140", "zvsh", "zfsh" },
  { 1, "GLSL/120", "zvsh", "zfsh" },
  { 1, "Final",    "vsh",  "psh" }
};
struct ZData {
    uint32_t m_nul;
    uint32_t m_offset;
    int m_len;
    int gap[3];
};
template<class T> struct ZArray {
    uint64_t m_count;
    T *m_ppData;
};
struct ZDataFile { //104 (0x68) bytes
    char m_signature[8]; //GATD HSZ
    int m_version;       //1
    int m_headerLength;  //0x68
    uint64_t field_10;   //0x400
    ZArray<uint64_t> m_dir;
    ZArray<ZData>    m_files;
    uint64_t m_sumLength;
    const uint8_t *m_payload;
    uint64_t field_48;
    const uint8_t *m_endPtr;
    uint64_t field_58;
    const uint8_t *m_endPtrBackup;
    ZDataFile() {
        size_t delta = (size_t)this;
        m_dir.m_ppData = (uint64_t *)((char *)m_dir.m_ppData + delta); // offsets to pointers
        m_files.m_ppData = (ZData *)((char *)m_files.m_ppData + delta);
        m_payload += delta;
        m_endPtr += delta;
        m_endPtrBackup += delta;
    }
    int Parse(uint16_t logicIdx, uint64_t **ppPhysIdx, ZData **ppDataDest) {
        if (logicIdx >= m_dir.m_count)
            return -1;
        auto dir = *ppPhysIdx = m_dir.m_ppData + logicIdx;
        if (*dir >= m_files.m_count) {
            return -1;
        } else {
            *ppDataDest = m_files.m_ppData + *dir;
            return 0;
        }
    }
};
int GFXAPI_ParseShaderFromZData(std::vector<byte> &vectorDest, ZDataFile **ppFileDest, uint64_t **ppPhysIdx, ZData **ppDataDest, void *data, size_t size, const GFX_CreateShaderParams &s, bool isFragment) {
    auto logicIdx = isFragment ? s.m_fragIdx : s.m_vertIdx;
    static_assert(104 == sizeof(ZDataFile));
    if (size < sizeof(ZDataFile))
        return -1;
    vectorDest.resize(size);
    memcpy(vectorDest.data(), data, size);
    *ppFileDest = new (vectorDest.data()) ZDataFile();
    return (*ppFileDest)->Parse(logicIdx, ppPhysIdx, ppDataDest);
}
int GFXAPI_ParseShaderFromZData(std::vector<byte> &vectorDest, ZDataFile **ppFileDest, uint64_t **ppPhysIdx, ZData **ppDataDest, const char *name, const GFX_CreateShaderParams &s, bool isFragment) {
    auto fullName = GAMEPATH(name);
    std::ifstream in(fullName, std::ios::ate);
    size_t size;
    if (in.good() && (size = in.tellg(), size > sizeof(ZDataFile))) {
        in.seekg(0, std::ios::beg);
        vectorDest.resize(size);
        auto data = vectorDest.data();
        in.read((char *)data, size);
        *ppFileDest = new (vectorDest.data()) ZDataFile();
        auto logicIdx = isFragment ? s.m_fragIdx : s.m_vertIdx;
        return (*ppFileDest)->Parse(logicIdx, ppPhysIdx, ppDataDest);
    }
    return -1;
}
int GFXAPI_CreateShaderFromZDataFile(const GFX_CreateShaderParams &s, int handle, const char *readableName, uint8_t modelIndex) {
    int ret = -1;
    char zvsh[MAX_PATH], zfsh[MAX_PATH];
    sprintf_s(zvsh, "data/shaders/%s/%s.zvsh", readableName, s.m_name);
    sprintf_s(zfsh, "data/shaders/%s/%s.zfsh", readableName, s.m_name);
    time_t touchV, touchF;
    uint64_t *dummy;
    auto pFileHdrV = g_WADManager.GetWadFileHeaderByItemName(zvsh + 5, WAD_ASSET_TYPE::SHADER, &touchV);
    auto pFileHdrF = g_WADManager.GetWadFileHeaderByItemName(zfsh + 5, WAD_ASSET_TYPE::SHADER, &touchF);
    std::vector<byte> V, F;
    ZData *zdVsh, *zdFsh;
    ZDataFile *dataFileV, *dataFileF;
    if (pFileHdrV && pFileHdrF && touchV != -1 && touchF != -1) {
        if (GFXAPI_ParseShaderFromZData(V, &dataFileV, &dummy, &zdVsh, pFileHdrV->FirstChar(), pFileHdrV->m_fileLength, s, false) < 0)
            return ret;
        if (GFXAPI_ParseShaderFromZData(F, &dataFileF, &dummy, &zdFsh, pFileHdrF->FirstChar(), pFileHdrF->m_fileLength, s, true) < 0)
            return ret;
    }
    if (ret == -1) {
        if (GFXAPI_ParseShaderFromZData(V, &dataFileV, &dummy, &zdVsh, zvsh, s, false) < 0)
            return ret;
        if (GFXAPI_ParseShaderFromZData(F, &dataFileF, &dummy, &zdFsh, zfsh, s, true) < 0)
            return ret;
    }
    ret = GFXAPI_CreateShaderFromBuffers(
        handle,
        zdVsh->m_len,
        (const char *)dataFileV->m_payload + zdVsh->m_offset,
        zvsh,
        zdFsh->m_len,
        (const char *)dataFileV->m_payload + zdVsh->m_offset,
        zfsh);
    if (ret != -1) {
        g_Shaders[ret].m_vertIdx = s.m_vertIdx;
        g_Shaders[ret].m_fragIdx = s.m_fragIdx;
        g_Shaders[ret].m_modelIndex = modelIndex;
    }
    return ret;
}
const char *g_ShaderConstantNames[/*29*/] = {
    "g_vEyePos",
    "g_Time",
    "g_LightDir",
    "g_LightColor",
    "g_ShadowColor",
    "headlightDirBrightness",
    "headlightWorldPosition",
    "g_TextureOffsets",
    "g_FogParams",
    "g_FogColor",
    "g_OutputColorExp",
    "g_ShadowPixelParams",
    "g_ShadowScaleParams",
    "g_ShadowBiasParams",
    "g_ShadowBounds[0]",
    "g_ShadowBounds[1]",
    "g_ShadowBounds[2]",
    "g_ShadowBounds[3]",
    "windMagnitude",
    "g_ClipPlane",
    "g_RenderBufferResolution",
    "g_FresnelParams",
    "g_SpecularParams",
    "g_MaterialProps",
    "g_Color",
    "g_HighlightSFXColor",
    "g_HighlightSFXFresnelParams",
    "tints",
    "ditherData"
};
const char *g_ShaderMatrixNames[/*9*/] = {
    "g_L2W",
    "g_WVP",
    "g_W2V",
    "g_PROJ",
    "g_SpotlightMatrix",
    "g_ShadowmapMatrix0",
    "g_ShadowmapMatrix1",
    "g_ShadowmapMatrix2",
    "g_ShadowmapMatrix3"
};
const char *g_ShaderMatrixArrayNames[/*2*/] = { "instanceMatrix", "g_Bones" };
const char *g_ShaderSamplerNames[/*16*/] = {
    "g_DiffuseTex",
    "g_NormalGlossTex",
    "g_Diffuse2Tex",
    "g_NormalGloss2Tex",
    "g_Diffuse3Tex",
    "g_NormalGloss3Tex",
    "g_CausticTex",
    "g_EnvMapTex",
    "g_DitherTex",
    "g_headlightPatternSampler",
    "g_ShadowMap0",
    "g_ShadowMap1",
    "g_ShadowMap2",
    "g_SpotlightShadowMap",
    "g_VertexTexture0",
    "g_VertexTexture1"
};
const char *g_ShaderAttributeNames[/*12*/] = {
    "inPos",
    "inNorm",
    "inTan",
    "inBinorm",
    "inColor0",
    "inColor1",
    "inTexCoord0",
    "inTexCoord1",
    "inIndices",
    "inWeights",
    "inPos_alt",
    "inNorm_alt"
};
void GFX_Internal_fixupShaderAddresses(GFX_ShaderPair *pShader) {
    glUseProgram(pShader->m_program);
    static_assert(_countof(g_ShaderConstantNames) == _countof(pShader->m_locations));
    auto pLoc = &pShader->m_locations[0];
    for (auto n : g_ShaderConstantNames) {
        auto ul = glGetUniformLocation(pShader->m_program, n);
        *pLoc++ = ul;
    }
    static_assert(_countof(g_ShaderMatrixNames) == _countof(pShader->m_matLocations));
    auto pMatLoc = &pShader->m_matLocations[0];
    for (auto n : g_ShaderMatrixNames) {
        auto ul = glGetUniformLocation(pShader->m_program, n);
        *pMatLoc++ = ul;
    }
    static_assert(_countof(g_ShaderMatrixArrayNames) == _countof(pShader->m_matArrLocations));
    auto pMatArLoc = &pShader->m_matArrLocations[0];
    for (auto n : g_ShaderMatrixArrayNames) {
        auto ul = glGetUniformLocation(pShader->m_program, n);
        *pMatArLoc++ = ul;
    }
    static_assert(_countof(g_ShaderSamplerNames) == _countof(pShader->m_samplers));
    auto pSampler = &pShader->m_matArrLocations[0];
    for (auto n : g_ShaderSamplerNames) {
        auto ul = glGetUniformLocation(pShader->m_program, n);
        *pSampler++ = ul;
    }
    static_assert(_countof(g_ShaderAttributeNames) == _countof(pShader->m_attribLocations));
    auto pAttrLoc = &pShader->m_attribLocations[0];
    for (auto n : g_ShaderAttributeNames) {
        auto ul = glGetAttribLocation(pShader->m_program, n);
        *pAttrLoc++ = ul;
    }
    for (auto &i : pShader->m_field_16C)
        i = 0x80000000;
    glUseProgram(0);
    g_pGFX_CurrentStates->m_shader = -1;
}
int GFXAPI_CreateShaderFromBuffers(int handle, int vshLength, const char *vshd, const char *vsh, int pshLength, const char *pshd, const char *psh) {
    bool newHandle = (handle == -1);
    if (newHandle)
        handle = GFX_Internal_GetNextShaderHandle();
    auto curShader = &g_Shaders[handle];
    if (curShader->m_vshId)
        glDeleteShader(curShader->m_vshId);
    curShader->m_vshId = glCreateShader(GL_VERTEX_SHADER);
    if (curShader->m_fshId)
        glDeleteShader(curShader->m_fshId);
    curShader->m_fshId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(curShader->m_vshId, 1, &vshd, &vshLength);
    glShaderSource(curShader->m_fshId, 1, &pshd, &pshLength);
    glCompileShader(curShader->m_vshId);
    int vCompileStatus = 0, vInfoLog = 0, dummyLength;
    glGetShaderiv(curShader->m_vshId, GL_COMPILE_STATUS, &vCompileStatus);
    glGetShaderiv(curShader->m_vshId, GL_INFO_LOG_LENGTH, &vInfoLog);
    if (vInfoLog > 1) {
        auto v22 = (char *)malloc(vInfoLog);
        glGetShaderInfoLog(curShader->m_vshId, vInfoLog, &dummyLength, v22);
        if (!vCompileStatus)
            LogTyped(LOG_ERROR, "[GFX]: GLSLvsherr log: %s", v22);
        free(v22);
    }
    if (vCompileStatus) {
        glCompileShader(curShader->m_fshId);
        glGetShaderiv(curShader->m_fshId, GL_COMPILE_STATUS, &vCompileStatus);
        glGetShaderiv(curShader->m_fshId, GL_INFO_LOG_LENGTH, &vInfoLog);
        if (vInfoLog > 1) {
            auto v24 = (char *)malloc(vInfoLog);
            glGetShaderInfoLog(curShader->m_fshId, vInfoLog, &dummyLength, v24);
            if (!vCompileStatus)
                LogTyped(LOG_ERROR, " [GFX]: GLSLpsherr log: %s", v24);
            free(v24);
        }
        if (vCompileStatus) {
            if (curShader->m_program)
                glDeleteProgram(curShader->m_program);
            curShader->m_program = glCreateProgram();
            glAttachShader(curShader->m_program, curShader->m_vshId);
            glAttachShader(curShader->m_program, curShader->m_fshId);
            uint32_t i = 0;
            for (auto san : g_ShaderAttributeNames)
                glBindAttribLocation(curShader->m_program, i++, san);
            glLinkProgram(curShader->m_program);
            int linkStatus;
            glGetProgramiv(curShader->m_program, GL_LINK_STATUS, &linkStatus);
            vInfoLog = 0;
            dummyLength = 0;
            glGetProgramiv(curShader->m_program, GL_INFO_LOG_LENGTH, &vInfoLog);
            if (vInfoLog > 1) {
                auto v31 = (char *)malloc(vInfoLog);
                glGetProgramInfoLog(curShader->m_program, vInfoLog, &dummyLength, v31);
                if (!linkStatus)
                    LogTyped(LOG_ERROR, "[GFX]: GLSLprogerr log: %s", v31);
                free(v31);
            }
            if (linkStatus) {
                Log("Shader Loaded successfully.  %s", psh);
                GFX_Internal_fixupShaderAddresses(curShader);
                return handle;
            }
        }
    }
    if (newHandle)
        --g_nShadersLoaded;
    return -1;
}
int GFXAPI_CreateShaderFromPFiles(const char *vsh, const char *psh, int handle) {
    auto fvsh = fopen(GAMEPATH(vsh), "rb");
    int ret = -1;
    if (fvsh) {
        fseek(fvsh, 0, SEEK_END);
        auto vshLength = ftell(fvsh);
        fseek(fvsh, 0, SEEK_SET);
        auto vshd = (char *)malloc(vshLength + 1);
        vshd[vshLength] = 0;
        fread(vshd, 1, vshLength, fvsh);
        fclose(fvsh);
        LogTyped(LOG_ASSET, "Loaded Vertex Shader asset %s", vsh);
        auto fpsh = fopen(GAMEPATH(psh), "rb");
        if (fpsh) {
            fseek(fpsh, 0, SEEK_END);
            auto pshLength = ftell(fpsh);
            fseek(fpsh, 0, SEEK_SET);
            auto pshd = (char *)malloc(pshLength + 1);
            pshd[pshLength] = 0;
            zassert(fread(pshd, 1, pshLength, fpsh) == pshLength);
            fclose(fpsh);
            LogTyped(LOG_ASSET, "Loaded Pixel Shader asset %s", psh);
            ret = GFXAPI_CreateShaderFromBuffers(handle, vshLength, vshd, vsh, pshLength, pshd, psh);
            if (vshd)
                free(vshd);
            if (pshd)
                free(pshd);
        } else {
            LogTyped(LOG_ERROR, "Missing Pixel Shader asset %s", psh);
            if (vshd)
                free(vshd);
            zassert(0);
        }
    } else {
        LogTyped(LOG_ERROR, "Missing Vertex Shader asset %s", vsh);
    }
    return ret;
}
int GFXAPI_CreateShaderFromPFile(const char *name, int handle) {
    char vsh[MAX_PATH], psh[MAX_PATH];
    //sprintf(FileName, "data/shaders/%s.shader", name);
    Log("Loading Shader %s", name);
    sprintf(vsh, "data/shaders/Final/%s.vsh", name);
    sprintf(psh, "data/shaders/Final/%s.psh", name);
    auto ret = GFXAPI_CreateShaderFromPFiles(vsh, psh, handle);
    GFX_ShaderPair *v8 = nullptr;
    if (ret < 0) {
        if (ret != -1 || handle < 0)
            return ret;
        v8 = &g_Shaders[handle];
    } else {
        v8 = &g_Shaders[ret];
    }
    if (v8) {
        //stat64i32(FileName, &v10);
        v8->m_modelIndex = 1;
    }
    return ret;
}
int GFXAPI_CreateShaderFromFile(int handle, const GFX_CreateShaderParams &s) {
    int result = -1;
    for (size_t i = 0; result == -1 && i < 4; ++i) {
        if (GFX_ShaderModelValue(g_gfxShaderModel) >= GFX_ShaderModelValue(g_shaderMetadata[i].m_modelIndex)) {
            if (g_shaderMetadata[i].m_fragmentExt[0] == 'z') {
                result = GFXAPI_CreateShaderFromZDataFile(s, handle, g_shaderMetadata[i].m_readableName, g_shaderMetadata[i].m_modelIndex);
            } else {
                char vsh[MAX_PATH], psh[MAX_PATH];
                sprintf_s(vsh, "shaders/Final/%s.vsh", s.m_name);
                sprintf_s(psh, "shaders/Final/%s.psh", s.m_name);
                time_t wadTouchTimePsh, wadTouchTimeVsh;
                auto pshh = g_WADManager.GetWadFileHeaderByItemName(psh, WAD_ASSET_TYPE::SHADER, &wadTouchTimePsh);
                auto vshh = g_WADManager.GetWadFileHeaderByItemName(vsh, WAD_ASSET_TYPE::SHADER, &wadTouchTimeVsh);
                if (pshh && vshh) {
                    if (wadTouchTimePsh != wadTouchTimeVsh)
                        zassert(wadTouchTimePsh == wadTouchTimeVsh && "GFX_CreateShaderFromFile assertion; both PSH and VSH should come from the same WAD file");
                    sprintf_s(psh, "data/%s", pshh->m_filePath);
                    sprintf_s(vsh, "data/%s", vshh->m_filePath);
                    result = GFXAPI_CreateShaderFromBuffers(
                        handle,
                        pshh->m_fileLength,
                        (const char *)vshh->FirstChar(),
                        vsh,
                        pshh->m_fileLength,
                        (const char *)pshh->FirstChar(),
                        psh);
                    if (result >= 0) {
                        g_Shaders[result].m_modelIndex = 1;
                        return result;
                    }
                }
                result = GFXAPI_CreateShaderFromPFile(s.m_name, handle);
                if (result == -1)
                    result = GFXAPI_CreateShaderFromPFile(s.m_name, handle);
            }
        }
    }
    return result;
}
void GFX_MatrixStackInitialize() {
    //TODO
}
bool GFX_Initialize3DTVSpecs(float, float) {
    //TODO
    return true;
}
void GFX_DrawInit() {
    g_DrawBuffers[0] = malloc(g_DrawBufferSize);
    g_DrawBuffers[1] = malloc(g_DrawBufferSize);
    g_DrawNoTextureShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_NoTexture", -1);
    g_DrawTexturedShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_Textured", -1);
    g_DrawTexturedSimpleShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_Textured_Simple", -1);
    g_DrawTexturedGammaCorrectShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_Textured_GammaCorrect", -1);
    //GFXAPI_DrawInit():
    if (g_openglCore)
        glGenBuffers(1, &g_DrawPrimVBO);
}
void GFXAPI_CalculateGraphicsScore() {
    if (g_GL_vendor) {
        if (strstr(g_GL_vendor, "Intel") || strstr(g_GL_vendor, "INTEL"))
            g_bUseEmptyShadowMapsHack = true;
    }
    char vendorName[256];
    vendorName[_countof(vendorName) - 1] = 0;
    for (size_t i = 0; i < _countof(vendorName); i++) {
        vendorName[i] = std::toupper(g_GL_vendor[i]);
        if (vendorName[i] == 0)
            break;
    }
    char renderer[256];
    renderer[_countof(renderer) - 1] = 0;
    for (size_t i = 0; i < _countof(renderer); i++) {
        renderer[i] = std::toupper(g_GL_renderer[i]);
        if (renderer[i] == 0)
            break;
    }
    const GraphicsCardProfile g_GraphicsCardProfileMap[196] = {
      { "NVIDIA", "GTX TITAN", GPG_ULTRA },
      { "NVIDIA", "TITAN X", GPG_ULTRA },
      { "NVIDIA", "TITAN V", GPG_ULTRA },
      { "NVIDIA", "RTX 2080", GPG_ULTRA },
      { "NVIDIA", "RTX 2070", GPG_ULTRA },
      { "NVIDIA", "RTX 2060", GPG_ULTRA },
      { "NVIDIA", "RTX 2050", GPG_ULTRA },
      { "NVIDIA", "GTX 1660", GPG_ULTRA },
      { "NVIDIA", "GTX 1650", GPG_ULTRA },
      { "NVIDIA", "GTX 1080", GPG_ULTRA },
      { "NVIDIA", "GTX 1080", GPG_ULTRA },
      { "NVIDIA", "GTX 1070", GPG_ULTRA },
      { "NVIDIA", "GTX 1060", GPG_ULTRA },
      { "NVIDIA", "GTX 1050 Ti", GPG_ULTRA },
      { "NVIDIA", "GTX 1050", GPG_ULTRA },
      { "NVIDIA", "GTX 980", GPG_ULTRA },
      { "NVIDIA", "GTX 970", GPG_ULTRA },
      { "NVIDIA", "GTX 960", GPG_ULTRA },
      { "NVIDIA", "GTX 880", GPG_ULTRA },
      { "NVIDIA", "GTX 870", GPG_ULTRA },
      { "NVIDIA", "GTX 780", GPG_ULTRA },
      { "NVIDIA", "GTX 770", GPG_ULTRA },
      { "NVIDIA", "QUADRO P2000", GPG_HIGH },
      { "NVIDIA", "QUADRO P1000", GPG_HIGH },
      { "NVIDIA", "GTX 680", GPG_HIGH },
      { "NVIDIA", "GTX 670", GPG_HIGH },
      { "NVIDIA", "GTX 580M", GPG_MEDIUM },
      { "NVIDIA", "GTX 675M", GPG_MEDIUM },
      { "NVIDIA", "GTX 660M", GPG_MEDIUM },
      { "NVIDIA", "GTX 560M", GPG_MEDIUM },
      { "NVIDIA", "GTX 460M", GPG_MEDIUM },
      { "NVIDIA", "GTX 285M", GPG_MEDIUM },
      { "NVIDIA", "GTX 280M", GPG_MEDIUM },
      { "NVIDIA", "GTX 660", GPG_MEDIUM },
      { "NVIDIA", "GTX 645", GPG_MEDIUM },
      { "NVIDIA", "GTX 560M", GPG_BASIC },
      { "NVIDIA", "GTX 560", GPG_MEDIUM },
      { "NVIDIA", "GTX 555", GPG_MEDIUM },
      { "NVIDIA", "GTX 550", GPG_MEDIUM },
      { "NVIDIA", "GTX 460", GPG_MEDIUM },
      { "NVIDIA", "GTX 295", GPG_MEDIUM },
      { "NVIDIA", "GTX 285", GPG_MEDIUM },
      { "NVIDIA", "GTX 275", GPG_MEDIUM },
      { "NVIDIA", "GTX 260", GPG_MEDIUM },
      { "NVIDIA", "820M", GPG_BASIC },
      { "NVIDIA", "GT 750M", GPG_MEDIUM },
      { "NVIDIA", "710M", GPG_BASIC },
      { "NVIDIA", "650M", GPG_MEDIUM },
      { "NVIDIA", "635M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "630M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "610M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "555M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "540M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "525M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "520M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GTS 450", GPG_MEDIUM, 0x1'000'000 },
      { "NVIDIA", "GT 630", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 625", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 435M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 430", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 325M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 320M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 230M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 230", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "G 105M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "G105M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "G210", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 405", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 840M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 320M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 310M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE G210M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 315", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 310", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 210", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 620", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 610", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 520", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 330", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 240", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 220", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 130", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GT 120", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 9800", GPG_MEDIUM },
      { "NVIDIA", "GEFORCE 9600", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 9500", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 9400", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 9300", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 9200", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 8800", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 8600", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 8500", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 8400", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "GEFORCE 8300", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "QUADRO 1000M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "QUADRO 600", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "QUADRO FX 880M", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "QUADRO FX 580", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "RTX", GPG_ULTRA },
      { "NVIDIA", "GTX", GPG_HIGH },
      { "NVIDIA", "GEFORCE GPU", GPG_HIGH },
      { "NVIDIA", "ION", GPG_BASIC, 0x1'000'000 },
      { "NVIDIA", "NVS", GPG_BASIC, 0x1'000'000 },
      { "ATI", "RADEON VII", GPG_ULTRA },
      { "ATI", "RADEON PRO VEGA", GPG_ULTRA },
      { "ATI", "WX 4100", GPG_HIGH, 0x4'000 },
      { "ATI", "WX 5100", GPG_HIGH, 0x4'000 },
      { "ATI", "WX 7100", GPG_HIGH, 0x4'000 },
      { "ATI", "RADEON PRO W6900", GPG_ULTRA },
      { "ATI", "RADEON PRO W6800", GPG_ULTRA },
      { "ATI", "RADEON PRO W6600", GPG_HIGH },
      { "ATI", "RADEON PRO 5500M", GPG_ULTRA },
      { "ATI", "RADEON PRO 580", GPG_ULTRA },
      { "ATI", "RADEON PRO 575", GPG_ULTRA },
      { "ATI", "RADEON PRO 570", GPG_HIGH, 0x4'000 },
      { "ATI", "RADEON PRO 560", GPG_HIGH, 0x4'000 },
      { "ATI", "RADEON PRO 555", GPG_HIGH, 0x4'000 },
      { "ATI", "RADEON PRO 550", GPG_HIGH },
      { "ATI", "RADEON PRO 460", GPG_HIGH, 0x4'000 },
      { "ATI", "RADEON PRO 455", GPG_HIGH, 0x4'000 },
      { "ATI", "RADEON PRO 450", GPG_HIGH },
      { "ATI", "RX VEGA 64", GPG_ULTRA },
      { "ATI", "RX VEGA 56", GPG_ULTRA },
      { "ATI", "RADEON RX VEGA", GPG_ULTRA },
      { "ATI", "RX 6900", GPG_ULTRA },
      { "ATI", "RX 6800", GPG_ULTRA },
      { "ATI", "RX 6700", GPG_ULTRA },
      { "ATI", "RX 6600", GPG_ULTRA },
      { "ATI", "RX 5700", GPG_ULTRA },
      { "ATI", "RX 6800", GPG_ULTRA },
      { "ATI", "RX 5600 XT", GPG_ULTRA },
      { "ATI", "RX 5700 XT", GPG_ULTRA },
      { "ATI", "RX 6700 XT", GPG_ULTRA },
      { "ATI", "AMD RADEON RX 6750 XT", GPG_ULTRA },
      { "ATI", "RX 6800 XT", GPG_ULTRA },
      { "ATI", "RX 6900 XT", GPG_ULTRA },
      { "ATI", "RX 590", GPG_ULTRA },
      { "ATI", "RX 580", GPG_ULTRA },
      { "ATI", "RX 570", GPG_HIGH },
      { "ATI", "RX 560", GPG_HIGH },
      { "ATI", "RX 480", GPG_ULTRA },
      { "ATI", "RX 470", GPG_HIGH },
      { "ATI", "RX 460", GPG_HIGH },
      { "ATI", "R4", GPG_BASIC, 0x1'000'000 },
      { "ATI", "R5", GPG_BASIC, 0x1'000'000 },
      { "ATI", "R6", GPG_BASIC, 0x1'000'000 },
      { "ATI", "R7 200", GPG_MEDIUM },
      { "ATI", "R7 370", GPG_HIGH },
      { "ATI", "R7 360", GPG_MEDIUM },
      { "ATI", "VEGA 8 MOBILE", GPG_MEDIUM },
      { "ATI", "R7", GPG_BASIC, 0x1'000'000 },
      { "ATI", "R8", GPG_MEDIUM },
      { "ATI", "R9 M295X", GPG_ULTRA },
      { "ATI", "R9", GPG_HIGH },
      { "ATI", "RADEON HD PITCAIRN XT PROTOTYPE", GPG_HIGH },
      { "ATI", "FIREPRO D700", GPG_ULTRA },
      { "ATI", "FIREPRO D500", GPG_HIGH },
      { "ATI", "FIREPRO D300", GPG_HIGH },
      { "ATI", "RADEON HD 7900", GPG_HIGH },
      { "ATI", "RADEON HD 7800", GPG_HIGH },
      { "ATI", "RADEON HD 7970", GPG_HIGH },
      { "ATI", "RADEON HD 7950", GPG_HIGH },
      { "ATI", "RADEON HD 6970", GPG_HIGH },
      { "ATI", "RADEON HD 6900", GPG_HIGH },
      { "ATI", "RADEON HD 6800", GPG_HIGH },
      { "ATI", "RADEON HD 5800", GPG_HIGH },
      { "ATI", "RADEON HD 6770", GPG_MEDIUM },
      { "ATI", "6770M", GPG_BASIC, 0x1'000'000 },
      { "ATI", "6750M", GPG_BASIC, 0x1'000'000 },
      { "ATI", "ATI RADEON 3000", GPG_BASIC, 0x1'000'000 },
      { "ATI", "ATI RADEON HD 7500", GPG_BASIC, 0x1'000'000 },
      { "ATI", "MOBILITY RADEON HD 4200", GPG_BASIC, 0x1'000'000 },
      { "ATI", "MOBILITY RADEON HD 4250", GPG_BASIC, 0x1'000'000 },
      { "ATI", "RADEON HD 6320", GPG_BASIC, 0x1'000'000 },
      { "INTEL", "IRIS(TM) PRO GRAPHICS 6200", GPG_BASIC, 0x125 },
      { "INTEL", "IRIS(TM) GRAPHICS 6100", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "IRIS(TM) PRO GRAPHICS 580", GPG_BASIC, 0x125 },
      { "INTEL", "IRIS(TM) PRO GRAPHICS 560", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "IRIS(TM) PRO GRAPHICS 550", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "IRIS(TM) PRO GRAPHICS 540", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "IRIS(TM) GRAPHICS 540", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "IRIS PRO", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "IRIS", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "HD GRAPHICS 2", GPG_BASIC, GPF_NO_AUTO_BRIGHT | GPF_NO_COLOR_CLAMP | 0x3'000'099 },
      { "INTEL", "HD GRAPHICS 3", GPG_BASIC, GPF_NO_AUTO_BRIGHT | GPF_NO_COLOR_CLAMP | 0x3'000'099 },
      { "INTEL", "HD GRAPHICS 4", GPG_BASIC, GPF_NO_AUTO_BRIGHT | GPF_NO_COLOR_CLAMP | 0x3'000'095 },
      { "INTEL", "HD GRAPHICS 5", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "HD GRAPHICS 6", GPG_BASIC, 0x1'000'125 },
      { "INTEL", "HD GRAPHICS", GPG_BASIC, GPF_NO_AUTO_BRIGHT | GPF_NO_COLOR_CLAMP | 0x3'000'099 },
      { "INTEL", "INTEL(R) ARC(TM) A770 GRAPHICS", GPG_ULTRA },
      { "AMD", "", GPG_MEDIUM },
      { "NVIDIA", "", GPG_MEDIUM },
      { "ATI", "", GPG_BASIC },
      { "INTEL", "", GPG_BASIC, GPF_NO_AUTO_BRIGHT | 0x1'000'000 },
      { "VMWARE", "", GPG_BASIC, GPF_NO_AUTO_BRIGHT },
      //{ NULL, NULL, GPG_BASIC }
    };
    for (const auto &i : g_GraphicsCardProfileMap) {
        if (strstr(vendorName, i.m_vendorName) && strstr(renderer, i.m_renderer)) {
            Log("GFX: Found a vendor/model match for %s", i.m_renderer);
            g_GFX_Performance = i.m_pg;
            g_GFX_PerformanceFlags = i.m_perfFlags;
            if (!glClampColorARB && !glClampColor)
                g_GFX_PerformanceFlags |= GPF_NO_COLOR_CLAMP;
            if (strstr("GEFORCE GPU", i.m_renderer)) {
                int32_t tmp = 0;
                glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &tmp);
                if (tmp != -1 && tmp / 1024 < 2047) //want 2GB VRAM
                    g_GFX_Performance = GPG_MEDIUM;
            }
            return;
        }
    }
    LogTyped(LOG_WARNING, "GFX: NO MATCH for vendor/model %s", g_GL_renderer);
}
int64_t GFX_GetPerformanceFlags() {
    return g_GFX_PerformanceFlags;
}
void GFX_AddPerformanceFlags(uint64_t f) {
    g_GFX_PerformanceFlags |= f;
}
PerformanceGroup GFX_GetPerformanceGroup() {
    return g_GFX_Performance;
}
void GFX_SetMaxFPSOnBattery(float fps) {
    g_TargetBatteryFPS = fps;
}
const char *GFX_GetVersion() {
    if (g_openglDebug)
    {
        if (g_openglCore)
            return "OpenGL Debug Core";
        else
            return "OpenGL Debug";
    } else {
        if (!g_openglCore)
            return "OpenGL";
    }
    return "OpenGL Core";
}
const char *GFX_GetAPIName() { return g_GL_apiName; }
const char *GFX_GetVendorName() { return g_GL_vendor; }
const char *GFX_GetRendererName() { return g_GL_renderer; }
bool GFX_CheckExtensions() {
    g_gfxCaps.draw_elements_base_vertex = (GLEW_ARB_draw_elements_base_vertex || GLEW_EXT_draw_elements_base_vertex);
    g_gfxCaps.ARB_base_instance = GLEW_ARB_base_instance != 0;
    g_gfxCaps.instanced_arrays = (GLEW_ARB_instanced_arrays || GLEW_EXT_instanced_arrays);
    g_gfxCaps.field_6 = g_gfxCaps.field_7 = true;
    g_gfxCaps.bglGenVertexArrays = (glGenVertexArrays != nullptr);
    g_gfxCaps.field_48 = 16i64;
    g_gfxCaps.field_50 = 4i64;
    g_gfxCaps.field_18 = g_gfxCaps.field_19 = true;
    int tmp = 0;
    if (GLEW_ARB_texture_filter_anisotropic || GLEW_EXT_texture_filter_anisotropic)
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &tmp);
    g_gfxCaps.max_anisotropy = std::clamp(tmp, 0, 16);
    g_gfxCaps.texture_compression_s3tc = GLEW_EXT_texture_compression_s3tc != 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
    g_gfxCaps.max_v_attribs = std::clamp(tmp, 0, 16);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &tmp);
    g_gfxCaps.max_color_atchs = std::clamp(tmp, 0, 4);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tmp);
    g_gfxCaps.max_tex_iu = std::clamp(tmp, 0, 16);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
    g_gfxCaps.max_v_tex_iu = std::clamp(tmp, 0, 4);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
    g_gfxCaps.max_ctex_iu = std::clamp(tmp, 0, 20);
    glGetIntegerv(GL_MAX_CLIP_PLANES, &tmp);
    g_gfxCaps.max_clip_planes = std::clamp(tmp, 0, 1);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tmp);
    g_gfxCaps.max_texture_size = tmp;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &tmp);
    g_gfxCaps.shader_group_vote = GLEW_ARB_shader_group_vote != 0;
    g_gfxCaps.max_3d_texture_size = tmp;
    g_gfxCaps.shader_ballot = GLEW_ARB_shader_ballot != 0;
    auto glv = g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | (g_glVersion >> 20) & 0x3FF;
    g_gfxCaps.clip_control = (glv >= 0x401400 || GLEW_ARB_clip_control);
    if (glv >= 0x300800) {
        glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &tmp);
        g_gfxCaps.dual_src_drb = tmp;
    }
    if (glv >= 0x300000 || GLEW_EXT_texture_array) {
        g_gfxCaps.texture_array = true;
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &tmp);
        g_gfxCaps.max_arr_tex_layers = tmp;
    }
    if (glv >= 0x400000 || GLEW_ARB_texture_cube_map_array)
        g_gfxCaps.tex_cube_map_array = true;
    if (glv >= 0x300400 || GLEW_ARB_texture_buffer_object || GLEW_EXT_texture_buffer_object) {
        g_gfxCaps.texture_buffer_object = true;
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &tmp);
        g_gfxCaps.max_tex_buf = tmp;
    }
    g_gfxCaps.texture_view = (glv >= 0x400C00 || GLEW_ARB_texture_view);
    g_gfxCaps.texture_storage = (glv >= 0x400800 || GLEW_ARB_texture_storage);
    g_gfxCaps.texture_swizzle = (glv >= 0x300C00 || GL_ARB_texture_swizzle || GLEW_EXT_texture_swizzle);
    if (GLEW_ARB_shader_image_load_store || GLEW_EXT_shader_image_load_store || glv >= 0x400800) {
        g_gfxCaps.field_B = true;
        g_gfxCaps.field_14 = true;
        glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &tmp);
        g_gfxCaps.max_v_img_uniforms = tmp;
        glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &tmp);
        g_gfxCaps.max_f_img_uniforms = tmp;
    }
    if (glv >= 0x300400 || GLEW_ARB_uniform_buffer_object) {
        g_gfxCaps.uniform_buffer_object = true;
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &tmp);
        g_gfxCaps.max_v_uniform_blocks = tmp;
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &tmp);
        g_gfxCaps.max_f_uniform_blocks = tmp;
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &tmp);
        g_gfxCaps.max_uniform_block_sz = tmp;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &tmp);
        g_gfxCaps.max_uniform_bo_align = tmp;
    }
    g_gfxCaps.conservative_depth = (glv >= 0x400800 || GLEW_ARB_conservative_depth);
    if ((g_gfxCaps.conservative_depth && glv >= 0x400C00) || GLEW_ARB_shader_storage_buffer_object) {
        g_gfxCaps.shader_storage_bo = true;
        glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &tmp);
        g_gfxCaps.max_v_shb = tmp;
        glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &tmp);
        g_gfxCaps.max_f_shb = tmp;
        int64_t tmp64;
        glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &tmp64);
        g_gfxCaps.max_sh_sbs = tmp64;
    }
    if (glv >= 0x400C00 || GLEW_ARB_compute_shader) {
        g_gfxCaps.compute_shader = true;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &tmp);
        g_gfxCaps.max_compute_wg_cnt0 = tmp;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &tmp);
        g_gfxCaps.max_compute_wg_cnt1 = tmp;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &tmp);
        g_gfxCaps.max_compute_wg_cnt2 = tmp;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &tmp);
        g_gfxCaps.max_compute_wg_sz0 = tmp;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &tmp);
        g_gfxCaps.max_compute_wg_sz1 = tmp;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &tmp);
        g_gfxCaps.max_compute_wg_sz2 = tmp;
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &tmp);
        g_gfxCaps.max_compute_wg_inv = tmp;
        glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &tmp);
        g_gfxCaps.max_compute_sm_sz = tmp;
        glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &tmp);
        g_gfxCaps.max_comp_tex_iu = tmp;
        if (g_gfxCaps.shader_storage_bo) {
            glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &tmp);
            g_gfxCaps.max_comp_uf_b = tmp;
            glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &tmp);
            g_gfxCaps.max_comp_s_b = tmp;
        }
        if (g_gfxCaps.field_14) {
            glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &tmp);
            g_gfxCaps.max_comp_img_uf = tmp;
        }
    }
    return true;
}
const uint32_t g_fillModes[] = { GL_POINT, GL_LINE, GL_FILL, GL_FALSE };
void GFX_SetFillMode(GFX_FILL_MODE fillMode) {
    g_pGFX_CurrentStates->m_fillMode = fillMode;
    glPolygonMode(GL_FRONT_AND_BACK, g_fillModes[fillMode]);
}
void GFX_PushStates() {
    auto next = g_pGFX_CurrentStates + 1;
    *next = *g_pGFX_CurrentStates;
    g_pGFX_CurrentStates = next;
}
void GFX_PopStates() {
    g_pGFX_CurrentStates--;
    if (g_pGFX_CurrentStates->m_depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    glDepthFunc(g_stensilFuncs[g_pGFX_CurrentStates->m_depthFuncIdx]);
    glDepthMask(g_pGFX_CurrentStates->m_depthMask ? 1 : 0);
    glPolygonMode(GL_FRONT_AND_BACK, g_fillModes[g_pGFX_CurrentStates->m_fillMode]);
    if (g_pGFX_CurrentStates->m_cullIdx != g_pGFX_CurrentStates->m_newCullIdx) //4
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_CULL;
    g_pGFX_CurrentStates->m_cullIdx = g_pGFX_CurrentStates->m_newCullIdx;
    bool blend = g_pGFX_CurrentStates->m_field_1C != 0; //5
    if (g_pGFX_CurrentStates->m_blend != blend)
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_BLEND;
    g_pGFX_CurrentStates->m_blend = blend;
    if (g_pGFX_CurrentStates->m_scissorTest) //12
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
    if (g_pGFX_CurrentStates->m_polyOffset == 0.0) { //13
        glDisable(GL_POLYGON_OFFSET_FILL);
    } else {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(g_pGFX_CurrentStates->m_polyOffset, 1.0);
    }
    if (g_pGFX_CurrentStates->m_blendIdxs != g_pGFX_CurrentStates->m_prBlendIdxs) //15
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_BLEND;
    g_pGFX_CurrentStates->m_blendIdxs = g_pGFX_CurrentStates->m_prBlendIdxs;
    if (g_pGFX_CurrentStates->m_enableStensilTest) //default
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);
    glStencilFunc(
        g_stensilFuncs[g_pGFX_CurrentStates->m_stensilFunc],
        g_pGFX_CurrentStates->m_stensilRef,
        g_pGFX_CurrentStates->m_stensilFuncMask);
    glStencilMask(g_pGFX_CurrentStates->m_stensilMask);
    glStencilOp(
        g_stensilOps[g_pGFX_CurrentStates->m_sopsFail],
        g_stensilOps[g_pGFX_CurrentStates->m_sopdpFail],
        g_stensilOps[g_pGFX_CurrentStates->m_sopdpPass]);
    auto sh = GFX_GetCurrentShaderHandle();
    GFX_UnsetShader();
    GFX_SetShader(sh);
    g_pGFX_CurrentStates->m_bits = -1;
    g_pGFX_CurrentStates->m_hasRegTypes = -1;
    g_pGFX_CurrentStates->m_field_A8 = 0;
    for (uint32_t j = 0; j < GFX_GetCaps().max_v_attribs; j++)
        glDisableVertexAttribArray(j);
    for (int a = 0; a < GFX_GetCaps().max_color_atchs; a++) {
        auto m = g_pGFX_CurrentStates->m_colorMask[a];
        if (a) {
            if (glColorMaskIndexedEXT)
                glColorMaskIndexedEXT(a, m & 1, (m & 2) != 0, (m & 4) != 0, (m & 8) != 0);
        } else {
            glColorMask(m & 1, (m & 2) != 0, (m & 4) != 0, (m & 8) != 0);
        }
    }
    glActiveTexture(g_pGFX_CurrentStates->m_actTex);
    glBindBuffer(GL_ARRAY_BUFFER, g_pGFX_CurrentStates->m_arrBuf);
}
int GFX_GetCurrentShaderHandle() {
    return g_CurrentShaderHandle;
}
void GFX_UnsetShader() {
    g_pCurrentShader = 0;
    g_CurrentShaderHandle = -1;
    glUseProgram(0); //GFXAPI_UnsetShader
    g_pGFX_CurrentStates->m_shader = -1;
}
void GFXAPI_SetShader(int sh) {
    glUseProgram(g_Shaders[sh].m_program);
    g_pGFX_CurrentStates->m_shader = sh;
}
const GFX_RegisterRef GFX_StateBlock::s_matrixArrayRefs[] = { { GFX_RegisterRef::Ty::Draw, 5, 16 }, { GFX_RegisterRef::Ty::Draw, 5, 48 } };
const GFX_RegisterRef GFX_StateBlock::s_matrixRefs[] = {
  { GFX_RegisterRef::Ty::Object, 0, 4 },
  { GFX_RegisterRef::Ty::Object, 4, 4 },
  { GFX_RegisterRef::Ty::Scene, 0, 4 },
  { GFX_RegisterRef::Ty::Scene, 4, 4 },
  { GFX_RegisterRef::Ty::Scene, 8, 4 },
  { GFX_RegisterRef::Ty::Scene, 12, 4 },
  { GFX_RegisterRef::Ty::Scene, 16, 4 },
  { GFX_RegisterRef::Ty::Scene, 20, 4 },
  { GFX_RegisterRef::Ty::Scene, 24, 4 },
};
const GFX_RegisterRef GFX_StateBlock::s_registerRefs[] = {
  { GFX_RegisterRef::Ty::Scene, 28, 1 },
  { GFX_RegisterRef::Ty::Scene, 29, 1 },
  { GFX_RegisterRef::Ty::Scene, 30, 1 },
  { GFX_RegisterRef::Ty::Scene, 31, 1 },
  { GFX_RegisterRef::Ty::Object, 8, 1 },
  { GFX_RegisterRef::Ty::Scene, 32, 1 },
  { GFX_RegisterRef::Ty::Scene, 33, 1 },
  { GFX_RegisterRef::Ty::Draw, 2, 1 },
  { GFX_RegisterRef::Ty::Scene, 34, 1 },
  { GFX_RegisterRef::Ty::Scene, 35, 1 },
  { GFX_RegisterRef::Ty::Scene, 36, 1 },
  { GFX_RegisterRef::Ty::Scene, 37, 1 },
  { GFX_RegisterRef::Ty::Object, 9, 1 },
  { GFX_RegisterRef::Ty::Object, 10, 1 },
  { GFX_RegisterRef::Ty::Scene, 38, 1 },
  { GFX_RegisterRef::Ty::Scene, 39, 1 },
  { GFX_RegisterRef::Ty::Scene, 40, 1 },
  { GFX_RegisterRef::Ty::Scene, 41, 1 },
  { GFX_RegisterRef::Ty::Object, 11, 1 },
  { GFX_RegisterRef::Ty::Scene, 42, 1 },
  { GFX_RegisterRef::Ty::Scene, 43, 1 },
  { GFX_RegisterRef::Ty::Scene, 44, 1 },
  { GFX_RegisterRef::Ty::Scene, 45, 1 },
  { GFX_RegisterRef::Ty::Draw, 0, 1 },
  { GFX_RegisterRef::Ty::Draw, 1, 1 },
  { GFX_RegisterRef::Ty::Draw, 3, 1 },
  { GFX_RegisterRef::Ty::Draw, 4, 1 },
  { GFX_RegisterRef::Ty::Draw, 69, 16 },
  { GFX_RegisterRef::Ty::Draw, 85, 16 }
};
void GFXAPI_ReUploadShaderCache() {
    bool extra = (0 == g_pGFX_CurrentStates->m_field_C8);
    if (g_pCurrentShader) {
        int i = 0;
        for (auto loc : g_pCurrentShader->m_locations) {
            if (loc >= 0 && (i < 27 || extra)) {
                auto uniform = GFX_StateBlock::GetUniform((GFX_SHADER_REGISTERS)i);
                glUniform4fv(loc, GFX_StateBlock::s_registerRefs[i].m_cnt, uniform->m_data);
            }
            i++;
        }
        i = 0;
        for (auto mloc : g_pCurrentShader->m_matLocations) {
            if (mloc >= 0) {
                auto uniform = GFX_StateBlock::GetUniform((GFX_SHADER_MATRICES)i);
                glUniformMatrix4fv(mloc, 1, 0, uniform->m_data);
            }
            i++;
        }
        auto mlocEx = g_pCurrentShader->m_matArrLocations[g_pGFX_CurrentStates->m_field_C8];
        if (mlocEx >= 0) {
            const auto &ref = GFX_StateBlock::s_matrixArrayRefs[g_pGFX_CurrentStates->m_field_C8];
            auto uniform = GFX_StateBlock::GetUniform(ref);
            glUniformMatrix4fv(mlocEx, ref.m_cnt, 0, uniform->m_data);
        }
    }
}
bool GFX_SetShader(int sh) {
    if (sh == -1 || sh >= _countof(g_Shaders) || g_pCurrentShader == g_Shaders + sh)
        return false;
    GFXAPI_SetShader(sh);
    g_pCurrentShader = &g_Shaders[sh];
    g_CurrentShaderHandle = sh;
    GFXAPI_ReUploadShaderCache();
    memset(g_pCurrentShader->m_field_12C, 0xFF, sizeof(g_pCurrentShader->m_field_12C));
    return true;
}
void GFX_StateBlock::UnbindBuffer(int arrBuf) {
    if (m_altArrBuf == arrBuf || m_arrBuf == arrBuf) {
        m_bits |= GSB_PEND_ALTB;
        m_arrBuf = -1;
        m_altArrBuf = -1;
    }
    if (m_elArrBuf == arrBuf) {
        m_bits |= GSB_PEND_EAB;
        m_elArrBuf = -1;
    }
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const VEC4 *vec, uint16_t sz, uint64_t skipTag) { //SetUniformVEC4_a, SetUniformVEC4_a2
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (skipTag && skipTag == u.m_pTags[ref.m_offset])
        return;
    for (uint16_t i = 0; i < sz; i++, vec++) {
        u.m_pRegs[i + ref.m_offset] = *vec;
        u.m_pTags[i + ref.m_offset] = 0;
    }
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + sz)
        u.m_size = ref.m_offset + sz;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const VEC4 &vec, uint64_t tag) { //SetUniformVEC4
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (tag && tag == u.m_pTags[ref.m_offset])
        return;
    u.m_pRegs[ref.m_offset] = vec;
    u.m_pTags[ref.m_offset] = tag;
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + 1)
        u.m_size = ref.m_offset + 1;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const VEC2 *vec, uint16_t sz, uint64_t skipTag) { //SetUniformVEC2_a
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (skipTag && skipTag == u.m_pTags[ref.m_offset])
        return;
    for (uint16_t i = 0; i < sz; i++, vec++) {
        auto &dest = u.m_pRegs[i + ref.m_offset];
        dest.m_data[0] = vec->m_data[0];
        dest.m_data[1] = vec->m_data[1];
        dest.m_data[2] = dest.m_data[3] = 0.0f;
        u.m_pTags[i + ref.m_offset] = 0;
    }
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + sz)
        u.m_size = ref.m_offset + sz;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const MATRIX44 *m, uint16_t sz, uint64_t skipTag) {
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (skipTag && skipTag == u.m_pTags[ref.m_offset])
        return;
    for (uint16_t i = 0; i < sz; i++, m++) {
        for (auto j = 0; j < 4; j++) {
            u.m_pRegs[4 * j + i + ref.m_offset] = m->m_data[j];
            u.m_pTags[4 * j + i + ref.m_offset] = 0;
        }
    }
    sz *= 4;
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + sz)
        u.m_size = ref.m_offset + sz;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const MATRIX44 &m, uint64_t tag) { //SetUniformMat
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (tag && tag == u.m_pTags[ref.m_offset])
        return;
    static_assert(sizeof(m) == 4 * sizeof(u.m_pRegs[ref.m_offset]));
    memcpy(&u.m_pRegs[ref.m_offset], &m, sizeof(m));
    u.m_pTags[ref.m_offset] = tag;
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + 4)
        u.m_size = ref.m_offset + 4;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::Reset() {
    m_bits = -1;
    m_cullIdx = 0;
    m_blend = 0;
    m_blendIdxs.m_modeIdx = 0;
    m_blendIdxs.m_sFactorIdx = 0;
    m_blendIdxs.m_dFactorIdx = 1;
    m_altArrBuf = -1;
    //TODO *(_QWORD *)&this->field_90 = 0i64;
    //TODO *(_QWORD *)&this->field_B8 = 0i64;
    m_elArrBuf = m_vaIdx = -1;
    m_shader = -1;
    m_field_A4 = 0;
    m_field_A8 = 0i64;
    m_actTex = GL_TEXTURE0;    
    int alt = -1;
    int64_t bits = -1;
    if (m_arrBuf) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        alt = m_altArrBuf;
        bits = m_bits;
        m_arrBuf = 0;
    }
    m_hasRegTypes = -1i64;
    m_field_C8 = 0;
    m_bits = bits | (alt ? GSB_PEND_ALTB : 0);
}
const uint32_t g_GFX_VertexFormat_size[] = { 0, 4, 4, 4, 4, 1, 2, 3 };
const GLenum g_GFX_VertexFormat_format[] = { GL_FALSE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT, GL_FLOAT, GL_FLOAT };
const uint8_t g_GFX_VertexFormat_normalized[] = { 0, 1, 1, 0, 1, 0, 0, 0 };
const GLenum g_GFX_TO_GL_CULL[] = { GL_FALSE, GL_FRONT, GL_BACK, GL_FALSE };
const GLenum g_GFX_TO_GL_BLENDFUNC[] = { GL_FALSE, GL_TRUE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA };
const GLenum g_GFX_TO_GL_BLENDOP[] = { GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_MIN, GL_MAX, GL_FUNC_REVERSE_SUBTRACT, GL_FALSE };
bool GFX_StateBlock::Realize() {
    if (m_shader == -1 || m_vaIdx == -1)
        return false;
    if (m_bits & GSB_PEND_ALTB)
        GFX_StateBlock::BindVertexBuffer(std::clamp(m_altArrBuf, 0, std::numeric_limits<int>::max()));
    if (m_bits & 0xF) {
        uint64_t field_B8 = 0;
        if (m_altArrBuf == -1)
            field_B8 = m_field_B8;
        uint64_t inv = 0i64;
        auto vx = g_vertexArray.fast64[m_vaIdx];
        auto pAttr = vx->m_attrs;
        for (int vi = 0; vi < vx->m_attrCnt; vi++) {
            auto atrIdx = pAttr->m_idx;
            auto addm = (1i64 << atrIdx);
            inv |= addm;
            if ((addm & g_pGFX_CurrentStates->m_field_A8) == 0)
                glEnableVertexAttribArray(atrIdx);
            glVertexAttribPointer(
                atrIdx,
                g_GFX_VertexFormat_size[pAttr->m_fmtIdx],
                g_GFX_VertexFormat_format[pAttr->m_fmtIdx],
                g_GFX_VertexFormat_normalized[pAttr->m_fmtIdx],
                vx->m_strides[pAttr[-1].m_strideIdx], //GLsizei stride
                &m_attrData[pAttr->m_dataOffset + field_B8]); //const void * pointer
            ++pAttr;
        }
        uint32_t index = 0;
        uint64_t mask = 1;
        for (auto i = g_pGFX_CurrentStates->m_field_A8 ^ inv; i; ++index) {
            if ((mask & i) && (mask & g_pGFX_CurrentStates->m_field_A8))
                glDisableVertexAttribArray(index);
            i &= (~mask);
            mask <<= 1;
        }
        g_pGFX_CurrentStates->m_field_A8 = inv;
    }
    if (m_bits & GSB_PEND_EAB) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, std::clamp(m_elArrBuf, 0, std::numeric_limits<int>::max()));
    }
    if (m_bits & GSB_PEND_CULL) {
        if (m_cullIdx) {
            glEnable(GL_CULL_FACE);
            glCullFace((GLenum)g_GFX_TO_GL_CULL[m_cullIdx]);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }
    if (m_bits & GSB_PEND_BLEND) {
        if (m_blend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        glBlendEquation(g_GFX_TO_GL_BLENDOP[m_blendIdxs.m_modeIdx]);
        glBlendFunc(g_GFX_TO_GL_BLENDFUNC[m_blendIdxs.m_sFactorIdx], g_GFX_TO_GL_BLENDFUNC[m_blendIdxs.m_dFactorIdx]);
    }
    m_bits = 0;
    if (m_hasRegTypes) {
        if (GFX_ShaderModelValue(g_Shaders[m_shader].m_modelIndex) >= GFX_ShaderModelValue(4)) {
            if (m_hasRegTypes & (1 << (int)GFX_RegisterRef::Ty::Scene)) {
                auto idx = (int)GFX_RegisterRef::Ty::Scene;
                glBindBufferBase(GL_UNIFORM_BUFFER, idx, g_UBOs[idx]);
                static_assert(736 == sizeof(bufferRegs_Scene));
                glBufferData(GL_UNIFORM_BUFFER, sizeof(bufferRegs_Scene), nullptr, GL_STREAM_DRAW);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(bufferRegs_Scene), bufferRegs_Scene, GL_STREAM_DRAW);
            }
            if (m_hasRegTypes & (1 << (int)GFX_RegisterRef::Ty::Object)) {
                auto idx = (int)GFX_RegisterRef::Ty::Object;
                glBindBufferBase(GL_UNIFORM_BUFFER, idx, g_UBOs[idx]);
                static_assert(192 == sizeof(bufferRegs_Object));
                glBufferData(GL_UNIFORM_BUFFER, sizeof(bufferRegs_Object), nullptr, GL_STREAM_DRAW);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(bufferRegs_Object), bufferRegs_Object, GL_STREAM_DRAW);
            }
            if (m_hasRegTypes & (1 << (int)GFX_RegisterRef::Ty::Draw)) {
                auto idx = (int)GFX_RegisterRef::Ty::Draw;
                auto sz = sizeof(VEC4) * GFX_StateBlock::uniformRegs[idx].m_size;
                glBindBufferBase(GL_UNIFORM_BUFFER, idx, g_UBOs[idx]);
                glBufferData(GL_UNIFORM_BUFFER, sz, nullptr, GL_STREAM_DRAW);
                glBufferData(GL_UNIFORM_BUFFER, sz, bufferRegs_Draw, GL_STREAM_DRAW);
                GFX_StateBlock::uniformRegs[idx].m_size = 5;
            }
            if (m_hasRegTypes & (1 << (int)GFX_RegisterRef::Ty::User)) {
                auto idx = (int)GFX_RegisterRef::Ty::User;
                static_assert(16 == sizeof(VEC4));
                auto sz = sizeof(VEC4) * GFX_StateBlock::uniformRegs[idx].m_size;
                glBindBufferBase(GL_UNIFORM_BUFFER, idx, g_UBOs[idx]);
                glBufferData(GL_UNIFORM_BUFFER, sz, nullptr, GL_STREAM_DRAW);
                glBufferData(GL_UNIFORM_BUFFER, sz, bufferRegs_User, GL_STREAM_DRAW);
                GFX_StateBlock::uniformRegs[idx].m_size = 0;
            }
            m_hasRegTypes = 0;
        }
    }
    return true;
}
const uint32_t g_GFX_ShaderModelValues[/*20*/] = {0, 120, 140, 300, 430, 10, 11, 12, 20, 21, 22, 23, 50, 51, 60, 64, 200, 310, 320, 0};
uint32_t GFX_ShaderModelValue(int idx) { return g_GFX_ShaderModelValues[idx]; }
const VEC4 *GFX_StateBlock::GetUniform(GFX_SHADER_REGISTERS reg) {
    return &GFX_StateBlock::uniformRegs[(int)s_registerRefs[reg].m_ty].m_pRegs[s_registerRefs[reg].m_offset];
}
const VEC4 *GFX_StateBlock::GetUniform(GFX_SHADER_MATRICES mat) {
    return &GFX_StateBlock::uniformRegs[(int)s_matrixRefs[mat].m_ty].m_pRegs[s_matrixRefs[mat].m_offset];
}
const VEC4 *GFX_StateBlock::GetUniform(const GFX_RegisterRef &ref) {
    return &GFX_StateBlock::uniformRegs[(int)ref.m_ty].m_pRegs[ref.m_offset];
}
void GFX_StateBlock::BindVertexBuffer(int arrBuf) {
    if (m_arrBuf != arrBuf) {
        glBindBuffer(GL_ARRAY_BUFFER, arrBuf);
        m_arrBuf = arrBuf;
    }
    m_bits |= (m_altArrBuf != arrBuf ? GSB_PEND_ALTB : 0);
}
void GFX_Begin() { g_pGFX_CurrentStates->Reset(); }
void GFXAPI_CreateVertex(int idx, GFX_CreateVertexParams *parms) {
    auto vx = new GFX_Vertex;
    //TODO fill new vertex data
    /*
    *(_OWORD *)&vx->m_attrCnt = *(_OWORD *)parms;
    *(_OWORD *)&vx->field_10 = *(_OWORD *)&parms[16];
    *(_OWORD *)&vx->m_attrs[3].m_strideIdx = *(_OWORD *)&parms[32];
    *(_OWORD *)&vx->m_attrs[7].m_strideIdx = *(_OWORD *)&parms[48];
    *(_OWORD *)&vx->m_attrs[11].m_strideIdx = *(_OWORD *)&parms[64];
    *(_OWORD *)&vx->m_attrs[15].m_strideIdx = *(_OWORD *)&parms[80];
    *(_OWORD *)&vx->m_attrs[19].m_strideIdx = *(_OWORD *)&parms[96];
    *(_OWORD *)&vx->m_attrs[23].m_strideIdx = *(_OWORD *)&parms[112];
    *(_QWORD *)&vx->m_attrs[27].m_strideIdx = *(_QWORD *)&parms[128];
    */
    memset(vx->m_strides, 0, sizeof(vx->m_strides));
    g_vertexArray.fast64[idx] = vx;
    /*TODO
    auto v7 = (uint8_t *)vx->m_field50;
    for (int ret = 0; ret < vx->m_field8; ret++) {
        *(&vx->field_88 + *v7) += v7[1];
        v7 += 4;
    }*/
}
int GFX_CreateVertex(GFX_CreateVertexParams *parms) {
    int idx;
    auto extra = g_vertexArray.extra;
    if (g_vertexArray.extra) {
        g_vertexArray.extra = (GFX_Vertex **)*g_vertexArray.extra;
        *extra = nullptr;
        idx = ((char *)extra - (char *)&g_vertexArray) >> 3;
    } else {
        if (g_vertexArray.size == _countof(g_vertexArray.fast64))
            return -1;
        idx = g_vertexArray.size;
        ++g_vertexArray.size;
    }
    if (idx != -1) {
        GFXAPI_CreateVertex(idx, parms);
        return idx;
    }
    return -1;
}
void GFXAPI_DestroyVertex(int idx) { delete g_vertexArray.fast64[idx]; }
void GFX_DestroyVertex(int *pIdx) {
    uint64_t idx = *pIdx;
    if (*pIdx >= 0 && idx < g_vertexArray.size && (
        (GFX_Vertex *)&g_vertexArray > g_vertexArray.fast64[idx] || g_vertexArray.fast64[idx] >= (GFX_Vertex *)&g_vertexArray.size
        )) {
        GFXAPI_DestroyVertex(idx);
        g_vertexArray.fast64[idx] = (GFX_Vertex *)g_vertexArray.extra;
        g_vertexArray.extra = &g_vertexArray.fast64[idx];
        *pIdx = -1;
    }
}
namespace GameShaders {
void LoadAll() {
    //TODO
}
}
int GFX_CreateShader(const GFX_CreateShaderParams &p) {
    if (p.m_name && *p.m_name)
        return GFX_CreateShaderFromFile(p, -1);
    else
        return -1;
}
int GFX_CreateAnimatedTextureFromTGAFiles(const char *name) {
    //TODO
    return -1;
}
std::string FixAssetFilename(const char *name) {
    std::string ret(name), data("data/"), tgax(".tgax");
    if (ret.length() < 5 || ret.find(data, 0) == -1)
        ret.insert(0, data);
    if (ret.length() >= 10) {
        auto tgaxPos = ret.find(tgax, 5);
        if (tgaxPos != -1)
            ret.replace(tgaxPos, tgax.length(), tgax.c_str(), 4);
    }
    for (auto &ch : ret)
        if (ch == '\\') ch = '/';
    std::string dataBikes("data/Bikes/");
    if (ret.length() >= dataBikes.length()) {
        auto bikesPos = ret.find(dataBikes, 0);
        if (bikesPos != -1)
            ret.replace(bikesPos, dataBikes.length(), "data/bikes/", dataBikes.length());
    }
    return ret;
}
int GFX_Internal_FindLoadedTexture(const char *name) {
    auto s = SIG_CalcCaseSensitiveSignature(name);
    int result = 0;
    if (g_nTexturesLoaded <= 0)
        return -1;
    for (auto &t : g_Textures) {
        if (t.m_nameSCRC == s)
            return result;
        if (++result >= g_nTexturesLoaded)
            break;
    }
    return -1;
}
int GFX_CreateTextureFromZTX(uint8_t *data, int size, int handle) {
    if (!data || !LOADER_IsValidCompAssetHeader((const char *)data))
        return -1;
    auto len = *((const uint32_t *)data + 1);
    auto unpacked = (uint8_t *)calloc(len, 1);
    if (!unpacked) {
        Log("[GFX]: malloc(%u) failed.", len);
        return -1;
    }
    auto realLen = ZLIB_Decompress(data + 16, size, unpacked);
    int handleRet = -1;
    if (len == realLen)
        handleRet = GFX_CreateTextureFromTGAX(unpacked, handle);
    else
        LogTyped(LOG_ERROR, "[GFX]: GFX_CreateTextureFromZTX(): size mismatch %u != %u", len, realLen);
    free(unpacked);
    return handleRet;
}
int GFX_CreateTextureFromTGA(uint8_t *data, int handle) {
    //TODO
    return -1;
}
int GFX_Internal_LoadTextureFromTGAFile(const char *name, int handle) {
    auto nameLen = strlen(name);
    if (nameLen > 0x800)
        return -1;
    struct _stat64i32 stat;
    if (_stat64i32(name, &stat) < 0)
        memset(&stat, 0, sizeof(stat));
    char buffer[MAX_PATH] = {};
    auto v9 = nameLen - 1;
    if (nameLen > 1) {
        auto v10 = v9;
        auto v11 = &name[v9];
        do {
            if (*v11 == '.')
                memmove(buffer, name, v10);
            --v9;
            --v10;
            --v11;
        } while (v9 != 0);
    }
    int ret = -1; //was 0
    char bufferTgax[MAX_PATH];
    if (buffer[0]) {
        sprintf_s(buffer, "%s.ztx", buffer);
        bool bZTX = true;
        auto f = fopen(buffer, "rb");
        if (!f) {
            bZTX = false;
            sprintf_s(bufferTgax, "%sx", name);
            f = fopen(bufferTgax, "rb");
        }
        if (f) {
            struct _stat64i32 stat2;
            _stat64i32(bZTX ? buffer : bufferTgax, &stat2);
            if (stat2.st_mtime <= stat.st_mtime) {
                fclose(f);
            } else {
                fseek(f, 0, SEEK_END);
                auto fileSize = ftell(f);
                fseek(f, 0, SEEK_SET);
                auto data = (uint8_t *)malloc(fileSize);
                if (!data) {
                    Log("GFX_Internal_LoadTextureFromTGAFile: FAILED MEMORY ALLOCATION of size %d. Aborting.", fileSize);
                    fclose(f);
                    return -1;
                }
                auto r = fread(data, fileSize, 1, f);
                fclose(f);
                if (!r) {
                    free(data);
                    return -1;
                }
                auto h = handle;
                if (h == -1) {
                    h = g_nTexturesLoaded;
                    if (g_nTexturesLoaded < _countof(g_Textures))
                        ++g_nTexturesLoaded;
                    else
                        h = -1;
                    if (h >= 0) {
                        g_Textures[h].m_name = strdup(name);
                        g_Textures[h].m_nameSCRC = SIG_CalcCaseSensitiveSignature(name);
                    }
                }
                if (h >= 0) {
                    if (!g_Textures[h].m_assetCategory)
                        g_Textures[h].m_assetCategory = g_CurrentAssetCategory;
                    auto nSkipMipCount = g_nSkipMipCount;
                    if (strstr(name, "/UI/")) {
                        g_nSkipMipCount = 0;
                        g_Textures[h].m_assetCategory = AC_1;
                    }
                    if (bZTX)
                        ret = GFX_CreateTextureFromZTX(data, fileSize, h);
                    else
                        ret = GFX_CreateTextureFromTGAX(data, h);
                    g_nSkipMipCount = nSkipMipCount;
                }
                free(data);
                if (ret != -1)
                    return ret;
            }
        }
        f = fopen(name, "rb");
        if (!f)
            return -1;
        fseek(f, 0, SEEK_END);
        auto fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);
        auto data = (uint8_t *)malloc(fileSize);
        if (!data) {
            Log("[GFX]: malloc(%d) for tga data failed", fileSize);
            fclose(f);
            return -1;
        }
        auto r = fread(data, fileSize, 1, f);
        fclose(f);
        if (r) {
            if (handle == -1) {
                handle = g_nTexturesLoaded;
                if (g_nTexturesLoaded < _countof(g_Textures))
                    ++g_nTexturesLoaded;
                else
                    handle = -1;
                if (handle != -1) {
                    g_Textures[handle].m_name = strdup(name);
                    g_Textures[handle].m_nameSCRC = SIG_CalcCaseSensitiveSignature(name);
                }
            }
            if (handle != -1) {
                if (!g_Textures[handle].m_assetCategory)
                    g_Textures[handle].m_assetCategory = g_CurrentAssetCategory;
                ret = GFX_CreateTextureFromTGA(data, handle);
            }
            free(data);
            return ret;
        }
        free(data);
        return -1;
    }
    return ret;
}
int GFX_CreateTextureFromTGAFile(const char *name, int handle, bool tryAnimated) {
    zassert(name && "IsWADFileLoaded failed - Must have non-NULL parameter");
    if (!name)
        return -1;
    auto nameLen = strlen(name);
    if (!nameLen)
        return -1;
    if (tryAnimated) {
        auto mutPos = strstr(name, "00.tga");
        if (mutPos) {
            char animName[272] = {};
            auto v9 = std::min(259ull, nameLen);
            memmove(animName, name, v9);
            animName[mutPos - name] = 0;
            return GFX_CreateAnimatedTextureFromTGAFiles(animName);
        }
    }
    //Streamer::GetFileRef -> 0
    std::string realName;
    int ret = handle;
    if (ret != -1) {
        realName = g_Textures[handle].m_name;
        ret = -1;
    } else {
        realName = FixAssetFilename(name);
        ret = GFX_Internal_FindLoadedTexture(GAMEPATH(realName.c_str()));
        if (ret >= 0) {
            if (!g_Textures[ret].InHardware()) {
                handle = ret;
                ret = -1;
            }
        }
    }
    if (ret < 0) {
        ret = GFX_Internal_LoadTextureFromTGAXFile(realName.c_str(), handle);
        if (ret == -1) {
            if (realName.length() >= 5) {
                std::string tgax(".tgax");
                auto tgaxPos = realName.find(tgax, 0);
                if (tgaxPos != -1)
                    realName.replace(tgaxPos, tgax.length(), tgax.c_str(), 4);
            }
            ret = GFX_Internal_LoadTextureFromTGAFile(GAMEPATH(realName.c_str()), handle);
        }
    }
    return ret;
}
void GFX_SetAnimatedTextureFramerate(int tex, float r) {
    //TODO
}
const int OIF_CNT = 13;
int gaGFXtoOGLaligns[OIF_CNT] = { 1, 4, 1, 4, 2, 8, 8, 8, 8, 0 };
GLenum gaGFXtoOGLInternalFormat[OIF_CNT] = { GL_RGBA, GL_RGBA, GL_R8, GL_LUMINANCE_FLOAT32_ATI, GL_R16UI, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0 };
bool gaIsCompressedInternalFormat[OIF_CNT] = { false, false, false, false, false, true, true , true , true , true , true , true , true };
bool GFX_IsCompressed(uint32_t formatIdx) { return (formatIdx >= OIF_CNT) ? 0 : gaIsCompressedInternalFormat[formatIdx]; }
void GFXAPI_CreateTexture(int handle, int w, int h, int mipMapLevelIdx) {
    glGenTextures(1, &g_Textures[handle].m_glid);
    glBindTexture(GL_TEXTURE_2D, g_Textures[handle].m_glid);
    int filter = GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    if (mipMapLevelIdx) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapLevelIdx - 1);
        filter = GL_LINEAR_MIPMAP_LINEAR;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}
void GFXAPI_UpdateTexture(int handle, int level, int w, int h, uint32_t formatIdx, const void *data, int dataBytes) {
    int align = g_Textures[handle].m_align;
    glBindTexture(GL_TEXTURE_2D, g_Textures[handle].m_glid);
    if (align < OIF_CNT)
        align = gaGFXtoOGLaligns[align];
    else
        align = 0;
    glPixelStorei(GL_UNPACK_ALIGNMENT, align);
    if (GFX_IsCompressed(formatIdx)) {
        GLenum internalformat = 0;
        if (formatIdx < OIF_CNT)
            internalformat = gaGFXtoOGLInternalFormat[formatIdx];
        glCompressedTexImage2D(GL_TEXTURE_2D, level, internalformat, w, h, 0 /* border */, dataBytes, data);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
int GFX_Internal_LoadTextureFromTGAXFile(const char *name, int handle) {
    std::string sname(name);
    if (sname.ends_with(".tga"))
        sname += 'x';
    if (sname.length() >= 5) {
        auto of = sname.find("data/", 0, 5);
        if (of != -1)
            sname.erase(of, 5);
    }
    time_t wadTouchTime;
    auto wh = g_WADManager.GetWadFileHeaderByItemName(sname.c_str(), WAD_ASSET_TYPE::TEXTURE, &wadTouchTime);
    int ret = -1;
    if (wh) {
        struct _stat64i32 stat = {};
        auto sr = _stat64i32(name, &stat);
        if (sr == -1 || stat.st_mtime < wadTouchTime) {
            if (handle == -1) {
                handle = g_nTexturesLoaded;
                if (g_nTexturesLoaded < _countof(g_Textures))
                    ++g_nTexturesLoaded;
                else
                    return -1;
                g_Textures[handle].m_name = strdup(name);
                g_Textures[handle].m_nameSCRC = SIG_CalcCaseSensitiveSignature(name);
            }
            auto nSkipMipCount = g_nSkipMipCount;
            if (g_Textures[handle].m_assetCategory == AC_UNK)
                g_Textures[handle].m_assetCategory = g_CurrentAssetCategory;
            if (strstr(name, "\\UI\\") || strstr(name, "/UI/")) {
                g_nSkipMipCount = 0;
                g_Textures[handle].m_assetCategory = AC_1;
            }
            ret = GFX_CreateTextureFromTGAX(wh->FirstChar(), handle);
            g_nSkipMipCount = nSkipMipCount;
        }
    }
    return ret;
}
int GFX_CreateTextureFromTGAX(uint8_t *data, int handle) {
    if (!g_gfxCaps.texture_compression_s3tc)
        return -1;
    if (handle == -1) {
        handle = g_nTexturesLoaded;
        if (g_nTexturesLoaded < _countof(g_Textures))
            ++g_nTexturesLoaded;
        else
            return -1;
    }
    const TGAX_HEADER *h = (const TGAX_HEADER *)data;
    uint16_t max_width = h->wWidth;
    uint16_t max_height = h->wHeight;
    int div = 0;
    while ((max_width >> div) > 4) ++div;
    int   nSkipMipCount = g_nSkipMipCount;
    if (div <= 6)
        nSkipMipCount = div + g_nSkipMipCount - 6;
    if (nSkipMipCount < 0)
        nSkipMipCount = 0;
    auto &curTex = g_Textures[handle];
    curTex.m_bestWidth = max_width >> nSkipMipCount;
    curTex.m_loaded |= 1;
    curTex.m_texTime = g_TextureTimeThisFrame;
    curTex.m_bestHeight = max_height >> nSkipMipCount;
    curTex.m_fromLevel = div - nSkipMipCount;
    curTex.m_toLevel = div - 1;
    curTex.m_align = 0;
    curTex.m_field_20_5 = 5;
    curTex.m_totalBytes = 0;
    curTex.m_field_39_0 = 0;
    curTex.m_field_36_3 = 3;
    GFXAPI_CreateTexture(handle, curTex.m_bestWidth, curTex.m_bestHeight, curTex.m_fromLevel);
    auto tdata = data + sizeof(TGAX_HEADER);
    int size_divider = 0;
    unsigned tdata_len, tdata_mult = 16, fmt_idx = 6;
    if (h->wType == 24) {
        tdata_mult = 8;
        fmt_idx = 5;
    }
    if (div > 0) do {
        int w = max_width >> size_divider;
        int h = max_height >> size_divider;
        tdata_len = tdata_mult * ((w + 3) >> 2) * ((h + 3) >> 2);
        if (size_divider >= nSkipMipCount) {
            GFXAPI_UpdateTexture(handle, size_divider - nSkipMipCount /*level*/, w, h, fmt_idx, tdata, tdata_len);
            g_VRAMBytes_Textures += tdata_len;
            curTex.m_totalBytes += tdata_len;
        }
        ++size_divider;
        tdata += tdata_len;
    } while (size_divider < div);
    return handle;
}

//Unit Tests
TEST(SmokeTest, VertexArray) {
    for (int i = 0; i < _countof(g_vertexArray.fast64); i++) {
        auto act = GFX_CreateVertex(nullptr);
        EXPECT_EQ(i, act) << "GFX_CreateVertex result";
    }
    auto act2 = GFX_CreateVertex(nullptr);
    EXPECT_EQ(-1, act2) << "Extra inaccessible";
    int idx = 123456;
    GFX_DestroyVertex(&idx);
    EXPECT_EQ(123456, idx) << "GFX_DestroyVertex neg";
    for (int i = 0; i < _countof(g_vertexArray.fast64); i++) {
        idx = i;
        GFX_DestroyVertex(&idx);
        EXPECT_EQ(-1, idx) << "GFX_DestroyVertex pos";
    }
    idx = 5;
    GFX_DestroyVertex(&idx);
    EXPECT_EQ(5, idx) << "GFX_DestroyVertex double";
}
TEST(SmokeTest, ZData) {
    ZData *pDataDest;
    uint64_t *pPhysIdx;
    namespace fs = std::filesystem;
    for (const auto &entry : fs::directory_iterator("zsh")) {
        if (!entry.is_regular_file()) continue;
        auto fn = entry.path().native();
        std::ifstream fs(fn, std::ios::binary);
        auto sz = entry.file_size();
        std::unique_ptr<byte[]> buf{ new byte[sz] };
        fs.read((char *)buf.get(), sz);
        auto f = new (buf.get()) ZDataFile();
        std::string sign((const char *)f->m_signature, 8);
        //printf("%S Sign: %s\n", entry.path().filename().c_str(), sign.c_str());
        EXPECT_STREQ("GATD HSZ", sign.c_str());
        EXPECT_EQ(1, f->m_version);
        EXPECT_EQ(0x68, f->m_headerLength);
        //EXPECT_EQ(0x400, f->field_10); //not always (bit field?)
        EXPECT_EQ(f->m_dir.m_count, f->m_files.m_count);
        EXPECT_EQ(0, f->field_48);
        EXPECT_EQ(0, f->field_58);
        EXPECT_EQ(f->m_endPtr, f->m_endPtrBackup);
        EXPECT_GE(f->m_endPtr, f->m_payload + f->m_sumLength);
        uint16_t logicIdx = 0;
        size_t sumLength = 0;
        uint32_t prevOffset = 0;
        int act;
        do {
            act = f->Parse(logicIdx, &pPhysIdx, &pDataDest);
            if (act >= 0) {
                EXPECT_EQ(logicIdx, *pPhysIdx);
                EXPECT_GE(pDataDest->m_offset, prevOffset);
                std::string sact((const char *)f->m_payload + pDataDest->m_offset, pDataDest->m_len);
                EXPECT_EQ(0, pDataDest->m_nul);
                EXPECT_EQ(0, pDataDest->gap[0]);
                EXPECT_EQ(0, pDataDest->gap[1]);
                EXPECT_EQ(0, pDataDest->gap[2]);
                if(pDataDest->m_offset > 0 || sumLength == 0) //GFX_Noesis.zvsh and GFX_Noesis.zfsh (only) - one offset repeats many times
                    sumLength += pDataDest->m_len;
                //printf("LogicIdx: %d, PhysIdx: %d, len: %d\n", logicIdx, int(*pPhysIdx), pDataDest->m_len);
                prevOffset = pDataDest->m_offset;
            }
            logicIdx++;
        } while (act >= 0);
        EXPECT_EQ(sumLength, f->m_sumLength);
    }
}
TEST(SmokeTest, ZDataLocal) {
    g_MainThread = GetCurrentThreadId();
    std::vector<byte> vectorDest;
    ZData *pDataDest;
    uint64_t *pPhysIdx;
    ZDataFile *f;
    GFX_CreateShaderParams s{};
    auto act = GFXAPI_ParseShaderFromZData(vectorDest, &f, &pPhysIdx, &pDataDest, "zsh\\GNLinearizeDepth.zvsh", s, false);
    EXPECT_EQ(0, act);
    std::string sign((const char *)f->m_signature, 8);
    //printf("%S Sign: %s\n", entry.path().filename().c_str(), sign.c_str());
    EXPECT_STREQ("GATD HSZ", sign.c_str());
    EXPECT_EQ(1, f->m_version);
    EXPECT_EQ(0x68, f->m_headerLength);
    //EXPECT_EQ(0x400, f->field_10); //not always (bit field?)
    EXPECT_EQ(f->m_dir.m_count, f->m_files.m_count);
    EXPECT_EQ(0, f->field_48);
    EXPECT_EQ(0, f->field_58);
    EXPECT_EQ(f->m_endPtr, f->m_endPtrBackup);
    EXPECT_GE(f->m_endPtr, f->m_payload + f->m_sumLength);
    EXPECT_EQ(0, *pPhysIdx);
//std::string sact((const char *)f->m_payload + pDataDest->m_offset, pDataDest->m_len);
    EXPECT_EQ(0, pDataDest->m_nul);
    EXPECT_EQ(0, pDataDest->gap[0]);
    EXPECT_EQ(0, pDataDest->gap[1]);
    EXPECT_EQ(0, pDataDest->gap[2]);
//printf("LogicIdx: %d, PhysIdx: %d, len: %d\n", logicIdx, int(*pPhysIdx), pDataDest->m_len);
}
