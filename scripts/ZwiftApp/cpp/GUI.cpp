#include "ZwiftApp.h"
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
            g_GiantFontW.RenderWString_u(width * 0.5f, height * 0.5f + 170.0f, text,
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
    Downloader::Instance()->Update();
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
void make_sound(const char *snd) {
    AUDIO_Event(snd, 1, false);
}
void GUIDRAW_LoadAssets(bool testServer) {
    //TODO
}
void GUI_Initialize(/*void (*mkSound)(char const *),*/ bool testServer) {
    //TODO
    GUIDRAW_LoadAssets(testServer);
    //qword_7FF6CA8C8DA0 = 0i64;
    //xmmword_7FF6CA8C8D90 = 0i64;
    //GUI_Obj::SetUIText(g_GUIRoot, "ROOT");
    //dword_7FF6CA8C8D7C = 0;
}
void GUI_INTERNAL_PlaySound(const char *snd) { make_sound(snd); }
bool GUI_Key(int a1, int a2) {
    //TODO
    return true;
}
void GUI_CreateTwoButtonsDialog(const char *, const char *, const char *, const char *, const char *, float, std::function<void(UI_TwoButtonsDialog::DIALOG_RESULTS)>, float, float, bool, float, bool) {
    //TODO
}
void GUI_CreateTwoButtonsDialog(const char *, const char *, const char *, const char *, const char *, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS), float, float, bool, float, bool) {
    //TODO
}
void GUI_CreateTwoButtonsDialog(const char *, const char *, const char *, int, const char *, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS), float, float, bool, float, bool) {
    //TODO
}
void GUI_CreateTwoButtonsDialog(float, float, const char *, const char *, const char *, const char *, float, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS)) {
    //TODO
}
void GUI_CreateTwoButtonsDialogWithHyperlink(const char *, void (*)(void *), const char *, const char *, const char *, const char *, const char *, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS), float, float, bool, float, bool) {
    //TODO
}
void kickCallback(UI_TwoButtonsDialog::DIALOG_RESULTS dr) {
    //TODO
}
void GUI_EditBox::SetText(char const *) { /*TODO*/ }
void GUI_EditBox::SetText(const std::string &) { /*TODO*/ }
void GUI_EditBox::SetText(const UChar *) { /*TODO*/ }
void GUI_EditBox::SetTextBuffer(const UChar *) { /*TODO*/ }
void GUI_EditBox::SetCaretPosition(int, int) { /*TODO*/ }
void GUI_TakeFocus(GUI_Obj *) {
    //TODO
}
void UI_FriendsList::FanView(BikeEntity *be, bool) {
    //TODO
}
void UI_GroupEventChat::AddChatMessage(const UChar *, const UChar *, int64_t, bool) {
    //TODO
}
void HandleEventAudioCB(uint32_t id, uint32_t playId, int state) {
    float vol = 50.0f;
    switch (state) {
    case 0:
        g_pAudioController.reset(new UI_AudioControl(playId));
        vol = fminf(g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\AMBIENT_VOL", 1.0f, true) * 100.0f, vol);
        break;
    case 3:
        g_pAudioController.reset();
        vol = g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\AMBIENT_VOL", 1.0f, true) * 100.0f;
        break;
    default:
        return;
    }
    AUDIO_SetVariable("ambient_volume", vol);
}
