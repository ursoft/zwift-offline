#include "ZwiftApp.h"
void GFX_SetLoadedAssetMode(bool mode) {
    g_CurrentAssetCategory = mode ? AC_1 : AC_2;
}
bool GFX_Initialize() {
    g_MinimalUI = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\MINIMAL_UI", 0, true) == 0;
    g_WorkoutDistortion = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\WORKOUTDISTORTION", true, true);
    g_bFullScreen = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\FULLSCREEN", 0, true) != 0;
    WINWIDTH = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\WINWIDTH", 0, true);
    WINHEIGHT = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\WINHEIGHT", 0, true);
    PREFERRED_MONITOR = g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\PREFERRED_MONITOR", -1, true);
    VSYNC = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\VSYNC", 1, true);
    GPU = g_UserConfigDoc.GetU32("ZWIFT\\CONFIG\\GPU", -1, true);
    GFX_TIER = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\GFX_TIER", -1, true);
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
    WINWIDTH = gip.WINWIDTH;
    WINHEIGHT = gip.WINHEIGHT;
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
    int cxs = 0, cys = 0;
    //if (pMon != glfwGetPrimaryMonitor()) {
        auto m = glfwGetVideoMode(pMon);
        if (m) {
            cys = m->height;
            cxs = m->width;
        }
    //}
    bool badWH = (!WINWIDTH || !WINHEIGHT);
    if (badWH) {
        WINWIDTH = cxs;
        WINHEIGHT = cys;
    }
    int tryh, tryw;
    if (gip.FullScreen) {
        tryh = cys;
        tryw = cxs;
    } else {
        tryh = WINHEIGHT;
        tryw = WINWIDTH;
    }
    auto selMon = gip.FullScreen ? pMon : nullptr;
    g_mainWindow = glfwCreateWindow(tryw, tryh, "Zwift", selMon, nullptr);
    if (!g_mainWindow) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, 0);
        g_mainWindow = glfwCreateWindow(WINWIDTH, WINHEIGHT, "Zwift", selMon, nullptr);
        if (!g_mainWindow) {
            g_openglFail = true;
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, 0);
            g_mainWindow = glfwCreateWindow(WINWIDTH, WINHEIGHT, "Zwift", selMon, nullptr);
            if (!g_mainWindow) {
                Log("Could not create graphics window of size %d x %d", WINWIDTH, WINHEIGHT);
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
        glfwSetWindowSize(g_mainWindow, WINWIDTH - left - right, WINHEIGHT - top - bottom);
    }
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        char buf[128];
        sprintf(buf, "glewInit returned: %d", (int)err);
        Log(buf);
        MsgBoxAndExit(buf);
    }
    g_GL.m_vendorName = (const char *)glGetString(GL_VENDOR);
    g_GL.m_renderer = (const char *)glGetString(GL_RENDERER);
    g_GL.m_version = (const char *)glGetString(GL_VERSION);
    if (gip.m_vendorName)
        g_GL.m_vendorName = gip.m_vendorName;
    if (gip.m_renderer)
        g_GL.m_renderer = gip.m_renderer;
    if (!g_GL.m_version) {
        Log("GLver == NULL!");
        MsgBoxAndExit(GetText("LOC_ERROR_NO_GRAPHICS"));
    }
    if (g_GL.m_vendorName && g_GL.m_renderer) {
        Log("GFX_Initialize: GL_version = %s", g_GL.m_version);
        Log("                GL_vendor = %s", g_GL.m_vendorName);
        Log("                GL_render = %s", g_GL.m_renderer);
    }
    auto glVerMajor = atoi(g_GL.m_version);
    auto glVerMinor = atoi(g_GL.m_version + 2);
    if (strstr(g_GL.m_renderer, "GDI") || glVerMajor == 1) {
        char buf[4096];
        sprintf(buf, "%s\n\nVersion: %s\nVendor: %s\nRenderer: %s", GetText("LOC_ERROR_UNSUPPORTED_GL"), g_GL.m_version, g_GL.m_vendorName, g_GL.m_renderer);
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
    if ((g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | (g_glVersion >> 20) & 0x3FF) < 0x300000
        && !GLEW_EXT_framebuffer_object) {
        auto err = "EXT_framebuffer_object OpenGL extension is required.  Try updating your video card drivers.";
        LogTyped(LOG_ERROR, err);
        MsgBoxAndExit(err);
    }
    if ((g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | (g_glVersion >> 20) & 0x3FF) < 0x300000
        && !glDeleteFramebuffersEXT) {
        auto err = "glDeleteFramebuffersEXT OpenGL extension is required.  Try updating your video card drivers.";
        LogTyped(LOG_ERROR, err);
        MsgBoxAndExit(err);
    }
    if ((g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | (g_glVersion >> 20) & 0x3FF) < 0x300000
        && !glDeleteRenderbuffersEXT) {
        auto err = "glDeleteRenderbuffersEXT OpenGL extension is required.  Try updating your video card drivers.";
        LogTyped(LOG_ERROR, err);
        MsgBoxAndExit(err);
    }
    if ((g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | (g_glVersion >> 20) & 0x3FF) < 0x200000
        && !GLEW_ARB_shader_objects) {
        MsgBoxAndExit("ARB_shader_objects OpenGL extension is required.  Try updating your video card drivers.");
    }
    //TODO
    //v8 = 0i64;
    if (GFX_CheckExtensions()) {
        return true;
    }
    return false;
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
        g_debugFont.Load(FS_GIANTW);
        GDEMESH_Initialize();
        return true;
    }
    return false;
}
void GFX_TextureSys_Initialize() {
    //TODO
}
uint32_t GFX_CreateShaderFromFile(const char *fileName, int) {
    //TODO
    return 0;
}
void GFX_MatrixStackInitialize() {
    //TODO
}
bool GFX_Initialize(uint32_t, uint32_t, bool, bool, uint32_t, const char *, const char *) {
    //TODO
    return true;
}
bool GFX_Initialize3DTVSpecs(float, float) {
    //TODO
    return true;
}
void GFX_DrawInit() {
    //TODO
}
void GFXAPI_CalculateGraphicsScore() {
    //TODO
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
const char *GFX_GetAPIName() { return g_GL.m_version; }
const char *GFX_GetVendorName() { return g_GL.m_vendorName; }
const char *GFX_GetRendererName() { return g_GL.m_renderer; }
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
    if (tmp > 16)
        tmp = 16;
    g_gfxCaps.max_anisotropy = tmp;
    g_gfxCaps.texture_compression_s3tc = GLEW_EXT_texture_compression_s3tc != 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
    g_gfxCaps.max_v_attribs = std::clamp(tmp, 0, 16);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &tmp);
    g_gfxCaps.max_color_atchs = std::clamp(tmp, 0, 4);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tmp);
    g_gfxCaps.max_txt_iu = std::clamp(tmp, 0, 16);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
    g_gfxCaps.max_v_txt_iu = std::clamp(tmp, 0, 4);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
    g_gfxCaps.max_ctxt_iu = std::clamp(tmp, 0, 20);
