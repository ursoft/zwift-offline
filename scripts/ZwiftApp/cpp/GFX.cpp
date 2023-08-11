#include "ZwiftApp.h"
void GFX_SetLoadedAssetMode(bool mode) {
    g_CurrentAssetCategory = mode ? AC_1 : AC_2;
}
bool GFX_Initialize() {
    g_tweakArray[TWI_MINIMALUI].IntValue() = (g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\MINIMAL_UI", 0, true) == 0);
    g_WorkoutDistortion = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\WORKOUTDISTORTION", true, true);
    g_bFullScreen = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\FULLSCREEN", 0, true) != 0;
    auto WINWIDTH = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\WINWIDTH", 0, true);
    auto WINHEIGHT = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\WINHEIGHT", 0, true);
    auto PREFERRED_MONITOR = g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\PREFERRED_MONITOR", (uint32_t)-1);
    auto VSYNC = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\VSYNC", 1, true);
    auto GPU = g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\GPU", (uint32_t)-1);
    auto GFX_TIER = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\GFX_TIER", (uint32_t)-1, true);
    Log("Initializing graphics window of size %d x %d", WINWIDTH, WINHEIGHT);
    GFX_InitializeParams gip{};
    gip.WINWIDTH = WINWIDTH;
    gip.WINHEIGHT = WINHEIGHT;
    gip.FullScreen = g_bFullScreen;
    gip.PREFERRED_MONITOR = PREFERRED_MONITOR;
    //OMIT (0 in debugger) gip.field_38 = qword_7FF71A893CD0;
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
    auto perf_flags = g_UserConfigDoc.GetU32("PERF", (uint32_t)-1);
    if (perf_flags != -1)
        GFX_AddPerformanceFlags(perf_flags);
    return true;
}
HWND g_Hwnd;
void MainWndCorrectByFrame(GLFWmonitor *m) {
    int xpos, ypos, left, top, right, bottom;// , w, h;
    if (m) {
        glfwGetMonitorPos(m, &xpos, &ypos);
        //OMIT glfwGetVideoMode(m);
        //OMIT glfwGetWindowSize(g_mainWindow, &w, &h);
        glfwGetWindowFrameSize(g_mainWindow, &left, &top, &right, &bottom);
        glfwSetWindowPos(g_mainWindow, xpos + left, ypos + top);
    }
}
void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar *message, const void *userParam) {
    char ods[1024];
    sprintf_s(ods, "[OPENGL] %s\n", message);
    OutputDebugStringA(ods);
}
//#define ZGL_DEBUG
bool GFXAPI_Initialize(const GFX_InitializeParams &gip) {
    glfwSetErrorCallback(glfwZwiftErrorCallback);
    if (!glfwInit()) {
        Log("Failed GLFW init. Returning");
        ZwiftExit(1);
    }
#ifdef ZGL_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
#endif
    auto ver = gip.GlContextVer;
    int prof;
    if (ver & 0xFFC00 | ((ver & 0x3FF) << 20) | (ver >> 20) & 0x3FF) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ver & 0x3FF);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (ver >> 10) & 0x3FF);
        if ((ver & 0xFFC00 | ((ver & 0x3FF) << 20) | (ver >> 20) & 0x3FF) < 0x300800)
            prof = 0;
        else
            prof = GLFW_OPENGL_COMPAT_PROFILE - gip.GlCoreProfile;
    } else {
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
    GLenum gerr = glewInit();
    if (GLEW_OK != gerr) {
        char buf[128];
        sprintf(buf, "glewInit returned: %d", (int)gerr);
        Log(buf);
        MsgBoxAndExit(buf);
    }
#ifdef ZGL_DEBUG
    GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugMessage, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif
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
        g_glCoreContext = glfwGetWindowAttrib(g_mainWindow, GLFW_OPENGL_PROFILE) == GLFW_OPENGL_CORE_PROFILE;
        Log("[GFX]: %s profile", g_glCoreContext ? "Core" : "Compatability");
        auto clamper = glClampColor;
        auto clamper_arg = GL_CLAMP_READ_COLOR;
        if (glClampColor) {
            if (!g_glCoreContext) {
                glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
                glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
            }
        } else {
            clamper = glClampColorARB;
            if (!g_glCoreContext) {
                glClampColorARB(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
                glClampColorARB(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
            }
            clamper_arg = GL_CLAMP_FRAGMENT_COLOR;
        }
        if (clamper) {
            clamper(clamper_arg, GL_FALSE);
        }
        if (!g_glCoreContext)
            glEnable(GL_POINT_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
        if (g_glCoreContext) {
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
void GFXAPI_UnloadTexture(int handle) {
    auto &id = g_Textures[handle].m_glid;
    glDeleteTextures(1, &id);
    id = (uint32_t)-1;
}
void GFX_Internal_UnloadTexture(int handle, TEX_STATE s) {
    GFXAPI_UnloadTexture(handle);
    static_assert(sizeof(GFX_TextureStruct) == 64);
    g_VRAMBytes_Textures -= g_Textures[handle].m_totalBytes;
    g_Textures[handle].m_texState = s;
    g_Textures[handle].m_wrapModeS = TWM_REPEAT;
    g_Textures[handle].m_wrapModeT = TWM_REPEAT;
    g_Textures[handle].m_aniso = 0.0f;
    g_Textures[handle].m_totalBytes = 0;
    g_Textures[handle].m_field_39_0 = 0;
    g_Textures[handle].m_toLevel = 0;
}
void GFX_UnloadTexture(int handle) {
    if (handle != -1)
        GFX_Internal_UnloadTexture(handle, TS_UNLOADED);
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
const uint32_t g_GFX_TO_GL_CMPFUNC[GCF_CNT] = {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};
const uint32_t g_stensilOps[GSO_CNT] = {GL_FALSE, GL_KEEP, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT, GL_FALSE, GL_FALSE};
void GFX_SetStencilRef(uint8_t ref) {
    if (g_pGFX_CurrentStates->m_stencil.m_ref != ref) {
        glStencilFunc(g_GFX_TO_GL_CMPFUNC[g_pGFX_CurrentStates->m_stencil.m_compFunc], ref, g_pGFX_CurrentStates->m_stencil.m_funcMask);
        g_pGFX_CurrentStates->m_stencil.m_ref = ref;
    }
}
void GFX_SetStencilFunc(bool enableTest, GFX_COMPARE_FUNC compareFunc, uint8_t stensilFuncMask, uint8_t stensilMask, GFX_StencilOp sfail, GFX_StencilOp dpfail, GFX_StencilOp dppass) {
    if (g_pGFX_CurrentStates->m_stencil.m_testEnabled != enableTest) {
        if (enableTest)
            glEnable(GL_STENCIL_TEST);
        else
            glDisable(GL_STENCIL_TEST);
        g_pGFX_CurrentStates->m_stencil.m_testEnabled = enableTest;
    }
    if (g_pGFX_CurrentStates->m_stencil.m_compFunc != compareFunc || g_pGFX_CurrentStates->m_stencil.m_funcMask != stensilFuncMask) {
        glStencilFunc(g_GFX_TO_GL_CMPFUNC[compareFunc], g_pGFX_CurrentStates->m_stencil.m_ref, stensilFuncMask);
        g_pGFX_CurrentStates->m_stencil.m_compFunc = compareFunc;
        g_pGFX_CurrentStates->m_stencil.m_funcMask = stensilFuncMask;
    }
    if (g_pGFX_CurrentStates->m_stencil.m_mask != stensilMask) {
        glStencilMask(stensilMask);
        g_pGFX_CurrentStates->m_stencil.m_mask = stensilMask;
    }
    if (g_pGFX_CurrentStates->m_stencil.m_sfail != sfail || g_pGFX_CurrentStates->m_stencil.m_dpfail != dpfail || g_pGFX_CurrentStates->m_stencil.m_dppass  != dppass) {
        glStencilOp(g_stensilOps[sfail], g_stensilOps[dpfail], g_stensilOps[dppass]);
        g_pGFX_CurrentStates->m_stencil.m_sfail = sfail;
        g_pGFX_CurrentStates->m_stencil.m_dpfail = dpfail;
        g_pGFX_CurrentStates->m_stencil.m_dppass = dppass;
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
        g_debugFont.Load(FS_SMALL);
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
    t.m_texState = TS_LOADED;
    GFXAPI_CreateTextureFromRGBA(g_nTexturesLoaded, w, h, data, genMipMap);
    return g_nTexturesLoaded++;
}
void GFX_TextureSys_Initialize() {
    //memset(g_Textures, 0, sizeof(g_Textures)); //и так будет забит нулями
    for (auto &i : g_Textures)
        i.m_texState = TS_INVALID;
    memset(g_WhiteTexture, 255, sizeof(g_WhiteTexture));
    g_WhiteHandle = GFXAPI_CreateTextureFromRGBA(32, 32, g_WhiteTexture, true);
}
int GFX_CreateShaderFromFile(const char *fileName, int handle) {
    GFX_CreateShaderParams s{ fileName };
    int ret = GFX_CreateShaderFromFile(s, handle);
    zassert(ret != -1);
    return ret;
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
    uint32_t t = timeGetTime();
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
    memmove(vectorDest.data(), data, size);
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
    ZData *zdVsh = nullptr, *zdFsh = nullptr;
    ZDataFile *dataFileV = nullptr, *dataFileF = nullptr;
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
    auto pSampler = &pShader->m_samplers[0];
    int c = 0;
    for (auto n : g_ShaderSamplerNames) {
        auto ul = glGetUniformLocation(pShader->m_program, n);
        glUniform1i(ul, c++);
        *pSampler++ = ul;
    }
    static_assert(_countof(g_ShaderAttributeNames) == _countof(pShader->m_attribLocations));
    auto pAttrLoc = &pShader->m_attribLocations[0];
    for (auto n : g_ShaderAttributeNames) {
        auto ul = glGetAttribLocation(pShader->m_program, n);
        *pAttrLoc++ = ul;
    }
    for (auto &i : pShader->m_uniformLocations)
        i = 0x80000000;
    glUseProgram(0);
    g_pGFX_CurrentStates->m_shader = (uint32_t)-1;
}
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds ms;
int dms(const Clock::time_point &from) {
    auto d = Clock::now() - from;
    return std::chrono::duration_cast<ms>(d).count();
}
int maxdms = -1;
//#define OWN_SHADER_CACHE //применял, пока в коде была ошибка (кеш шейдеров распухал из-за неправильной длины буфера)
#define TIME(f) f
#define TIME2(f) ret=f
#define TIME3(f) { \
auto s = Clock::now(); ret=f; auto d=dms(s);\
if(d > maxdms) { /*maxdms=d;*/ printf("%s: %s=%d\n", vsh, #f, d);} \
}
int GFXAPI_CreateShaderFromBuffers_(int handle, int vshLength, const char *vshd, const char *vsh, int pshLength, const char *pshd, const char *psh);
int GFXAPI_CreateShaderFromBuffers(int handle, int vshLength, const char *vshd, const char *vsh, int pshLength, const char *pshd, const char *psh) {
    int ret;
    TIME2(GFXAPI_CreateShaderFromBuffers_(handle, vshLength, vshd, vsh, pshLength, pshd, psh));
    return ret;
}
void GFX_CacheShader(int program, const char *fn) {
    GLint binaryLength;
    GLenum binaryFormat;
    auto outfile = fopen(fn, "wb");
    if (outfile) {
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
        auto binary = (void *)malloc(binaryLength);
        glGetProgramBinary(program, binaryLength, nullptr, &binaryFormat, binary);
        fwrite(&binaryFormat, sizeof(binaryFormat), 1, outfile);
        fwrite(binary, binaryLength, 1, outfile);
        fclose(outfile);
        free(binary);
    }
}
bool GFX_UseCachedShader(GFX_ShaderPair *curShader, const char *fn) {
    GLint   success;
    GLenum binaryFormat;
    auto infile = fopen(fn, "rb");
    if (!infile) 
        return false;
    fseek(infile, 0, SEEK_END);
    auto binaryLength = (GLint)ftell(infile) - sizeof(binaryFormat);
    auto binary = (void *)malloc(binaryLength);
    fseek(infile, 0, SEEK_SET);
    fread(&binaryFormat, sizeof(binaryFormat), 1, infile);
    fread(binary, binaryLength, 1, infile);
    fclose(infile);
    if (curShader->m_program)
        glDeleteProgram(curShader->m_program);
    curShader->m_program = glCreateProgram();
    glAttachShader(curShader->m_program, curShader->m_vshId);
    glAttachShader(curShader->m_program, curShader->m_fshId);
    uint32_t i = 0;
    for (auto san : g_ShaderAttributeNames)
        glBindAttribLocation(curShader->m_program, i++, san);
    glProgramBinary(curShader->m_program, binaryFormat, binary, GLsizei(binaryLength));
    free(binary);
    glGetProgramiv(curShader->m_program, GL_LINK_STATUS, &success);
    if (!success) {
        return false;
    }
    return true;
}
int GFXAPI_CreateShaderFromBuffers_(int handle, int vshLength, const char *vshd, const char *vsh, int pshLength, const char *pshd, const char *psh) {
#ifdef OWN_SHADER_CACHE
    char cacheFileName[MAX_PATH];
    const char *envCacheRoot = getenv("__GL_SHADER_DISK_CACHE_PATH");
    envCacheRoot = envCacheRoot ? envCacheRoot : getenv("__ZGL_SHADER_DISK_CACHE_PATH");
    envCacheRoot = envCacheRoot ? envCacheRoot : "";
    strcpy(cacheFileName, envCacheRoot);
    char *pCacheFileName = cacheFileName + strlen(cacheFileName);
    if (*cacheFileName) {
        *pCacheFileName++ = '\\';
        *pCacheFileName = 0;
    }
    uint32_t signature = SIG_CalcCaseSensitiveSignature(vshd) ^ SIG_CalcCaseSensitiveSignature(pshd);
    sprintf(pCacheFileName, "%x", signature);
    static bool once = true;
    if (once) {
        once = false;
        //glMaxShaderCompilerThreadsARB(-1); //already, default
    }
#endif
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
#ifdef OWN_SHADER_CACHE
    if (GFX_UseCachedShader(curShader, cacheFileName)) {
        GFX_Internal_fixupShaderAddresses(curShader);
        return handle;
    }
#endif
    glShaderSource(curShader->m_vshId, 1, &vshd, &vshLength);
    glShaderSource(curShader->m_fshId, 1, &pshd, &pshLength);
    TIME(glCompileShader(curShader->m_vshId));
    int vCompileStatus = 0, vInfoLog = 0, dummyLength;
    glGetShaderiv(curShader->m_vshId, GL_COMPILE_STATUS, &vCompileStatus);
    if (!vCompileStatus) {
        glGetShaderiv(curShader->m_vshId, GL_INFO_LOG_LENGTH, &vInfoLog);
        if (vInfoLog > 1) {
            auto v22 = (char *)malloc(vInfoLog);
            glGetShaderInfoLog(curShader->m_vshId, vInfoLog, &dummyLength, v22);
            //if (!vCompileStatus)
            LogTyped(LOG_ERROR, "[GFX]: GLSLvsherr log: %s", v22);
            free(v22);
        }
    } else {
        TIME(glCompileShader(curShader->m_fshId));
        glGetShaderiv(curShader->m_fshId, GL_COMPILE_STATUS, &vCompileStatus);
        if (!vCompileStatus) {
            glGetShaderiv(curShader->m_fshId, GL_INFO_LOG_LENGTH, &vInfoLog);
            if (vInfoLog > 1) {
                auto v24 = (char *)malloc(vInfoLog);
                glGetShaderInfoLog(curShader->m_fshId, vInfoLog, &dummyLength, v24);
                //if (!vCompileStatus)
                LogTyped(LOG_ERROR, " [GFX]: GLSLpsherr log: %s", v24);
                free(v24);
            }
        } else {
            if (curShader->m_program)
                glDeleteProgram(curShader->m_program);
            curShader->m_program = glCreateProgram();
            glAttachShader(curShader->m_program, curShader->m_vshId);
            glAttachShader(curShader->m_program, curShader->m_fshId);
            uint32_t i = 0;
            for (auto san : g_ShaderAttributeNames)
                glBindAttribLocation(curShader->m_program, i++, san);
            TIME(glLinkProgram(curShader->m_program));
            int linkStatus;
            glGetProgramiv(curShader->m_program, GL_LINK_STATUS, &linkStatus);
            if (!linkStatus) {
                vInfoLog = 0;
                dummyLength = 0;
                glGetProgramiv(curShader->m_program, GL_INFO_LOG_LENGTH, &vInfoLog);
                if (vInfoLog > 1) {
                    auto v31 = (char *)malloc(vInfoLog);
                    glGetProgramInfoLog(curShader->m_program, vInfoLog, &dummyLength, v31);
                    //if (!linkStatus)
                    LogTyped(LOG_ERROR, "[GFX]: GLSLprogerr log: %s", v31);
                    free(v31);
                }
            } else {
                Log("Shader Loaded successfully. %s", psh);
#ifdef OWN_SHADER_CACHE
                GFX_CacheShader(curShader->m_program, cacheFileName); //QUEST: or after fixup?
#endif
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
                        vshh->m_fileLength,
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
void GFX_MatrixMode(GFX_MatrixType newMode) {
    g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_matrix = g_MatrixContext.m_matrix;
    g_MatrixContext.m_curMode = newMode;
    g_MatrixContext.m_matrix = g_MatrixContext.m_stacks[newMode].m_matrix;
}
void GFX_LoadMatrix(const MATRIX44 &src) {
    *g_MatrixContext.m_matrix = src;
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_LookAt(VEC3 *a1, VEC3 *a2, VEC3 *a3) {
    MATRIX44 m2;
    VEC3 dest, src;
    src.m_data[0] = a1->m_data[0] - a2->m_data[0];
    src.m_data[1] = a1->m_data[1] - a2->m_data[1];
    src.m_data[2] = a1->m_data[2] - a2->m_data[2];
    dest = src.Normalized();
    float v8_x = dest.m_data[0], v8_y = dest.m_data[1];
    float v8_z = dest.m_data[2];
    src.m_data[0] = a3->m_data[1] * v8_z - a3->m_data[2] * v8_y;
    src.m_data[1] = a3->m_data[2] * v8_x - a3->m_data[0] * v8_z;
    src.m_data[2] = a3->m_data[0] * v8_y - a3->m_data[1] * v8_x;
    dest = src.Normalized();
    m2.m_data[0].m_data[3] = 0.0f;
    m2.m_data[1].m_data[3] = 0.0f;
    m2.m_data[2].m_data[3] = 0.0f;
    m2.m_data[0].m_data[0] = dest.m_data[0];
    m2.m_data[1].m_data[1] = v8_z * dest.m_data[0] - dest.m_data[2] * v8_x;
    m2.m_data[0].m_data[1] = dest.m_data[2] * v8_y - v8_z * dest.m_data[1];
    m2.m_data[2].m_data[1] = dest.m_data[1] * v8_x - v8_y * dest.m_data[0];
    m2.m_data[0].m_data[2] = v8_x;
    m2.m_data[1].m_data[0] = dest.m_data[1];
    m2.m_data[1].m_data[2] = v8_y;
    m2.m_data[2].m_data[0] = dest.m_data[2];
    m2.m_data[2].m_data[2] = v8_z;
    m2.m_data[3].m_data[1] = -m2.m_data[1].m_data[1] * a1->m_data[1] - m2.m_data[0].m_data[1] * a1->m_data[0] - m2.m_data[2].m_data[1] * a1->m_data[2];
    m2.m_data[3].m_data[0] = -a1->m_data[1] * dest.m_data[1] - a1->m_data[0] * dest.m_data[0] - a1->m_data[2] * dest.m_data[2];
    m2.m_data[3].m_data[3] = 1.0f;
    m2.m_data[3].m_data[2] = -a1->m_data[1] * v8_y - a1->m_data[0] * v8_x - a1->m_data[2] * v8_z;
    MAT_MulMat(g_MatrixContext.m_matrix, *g_MatrixContext.m_matrix, m2);
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_RotateY(float angle) {
    auto mx = (float *)g_MatrixContext.m_matrix->m_data;
    auto ca = cosf(angle), sa = sinf(angle), v5 = mx[2], v6 = mx[10], v7 = mx[3], v8 = mx[11], v9 = *mx, v10 = mx[8], v11 = mx[9], v12 = mx[1] * ca, v13 = mx[1] * sa;
    *mx = (*mx * ca) - (v10 * sa);
    mx[1] = v12 - (v11 * sa);
    mx[10] = (v6 * ca) + (v5 * sa);
    mx[11] = (v8 * ca) + (v7 * sa);
    mx[3] = (v7 * ca) - (v8 * sa);
    mx[2] = (v5 * ca) - (v6 * sa);
    mx[8] = (v10 * ca) + (v9 * sa);
    mx[9] = (v11 * ca) + v13;
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_RotateX(float angle) {
    auto mx = (float *)g_MatrixContext.m_matrix->m_data;
    auto ca = cosf(angle), sa = sinf(angle), v5 = mx[5], v6 = mx[6], v7 = mx[10], v8 = mx[7], v9 = mx[11], v10 = mx[8], v11 = mx[9], v12 = mx[4] * sa;
    mx[4] = (v10 * sa) + (mx[4] * ca);
    mx[5] = (v11 * sa) + (v5 * ca);
    mx[10] = (v7 * ca) - (v6 * sa);
    mx[11] = (v9 * ca) - (v8 * sa);
    mx[7] = (v9 * sa) + (v8 * ca);
    mx[6] = (v7 * sa) + (v6 * ca);
    mx[8] = (v10 * ca) - v12;
    mx[9] = (v11 * ca) - (v5 * sa);
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_RotateZ(float angle) {
    auto mx = *(float **)g_MatrixContext.m_matrix;
    auto ca = cosf(angle), sa = sinf(angle), v5 = mx[1], v6 = mx[2], v7 = mx[6], v8 = mx[3], v9 = mx[7], v10 = mx[4], v11 = mx[5], v12 = mx[0] * sa;
    mx[0] = (v10 * sa) + (mx[0] * ca);
    mx[1] = (v11 * sa) + (v5 * ca);
    mx[6] = (v7 * ca) - (v6 * sa);
    mx[7] = (v9 * ca) - (v8 * sa);
    mx[3] = (v9 * sa) + (v8 * ca);
    mx[2] = (v7 * sa) + (v6 * ca);
    mx[4] = (v10 * ca) - v12;
    mx[5] = (v11 * ca) - (v5 * sa);
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
VEC4 *GFX_GetFrustumPlanes() { return g_frustumPlanes; }
void GFX_Scale(const VEC3 &m) {
    auto mat = *(float **)g_MatrixContext.m_matrix;
    mat[0] *= m.m_data[0];
    mat[1] *= m.m_data[0];
    mat[2] *= m.m_data[0];
    mat[3] *= m.m_data[0];
    mat[4] *= m.m_data[1];
    mat[5] *= m.m_data[1];
    mat[6] *= m.m_data[1];
    mat[7] *= m.m_data[1];
    mat[8] *= m.m_data[2];
    mat[9] *= m.m_data[2];
    mat[10] *= m.m_data[2];
    mat[11] *= m.m_data[2];
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_MulMatrixInternal(float *dest, const float *mul) {
    auto v2 = *mul, v3 = mul[2], v33 = dest[1], v35 = dest[2], v31 = dest[3], v34 = dest[6], v30 = dest[7], v36 = dest[10], v32 = dest[11], v4 = mul[1], v5 = mul[3], v6 = *dest, v7 = dest[4], v8 = dest[8], v9 = dest[5], v10 = dest[9], v11 = *mul * v33;
    *dest = (((v4 * v7) + (*mul * *dest)) + (v3 * v8)) + (v5 * dest[12]);
    auto v12 = v4 * v34;
    dest[1] = (((v4 * v9) + v11) + (v3 * v10)) + (v5 * dest[13]);
    auto v13 = (((v4 * v30) + (v2 * v31)) + (v3 * v32)) + (v5 * dest[15]);
    dest[2] = ((v12 + (v2 * v35)) + (v3 * v36)) + (v5 * dest[14]);
    dest[3] = v13;
    auto v14 = mul[5], v15 = mul[4], v16 = mul[6], v17 = mul[7];
    dest[4] = (((v14 * v7) + (v15 * v6)) + (v16 * v8)) + (v17 * dest[12]);
    auto v18 = v14 * v34;
    dest[5] = (((v14 * v9) + (v15 * v33)) + (v16 * v10)) + (v17 * dest[13]);
    auto v19 = (((v14 * v30) + (v15 * v31)) + (v16 * v32)) + (v17 * dest[15]);
    dest[6] = ((v18 + (v15 * v35)) + (v16 * v36)) + (v17 * dest[14]);
    dest[7] = v19;
    auto v20 = mul[8], v21 = mul[9], v22 = mul[10], v23 = mul[11];
    dest[8] = (((v21 * v7) + (v20 * v6)) + (v22 * v8)) + (v23 * dest[12]);
    dest[9] = (((v21 * v9) + (v20 * v33)) + (v22 * v10)) + (v23 * dest[13]);
    dest[10] = (((v21 * v34) + (v20 * v35)) + (v22 * v36)) + (v23 * dest[14]);
    dest[11] = (((v21 * v30) + (v20 * v31)) + (v22 * v32)) + (v23 * dest[15]);
    auto v24 = mul[13], v25 = mul[15], v26 = mul[12], v27 = mul[14];
    dest[12] = (((v24 * v7) + (v26 * v6)) + (v27 * v8)) + (v25 * dest[12]);
    dest[13] = (((v24 * v9) + (v26 * v33)) + (v27 * v10)) + (v25 * dest[13]);
    dest[14] = (((v24 * v34) + (v26 * v35)) + (v27 * v36)) + (v25 * dest[14]);
    dest[15] = (((v24 * v30) + (v26 * v31)) + (v27 * v32)) + (v25 * dest[15]);
}
void GFX_MulMatrix(const MATRIX44 &m) {
    GFX_MulMatrixInternal(g_MatrixContext.m_matrix->m_data->m_data, m.m_data->m_data);
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_PopMatrix() {
    auto pPrev = g_MatrixContext.m_matrix - 1;
    zassert(pPrev >= g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_pStack);
    g_MatrixContext.m_matrix = pPrev;
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_PushMatrix() {
    auto pNext = g_MatrixContext.m_matrix + 1;
    zassert(pNext < g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_pStack + g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_mxCount);
    memmove(pNext, g_MatrixContext.m_matrix, sizeof(MATRIX44));
    g_MatrixContext.m_matrix = pNext;
}
bool g_bFlipRenderTexture;
bool GFX_GetFlipRenderTexture() { return g_bFlipRenderTexture; }
void GFX_SetFlipRenderTexture(bool newVal) { g_bFlipRenderTexture = newVal; }
void GFX_LoadIdentity() {
    memmove(g_MatrixContext.m_matrix->m_data, &g_mxIdentity, sizeof(g_mxIdentity));
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void doMatrixStackInitialize(GFX_MatrixType mode, unsigned int mxCount) {
    auto &dest = g_MatrixContext.m_stacks[mode];
    dest.m_pStack = dest.m_matrix = (MATRIX44 *)_aligned_malloc(sizeof(MATRIX44) * mxCount, 16);
    dest.m_mxCount = mxCount;
    memmove(dest.m_matrix, &g_mxIdentity, sizeof(g_mxIdentity));
    ++g_MatrixContext.m_modesUpdCnt[mode];
}
void GFX_UploadShaderMAT4(GFX_SHADER_MATRICES where, const MATRIX44 &what, uint64_t counter) {
    g_pGFX_CurrentStates->SetUniform(GFX_StateBlock::s_matrixRefs[where], what, counter);
    if (g_pCurrentShader) {
        auto loc = g_pCurrentShader->m_matLocations[where];
        if (loc >= 0)
            glUniformMatrix4fv(loc, 1, 0, what.m_data->m_data);
    }
}
const MATRIX44 g_mxIdentity{ {
    {{1.0f, 0.0f, 0.0f, 0.0f}},
    {{0.0f, 1.0f, 0.0f, 0.0f}},
    {{0.0f, 0.0f, 1.0f, 0.0f}},
    {{0.0f, 0.0f, 0.0f, 1.0f}}
} };
void GFX_UpdateFrustum(const MATRIX44 &a1, const MATRIX44 &a2) {
    auto v9 = a1.m_data[0].m_data[3];
    auto v62 = a1.m_data[0].m_data[0] * a2.m_data[0].m_data[0] + a2.m_data[1].m_data[0] * a1.m_data[0].m_data[1] + a2.m_data[2].m_data[0] * a1.m_data[0].m_data[2] + a2.m_data[3].m_data[0] * v9;
    auto v63 = a2.m_data[0].m_data[1] * a1.m_data[0].m_data[0] + a2.m_data[1].m_data[1] * a1.m_data[0].m_data[1] + a2.m_data[2].m_data[1] * a1.m_data[0].m_data[2] + a2.m_data[3].m_data[1] * v9;
    auto v60 = a2.m_data[0].m_data[2] * a1.m_data[0].m_data[0] + a2.m_data[1].m_data[2] * a1.m_data[0].m_data[1] + a2.m_data[2].m_data[2] * a1.m_data[0].m_data[2] + a2.m_data[3].m_data[2] * v9;
    auto v6 = a2.m_data[0].m_data[3];
    auto v5 = a2.m_data[1].m_data[3];
    auto v4 = a2.m_data[2].m_data[3];
    auto v3 = a2.m_data[3].m_data[3];
    auto v59 = v6 * a1.m_data[0].m_data[0] + v5 * a1.m_data[0].m_data[1] + v4 * a1.m_data[0].m_data[2] + v3 * v9;
    auto v18 = a1.m_data[1].m_data[1];
    auto v71 = a1.m_data[1].m_data[0] * a2.m_data[0].m_data[0] + a1.m_data[1].m_data[1] * a2.m_data[4].m_data[0] + a1.m_data[1].m_data[2] * a2.m_data[2].m_data[0] + v18 * a2.m_data[3].m_data[0];
    auto v57 = a1.m_data[1].m_data[0] * a2.m_data[0].m_data[1] + a1.m_data[1].m_data[1] * a2.m_data[1].m_data[1] + a1.m_data[1].m_data[2] * a2.m_data[2].m_data[1] + v18 * a2.m_data[3].m_data[1];
    auto v69 = a1.m_data[1].m_data[0] * a2.m_data[0].m_data[2] + a1.m_data[1].m_data[1] * a2.m_data[1].m_data[2] + a1.m_data[1].m_data[2] * a2.m_data[2].m_data[2] + v18 * a2.m_data[3].m_data[2];
    auto v65 = a1.m_data[1].m_data[0] * v6 + a1.m_data[1].m_data[1] * v5 + a1.m_data[1].m_data[2] * v4 + v18 * v3;
    auto v20 = a1.m_data[2].m_data[3];
    auto v72 = a1.m_data[2].m_data[0] * a2.m_data[0].m_data[0] + a1.m_data[2].m_data[1] * a2.m_data[4].m_data[0] + a1.m_data[2].m_data[2] * a2.m_data[2].m_data[0] + v20 * a2.m_data[3].m_data[0];
    auto v58 = a1.m_data[2].m_data[0] * a2.m_data[0].m_data[1] + a1.m_data[2].m_data[1] * a2.m_data[1].m_data[1] + a1.m_data[2].m_data[2] * a2.m_data[2].m_data[1] + v20 * a2.m_data[3].m_data[1];
    auto v70 = a1.m_data[2].m_data[0] * a2.m_data[0].m_data[2] + a1.m_data[2].m_data[1] * a2.m_data[1].m_data[2] + a1.m_data[2].m_data[2] * a2.m_data[2].m_data[2] + v20 * a2.m_data[3].m_data[2];
    auto v21 = a1.m_data[2].m_data[0] * v6 + a1.m_data[2].m_data[1] * v5 + a1.m_data[2].m_data[2] * v4 + v20 * v3;
    auto v23 = a1.m_data[3].m_data[3];
    auto v56 = a1.m_data[3].m_data[0] * a2.m_data[0].m_data[0] + a1.m_data[3].m_data[1] * a2.m_data[4].m_data[0] + a1.m_data[3].m_data[2] * a2.m_data[2].m_data[0] + v23 * a2.m_data[3].m_data[0];
    auto v61 = a1.m_data[3].m_data[0] * a2.m_data[0].m_data[1] + a1.m_data[3].m_data[1] * a2.m_data[1].m_data[1] + a1.m_data[3].m_data[2] * a2.m_data[2].m_data[1] + v23 * a2.m_data[3].m_data[1];
    auto v24 = a1.m_data[3].m_data[0] * a2.m_data[0].m_data[2] + a1.m_data[3].m_data[1] * a2.m_data[1].m_data[2] + a1.m_data[3].m_data[2] * a2.m_data[2].m_data[2] + v23 * a2.m_data[3].m_data[2];
    auto v25 = a1.m_data[3].m_data[0] * v6 + a1.m_data[3].m_data[1] * v5 + a1.m_data[3].m_data[2] * v4 + v23 * v3;
    float v26, v27, v28, v29, v30, v31, v32, v33;
    if (*((uint32_t *)GFX_GetCoordinateMap() + 10) == 1) {
        v26 = v60;
        v27 = v24;
        v28 = v69;
        v29 = v60;
        v30 = v70;
        v31 = v65;
        v32 = v21;
        v33 = v59;
        g_frustumPlanes[0].m_data[0] = v60;
        g_frustumPlanes[0].m_data[3] = v24;
    } else {
        v33 = v59;
        v26 = v60;
        v27 = v25 + v24;
        v31 = v65;
        v32 = v21;
        v29 = v59 + v60;
        g_frustumPlanes[0].m_data[3] = v25 + v24;
        v28 = v65 + v69;
        v30 = v21 + v70;
        g_frustumPlanes[0].m_data[0] = v59 + v60;
    }
    g_frustumPlanes[0].m_data[2] = v30;
    g_frustumPlanes[0].m_data[1] = v28;
    auto v34 = sqrtf(((v28 * v28) + (v29 * v29)) + (v30 * v30));
    if (v34 != 0.0) {
        g_frustumPlanes[0].m_data[0] = v29 / v34;
        g_frustumPlanes[0].m_data[1] = v28 / v34;
        g_frustumPlanes[0].m_data[2] = v30 / v34;
        g_frustumPlanes[0].m_data[3] = v27 / v34;
    }
    auto v35 = v33 - v62;
    auto v36 = v31 - v71;
    auto v37 = v32 - v72;
    g_frustumPlanes[1].m_data[0] = v33 - v62;
    g_frustumPlanes[1].m_data[1] = v31 - v71;
    g_frustumPlanes[1].m_data[2] = v32 - v72;
    g_frustumPlanes[1].m_data[3] = v25 - v56;
    auto v38 = sqrtf(((v36 * v36) + (v35 * v35)) + (v37 * v37));
    if (v38 != 0.0) {
        g_frustumPlanes[1].m_data[0] = v35 / v38;
        g_frustumPlanes[1].m_data[1] = v36 / v38;
        g_frustumPlanes[1].m_data[2] = v37 / v38;
        g_frustumPlanes[1].m_data[3] = (v25 - v56) / v38;
    }
    auto v103 = v33 + v62;
    auto v40 = v31 + v71;
    auto v41 = v32 + v72;
    g_frustumPlanes[2].m_data[0] = v33 + v62;
    g_frustumPlanes[2].m_data[1] = v31 + v71;
    g_frustumPlanes[2].m_data[2] = v32 + v72;
    g_frustumPlanes[2].m_data[3] = v25 + v56;
    auto v42 = sqrtf(((v40 * v40) + (v103 * v103)) + (v41 * v41));
    if (v42 != 0.0) {
        g_frustumPlanes[2].m_data[0] = v103 / v42;
        g_frustumPlanes[2].m_data[1] = v40 / v42;
        g_frustumPlanes[2].m_data[2] = v41 / v42;
        g_frustumPlanes[2].m_data[3] = (v25 + v56) / v42;
    }
    auto v43 = v33 + v63;
    auto v44 = v31 + v57;
    auto v45 = v32 + v58;
    g_frustumPlanes[3].m_data[0] = v33 + v63;
    g_frustumPlanes[3].m_data[1] = v31 + v57;
    g_frustumPlanes[3].m_data[2] = v32 + v58;
    g_frustumPlanes[3].m_data[3] = v25 + v61;
    auto v46 = sqrtf(((v44 * v44) + (v43 * v43)) + (v45 * v45));
    if (v46 != 0.0) {
        g_frustumPlanes[3].m_data[0] = v43 / v46;
        g_frustumPlanes[3].m_data[1] = v44 / v46;
        g_frustumPlanes[3].m_data[2] = v45 / v46;
        g_frustumPlanes[3].m_data[3] = (v25 + v61) / v46;
    }
    auto v47 = v33 - v63;
    auto v48 = v31 - v57;
    auto v49 = v32 - v58;
    g_frustumPlanes[4].m_data[0] = v33 - v63;
    g_frustumPlanes[4].m_data[1] = v31 - v57;
    g_frustumPlanes[4].m_data[2] = v32 - v58;
    g_frustumPlanes[4].m_data[3] = v25 - v61;
    auto v50 = sqrtf(((v48 * v48) + (v47 * v47)) + (v49 * v49));
    if (v50 != 0.0) {
        g_frustumPlanes[4].m_data[0] = v47 / v50;
        g_frustumPlanes[4].m_data[1] = v48 / v50;
        g_frustumPlanes[4].m_data[2] = v49 / v50;
        g_frustumPlanes[4].m_data[3] = (v25 - v61) / v50;
    }
    auto v51 = v31 - v69;
    auto v52 = v32 - v70;
    auto v53 = v25 - v24;
    auto v54 = v33 - v26;
    g_frustumPlanes[5].m_data[1] = v51;
    g_frustumPlanes[5].m_data[0] = v54;
    g_frustumPlanes[5].m_data[2] = v52;
    g_frustumPlanes[5].m_data[3] = v53;
    auto v55 = sqrtf(((v51 * v51) + (v54 * v54)) + (v52 * v52));
    if (v55 != 0.0) {
        g_frustumPlanes[5].m_data[0] = v54 / v55;
        g_frustumPlanes[5].m_data[1] = v51 / v55;
        g_frustumPlanes[5].m_data[2] = v52 / v55;
        g_frustumPlanes[5].m_data[3] = v53 / v55;
    }
}
void GFX_TransposeMatrix44(MATRIX44 *dest, const MATRIX44 &src) {
    auto v2 = src.m_data[1].m_data[0], v3 = src.m_data[2].m_data[0], v4 = src.m_data[2].m_data[1], v5 = src.m_data[3].m_data[0], v6 = src.m_data[3].m_data[1], v7 = src.m_data[3].m_data[2];
    auto v8 = src.m_data[0].m_data[0], v9 = src.m_data[1].m_data[1], v10 = src.m_data[2].m_data[2], v11 = src.m_data[3].m_data[3], v12 = src.m_data[0].m_data[1], v13 = src.m_data[1].m_data[2], v14 = src.m_data[2].m_data[3], v15 = src.m_data[0].m_data[2], v16 = src.m_data[1].m_data[3];
    dest->m_data[3].m_data[0] = src.m_data[0].m_data[3]; dest->m_data[1].m_data[0] = v12; dest->m_data[1].m_data[1] = v9; dest->m_data[2].m_data[0] = v15; dest->m_data[2].m_data[1] = v13; dest->m_data[2].m_data[2] = v10; dest->m_data[3].m_data[1] = v16; dest->m_data[3].m_data[2] = v14;
    dest->m_data[3].m_data[3] = v11; dest->m_data[0].m_data[1] = v2; dest->m_data[0].m_data[2] = v3; dest->m_data[0].m_data[3] = v5; dest->m_data[1].m_data[2] = v4; dest->m_data[1].m_data[3] = v6; dest->m_data[2].m_data[3] = v7; dest->m_data[0].m_data[0] = v8;
}
void GFX_StoreMatrix(MATRIX44 *dest) { memmove(dest, g_MatrixContext.m_matrix->m_data->m_data, sizeof(MATRIX44)); }
MATRIX44 mx_1 = g_mxIdentity, mx_2 = g_mxIdentity, mx_3 = g_mxIdentity, mx_4 = g_mxIdentity, *g_pLastMx = &mx_1;
void GFX_UpdateMatrices(bool force) {
    static_assert(sizeof(GFX_MatrixContext) == 0x170);
    auto updMask = (g_MatrixContext.m_modesUpdCntCache[2] != g_MatrixContext.m_modesUpdCnt[2] ? 4 : 0)
                 | (g_MatrixContext.m_modesUpdCntCache[1] != g_MatrixContext.m_modesUpdCnt[1] ? 2 : 0) 
                 | (g_MatrixContext.m_modesUpdCntCache[0] != g_MatrixContext.m_modesUpdCnt[0]);
    MATRIX44 *_matrix, tmp;
    if (updMask) {
        auto modeSaved = g_MatrixContext.m_curMode;
        if (updMask & 1) {
            g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_matrix = g_MatrixContext.m_matrix;
            _matrix = g_MatrixContext.m_stacks[0].m_matrix;
            g_MatrixContext.m_matrix = g_MatrixContext.m_stacks[0].m_matrix;
            g_pLastMx = g_MatrixContext.m_stacks[0].m_matrix;
            g_MatrixContext.m_curMode = GMT_0;
            memmove(&g_MatrixContext.m_field_100, _matrix, sizeof(MATRIX44));
            g_MatrixContext.m_modesUpdCntCache[0] = g_MatrixContext.m_modesUpdCnt[0];
        } else {
            _matrix = g_MatrixContext.m_matrix;
        }
        if (updMask & 2) {
            g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_matrix = _matrix;
            _matrix = g_MatrixContext.m_stacks[1].m_matrix;
            g_MatrixContext.m_matrix = g_MatrixContext.m_stacks[1].m_matrix;
            g_MatrixContext.m_curMode = GMT_1;
            GFX_StoreMatrix(&mx_3);
            g_MatrixContext.m_modesUpdCntCache[1] = g_MatrixContext.m_modesUpdCnt[1];
        }
        if (updMask & 4) {
            g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_matrix = _matrix;
            _matrix = g_MatrixContext.m_stacks[2].m_matrix;
            g_MatrixContext.m_matrix = g_MatrixContext.m_stacks[2].m_matrix;
            g_MatrixContext.m_curMode = GMT_2;
            GFX_StoreMatrix(&mx_4);
            g_MatrixContext.m_modesUpdCntCache[2] = g_MatrixContext.m_modesUpdCnt[2];
        }
        memmove(&tmp, &g_mxIdentity, sizeof(g_mxIdentity));
        if (g_MatrixContext.m_field_5 && mx_4.m_data[0].m_data[0] > 0.0)
            mx_4.m_data[0].m_data[0] = -mx_4.m_data[0].m_data[0];
        MAT_MulMat(&tmp, *g_pLastMx, mx_3);
        MAT_MulMat(&mx_2, tmp, mx_4);
        if (updMask & 6)
            MAT_MulMat(&mx_1, mx_3, mx_4);
        g_MatrixContext.m_stacks[g_MatrixContext.m_curMode].m_matrix = _matrix;
        g_MatrixContext.m_curMode = modeSaved;
        g_MatrixContext.m_matrix = g_MatrixContext.m_stacks[modeSaved].m_matrix;
        if (updMask & 6)
            GFX_UpdateFrustum(mx_3, mx_4);
        if (updMask & 2) {
            memmove(&tmp, &g_mxIdentity, sizeof(g_mxIdentity));
            GFX_TransposeMatrix44(&tmp, mx_3);
            MAT_MulVecXYZW(&g_MatrixContext.m_field_140, VEC4{ -mx_3.m_data[3].m_data[0], -mx_3.m_data[3].m_data[1], -mx_3.m_data[3].m_data[2], 0 }, tmp);
        }
        ++g_MatrixContext.m_applUpdatesCnt;
        g_MatrixContext.m_field_80 = mx_2;
    }
    if ((updMask & 1) || force)
        GFX_UploadShaderMAT4(GSM_0, *g_pLastMx, g_MatrixContext.m_modesUpdCnt[0]);
    if ((updMask & 4) || force)
        GFX_UploadShaderMAT4(GSM_3, mx_4, g_MatrixContext.m_modesUpdCnt[2]);
    if ((updMask & 2) || force) {
        GFX_UploadShaderVEC4(GSR_0, g_MatrixContext.m_field_140, g_MatrixContext.m_modesUpdCnt[1]);
        GFX_UploadShaderMAT4(GSM_2, mx_3, g_MatrixContext.m_modesUpdCnt[1]);
    }
    if (updMask || force)
        GFX_UploadShaderMAT4(GSM_1, mx_2, g_MatrixContext.m_applUpdatesCnt);
}
VEC4 g_ShadowPixelParams;
MATRIX44 g_ShadowmapMatrices[4];
VEC4 g_ShadowBounds[4];
void SetupLightmaps(bool spotLight) {
    int v3;
    if (spotLight) {
        for (int v2 = 0; v2 < g_nShadowMaps; ++v2)
            VRAM_RenderFromDepth(g_RT_ShadowMaps + v2, v2 + 10, nullptr, true);
        VRAM_RenderFromDepth(&g_RT_SpotlightShadowMap, 13, nullptr, true);
        v3 = g_HeadlightTexture;
    } else {
        if (g_bUseEmptyShadowMapsHack) {
            for (int v2 = 0; v2 < g_nShadowMaps; ++v2)
                VRAM_RenderFromDepth(&g_RT_EmptyShadowMap, v2 + 10, nullptr, true);
            VRAM_RenderFromDepth(&g_RT_EmptyShadowMap, 13, nullptr, true);
        } else {
            for (int v2 = 0; v2 < g_nShadowMaps; ++v2)
                GFX_ActivateTexture(g_WhiteHandle, v2 + 10, nullptr, TWM_REPEAT);
            GFX_ActivateTexture(g_WhiteHandle, 13, nullptr, TWM_REPEAT);
        }
        v3 = g_WhiteHandle;
    }
    GFX_ActivateTexture(v3, 9, nullptr, TWM_CLAMP_TO_EDGE);
    g_ShadowPixelParams.m_data[2] = (float)(GFX_GetFrameCount() & 3);
    g_ShadowPixelParams.m_data[0] = (float)SHADOWMAP_WIDTH;
    g_ShadowPixelParams.m_data[1] = 1.0f / (float)SHADOWMAP_WIDTH;
    g_ShadowPixelParams.m_data[3] = 0.0f;
    GFX_UploadShaderVEC4(GSR_11, g_ShadowPixelParams, 0);
    for (int v2 = 0; v2 < g_nShadowMaps; ++v2) {
        GFX_UploadShaderMAT4((GFX_SHADER_MATRICES)(v2 + 5), g_ShadowmapMatrices[v2], 0);
        GFX_UploadShaderVEC4((GFX_SHADER_REGISTERS)(v2 + 14), g_ShadowBounds[v2], 0);
    }
#if 0 //TODO
    v4 = qword_7FF66E65F720;                      // GameWorld::GetAllowWaterCaustics inlined
    v5 = *(_QWORD *)(qword_7FF66E65F720 + 8);
    while (!*(_BYTE *)(v5 + 25))
    {
        if (*(_DWORD *)(v5 + 32) >= g_pGameWorld->m_WorldID)
        {
            v4 = v5;
            v5 = *(_QWORD *)v5;
        } else
        {
            v5 = *(_QWORD *)(v5 + 16);
        }
    }
    if (*(_BYTE *)(v4 + 25)
        || g_pGameWorld->m_WorldID < *(_DWORD *)(v4 + 32)
        || v4 == qword_7FF66E65F720
        || (v6 = *(_QWORD *)sub_7FF66D4AFCC0(v4, (int *)&g_pGameWorld->m_WorldID)) == 0
        || (v7 = *(_BYTE *)(v6 + 84) == 0, v8 = g_CausticTexture, v7))
    {
        v8 = g_WhiteHandle;
    }
    GFX_ActivateTexture(v8, 6u, 0i64, TWM_REPEAT);
    GFX_UploadShaderMAT4(GSM_4, &what, 0i64);
    GFX_UploadShaderVEC4(GSR_5, &stru_7FF66E633E88, 0i64);
    GFX_UploadShaderVEC3(6, (float *)&qword_7FF66E633F10);
#endif
}
void GFX_UploadShaderVEC4(const GFX_UserRegister &r, const VEC4 &v, uint64_t skipTag) { //GFX_UploadShaderVEC4_0
    GFX_RegisterRef ref{ .m_ty = GFX_RegisterRef::Ty::User, .m_offset = (uint16_t)r.m_offset, .m_cnt = 1 };
    g_pGFX_CurrentStates->SetUniform(ref, v, skipTag);
    if (g_pCurrentShader) {
        int UniformLocation;
        if (r.m_offset >= 8) {
            UniformLocation = glGetUniformLocation(g_pCurrentShader->m_program, r.m_name);
        } else {
            auto &ul = g_pCurrentShader->m_uniformLocations[r.m_offset];
            if (ul == (int)0x80000000)
                ul = glGetUniformLocation(g_pCurrentShader->m_program, r.m_name);
            UniformLocation = ul;
        }
        if (UniformLocation >= 0)
            glUniform4fv(UniformLocation, 1, v.m_data);
    }
}
void GFX_UploadShaderVEC4(GFX_SHADER_REGISTERS a1, const VEC4 &a2, uint64_t a3) {
    g_pGFX_CurrentStates->SetUniform(GFX_StateBlock::s_registerRefs[a1], a2, a3);
    if (g_pCurrentShader) {
        auto v4 = g_pCurrentShader->m_locations[a1];
        if (v4 >= 0)
            glUniform4fv(v4, 1, g_pGFX_CurrentStates->GetUniform(a1).m_data);
    }
}
void GFX_MatrixStackInitialize() {
    doMatrixStackInitialize(GMT_0, 8);
    doMatrixStackInitialize(GMT_1, 8);
    doMatrixStackInitialize(GMT_2, 8);
    g_MatrixContext.m_field_5 = false;
    g_MatrixContext.m_curMode = GMT_0;
    g_MatrixContext.m_matrix = g_MatrixContext.m_stacks[0].m_matrix;
}
bool GFX_Initialize3DTVSpecs(float, float) {
    //OMIT
    return true;
}
void GFX_DrawInit() {
    g_DrawBuffers[0] = (uint8_t *)malloc(g_DrawBufferSize);
    g_DrawBuffers[1] = (uint8_t *)malloc(g_DrawBufferSize);
    g_DrawNoTextureShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_NoTexture", -1);
    g_DrawTexturedShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_Textured", -1);
    g_DrawTexturedSimpleShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_Textured_Simple", -1);
    g_DrawTexturedGammaCorrectShaderHandle = GFX_CreateShaderFromFile("GFXDRAW_Textured_GammaCorrect", -1);
    //GFXAPI_DrawInit():
    if (g_glCoreContext)
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
        if (g_glCoreContext)
            return "OpenGL Debug Core";
        else
            return "OpenGL Debug";
    } else {
        if (!g_glCoreContext)
            return "OpenGL";
    }
    return "OpenGL Core";
}
const char *GFX_GetAPIName() { return g_GL_apiName; }
const char *GFX_GetVendorName() { return g_GL_vendor; }
const char *GFX_GetRendererName() { return g_GL_renderer; }
void GFX_Present() {
    glfwSwapBuffers(g_mainWindow);
    glfwPollEvents();
    GFX_DrawFlip();
    //v0 = 1;
    //WDT_Tick(&v0);
}
void GFX_EndFrame() {
    auto t = timeGetTime();
    static DWORD g_frTime, g_frDurations[30];
    auto lastTime = g_frTime;
    if (g_frTime == 0)
        lastTime = t;
    g_frTime = t;
    auto dtime = t - lastTime;
    if (dtime > 5000)
        dtime = 5000;
    g_frDurations[(g_nTotalFrames++) % 30] = dtime;
    auto fdtime = dtime / 1000.0f;
    if (g_nTotalFrames > 300 && fdtime > 0.001 && fdtime < 0.2)
        g_instantaniousFPS = 1.0 / fdtime;
    DWORD sum = 0;
    for (auto d : g_frDurations) sum += d;
    g_smoothedFPS = sum ? 30000.0f / sum : 0.0f;
    g_TotalRenderTime += fdtime;
    static DWORD lastCheck;
    static bool lastOnBattery;
    bool nowOnBattery = lastOnBattery;
    if (t - lastCheck > 5000) {
        lastOnBattery = nowOnBattery = OS_IsOnBattery();
        if (nowOnBattery && lastCheck)
            g_SecondsUnplugged += 5.0;
        lastCheck = t;
    }
    static float dword_7FF7AD5F6EB8;
    if (nowOnBattery && g_TargetBatteryFPS > 0.0 && g_TargetBatteryFPS < 61.0 && g_instantaniousFPS > 1.0) {
        auto v13 = dword_7FF7AD5F6EB8;
        auto v14 = 1000.0f / g_TargetBatteryFPS - 1000.0f / g_instantaniousFPS;
        if (v14 > 1.0f) {
            v13 += 1.0f;
            dword_7FF7AD5F6EB8 = v13;
        } else if (v14 < 0.0f) {
            v13 -= 1.0f;
            dword_7FF7AD5F6EB8 = v13;
        }
        if (v13 > 5.0f)
            Sleep(v13);
    }
}
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
const uint32_t g_fillModes[GFM_CNT] = { GL_POINT, GL_LINE, GL_FILL, GL_FALSE };
void GFX_SetFillMode(GFX_FILL_MODE fillMode) {
    g_pGFX_CurrentStates->m_fillMode = fillMode;
    glPolygonMode(GL_FRONT_AND_BACK, g_fillModes[fillMode]);
}
void GFX_PushStates() {
    auto next = g_pGFX_CurrentStates + 1;
    *next = *g_pGFX_CurrentStates;
    g_pGFX_CurrentStates = next;
}
void GFX_SetAlphaBlendEnable(bool en) {
    int ien = en;
    g_pGFX_CurrentStates->m_alphaBlend1 = ien;
    if (g_pGFX_CurrentStates->m_alphaBlend2 != ien) {
        g_pGFX_CurrentStates->m_alphaBlend2 = ien;
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_BLEND;
    }
}
void GFX_SetIndexBuffer(int ib) {
    if (g_pGFX_CurrentStates->m_indexBuffer != ib) {
        g_pGFX_CurrentStates->m_indexBuffer = ib;
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_IB;
    }
}
void GFX_SetScissorTestEnable(bool en) {
    g_pGFX_CurrentStates->m_scissorTest = en;
    if (en)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}
void GFX_SetDepthTestEnable(bool en) {
    g_pGFX_CurrentStates->m_depthTest = en;
    if (en)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}
void GFX_SetDepthWrite(bool en) {
    g_pGFX_CurrentStates->m_depthMask = en;
    glDepthMask(en);
}
void GFX_SetVertex(int handle) {
    if (g_pGFX_CurrentStates->m_vertex != handle) {
        g_pGFX_CurrentStates->m_vertex = handle;
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VAIDX;
    }
}
void GFX_SetVertexBuffer(int vb, uint64_t notUsed, uint64_t attrData) {
    if (g_pGFX_CurrentStates->m_vertexBuffer != vb) {
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VBO;
        g_pGFX_CurrentStates->m_vertexBuffer = vb;
    }
    if (g_pGFX_CurrentStates->m_attrData != attrData) {
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_ATTRDATA;
        g_pGFX_CurrentStates->m_attrData = attrData;
    }
    if (vb != -1) {
        if (g_pGFX_CurrentStates->m_VAO != 0) {
            g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_VAO;
            g_pGFX_CurrentStates->m_VAO = 0;
        }
    }
}
void GFX_PopStates() {
    g_pGFX_CurrentStates--;
    for (int i = 0; i < 16; i++) {
        auto pfltVal = (float*)(&g_pGFX_CurrentStates->m_depthTest) + i;
        auto uval = *(uint32_t *)pfltVal;
        switch (i) {
        case 0:
            if (uval)
                glEnable(GL_DEPTH_TEST);
            else 
                glDisable(GL_DEPTH_TEST);
            break;
        case 1:
            glDepthFunc(g_GFX_TO_GL_CMPFUNC[uval]);
            break;
        case 2:
            if (uval)
                glDepthMask(1u);
            else
                glDepthMask(0);
            break;
        case 3:
            glPolygonMode(GL_FRONT_AND_BACK, g_fillModes[uval]);
            break;
        case 4:
            if (g_pGFX_CurrentStates->m_cullIdx2 != uval) {
                g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_CULL;
                assert(uval < GFC_CNT);
                g_pGFX_CurrentStates->m_cullIdx2 = (GFX_CULL)uval;
            }
            break;
        case 5:
            if (g_pGFX_CurrentStates->m_alphaBlend2 != (uval != 0)) {
                g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_BLEND;
                g_pGFX_CurrentStates->m_alphaBlend2 = (uval != 0);
            }
            break;
        case 6: case 7: case 8: case 9: case 10: case 11: case 14:
            break;
        case 12:
            if (uval)
                glEnable(GL_SCISSOR_TEST);
            else
                glDisable(GL_SCISSOR_TEST);
            break;
        case 13:
            if (*pfltVal == 0.0) {
                glDisable(GL_POLYGON_OFFSET_FILL);
            } else {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(*pfltVal, 1.0);
            }
            break;
        case 15:
            assert(pfltVal == (void*)&g_pGFX_CurrentStates->m_blendFunc1);
            if (g_pGFX_CurrentStates->m_blendFunc2 != g_pGFX_CurrentStates->m_blendFunc1) {
                g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_BLEND;
                g_pGFX_CurrentStates->m_blendFunc2 = g_pGFX_CurrentStates->m_blendFunc1;
            }
            //no break
        default:
            if (g_pGFX_CurrentStates->m_stencil.m_testEnabled)
                glEnable(GL_STENCIL_TEST);
            else
                glDisable(GL_STENCIL_TEST);
            glStencilFunc(
                g_GFX_TO_GL_CMPFUNC[g_pGFX_CurrentStates->m_stencil.m_compFunc],
                g_pGFX_CurrentStates->m_stencil.m_ref,
                g_pGFX_CurrentStates->m_stencil.m_funcMask);
            glStencilMask(g_pGFX_CurrentStates->m_stencil.m_mask);
            glStencilOp(
                g_stensilOps[g_pGFX_CurrentStates->m_stencil.m_sfail],
                g_stensilOps[g_pGFX_CurrentStates->m_stencil.m_dpfail],
                g_stensilOps[g_pGFX_CurrentStates->m_stencil.m_dppass]);
            {
                auto sh = GFX_GetCurrentShaderHandle();
                GFX_UnsetShader();
                GFX_SetShader(sh);
            }
            g_pGFX_CurrentStates->m_bits = (uint32_t)-1;
            g_pGFX_CurrentStates->m_hasRegTypes = (uint32_t)-1;
            g_pGFX_CurrentStates->m_shader = 0;
            g_pGFX_CurrentStates->m_vaIdx = 0;
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
            return;
        }
    }
}
int GFX_GetCurrentShaderHandle() { return g_CurrentShaderHandle; }
void GFX_Internal_SetActiveTexture(int glHandle) {
    if (g_pGFX_CurrentStates->m_actTex != glHandle) {
        glActiveTexture(glHandle);
        g_pGFX_CurrentStates->m_actTex = glHandle;
    }
}
void GFXAPI_ActivateTexture(int handle, int glOffset, const char *uniformName, GFX_TEXTURE_WRAP_MODE wm) {
    auto sh = GFX_GetCurrentShader();
    if (sh) {
        g_FFtextureHandle = -1;
        if (uniformName) {
            auto ul = glGetUniformLocation(sh->m_program, uniformName);
            if (ul > -1) {
                if ((uint32_t)handle >= _countof(g_Textures)) {
                    GFX_Internal_SetActiveTexture(ul + GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, g_Textures[g_WhiteHandle].m_glid);
                    glUniform1i(ul, ul);
                    if (ul < _countof(sh->m_texHandles))
                        sh->m_texHandles[ul] = handle;
                } else {
                    GFX_Internal_SetActiveTexture(ul + GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, g_Textures[handle].m_glid);
                    glUniform1i(ul, ul);
                    if (ul < _countof(sh->m_texHandles))
                        sh->m_texHandles[ul] = handle;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[wm]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[wm]);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, g_Aniso);
                }
            }
        } else {
            int glo = 0;
            if (glOffset != -1)
                glo = glOffset;
            if ((uint32_t)handle >= _countof(g_Textures)) {
                GFX_Internal_SetActiveTexture(glo + GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_Textures[g_WhiteHandle].m_glid);
                glUniform1i(sh->m_samplers[glo], glo);
                sh->m_texHandles[glo] = handle;
            } else if (sh->m_texHandles[glo] != handle || (uint32_t)handle >= _countof(g_Textures)) {
                GFX_Internal_SetActiveTexture(glo + GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_Textures[handle].m_glid);
                if (g_Textures[handle].m_wrapModeS != wm) {
                    g_Textures[handle].m_wrapModeS = wm;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[wm]);
                }
                if (g_Textures[handle].m_wrapModeT != wm) {
                    g_Textures[handle].m_wrapModeT = wm;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[wm]);
                }
                if (g_Textures[handle].m_aniso != g_Aniso) {
                    g_Textures[handle].m_aniso = g_Aniso;
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, g_Aniso);
                }
                sh->m_texHandles[glo] = handle;
            }
        }
    } else {
        if ((uint32_t)handle >= _countof(g_Textures)) {
            if (g_FFtextureHandle != handle)
                glBindTexture(GL_TEXTURE_2D, g_Textures[g_WhiteHandle].m_glid);
        } else if (g_FFtextureHandle != handle) {
            glBindTexture(GL_TEXTURE_2D, g_Textures[handle].m_glid);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[wm]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[wm]);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, g_Aniso);
        }
        g_FFtextureHandle = handle;
    }
    GFX_Internal_SetActiveTexture(GL_TEXTURE0);
}
bool g_bEngineShowLighting; //QUEST where set to 1???
void GFX_ActivateAnimatedTexture_INTERNAL(int handle, int offset, const char *name, GFX_TEXTURE_WRAP_MODE wm) {
    auto v4 = &g_AnimatedTextures[handle - _countof(g_Textures)];
    if (g_FlipbookTextureIndexOverride == -1 || g_FlipbookTextureIndexOverride >= v4->m_framesCnt)
        handle = v4->m_frameHandles[v4->m_curFrame];
    else
        handle = v4->m_frameHandles[g_FlipbookTextureIndexOverride];
    GFX_ActivateTexture(handle, offset, name, wm);
}
void GFX_ActivateTexture(int handle, int offset, const char *name, GFX_TEXTURE_WRAP_MODE wm) {
    if ((unsigned)(handle - _countof(g_Textures)) >= _countof(g_AnimatedTextures)) {
        auto h = handle;
        if (handle <= -1 || handle >= g_nTexturesLoaded && handle != _countof(g_Textures) - 1)
            h = g_WhiteHandle;
        if (g_bEngineShowLighting && offset + 1 <= 1)
            h = g_WhiteHandle;
        if ((unsigned)h < _countof(g_Textures)) {
            if (g_Textures[h].m_texState == TS_1 && GFX_Internal_LoadTextureFromTGAFile(g_Textures[h].m_name, h) == -1)
                g_Textures[h].m_texState = TS_2;
        }
        g_Textures[h].m_texTime = g_TextureTimeThisFrame;
        GFXAPI_ActivateTexture(h, offset, name, wm);
    } else {
        GFX_ActivateAnimatedTexture_INTERNAL(handle, offset, name, wm);
    }
}
void GFX_ActivateTextureEx(int tn, GLfloat lodBias) {
    uint32_t gltn = tn + GL_TEXTURE0;
    if (!tn || gltn == g_pGFX_CurrentStates->m_actTex) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, lodBias);
        if (!tn)
            return;
    } else {
        glActiveTexture(gltn);
        g_pGFX_CurrentStates->m_actTex = gltn;
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, lodBias);
    }
    if (g_pGFX_CurrentStates->m_actTex != GL_TEXTURE0) {
        glActiveTexture(GL_TEXTURE0);
        g_pGFX_CurrentStates->m_actTex = GL_TEXTURE0;
    }
}
void GFX_SetDepthTestFunc(GFX_COMPARE_FUNC fu) {
    g_pGFX_CurrentStates->m_depthFunc = fu;
    glDepthFunc(g_GFX_TO_GL_CMPFUNC[fu]);
}
void GFX_SetBlendFunc(GFX_BLEND_OP op, GFX_BLEND src, GFX_BLEND dst) {
    static_assert(sizeof(GFX_BlendFunc) == 4);
    g_pGFX_CurrentStates->m_blendFunc1 = {op, src, dst};
    if (g_pGFX_CurrentStates->m_blendFunc2 != g_pGFX_CurrentStates->m_blendFunc1) {
        g_pGFX_CurrentStates->m_blendFunc2 = g_pGFX_CurrentStates->m_blendFunc1;
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_BLEND;
    }
}
void GFX_SetCullMode(GFX_CULL cm) {
    auto newCM = GFX_CULL((g_bInvertCulling && cm) ? (cm ^ 3) : cm);
    g_pGFX_CurrentStates->m_cullIdx1 = newCM;
    if (g_pGFX_CurrentStates->m_cullIdx2 != newCM) {
        g_pGFX_CurrentStates->m_bits |= GFX_StateBlock::GSB_PEND_CULL;
        g_pGFX_CurrentStates->m_cullIdx2 = newCM;
    }
}
void GFX_SetDepthBias(float b) {
    g_pGFX_CurrentStates->m_polyOffset = b;
    if (b == 0.0f) {
        glDisable(GL_POLYGON_OFFSET_FILL);
    } else {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(b, 1.0);
    }
}
void GFX_SetTextureFilter(uint32_t tn, GFX_FILTER filter) {
    static_assert(sizeof(GFX_StencilFunc) == 8);    
    static_assert(sizeof(GFX_StateBlock) == 208);
    static const GLint p[GFF_CNT]{ GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR };
    assert(filter < _countof(p));
    if (tn) {
        auto gltn = tn + GL_TEXTURE0;
        if (g_pGFX_CurrentStates->m_actTex != tn + GL_TEXTURE0) {
            glActiveTexture(gltn);
            g_pGFX_CurrentStates->m_actTex = gltn;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p[filter]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, g_Aniso);
    if (tn && g_pGFX_CurrentStates->m_actTex != GL_TEXTURE0) {
        glActiveTexture(GL_TEXTURE0);
        g_pGFX_CurrentStates->m_actTex = GL_TEXTURE0;
    }
    g_pGFX_CurrentStates->m_filters[tn] = filter;
}
int32_t GFX_GetStateU32(GFX_STATE idx) { 
    assert((int)idx == 4 || (int)idx == 12);
    return idx == 4 ? g_pGFX_CurrentStates->m_cullIdx1 : g_pGFX_CurrentStates->m_scissorTest;
}
void GFX_SetupUIProjection() {
    auto pRT = VRAM_GetCurrentRT();
    if (pRT) {
        auto asp = g_width / (float)g_height;
        if (g_bIsAwareOfWideAspectUI && asp > 1.7777778) {
            g_CurrentUISpace_Height = 720.0;
            g_WideUISpace_Height = 720.0;
            g_CurrentUISpace_Width = asp * 720.0;
            g_WideUISpace_Width = asp * 720.0;
            GFX_LoadIdentity();
            GFX_Ortho(0.0, g_CurrentUISpace_Width, g_CurrentUISpace_Height, 0.0, -1.0, 1.0);
        } else {
            auto v3 = 1.7777778;
            if (pRT->m_dw_height)
                v3 = (float)pRT->m_dw_width / (float)pRT->m_dw_height;
            g_CurrentUISpace_Width = 1280.0;
            g_CurrentUISpace_Height = 1280.0 / v3;
            if (asp <= 1.7777778) {
                g_WideUISpace_Height = 1280.0 / v3;
                g_WideUISpace_Width = 1280.0;
            }
            GFX_LoadIdentity();
            GFX_Ortho(0.0, g_CurrentUISpace_Width, g_CurrentUISpace_Height, 0.0, -1.0, 1.0);
        }
    }
}
void GFX_BEGIN_2DUISpace() {
    GFX_SetShader(g_DrawTexturedShaderHandle);
    zassert(VRAM_GetCurrentRT());
    GFX_MatrixMode(GMT_2);
    GFX_PushMatrix();
    GFX_LoadIdentity();
    GFX_SetupUIProjection();
    GFX_MatrixMode(GMT_1);
    GFX_PushMatrix();
    GFX_LoadIdentity();
    GFX_MatrixMode(GMT_0);
    GFX_PushMatrix();
    GFX_LoadIdentity();
    GFX_UpdateMatrices(0);
    g_b2D720pRenderIsSetup = true;
}
void GFX_SetWideAspectAwareUI(bool val) {
    if (val != g_bIsAwareOfWideAspectUI) {
        g_b2D720pRenderIsSetup = false;
        g_bIsAwareOfWideAspectUI = val;
    }
}
void GFX_Ortho(float a1, float w, float h, float a4, float a5, float a6) {
    MATRIX44 m2;
    m2.m_data[1].m_data[0] = 0.0f;
    m2.m_data[1].m_data[2] = 0.0f;
    m2.m_data[1].m_data[3] = 0.0f;
    m2.m_data[2].m_data[0] = 0.0f;
    m2.m_data[2].m_data[1] = 0.0f;
    m2.m_data[2].m_data[3] = 0.0f;
    auto v7 = h - a4;
    auto v8 = 1.0f;
    m2.m_data[3].m_data[3] = 1.0f;
    auto v9 = (w - a1) * 0.5f;
    auto v10 = v7 * 0.5f;
    auto v11 = 0.0f;
    if (g_OrthoScalarH != 0.0f)
        v11 = (w - a1) / g_OrthoScalarH;
    auto v12 = 0.0f;
    if (g_OrthoScalarW != 0.0f)
        v12 = v7 / g_OrthoScalarW;
    auto v13 = v11 * 0.5f;
    auto v14 = v12 * 0.5f;
    auto v15 = v9 - v13;
    auto v16 = v13 + v9;
    auto v17 = v10 - v14;
    auto v18 = v14 + v10;
    if (GFX_GetFlipRenderTexture()) {
        auto v19 = v18;
        v18 = v17;
        v17 = v19;
    }
    auto v20 = -0.001f;
    auto v21 = *((uint32_t *)GFX_GetCoordinateMap() + 10);
    auto v24 = a6 - a5;
    auto v23 = a5;
    auto v22 = 0.0f;
    if (v21 <= 1) {
        if (v21 == 1) {
            if (v24 == 0.0f)
                v8 = -1.0f;
            else
                v8 = -1.0f / v24;
        } else if (v21 == 0) {
            if (v24 == 0.0f)
                v8 = -2.0f;
            else
                v8 = -2.0f / v24;
            v23 += a6;
        }
        if (v24 == 0.0f) {
            v22 = -0.001f;
        } else {
            v22 = -v23 / v24;
        }
    }
    m2 = g_mxIdentity;
    auto v25 = v16 - v15;
    auto v26 = 2.0f;
    auto v27 = 2.0f;
    if (v25 != 0.0f)
        v27 /= v25;
    m2.m_data[0].m_data[0] = v27;
    auto v28 = v17 - v18;
    if (v28 != 0.0f)
        v26 /= v28;
    m2.m_data[1].m_data[1] = v26;
    m2.m_data[2].m_data[2] = v8;
    auto v29 = -0.001f;
    if (v25 != 0.0f)
        v29 = -(v15 + v16) / v25;
    m2.m_data[3].m_data[0] = v29;
    if (v28 != 0.0f)
        v20 = -(v18 + v17) / v28;
    m2.m_data[3].m_data[1] = v20;
    m2.m_data[3].m_data[2] = v22;
    MAT_MulMat(g_MatrixContext.m_matrix, *g_MatrixContext.m_matrix, m2);
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_UnsetShader() {
    g_pCurrentShader = 0;
    g_CurrentShaderHandle = -1;
    glUseProgram(0); //GFXAPI_UnsetShader
    g_pGFX_CurrentStates->m_shader = (uint32_t)-1;
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
    bool extra = (0 == g_pGFX_CurrentStates->m_shaderMAT4ARRAY);
    if (g_pCurrentShader) {
        int i = 0;
        for (auto loc : g_pCurrentShader->m_locations) {
            if (loc >= 0 && (i < 27 || extra)) {
                const auto &uniform = GFX_StateBlock::GetUniform((GFX_SHADER_REGISTERS)i);
                glUniform4fv(loc, GFX_StateBlock::s_registerRefs[i].m_cnt, uniform.m_data);
            }
            i++;
        }
        i = 0;
        for (auto mloc : g_pCurrentShader->m_matLocations) {
            if (mloc >= 0) {
                const auto &uniform = GFX_StateBlock::GetUniform((GFX_SHADER_MATRICES)i);
                glUniformMatrix4fv(mloc, 1, 0, uniform.m_data[0].m_data);
            }
            i++;
        }
        auto mlocEx = g_pCurrentShader->m_matArrLocations[g_pGFX_CurrentStates->m_shaderMAT4ARRAY];
        if (mlocEx >= 0) {
            const auto &ref = GFX_StateBlock::s_matrixArrayRefs[g_pGFX_CurrentStates->m_shaderMAT4ARRAY];
            const auto &uniform = GFX_StateBlock::GetUniform(ref);
            glUniformMatrix4fv(mlocEx, ref.m_cnt, 0, uniform.m_data);
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
    memset(g_pCurrentShader->m_texHandles, 0xFF, sizeof(g_pCurrentShader->m_texHandles));
    return true;
}
void GFX_StateBlock::UnbindBuffer(int arrBuf) {
    if (m_vertexBuffer == arrBuf || m_arrBuf == arrBuf) {
        m_bits |= GSB_PEND_VBO;
        m_arrBuf = -1;
        m_vertexBuffer = -1;
    }
    if (m_indexBuffer == arrBuf) {
        m_bits |= GSB_PEND_IB;
        m_indexBuffer = -1;
    }
}
void GFX_DestroyBuffer(int *pHandle) {
    if (*pHandle != -1) {
        GFXAPI_DestroyBuffer((GLuint)*pHandle);
        *pHandle = -1;
    }
}
void GFXAPI_DestroyBuffer(GLuint handle) {
    g_pGFX_CurrentStates->UnbindBuffer(handle);
    glDeleteBuffers(1, &handle);
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const VEC4 &vec, uint16_t sz, uint64_t skipTag) { //SetUniformVEC4_a, SetUniformVEC4_a2
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (skipTag && skipTag == u.m_pTags[ref.m_offset])
        return;
    const VEC4 *pVec = &vec;
    for (uint16_t i = 0; i < sz; i++, pVec++) {
        u.m_pRegs[i + ref.m_offset] = *pVec;
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
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const VEC2 &vec, uint16_t sz, uint64_t skipTag) { //SetUniformVEC2_a
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (skipTag && skipTag == u.m_pTags[ref.m_offset])
        return;
    const VEC2 *pVec = &vec;
    for (uint16_t i = 0; i < sz; i++, pVec++) {
        auto &dest = u.m_pRegs[i + ref.m_offset];
        dest.m_data[0] = pVec->m_data[0];
        dest.m_data[1] = pVec->m_data[1];
        dest.m_data[2] = dest.m_data[3] = 0.0f;
        u.m_pTags[i + ref.m_offset] = 0;
    }
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + sz)
        u.m_size = ref.m_offset + sz;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::SetUniform(const GFX_RegisterRef &ref, const MATRIX44 &m, uint16_t sz, uint64_t skipTag) {
    auto &u = GFX_StateBlock::uniformRegs[(int)ref.m_ty];
    if (skipTag && skipTag == u.m_pTags[ref.m_offset])
        return;
    for (auto i = 0; i < sz; i++) {
        for (auto j = 0; j < 4; j++) {
            u.m_pRegs->m_data[4 * j + i + ref.m_offset] = m.m_data[i].m_data[j];
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
    memmove(&u.m_pRegs[ref.m_offset], &m, sizeof(m));
    u.m_pTags[ref.m_offset] = tag;
    if (ref.m_offset < u.m_offset)
        u.m_offset = ref.m_offset;
    if (u.m_size < ref.m_offset + 4)
        u.m_size = ref.m_offset + 4;
    m_hasRegTypes |= (1ull << (int)ref.m_ty);
}
void GFX_StateBlock::Reset() {
    m_bits = (uint32_t)-1;
    m_cullIdx2 = GFC_NONE;
    m_alphaBlend2 = 0;
    m_blendFunc2 = { GBO_FUNC_ADD, GB_FALSE, GB_TRUE, GB_FALSE };
    m_attrData = 0;
    m_VAO = 0;
    m_indexBuffer = m_vertex = m_shader = (uint32_t)-1;
    m_vaIdx = 0;
    m_field_A8 = 0i64;
    m_actTex = GL_TEXTURE0;
    if (m_arrBuf) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_arrBuf = 0;
    }
    m_hasRegTypes = (uint32_t)-1;
    m_shaderMAT4ARRAY;
}
const uint32_t g_GFX_VertexFormat_size[GVF_CNT] = { 0, 4, 4, 4, 4, 1, 2, 3 };
const GLenum g_GFX_VertexFormat_format[GVF_CNT] = { GL_FALSE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT, GL_FLOAT, GL_FLOAT };
const uint8_t g_GFX_VertexFormat_normalized[GVF_CNT] = { 0, 1, 1, 0, 1, 0, 0, 0 };
const GLenum g_GFX_TO_GL_CULL[GFC_CNT + 1] = { GL_FALSE, GL_FRONT, GL_BACK, GL_FALSE }; //gap, maybe
const GLenum g_GFX_TO_GL_BLENDFUNC[GB_CNT] = { GL_FALSE, GL_TRUE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA };
const GLenum g_GFX_TO_GL_BLENDOP[GBO_CNT] = { GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_MIN, GL_MAX, GL_FUNC_REVERSE_SUBTRACT, GL_FALSE };
bool GFX_StateBlock::Realize() {
    if (m_shader == -1 || m_vertex == -1)
        return false;
    if (m_bits & GSB_PEND_VBO) {
        auto vb = m_vertexBuffer;
        if (m_vertexBuffer == -1)
            vb = 0;
        GFX_StateBlock::BindVertexBuffer(vb);
    }
    if (m_bits & 0xF) {
        uint64_t field_B8 = 0;
        if (m_vertexBuffer == -1)
            field_B8 = m_VAO;
        uint64_t inv = 0i64;
        auto vx = g_vertexArray.fast64[m_vertex];
        auto pAttr = vx->m_creParams.m_attrs;
        for (int vi = 0; vi < vx->m_creParams.m_attrCnt; vi++) {
            auto atrIdx = pAttr->m_atrIdx;
            auto addm = (1i64 << atrIdx);
            inv |= addm;
            if ((addm & g_pGFX_CurrentStates->m_field_A8) == 0)
                glEnableVertexAttribArray(atrIdx);
            static_assert(sizeof(GFX_VertexAttr) == 4);
            glVertexAttribPointer(
                atrIdx,
                g_GFX_VertexFormat_size[pAttr->m_fmtIdx],
                g_GFX_VertexFormat_format[pAttr->m_fmtIdx],
                g_GFX_VertexFormat_normalized[pAttr->m_fmtIdx],
                vx->m_strides[pAttr->m_strideIdx], //GLsizei stride
                (const void *)uintptr_t(m_attrData + field_B8 + pAttr->m_dataOffset)); //const void * pointer
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
    if (m_bits & GSB_PEND_IB) {
        auto field_98 = m_indexBuffer;
        if (field_98 == -1)
            field_98 = 0;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, field_98);
    }
    if (m_bits & GSB_PEND_CULL) {
        if (m_cullIdx2) {
            glEnable(GL_CULL_FACE);
            glCullFace((GLenum)g_GFX_TO_GL_CULL[m_cullIdx2]);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }
    if (m_bits & GSB_PEND_BLEND) {
        if (m_alphaBlend2)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        glBlendEquation(g_GFX_TO_GL_BLENDOP[m_blendFunc2.m_mode]);
        glBlendFunc(g_GFX_TO_GL_BLENDFUNC[m_blendFunc2.m_srcFactor], g_GFX_TO_GL_BLENDFUNC[m_blendFunc2.m_dstFactor]);
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
const VEC4 &GFX_StateBlock::GetUniform(GFX_SHADER_REGISTERS reg) {
    return GFX_StateBlock::uniformRegs[(int)s_registerRefs[reg].m_ty].m_pRegs[s_registerRefs[reg].m_offset];
}
const MATRIX44 &GFX_StateBlock::GetUniform(GFX_SHADER_MATRICES mat) {
    return *(MATRIX44 *)(GFX_StateBlock::uniformRegs[(int)s_matrixRefs[mat].m_ty].m_pRegs + s_matrixRefs[mat].m_offset);
}
const VEC4 &GFX_StateBlock::GetUniform(const GFX_RegisterRef &ref) {
    return GFX_StateBlock::uniformRegs[(int)ref.m_ty].m_pRegs[ref.m_offset];
}
void GFX_StateBlock::BindVertexBuffer(int arrBuf) {
    if (m_arrBuf != arrBuf) {
        glBindBuffer(GL_ARRAY_BUFFER, arrBuf);
        m_arrBuf = arrBuf;
    }
    m_bits |= (m_vertexBuffer != arrBuf ? GSB_PEND_VBO : 0);
}
void GFX_Begin() { g_pGFX_CurrentStates->Reset(); }
void GFXAPI_CreateVertex(int idx, const GFX_CreateVertexParams &parms) {
    auto vx = new GFX_Vertex;
    vx->m_creParams = parms;
    memset(vx->m_strides, 0, sizeof(vx->m_strides));
    g_vertexArray.fast64[idx] = vx;
    for (int i = 0; i < vx->m_creParams.m_stridesCnt; i++)
        vx->m_strides[vx->m_creParams.m_strides[i].m_strideIdx] += vx->m_creParams.m_strides[i].m_strideCnt;
}
int GFX_CreateVertex(const GFX_CreateVertexParams &parms) {
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
namespace GameShaders { void LoadAll() {
    dualAlphaShader = GFX_CreateShaderFromFile("GFXDRAW_Textured_ExtAlpha", -1);
    shGaussianBlur = -1; //FIX RTL GFX_CreateShaderFromFile("GaussianBlur", -1);
} }
int GFX_CreateShader(const GFX_CreateShaderParams &p) {
    if (p.m_name && *p.m_name)
        return GFX_CreateShaderFromFile(p, -1);
    else
        return -1;
}
int GFX_Internal_FindLoadedAnimatedTexture(const char *name) {
    auto crc = SIG_CalcCaseSensitiveSignature(name);
    if (g_nAnimatedTexturesLoaded > 0)
        for (int i = 0; i < g_nAnimatedTexturesLoaded; i++)
            if (g_AnimatedTextures[i].m_nameSCRC == crc)
                return _countof(g_Textures) + i;
    return -1;
}
void GFX_Draw2DQuad(float l, float t, float w, float h, uint32_t color, bool uiProjection) {
    static_assert(sizeof(DRAW_VERT_POS_COLOR) == 16);
    const int cnt = 6;
    auto ptr = (DRAW_VERT_POS_COLOR *)GFX_DrawMalloc(sizeof(DRAW_VERT_POS_COLOR) * cnt, 4);
    if (ptr) {
        for (int i = 0; i < cnt; i++) {
            ptr[i].m_dummy = 0;
            ptr[i].m_color = color;
        }
        ptr[0].m_point = { l, t };
        ptr[1].m_point = { l + w, t };
        ptr[2].m_point = { l + w, t + h };
        ptr[3].m_point = { l, t + h };
        ptr[4].m_point = { l, t };
        ptr[5].m_point = { l + w, t + h };
        bool mx;
        if (g_b2D720pRenderIsSetup && uiProjection) {
            mx = false;
        } else {
            mx = true;
            auto pCurrentRT = VRAM_GetCurrentRT();
            zassert(pCurrentRT);
            GFX_SetShader(g_DrawNoTextureShaderHandle);
            GFX_UploadShaderVEC4(GSR_24, g_Vec4White, 0);
            GFX_MatrixMode(GMT_2);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            if (uiProjection)
                GFX_SetupUIProjection();
            else
                GFX_Ortho(0.0f, (float)pCurrentRT->m_dw_width, (float)pCurrentRT->m_dw_height, 0.0f, -1.0f, 1.0f);
            GFX_MatrixMode(GMT_1);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_MatrixMode(GMT_0);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_UpdateMatrices(false);
        }
        GFX_DrawPrimitive(GPT_TRIANGLES, ptr, cnt);
        if (mx) {
            GFX_MatrixMode(GMT_2);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_1);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_0);
            GFX_PopMatrix();
        }
    }
}
int GFX_CreateAnimatedTextureFromTGAFiles(const char *name) {
    auto found = GFX_Internal_FindLoadedAnimatedTexture(name);
    if (found != -1) {
        if (found >= 0) {
            LogDebug("========== FOUND CACHED ANIM TEXTURE (%s) ==========", name);
            int idx = found - _countof(g_Textures);
            if (g_AnimatedTextures[idx].m_framesCnt <= 0)
                return found;
            auto pFrameHandle = g_AnimatedTextures[idx].m_frameHandles;
            for (int curFrame = 0; curFrame < g_AnimatedTextures[idx].m_framesCnt; ++curFrame) {
                zassert(*pFrameHandle < _countof(g_Textures));
                if (*pFrameHandle < _countof(g_Textures)) {
                    if (!g_Textures[*pFrameHandle].InHardware())
                        GFX_Internal_LoadTextureFromTGAFile(g_Textures[*pFrameHandle].m_name, *pFrameHandle);
                }
                ++pFrameHandle;
            }
            return found;
        }
        LogDebug("========== ANIM TEXTURE ERROR (%s) found 0 ==========");
        return found;
    }
    if (g_nAnimatedTexturesLoaded >= _countof(g_AnimatedTextures) - 1) {
        LogDebug("========== ANIM TEXTURE ERROR (%s) g_nAnimatedTexturesLoaded overflow ==========", name);
        return found;
    }
    int newIdx = g_nAnimatedTexturesLoaded++;
    auto &dest = g_AnimatedTextures[newIdx];
    LogDebug("========== NEW ANIM TEXTURE %d (%s) ============", newIdx, name);
    dest.m_nameSCRC = SIG_CalcCaseSensitiveSignature(name);
    auto pCurFrameHandle = dest.m_frameHandles;
    int frameIdx = 0;
    char buf[MAX_PATH];
    while (1) {
        sprintf_s(buf, "%s%02d.tga", name, frameIdx);
        *pCurFrameHandle = GFX_CreateTextureFromTGAFile(buf, -1, false);
        if (*pCurFrameHandle == -1)
            break;
        ++frameIdx;
        ++pCurFrameHandle;
        if (frameIdx >= _countof(dest.m_frameHandles)) {
            frameIdx = dest.m_framesCnt;
            break;
        }
    }
    if (!frameIdx) {
        --g_nAnimatedTexturesLoaded;
        dest.m_nameSCRC = 0;
        return -1;
    }
    dest.m_framesCnt = frameIdx;
    dest.m_frameRate = 10.0;
    dest.m_delay = (float)frameIdx / dest.m_frameRate;
    return newIdx + _countof(g_Textures);
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
/*struct TGA_HEADER {
    char  idlength;
    char  colourmaptype;
    char  datatypecode;
    int16_t colourmaporigin;
    int16_t colourmaplength;
    char  colourmapdepth;
    int16_t x_origin;
    int16_t y_origin;
    int16_t width;   //[w6]
    int16_t height;  //[w7]
    char  bitsperpixel;
    char  imagedescriptor;
};*/
int GFX_CreateTextureFromTGA(uint8_t *data, int handle) {
    if (handle == -1) {
        if (g_nTexturesLoaded >= _countof(g_Textures) - 1)
            return -1;
        handle = g_nTexturesLoaded;
        ++g_nTexturesLoaded;
    }
    auto &curTex = g_Textures[handle];
    curTex.m_loaded &= ~1;
    curTex.m_texTime = g_TextureTimeThisFrame;
    curTex.m_texState = TS_LOADED;
    auto srcPxd = data + 18; //TGA_HEADER size
    int mipMapSizes[16];
    uint8_t *mipMapBuffers[16];
    uint8_t *mipMapAllocs[16];
    uint8_t bgra[64];
    if (data[2] < 8) { //datatypecode: uncompressed
        auto tgaHeight = *((uint16_t *)data + 7), tgaWidth = *((uint16_t *)data + 6);
        char fileName[MAX_PATH];
        if (((data[16] - 8) & 0xEF) == 0) { //bitsperpixel: 24, not 32
            auto new3pxd = (uint8_t *)malloc(3 * tgaWidth * tgaHeight);
            curTex.m_bestWidth = tgaWidth;
            curTex.m_bestHeight = tgaHeight;
            curTex.m_align = 0;
            if (data[16] == 8) { //grayscale?
                data[16] = 24;
                for (int y = 0; y < tgaHeight; ++y) {
                    auto src = &srcPxd[tgaWidth * (tgaHeight - y - 1)];
                    auto dst = &new3pxd[3 * y * tgaWidth];
                    for (int x = 0; x < tgaWidth; ++x) {
                        auto v79 = *src++;
                        *dst++ = v79;
                        *dst++ = v79;
                        *dst++ = v79;
                    }
                }
            } else {
                for (int y = 0; y < tgaHeight; ++y) {
                    auto dst = &new3pxd[3 * y * tgaWidth];
                    auto src = &srcPxd[3 * tgaWidth * (tgaHeight - y - 1)];
                    for (int x = 0; x < tgaWidth; ++x) {
                        auto v85 = *src++;
                        auto v87 = *src++;
                        auto v88 = *src++;
                        *dst++ = v88; //color conversion?
                        *dst++ = v87;
                        *dst++ = v85;
                    }
                }
            }
            curTex.m_field_36_3 = 3;
            if (g_gfxCaps.texture_compression_s3tc /* && byte_7FF604466124 probably always true */) {
                auto maxMipMapLevel = 0;
                if (tgaWidth > 4) {
                    do {
                        if ((tgaHeight >> maxMipMapLevel) <= 4)
                            break;
                        ++maxMipMapLevel;
                    } while ((tgaWidth >> maxMipMapLevel) > 4);
                }
                for (int m = 0; m < maxMipMapLevel; ++m) {
                    auto h = tgaHeight >> m;
                    auto w = tgaWidth >> m;
                    if (m) {
                        mipMapAllocs[m] = (uint8_t *)malloc(3 * w * h);
                        auto pSrc = mipMapAllocs[m - 1];
                        auto v97 = tgaWidth >> (m - 1);
                        for(int v98 = 0; v98 < h; v98++) {
                            if (w) {
                                auto v99 = v97 * v98;
                                auto v100 = v97 * (6 * v98 + 3);
                                auto v101 = v98 * (w - v97);
                                auto v102 = -v99;
                                do {
                                    auto v103 = 6 * v99;
                                    auto v104 = v100 + 6 * (v99 + v102);
                                    auto v105 = 3 * (v101 + v99);
                                    auto v106 = (pSrc[v103 + 2] + pSrc[v103 + 5] + pSrc[v104 + 2] + pSrc[v104 + 5]) >> 2;
                                    auto v107 = pSrc[v103] + pSrc[v104] + pSrc[v103 + 3] + pSrc[v104 + 3];
                                    mipMapAllocs[m][v105 + 1] = (pSrc[v103 + 1] + pSrc[v103 + 4] + pSrc[v104 + 1] + pSrc[v104 + 4]) >> 2;
                                    mipMapAllocs[m][v105 + 2] = v106;
                                    mipMapAllocs[m][v105] = v107 >> 2;
                                } while (++v99 + v102 < w);
                            }
                        }
                    } else {
                        mipMapAllocs[0] = new3pxd;
                    }
                    mipMapSizes[m] = 8 * ((w + 3) >> 2) * ((h + 3) >> 2);
                    auto pBuffer = mipMapBuffers[m] = (uint8_t *)calloc(mipMapSizes[m], 1);
                    for (int v108 = 0; v108 < (h >> 2); v108++) {
                        for (int v112 = 0; v112 < (w >> 2); v112++) {
                            auto v115 = 4 * v108;
                            auto v116 = bgra;
                            for (int i = 0; i < 4; i++) {
                                for (int j = 0; j < 4; j++) {
                                    auto v119 = 4 * v112 + w * v115;
                                    v116[3] = 0xFF;
                                    auto v120 = &mipMapAllocs[m][3 * v119++];
                                    v116[0] = *v120;
                                    v116[1] = v120[1];
                                    v116[2] = v120[2];
                                    v116 += 4;
                                }
                                ++v115;
                            }
                            squish::Decompress(bgra, pBuffer++, squish::kDxt1);
                        }
                    }
                }
                sprintf(fileName, "%sx", curTex.m_name);
                auto f = fopen(fileName, "wb");
                if (f) {
                    fwrite(data, 18, 1, f);
                    auto pBufferf = mipMapBuffers;
                    auto pSizesf = mipMapSizes;
                    for(int m = maxMipMapLevel; m > 0; m--)
                        fwrite(*pBufferf++, *pSizesf++, 1, f);
                    fclose(f);
                }
                GFXAPI_CreateTexture(handle, tgaWidth, tgaHeight, maxMipMapLevel);
                auto pSizes = mipMapSizes;
                auto pBuffer = mipMapBuffers;
                auto pAlloc = mipMapAllocs;
                for(int m = 0; m < maxMipMapLevel; m++) {
                    GFXAPI_UpdateTexture(handle, m, tgaWidth >> m, tgaHeight >> m, 5, *pBuffer++, *pSizes);
                    free(*pAlloc++);
                    g_VRAMBytes_Textures += *pSizes++;
                }
                return handle;
            }
            GFXAPI_CreateTextureFromRGBA(handle, tgaWidth, tgaHeight, new3pxd, true);
            free(new3pxd);
        } else { //32bpp
            auto new4pxd = (uint8_t *)malloc(4 * tgaWidth * tgaHeight);
            curTex.m_bestWidth = tgaWidth;
            curTex.m_bestHeight = tgaHeight;
            curTex.m_align = 1;
            for (int y = 0; y < tgaHeight; ++y) {
                auto dst = &new4pxd[4 * y * tgaWidth];
                for (int x = 0; x < tgaWidth; ++x) {
                    auto m = &srcPxd[4 * tgaWidth * (tgaHeight - y - 1)];
                    auto v19 = *m++;
                    auto v21 = *m++;
                    auto v22 = *m++;
                    auto v23 = *m;
                    *dst++ = v22; //color conversion?
                    *dst++ = v21;
                    *dst++ = v19;
                    *dst++ = v23;
                }
            }
            curTex.m_field_36_3 = 3;
            if (g_gfxCaps.texture_compression_s3tc /* && byte_7FF604466125 probably always true */) {
                auto maxMipMapLevel = 0;
                if (tgaWidth > 4) {
                    do {
                        if ((tgaHeight >> maxMipMapLevel) <= 4)
                            break;
                        ++maxMipMapLevel;
                    } while ((tgaWidth >> maxMipMapLevel) > 4);
                }
                for (int m = 0; m < maxMipMapLevel; ++m) {
                    auto h = tgaHeight >> m;
                    auto w = tgaWidth >> m;
                    if (m) {
                        mipMapAllocs[m] = (uint8_t *)malloc(4 * w * h);
                        auto pSrc = mipMapAllocs[m - 1];
                        auto v97 = tgaWidth >> (m - 1);
                        for (int v98 = 0; v98 < h; v98++) {
                            if (w) {
                                auto v100 = v97 * (8 * v98 + 4);
                                auto v99 = v97 * v98;
                                auto v101 = v98 * (w - v97);
                                auto v102 = -v99;
                                do {
                                    auto v103 = 8 * v99;
                                    auto v104 = v100 + 8 * (v99 + v102);
                                    auto v105 = 4 * (v101 + v99);
                                    mipMapAllocs[m][v105] = (pSrc[v104] + pSrc[v103 + 4] + pSrc[v104 + 4] + pSrc[v103]) >> 2;
                                    mipMapAllocs[m][v105 + 1] = (pSrc[v103 + 1] + pSrc[v103 + 5] + pSrc[v104 + 1] + pSrc[v104 + 5]) >> 2;
                                    mipMapAllocs[m][v105 + 2] = (pSrc[v103 + 2] + pSrc[v103 + 6] + pSrc[v104 + 2] + pSrc[v104 + 6]) >> 2;
                                    mipMapAllocs[m][v105 + 3] = (pSrc[v103 + 3] + pSrc[v103 + 7] + pSrc[v104 + 3] + pSrc[v104 + 7]) >> 2;
                                } while (++v99 + v102 < w);
                            }
                        }
                    } else {
                        mipMapAllocs[0] = new4pxd;
                    }
                    mipMapSizes[m] = 16 * ((w + 3) >> 2) * ((h + 3) >> 2);
                    auto pBuffer = mipMapBuffers[m] = (uint8_t *)calloc(mipMapSizes[m], 1);
                    for (int v108 = 0; v108 < (h >> 2); v108++) {
                        for (int v112 = 0; v112 < (w >> 2); v112++) {
                            auto v115 = 4 * v108;
                            auto v116 = bgra;
                            for (int i = 0; i < 4; i++) {
                                for (int j = 0; j < 4; j++) {
                                    auto v119 = 4 * v112 + w * v115;
                                    auto v120 = &mipMapAllocs[m][4 * v119++];
                                    v116[0] = *v120;
                                    v116[1] = v120[1];
                                    v116[2] = v120[2];
                                    v116[3] = v120[3];
                                    v116 += 4;
                                }
                                ++v115;
                            }
                            squish::Decompress(bgra, pBuffer++, squish::kDxt5);
                        }
                    }
                }
                sprintf(fileName, "%sx", curTex.m_name);
                auto f = fopen(fileName, "wb");
                if (f) {
                    fwrite(data, 18, 1, f);
                    auto pBufferf = mipMapBuffers;
                    auto pSizesf = mipMapSizes;
                    for (int m = maxMipMapLevel; m > 0; m--)
                        fwrite(*pBufferf++, *pSizesf++, 1, f);
                    fclose(f);
                }
                GFXAPI_CreateTexture(handle, tgaWidth, tgaHeight, maxMipMapLevel);
                auto pSizes = mipMapSizes;
                auto pBuffer = mipMapBuffers;
                auto pAlloc = mipMapAllocs;
                for (int m = 0; m < maxMipMapLevel; m++) {
                    GFXAPI_UpdateTexture(handle, m, tgaWidth >> m, tgaHeight >> m, 6, *pBuffer++, *pSizes);
                    free(*pAlloc++);
                    g_VRAMBytes_Textures += *pSizes++;
                }
                return handle;
            }
            GFXAPI_CreateTextureFromRGBA(handle, tgaWidth, tgaHeight, new4pxd,true);
            free(new4pxd);
        }
        return handle;
    }
    zassert(0);
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
                        g_Textures[h].m_name = _strdup(name);
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
                    g_Textures[handle].m_name = _strdup(name);
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
void GFX_SetAnimatedTextureFramerate(int handle, float frameRate) {
    if (handle >= _countof(g_Textures) && handle < g_nAnimatedTexturesLoaded + _countof(g_Textures)) {
        static_assert(sizeof(GFX_AnimatedTexture) == 260);
        auto &t = g_AnimatedTextures[handle - _countof(g_Textures)];
        t.m_frameRate = frameRate;
        t.m_delay = t.m_framesCnt / frameRate;
    }
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
                g_Textures[handle].m_name = _strdup(name);
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
    const TGAX_HEADER *hdr = (const TGAX_HEADER *)data;
    uint16_t max_width = hdr->wWidth;
    uint16_t max_height = hdr->wHeight;
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
    curTex.m_texState = TS_LOADED;
    curTex.m_totalBytes = 0;
    curTex.m_field_39_0 = 0;
    curTex.m_field_36_3 = 3;
    GFXAPI_CreateTexture(handle, curTex.m_bestWidth, curTex.m_bestHeight, curTex.m_fromLevel);
    auto tdata = data + sizeof(TGAX_HEADER);
    int size_divider = 0;
    unsigned tdata_len, tdata_mult = 16, fmt_idx = 6;
    if (hdr->wType == 24) {
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
int g_nMSU_Unloads, g_curMSUhandle;
uint32_t g_lastMSUtime;
bool g_bEnableDynamicMeshUnloading;
void GFX_MeshSystem_Update() {
    g_MeshTimeThisFrame = timeGetTime();
    if (g_bEnableDynamicMeshUnloading && g_MeshTimeThisFrame - g_lastMSUtime < 1000) {
        const int perOnce = 40;
        for (auto i = g_curMSUhandle; i < g_curMSUhandle + perOnce; ++i) {
            auto gdeHandle = i % g_ENG_nResources;
            auto item = g_ENG_InstanceResources[gdeHandle];
            if (!item.m_isSkin && item.m_state == IRS_INITIAL) {
                auto td = g_MeshTimeThisFrame - item.m_creationTime;
                auto fago = GFX_GetFrameCount() - item.m_frameCnt;
                if (td > 1000) {
                    auto gdeName = item.m_gdeName;
                    if (fago <= 3) {
                        LogDebug("Would have unloaded mesh %s but it was used only %d frames ago. Timedelta = %d", gdeName, fago, td);
                    } else {
                        ++g_nMSU_Unloads;
                        LogDebug("auto-Unloading timed out mesh %s [nUnloads  %d]", gdeName, g_nMSU_Unloads);
                        LOADER_UnloadGdeFile(gdeHandle);
                        item.m_state = IRS_NEED_LOAD;
                    }
                }
            }
        }
        g_curMSUhandle += perOnce;
        if (g_curMSUhandle > g_ENG_nResources)
            g_curMSUhandle = 0;
    }
    g_lastMSUtime = g_MeshTimeThisFrame;
}
void GFXAPI_CreateBuffer(int *dest, const GFX_CreateBufferParams &p) {
    int VBOHandle = 0;
    glGenBuffers(1, (GLuint *)&VBOHandle);
    zassert(VBOHandle);
    g_pGFX_CurrentStates->BindVertexBuffer(VBOHandle);
    glBufferData(GL_ARRAY_BUFFER, p.m_size, p.m_pData, GL_STATIC_DRAW);
    *dest = VBOHandle;
}
int GFX_CreateBuffer(const GFX_CreateBufferParams &p) {
    int ret;
    GFXAPI_CreateBuffer(&ret, p);
    if (ret != -1)
        g_VRAMBytes_VBO += p.m_size;
    return ret;
}
void GFX_CreateVertexBuffer(int *pHandle, uint32_t size, void *data) { *pHandle = GFX_CreateBuffer(GFX_CreateBufferParams{size, data}); }
void GFX_CreateIndexBuffer(int *dest, uint32_t size, void *data) {
    *dest = GFX_CreateBuffer(GFX_CreateBufferParams{ size, data });
    if (*dest != -1)
        g_VRAMBytes_VBO += size;
}
uint32_t GFX_Align(uint32_t addr, uint32_t align) {
    auto v2 = align - 1;
    zassert((align & (align - 1)) == 0);
    return ~v2 & (addr + v2);
}
uint8_t *GFX_DrawMalloc(int size, uint32_t align) {
    auto v3 = GFX_Align(g_CurrentBufferOffset, align);
    g_CurrentBufferOffset = v3;
    if (v3 + size > g_DrawBufferSize)
        return nullptr;
    g_CurrentBufferOffset = v3 + size;
    return g_DrawBuffers[g_CurrentBuffer] + v3;
}
void GFX_DrawFlip() {
    g_CurrentBuffer = (g_CurrentBuffer & 1) == 0;
    g_PreviousBufferOffset = g_CurrentBufferOffset;
    g_CurrentBufferOffset = 0;
}
void GFX_DrawIndexedInstancedPrimitive(GFX_PRIM_TYPE ty, uint32_t a2, uint32_t a3, GFX_IndexFormat gif, uint32_t a5, uint32_t a6) {
    assert(a6 == 0);
    if (g_pGFX_CurrentStates->m_indexBuffer != -1 && g_pGFX_CurrentStates->Realize())
        glDrawElementsInstanced(g_PRIM_TO_GLPRIM[ty], a3, g_IF_TO_GLIF[gif], nullptr, a5);
}
void GFX_internal_DrawPrimitive(GFX_PRIM_TYPE ty, int first, uint32_t count) {
    if (g_pGFX_CurrentStates->Realize())
        glDrawArrays(g_PRIM_TO_GLPRIM[ty], first, count);
}
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE ty, int baseVertex, uint32_t cnt, GFX_IndexFormat gif, const void *indices) {
    if (g_pGFX_CurrentStates->m_indexBuffer != -1 && g_pGFX_CurrentStates->Realize()) {
        if (baseVertex && GLEW_ARB_draw_elements_base_vertex)
            glDrawElementsBaseVertex(g_PRIM_TO_GLPRIM[ty], cnt, g_IF_TO_GLIF[gif], indices, baseVertex);
        else
            glDrawElements(g_PRIM_TO_GLPRIM[ty], cnt, g_IF_TO_GLIF[gif], indices);
    }
}
void GFX_Translate(const VEC3 &v) {
    auto v2 = &g_MatrixContext.m_matrix->m_data[3]; (void)v2;
    for (int i = 0; i < 4; i++) {
        g_MatrixContext.m_matrix->m_data[3].m_data[i] += v.m_data[1] * g_MatrixContext.m_matrix->m_data[1].m_data[i]
            + v.m_data[0] * g_MatrixContext.m_matrix->m_data[0].m_data[i] + v.m_data[2] * g_MatrixContext.m_matrix->m_data[2].m_data[i];
    }
    ++g_MatrixContext.m_modesUpdCnt[g_MatrixContext.m_curMode];
}
void GFX_Draw2DQuad(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, int a9, float a10, int a11, bool uiProjection, int a13) { //_13
    auto bptr = GFX_DrawMalloc(36 * 4, 4);
    auto fptr = (float *)bptr;
    if (bptr) {
        bool v14;
        if ((g_b2D720pRenderIsSetup || !uiProjection) && uiProjection && a10 == 0.0f) {
            v14 = false;
        } else {
            v14 = true;
            if (a11 == -1)
                GFX_SetShader(g_DrawTexturedShaderHandle);
            auto pCurrentRT = VRAM_GetCurrentRT();
            zassert(pCurrentRT);
            GFX_MatrixMode(GMT_2);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            if (uiProjection)
                GFX_SetupUIProjection();
            else
                GFX_Ortho(0.0f, (float)pCurrentRT->m_dw_width, (float)pCurrentRT->m_dw_height, 0.0f, -1.0f, 1.0f);
            GFX_MatrixMode(GMT_1);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            GFX_MatrixMode(GMT_0);
            GFX_PushMatrix();
            GFX_LoadIdentity();
            if (a10 != 0.0f) {
                GFX_Translate(VEC3{ (a3 * 0.5f) + a1, (a4 * 0.5f) + a2 });
                GFX_RotateZ((a10 / 180.0f) * std::numbers::pi_v<float>);
                GFX_Translate(VEC3{ -(a3 * 0.5f + a1), -a2 - (a4 * 0.5f) });
            }
            GFX_UpdateMatrices(0);
            GFX_UploadShaderVEC4(GSR_24, g_Vec4White, 0);
        }
        int *iptr = (int *)bptr;
        *fptr = a1;
        fptr[1] = a2;
        iptr[2] = a13;
        fptr[10] = a5 + a7;
        fptr[16] = a5 + a7;
        fptr[17] = a6 + a8;
        fptr[22] = a5 + a7;
        fptr[23] = a6 + a8;
        fptr[29] = a6 + a8;
        fptr[4] = a5;
        fptr[5] = a6;
        fptr[6] = a1 + a3;
        fptr[7] = a2;
        iptr[8] = a13;
        fptr[11] = a6;
        fptr[12] = a1 + a3;
        fptr[13] = a2 + a4;
        iptr[14] = a13;
        fptr[18] = a1 + a3;
        fptr[19] = a2 + a4;
        iptr[20] = a13;
        fptr[24] = a1;
        fptr[25] = a2 + a4;
        iptr[26] = a13;
        fptr[28] = a5;
        fptr[30] = a1;
        fptr[31] = a2;
        iptr[32] = a13;
        fptr[34] = a5;
        fptr[35] = a6;
        iptr[3] = a9;
        iptr[9] = a9;
        iptr[15] = a9;
        iptr[21] = a9;
        iptr[27] = a9;
        iptr[33] = a9;
        if (a11 == -1)
            GFX_SetShader(g_DrawTexturedShaderHandle);
        GFX_DrawPrimitive(GPT_TRIANGLES, (DRAW_VERT_POS_COLOR_1UV *)bptr, 6);
        if (v14) {
            GFX_MatrixMode(GMT_2);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_1);
            GFX_PopMatrix();
            GFX_MatrixMode(GMT_0);
            GFX_PopMatrix();
        }
    }
}
void GFX_Draw2DQuad_720p(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, int color, float a10, int a11, int a12) {
    GFX_Draw2DQuad(a1, a2, a3, a4, a5, a6, a7, a8, color, a10, a11, true, a12);
}
/*TODO: int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR_2UV_NORM_TAN_PACKED>();
int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR_UV_NORM_TAN>();*/
template <> int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR_1UV>() {
    static int stDRAW_VERT_POS_COLOR_1UV = -1;
    if (stDRAW_VERT_POS_COLOR_1UV == -1)
        stDRAW_VERT_POS_COLOR_1UV = GFX_CreateVertex(GFX_CreateVertexParams{ 3, 1,
            { {0, 0, GVF_FLOAT7, 0}, {0, 4, GVF_UNSIGNED_BYTE1, 12}, {0, 6, GVF_FLOAT6, 16} },
            { {0, DRAW_VERT_POS_COLOR_1UV::MULT, GVF_UNSIGNED_BYTE1, 0} }
            });
    return stDRAW_VERT_POS_COLOR_1UV;
}
template <> int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR_UV>() {
    static int stDRAW_VERT_POS_COLOR_UV = -1;
    if (stDRAW_VERT_POS_COLOR_UV == -1)
        stDRAW_VERT_POS_COLOR_UV = GFX_CreateVertex(GFX_CreateVertexParams{ 4, 1,
            { {0, 0, GVF_FLOAT7, 0}, {0, 4, GVF_UNSIGNED_BYTE1, 12}, {0, 6, GVF_FLOAT6, 16}, {0, 7, GVF_FLOAT6, 24} },
            { {0, DRAW_VERT_POS_COLOR_UV::MULT, GVF_UNSIGNED_BYTE1, 0} }
            });
    return stDRAW_VERT_POS_COLOR_UV;
}
template <> int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR>() {
    static int stDRAW_VERT_POS_COLOR = -1;
    if (stDRAW_VERT_POS_COLOR == -1)
        stDRAW_VERT_POS_COLOR = GFX_CreateVertex(GFX_CreateVertexParams{ 2, 1,
            { {0, 0, GVF_FLOAT7, 0}, {0, 4, GVF_UNSIGNED_BYTE1, 12} },
            { {0, DRAW_VERT_POS_COLOR::MULT, GVF_UNSIGNED_BYTE1, 0} }
            });
    return stDRAW_VERT_POS_COLOR;
}
template <> int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR_UV_NORM>() {
    static int stDRAW_VERT_POS_COLOR_UV_NORM = -1;
    if (stDRAW_VERT_POS_COLOR_UV_NORM == -1) {
        stDRAW_VERT_POS_COLOR_UV_NORM = GFX_CreateVertex(GFX_CreateVertexParams{ 5, 1,
            { {0, 0, GVF_FLOAT7, 0}, {0, 4, GVF_UNSIGNED_BYTE1, 12}, {0, 6, GVF_FLOAT6, 16}, {0, 7, GVF_FLOAT6, 24}, {0, 1, GVF_FLOAT7, 32} },
            { {0, DRAW_VERT_POS_COLOR_UV_NORM::MULT, GVF_UNSIGNED_BYTE1, 0} }
            });
    }
    return stDRAW_VERT_POS_COLOR_UV_NORM;
}
template <> int GFX_GetVertexHandle<DRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED>() {
    static int stDRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED = -1;
    if (stDRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED == -1) {
        stDRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED = GFX_CreateVertex(GFX_CreateVertexParams{ 5, 1,
            { {0, 0, GVF_FLOAT7, 0}, {0, 4, GVF_UNSIGNED_BYTE1, 12}, {0, 6, GVF_FLOAT6, 16}, {0, 1, GVF_BYTE, 24}, {0, 2, GVF_BYTE, 28} },
            { {0, DRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED::MULT, GVF_UNSIGNED_BYTE1, 0} }
            });
    }
    return stDRAW_VERT_POS_COLOR_UV_NORM_TAN_PACKED;
}
void GFX_SetTextureWrap(uint32_t tn, GFX_TEXTURE_WRAP_MODE t, GFX_TEXTURE_WRAP_MODE s) {
    if (tn) {
        auto v6 = tn + GL_TEXTURE0;
        if (g_pGFX_CurrentStates->m_actTex != v6) {
            glActiveTexture(v6);
            g_pGFX_CurrentStates->m_actTex = v6;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[t]);
    g_pGFX_CurrentStates->m_wrapS[tn] = t;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, g_GFX_TO_GL_TEXTURE_ADDRESS_MODE[s]);
    g_pGFX_CurrentStates->m_wrapT[tn] = s;
    if (tn) {
        if (g_pGFX_CurrentStates->m_actTex != GL_TEXTURE0) {
            glActiveTexture(GL_TEXTURE0);
            g_pGFX_CurrentStates->m_actTex = GL_TEXTURE0;
        }
    }
}
uint8_t *GFXAPI_MapBuffer(int bufId, const GFX_MapBufferParams &p) {
    g_pGFX_CurrentStates->BindVertexBuffer(bufId);
    if ((p.m_access & p.BUF_READ_WRITE) == p.BUF_READ_WRITE)
        return p.m_offset + (uint8_t *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if (p.m_access & p.BUF_READ)
        return p.m_offset + (uint8_t *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    if (p.m_access & p.BUF_WRITE)
        return p.m_offset + (uint8_t *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    return nullptr;
}
uint8_t *GFX_MapBuffer(int bufId, const GFX_MapBufferParams &p) {
    if (bufId == -1)
        return nullptr;
    else
        return GFXAPI_MapBuffer(bufId, p);
}
void GFX_SetShadowParameters(float sp) {
    //TODO
}
void GFXAPI_CreateTexture(int, const GFX_TextureDef &, const GFX_TextureBytes &, uint64_t) {
    //TODO
}
int GFX_CreateTexture(const GFX_TextureDef &def, const GFX_TextureBytes &ptr, uint64_t) {
    //TODO
    return 0;
}
int GFX_CreateTextureFromLuminanceF32(uint32_t, uint32_t, float *) {
    //TODO
    return 0;
}
void GFXAPI_UnmapBuffer(int bufId) {
    g_pGFX_CurrentStates->BindVertexBuffer(bufId);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}
void GFX_UnmapBuffer(int bufId) {
    if (bufId != -1)
        GFXAPI_UnmapBuffer(bufId);
}
bool GFX_AABBInCurrentFrustum(const VEC3 &, const VEC3 &, float *) {
    //TODO
    return false;
}
void GFX_SpheresInCurrentFrustumDist(Sphere *, int, float *) {
    //TODO
}
void GFX_DrawIndexedPrimitive(GFX_PRIM_TYPE, uint32_t, uint32_t, GFX_IndexFormat, uint32_t) {
    //TODO
}

//Unit Tests
TEST(SmokeTest, DISABLED_VertexArray) {
    static bool bFirst = true;
    if (bFirst) {
        bFirst = false;
        EXPECT_EQ(0, g_vertexArray.size);
    } else {
        g_vertexArray.size = 0;
        g_vertexArray.extra = nullptr;
    }
    GFX_CreateVertexParams p{ 4, 1,
        { {0, 0, GVF_FLOAT7, 0}, {0, 3, GVF_UNSIGNED_BYTE1, 12}, {0, 6, GVF_FLOAT6, 16}, {0, 7, GVF_FLOAT6, 24} },
        { {0, 32, GVF_UNSIGNED_BYTE1, 0} }
        };
    for (int i = 0; i < _countof(g_vertexArray.fast64); i++) {
        auto act = GFX_CreateVertex(p);
        EXPECT_EQ(i, act) << "GFX_CreateVertex result";
    }
    auto act2 = GFX_CreateVertex(p);
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
TEST(SmokeTest, Transpose) {
    MATRIX44 dest, src{ { { 0.0f, 1.0f, 2.0f, 3.0f }, { 0.1f, 1.1f, 2.1f, 3.1f }, { 0.2f, 1.2f, 2.2f, 3.2f }, { 0.3f, 1.3f, 2.3f, 3.3f } } };
    GFX_TransposeMatrix44(&dest, src);
    EXPECT_FLOAT_EQ(dest.m_data[0].m_data[0], src.m_data[0].m_data[0]);
    EXPECT_FLOAT_EQ(dest.m_data[0].m_data[1], src.m_data[1].m_data[0]);
    EXPECT_FLOAT_EQ(dest.m_data[0].m_data[2], src.m_data[2].m_data[0]);
    EXPECT_FLOAT_EQ(dest.m_data[0].m_data[3], src.m_data[3].m_data[0]);
    EXPECT_FLOAT_EQ(dest.m_data[1].m_data[0], src.m_data[0].m_data[1]);
    EXPECT_FLOAT_EQ(dest.m_data[1].m_data[1], src.m_data[1].m_data[1]);
    EXPECT_FLOAT_EQ(dest.m_data[1].m_data[2], src.m_data[2].m_data[1]);
    EXPECT_FLOAT_EQ(dest.m_data[1].m_data[3], src.m_data[3].m_data[1]);
    EXPECT_FLOAT_EQ(dest.m_data[2].m_data[0], src.m_data[0].m_data[2]);
    EXPECT_FLOAT_EQ(dest.m_data[2].m_data[1], src.m_data[1].m_data[2]);
    EXPECT_FLOAT_EQ(dest.m_data[2].m_data[2], src.m_data[2].m_data[2]);
    EXPECT_FLOAT_EQ(dest.m_data[2].m_data[3], src.m_data[3].m_data[2]);
    EXPECT_FLOAT_EQ(dest.m_data[3].m_data[0], src.m_data[0].m_data[3]);
    EXPECT_FLOAT_EQ(dest.m_data[3].m_data[1], src.m_data[1].m_data[3]);
    EXPECT_FLOAT_EQ(dest.m_data[3].m_data[2], src.m_data[2].m_data[3]);
    EXPECT_FLOAT_EQ(dest.m_data[3].m_data[3], src.m_data[3].m_data[3]);
}
TEST(SmokeTest, DISABLED_ShaderCacheGrow) {
    EXPECT_TRUE(glfwInit());
    g_mainWindow = glfwCreateWindow(10, 10, "Zwift", nullptr, nullptr);
    EXPECT_TRUE(g_mainWindow != nullptr);
    glfwMakeContextCurrent(g_mainWindow);
    EXPECT_EQ(GLEW_OK, glewInit());
    g_WADManager.LoadWADFile("assets/global.wad");
    g_gfxShaderModel = 4;
    g_BlurShaderHandle = GFX_CreateShaderFromFile("Blur", -1);
    EXPECT_TRUE(g_BlurShaderHandle != -1);
    glfwTerminate();
    g_mainWindow = nullptr;
}