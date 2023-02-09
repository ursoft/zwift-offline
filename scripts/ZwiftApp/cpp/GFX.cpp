#include "ZwiftApp.h"
void GFX_SetLoadedAssetMode(bool mode) {
    g_CurrentAssetCategory = mode ? AC_1 : AC_2;
}
bool GFX_Initialize() {
    //TODO
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
#if 0 //TODO
    v366 = 0;
    v368 = 0;
    v369 = 0;
    v373 = 0;
    LODWORD(v376._Myend) = 0;
    *&v364 = WINWIDTH;
    v365 = WINHEIGHT;
    v367 = g_bFullScreen;
    v374 = PREFERRED_MONITOR;
    v375 = 0i64;
    v376._Myfirst = 0i64;
    v376._Mylast = qword_7FF71A893CD0;
    v370 = VSYNC;
    v371 = GPU;
    v372 = GFX_TIER;
    WORD2(v376._Myend) = 266;
    if (!GFX_Initialize(&v364))
    {
        Log("Failed initializing graphics", v121, v122, v123);
        v304 = GetText("LOC_ERROR_NO_GRAPHICS");
        MsgBoxAndExit(v304);
    }
    RendererName = GFX_GetRendererName();
    v396._Bx._Ptr = 0i64;
    v396._Mysize = 0i64;
    v396._Myres = 15i64;
    v125 = -1i64;
    do
        ++v125;
    while (RendererName[v125]);
    string_assign(&v396, RendererName, v125);
    VendorName = GFX_GetVendorName();
    path._Bx._Ptr = 0i64;
    path._Mysize = 0i64;
    path._Myres = 15i64;
    v127 = -1i64;
    do
        ++v127;
    while (VendorName[v127]);
    string_assign(&path, VendorName, v127);
    coa1.vptr = 0i64;
    v387 = 0i64;
    v388 = 0i64;
    v128 = &v396;
    v313 = v396._Myres >= 0x10ui64;
    v129 = v396._Bx._Ptr;
    if (v396._Myres >= 0x10ui64)
        v128 = v396._Bx._Ptr;
    Mysize = v396._Mysize;
    if (v396._Mysize >= 0x10ui64)
    {
        v131 = v396._Mysize | 0xF;
        argv_ = (v396._Mysize | 0xF);
        if ((v396._Mysize | 0xFui64) > 0x7FFFFFFFFFFFFFFFi64)
            v131 = 0x7FFFFFFFFFFFFFFFi64;
        coa1.vptr = stl_allocator_new(v131 + 1);
        memmove(coa1.vptr, v128, Mysize + 1);
    } else
    {
        coa1 = v128->_Bx;
        v131 = 15i64;
    }
    v132 = Mysize;
    v388 = v131;
    v387 = Mysize;
    Str[0] = 0i64;
    v398 = 0i64;
    v399 = 0i64;
    p_path = &path;
    Myres = path._Myres;
    v312 = path._Myres >= 0x10ui64;
    v135 = path._Bx._Ptr;
    if (path._Myres >= 0x10ui64)
        p_path = path._Bx._Ptr;
    v136 = path._Mysize;
    if (path._Mysize >= 0x10ui64)
    {
        v137 = path._Mysize | 0xF;
        argv_ = (path._Mysize | 0xF);
        if ((path._Mysize | 0xFui64) > 0x7FFFFFFFFFFFFFFFi64)
            v137 = 0x7FFFFFFFFFFFFFFFi64;
        Str[0] = stl_allocator_new(v137 + 1);
        memmove(Str[0], p_path, v136 + 1);
        v399 = v137;
        v131 = v388;
        v132 = v387;
    } else
    {
        *Str = p_path->_Bx;
        v399 = 15i64;
    }
    v398 = v136;
    p_coa1 = &coa1;
    if (v131 < 0x10)
    {
        v140 = &coa1 + v132;
        v139 = &coa1;
    } else
    {
        v139 = coa1.vptr;
        v140 = coa1.vptr + v132;
        p_coa1 = coa1.vptr;
    }
    std::foreach(&argv_, v139, v140, p_coa1, toupper);
    v141 = Str;
    if (v399 < 0x10)
    {
        v143 = Str + v398;
        v142 = Str;
    } else
    {
        v142 = Str[0];
        v143 = &Str[0][v398];
        v141 = Str[0];
    }
    std::foreach(&argv_, v142, v143, v141, toupper);
    if (Mysize)
    {
        v144 = &coa1;
        if (v388 >= 0x10)
            v144 = coa1.vptr;
        if (strstr(v144, "HD GRAPHICS 2"))
            goto LABEL_220;
        v145 = &coa1;
        if (v388 >= 0x10)
            v145 = coa1.vptr;
        if (strstr(v145, "HD GRAPHICS 3"))
        {
        LABEL_220:
            if (v398)
            {
                v146 = Str;
                if (v399 >= 0x10)
                    v146 = Str[0];
                if (strstr(v146, "INTEL"))
                {
                    v305 = sub_7FF71917CB40(&argv_, &path, word_7FF71A2A1168);
                    sub_7FF71917CBB0(&coa2, v305, &v396);
                    j_vector_dtr(&argv_);
                    v306 = sub_7FF7190E67D0(&coa2);
                    v307 = GetText("LOC_UPDATE_GFX_MESSAGE");
                    sprintf_s(Text, 0x400ui64, v307, v306);
                    v308 = sub_7FF71999156C(Text);
                    v309 = GetText_0("LOC_UPDATE_GFX_TITLE");
                    MessageBoxW(0i64, v308, v309, 0);
                    ZwiftExit(-1);
                }
            }
        }
    }
    if (v399 >= 0x10)
    {
        v147 = Str[0];
        if (v399 + 1 >= 0x1000)
        {
            v147 = *(Str[0] - 1);
            if ((Str[0] - v147 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v147);
    }
    v398 = 0i64;
    v399 = 15i64;
    LOBYTE(Str[0]) = 0;
    if (v388 >= 0x10)
    {
        v148 = coa1.vptr;
        if (v388 + 1 >= 0x1000)
        {
            v148 = *(coa1.vptr - 1);
            if ((coa1.vptr - v148 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v148);
    }
    v387 = 0i64;
    v388 = 15i64;
    LOBYTE(coa1.vptr) = 0;
    if (v312)
    {
        v149 = v135;
        if ((Myres + 1) >= 0x1000)
        {
            v135 = *(v135 - 1);
            if ((v149 - v135 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v135);
    }
    path._Mysize = 0i64;
    path._Myres = 15i64;
    path._Bx._Buf[0] = 0;
    if (v313)
    {
        v150 = v129;
        if ((v396._Myres + 1) >= 0x1000)
        {
            v129 = *(v129 - 1);
            if ((v150 - v129 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v129);
    }
    v396._Mysize = 0i64;
    v396._Myres = 15i64;
    v396._Bx._Buf[0] = 0;
    glfwGetWindowSize(g_mainWindow, &WIN32_WindowWidth, &WIN32_WindowHeight);
    g_width = WIN32_WindowWidth;
    v152 = WIN32_WindowHeight;
    g_height = WIN32_WindowHeight;
    if (!WIN32_WindowWidth || !WIN32_WindowHeight)
    {
        if (IsDebuggerPresent_1())
            __debugbreak();
        if (ZwiftAssert::BeforeAbort(
            "WIN32_WindowWidth != 0 && WIN32_WindowHeight != 0",
            "D:\\git\\zwift-game-client\\Projects\\ZwiftApp\\CODE\\ZwiftApp.cpp",
            0x494Au,
            0))
        {
            ZwiftAssert::Abort();
        }
        v152 = g_height;
    }
    v153 = v152;
    v154 = g_width;
    if ((g_width / v152) <= 1.78)
    {
        v155 = 0.0;
    } else
    {
        v153 = v152;
        v154 = VRAM_GetUIAspectRatio() * v152;
        v155 = (g_width - v154) * 0.5;
    }
    g_view_w = v154;
    g_view_h = v153;
    g_view_x = v155;
    g_view_y = 0.0;
    v156 = 0i64;
    do
    {
        v157 = *(&aZwiftConfig + v156);
        *(&coa1.vptr + v156++) = v157;
    } while (v157);                               // end of GFX_Initialize
#endif
    auto perf_flags = g_UserConfigDoc.GetU32("PERF", -1, true);
    if (perf_flags != -1)
        GFX_AddPerformanceFlags(perf_flags);
    return true;
}
bool GFXAPI_Initialize(const GFX_InitializeParams &) {
    //TODO
    return true;
}
bool GFX_Initialize(const GFX_InitializeParams &) {
    //TODO
    return true;
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
int64_t GFX_GetPerformanceFlags() {
    return g_GFX_PerformanceFlags;
}
void GFX_AddPerformanceFlags(uint64_t f) {
    g_GFX_PerformanceFlags |= f;
}