#if 0
    glGetIntegerv(GL_MAX_CLIP_PLANES, &v22);
    v8 = v22;
    if (v22 > 1)
        v8 = 1;
    g_gfxCaps.field_24 = v8;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, v15);
    g_gfxCaps.field_88 = v15[0];
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, v15);
    g_gfxCaps.field_15 = GLEW_ARB_shader_group_vote != 0;
    g_gfxCaps.field_8C = v15[0];
    v9 = g_glVersion;
    g_gfxCaps.field_16 = GLEW_ARB_shader_ballot != 0;
    v10 = g_glVersion & 0xFFC00 | ((g_glVersion & 0x3FF) << 20) | ((unsigned int)g_glVersion >> 20) & 0x3FF;
    if (v10 >= 0x401400 || (g_gfxCaps.field_D = 0, GLEW_ARB_clip_control))
        g_gfxCaps.field_D = 1;
    if (v10 >= 0x300800)
    {
        glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, v15);
        g_gfxCaps.field_2F = v15[0];
        v9 = g_glVersion;
    }
    if ((v9 & 0xFFC00 | ((v9 & 0x3FF) << 20) | (v9 >> 20) & 0x3FF) >= 0x300000 || GLEW_EXT_texture_array)
    {
        g_gfxCaps.field_E = 1;
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, v15);
        g_gfxCaps.field_90 = v15[0];
        v9 = g_glVersion;
    }
    v11 = v9 & 0xFFC00 | ((v9 & 0x3FF) << 20) | (v9 >> 20) & 0x3FF;
    if (v11 >= 0x400000 || GLEW_ARB_texture_cube_map_array)
        g_gfxCaps.field_F = 1;
    if (v11 >= 0x300400 || GLEW_ARB_texture_buffer_object || GLEW_EXT_texture_buffer_object)
    {
        g_gfxCaps.field_13 = 1;
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, v15);
        g_gfxCaps.field_94 = v15[0];
        v9 = g_glVersion;
    }
    v12 = v9 & 0xFFC00 | ((v9 & 0x3FF) << 20) | (v9 >> 20) & 0x3FF;
    if (v12 >= 0x400C00 || (g_gfxCaps.field_10 = 0, GLEW_ARB_texture_view))
        g_gfxCaps.field_10 = 1;
    if (v12 >= 0x400800 || (g_gfxCaps.field_12 = 0, GLEW_ARB_texture_storage))
        g_gfxCaps.field_12 = 1;
    if (v12 >= 0x300C00 || GL_ARB_texture_swizzle || GLEW_EXT_texture_swizzle)
    {
        g_gfxCaps.field_11 = 1;
        if (v12 >= 0x400800)
        {
        LABEL_54:
            g_gfxCaps.field_B = 1;
            g_gfxCaps.field_14 = 1;
            glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, v15);
            g_gfxCaps.field_2C = v15[0];
            glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, v15);
            g_gfxCaps.field_2D = v15[0];
            v9 = g_glVersion;
            goto LABEL_55;
        }
    } else
    {
        g_gfxCaps.field_11 = 0;
    }
    if (GLEW_ARB_shader_image_load_store || GLEW_EXT_shader_image_load_store)
        goto LABEL_54;
LABEL_55:
    if ((v9 & 0xFFC00 | ((v9 & 0x3FF) << 20) | (v9 >> 20) & 0x3FF) >= 0x300400 || GLEW_ARB_uniform_buffer_object)
    {
        g_gfxCaps.field_8 = 1;
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, v15);
        g_gfxCaps.field_29 = v15[0];
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, v15);
        g_gfxCaps.field_2A = v15[0];
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, v15);
        g_gfxCaps.field_58 = v15[0];
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, v15);
        g_gfxCaps.field_40 = v15[0];
        v9 = g_glVersion;
    }
    v13 = v9 & 0xFFC00 | ((v9 & 0x3FF) << 20) | (v9 >> 20) & 0x3FF;
    if ((v13 >= 0x400800 || GLEW_ARB_conservative_depth) && (g_gfxCaps.field_C = 1, v13 >= 0x400C00)
        || GLEW_ARB_shader_storage_buffer_object)
    {
        g_gfxCaps.field_9 = 1;
        glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, v15);
        g_gfxCaps.field_26 = v15[0];
        glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, v15);
        g_gfxCaps.field_27 = v15[0];
        glGetInteger64v(37086i64, v15);
        g_gfxCaps.field_60 = *(_QWORD *)v15;
        v9 = g_glVersion;
    }
    if ((v9 & 0xFFC00 | ((v9 & 0x3FF) << 20) | (v9 >> 20) & 0x3FF) >= 0x400C00 || GLEW_ARB_compute_shader)
    {
        g_gfxCaps.field_A = 1;
        glGetIntegeri_v(37310i64, 0i64, v15);
        g_gfxCaps.field_68 = v15[0];
        glGetIntegeri_v(37310i64, 1i64, v15);
        g_gfxCaps.field_6C = v15[0];
        glGetIntegeri_v(37310i64, 2i64, v15);
        g_gfxCaps.field_70 = v15[0];
        glGetIntegeri_v(37311i64, 0i64, v15);
        g_gfxCaps.field_74 = v15[0];
        glGetIntegeri_v(37311i64, 1i64, v15);
        g_gfxCaps.field_78 = v15[0];
        glGetIntegeri_v(37311i64, 2i64, v15);
        g_gfxCaps.field_7C = v15[0];
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, v15);
        g_gfxCaps.field_80 = v15[0];
        glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, v15);
        g_gfxCaps.field_84 = v15[0];
        glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, v15);
        g_gfxCaps.field_22 = v15[0];
        if (g_gfxCaps.field_8)
        {
            glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, v15);
            g_gfxCaps.field_2B = v15[0];
        }
        if (g_gfxCaps.field_9)
        {
            glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, v15);
            g_gfxCaps.field_28 = v15[0];
        }
        if (g_gfxCaps.field_14)
        {
            glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, v15);
            g_gfxCaps.field_2E = v15[0];
        }
}
#endif
    return true;
}