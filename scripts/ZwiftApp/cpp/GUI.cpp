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
void ZWIFT_SetupUIViewport() {
    if (GFX_GetWideAspectAwareUI()) {
        auto l = (int)GFX_UI_GetLeftEdgePad();
        auto r = (int)GFX_UI_GetRightEdgePad();
        glViewport(l, 0, g_WIN32_WindowWidth - l - r, g_WIN32_WindowHeight);
    } else {
        glViewport((int)g_UI_WindowOffsetX, (int)g_UI_WindowOffsetY, (int)g_UI_WindowWidth, (int)g_UI_WindowHeight);
    }
}
void ZWIFT_SetupDeviceViewport() {
    glViewport(0, 0, g_WIN32_WindowWidth, g_WIN32_WindowHeight);
}
void GFX_PopUIScissor() {
    //TODO
}
void GFX_PushUIScissor() {
    //TODO
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
int g_bgTex = -1;
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
GUI_Obj *g_GUI_CurrentFocused;
GUI_Obj g_GUIRoot;
bool GUI_Paste() {
    if (!OpenClipboard(nullptr))
        return false;
    auto ClipboardData = GetClipboardData(CF_UNICODETEXT);
    SIZE_T sz;
    wchar_t *txt;
    if (!ClipboardData || (sz = GlobalSize(ClipboardData), sz - 1 > 0xFF) || (txt = (wchar_t *)GlobalLock(ClipboardData)) == nullptr) {
        CloseClipboard();
        return false;
    }
    for (int i = 0; i < sz; ++txt, ++i) {
        auto wch = *txt;
        if (!wch)
            break;
        if (wch == GLFW_KEY_TAB || wch < L' ')
            wch = GLFW_KEY_SPACE;
        GUI_Key(wch, *txt != GLFW_KEY_TAB ? 0x1000 : 0);
    }
    CloseClipboard();
    return true;
}
bool GUI_Key(int key, int mods) {
    if (key == GLFW_KEY_TAB) {
        auto i = &g_GUIRoot;
        if ((mods & GLFW_MOD_SHIFT) == 0) {
            if (g_GUI_CurrentFocused) {
                for (i = g_GUI_CurrentFocused->m_next; i; i = i->m_next) {
                    if (i->m_wantKeys && !i->m_disabled && i->m_visible) {
                        GUI_TakeFocus(i);
                        return true;
                    }
                }
            } else {
                if (i->m_wantKeys && !i->m_disabled && i->m_visible) {
                    GUI_TakeFocus(i);
                    return true;
                }
            }
            i = &g_GUIRoot;
            while (i) {
                if (i->m_wantKeys && !i->m_disabled && i->m_visible) {
                    GUI_TakeFocus(i);
                    return true;
                }
                i = i->m_next;
            }
        } else {
            if (g_GUI_CurrentFocused) {
                for (i = g_GUI_CurrentFocused->m_prev; i; i = i->m_prev) {
                    if (i->m_wantKeys && !i->m_disabled && i->m_visible) {
                        GUI_TakeFocus(i);
                        return true;
                    }
                }
            } else {
                if (i->m_wantKeys && !i->m_disabled && i->m_visible) {
                    GUI_TakeFocus(i);
                    return true;
                }
            }
        }
    } else if ((mods & GLFW_MOD_CONTROL) != 0 && ('v' == key || 'V' == key)) {
        return GUI_Paste();
    }
    if (g_GUI_CurrentFocused && g_GUI_CurrentFocused->OnKey(key, mods))
        return true;
    auto v7 = &g_GUIRoot;
    auto v3 = v7;
    do {
        if (v3->m_field_74)
            v7 = v3;
        v3 = v3->m_next;
    } while (v3);
    while (v7) {
        if (v7->OnKey(key, mods))
            return true;
        v7 = v7->m_next;
    }
    return false;
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
void GUI_MouseWheel(int) {
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
void GUI_Obj::SetFlag(GUI_Obj::Flag flag, bool val) {
    auto msk = 1 << flag;
    if (val)
        m_flag |= msk;
    else
        m_flag &= (~msk);
}
void GUI_Obj::SetFont(CFont2D *fnt) {
    m_font = fnt;
}
void GUI_Obj::Destroy() {
    GUI_RemoveDescendants(this);
    GUI_RemoveObject(this);
}
void GUI_Obj::SetBorderStyle(GUI_BorderStyle bst) {
    m_borderStyle = bst;
}
float GUI_Obj::GetCustomLength(bool ish) {
    if (ish)
        return m_pos.m_height;
    else
        return m_pos.m_width;
}
void GUI_Obj::SetUIWText(const UChar *txt) {
    if (txt)
        m_txt.assign((const wchar_t *)txt);
   m_hasText = true;
}
void GUI_Obj::SetUIText(const char *txt) {
    if (txt)
        SetUIWText(ToUTF8_ib(txt));
}
void GUI_Obj::SetWHCentered(float w, float h) {
    m_pos = { 640.0f - w * 0.5f, 640.0f / VRAM_GetUIAspectRatio() - h * 0.5f, w, h };
}
void GUI_Obj::SetDisabled(bool d) {
    m_disabled = d;
    m_visible = !d;
}
void GUI_Obj::SetHeight(float h) {
    m_pos.m_height = h;
}
void GUI_Obj::SetWidth(float w) {
    m_pos.m_width = w;
}
void GUI_Obj::SetXVisualOffset(float xvo) {
    m_xvo = xvo;
}
void GUI_Obj::SetYVisualOffset(float yvo) {
    m_yvo = yvo;
}
void GUI_Obj::SetX(float x) {
    m_pos.m_left = x;
}
void GUI_Obj::SetY(float y) {
    m_pos.m_top = y;
}
void GUI_Obj::SetupScissoring(bool par) {
    if (m_pos.m_width != 0.0f && m_pos.m_height != 0.0f && !par) {
        GFX_SetScissorTestEnable(true);
        auto p2 = m_padding + m_padding;
        GFX_SetUIScissor(GetX() - m_padding, GetY() - m_padding, p2 + m_pos.m_width, p2 + m_pos.m_height, false);
    } else if (m_parent && m_parent->m_pos.m_width != 0.0f && m_parent->m_pos.m_height != 0.0f) {
        GFX_SetScissorTestEnable(true);
        auto p2 = m_padding + m_padding;
        GFX_SetUIScissor(m_parent->GetX() - m_padding, m_parent->GetY() - m_padding, p2 + m_parent->m_pos.m_width, p2 + m_parent->m_pos.m_height, false);
    } else
        GFX_SetScissorTestEnable(false);
}
GUI_Obj::GUI_Obj() {
    m_mouseOverSid = g_GlobalMouseOverSID;
    m_toggleOnSid = g_GlobalToggleOnSID;
    m_toggleOffSid = g_GlobalToggleOffSID;
    m_selectSid = g_GlobalSelectSID;
    m_font = g_pGUI_GlobalFont;
    /*TODO this->field_98 = -1;
    this->field_9C = -1;*/
}
GUI_Obj::~GUI_Obj() {
    /*TODO v2 = this->m_wtext5;
    if (v2)
    {
        free(v2);
        this->m_wtext5 = 0i64;
    }
    v3 = this->m_wtext2;
    if (v3)
    {
        free(v3);
        this->m_wtext2 = 0i64;
    }*/
    GUI_RemoveDescendants(this);
    GUI_RemoveObject(this);
}
float GUI_Obj::GetX() {
    if (m_parent)
        return m_parent->GetX() + m_pos.m_left;
    else
        return m_pos.m_left;
}
float GUI_Obj::GetY() {
    if (m_parent)
        return m_parent->GetY() + m_pos.m_top;
    else
        return m_pos.m_top;
}
bool GUI_Obj::IsVisible() {
    if (m_visible) 
        if (!m_field_70 || !m_parent || m_parent->IsVisible())
            return true;
    return false;
}
bool GUI_Obj::IsDisabled() {
    if (!m_field_70 || !m_parent || m_disabled)
        return m_disabled;
    return m_parent->IsDisabled();
}
void GUI_Obj::SetupScissoringRespectingParent() {
    GFX_SetScissorTestEnable(true);
    if (m_parent) {
        auto v3 = GetHeirarchyScissorRect();
        GFX_SetUIScissor(v3.m_left, v3.m_top, fmaxf(0.0f, v3.m_width - v3.m_left), fmaxf(0.0f, v3.m_height - v3.m_top), false);
        return;
    }
    SetupScissoring(false);
}
bool GUI_Obj::HitTest(float x, float y) {
    if (m_disabled)
        return false;
    auto myx = GetX(), myy = GetY();
    if (m_field_76 || (m_field_75 && m_parent))
        return IsVisibleWithScissoring(x, y, false);
    return x > myx && (myx + m_pos.m_width) > x && y > myy && (myy + m_pos.m_height) > y;
}
bool GUI_Obj::IsVisibleWithScissoring(float x, float y, bool center) {
    if (!m_field_76 && (!m_field_75 || !m_parent))
        return true;
    auto mx = GetX(), my = GetY();
    auto r = GetHeirarchyScissorRect();
    if (r.m_left <= mx)
        r.m_left = mx;
    if (r.m_top <= my)
        r.m_top = my;
    if (m_pos.m_width + mx <= r.m_width)
        r.m_width = m_pos.m_width + mx;
    if (m_pos.m_height + my <= r.m_height)
        r.m_height = m_pos.m_height + my;
    if (center) {
        y += m_pos.m_height * 0.5f;
        x += m_pos.m_width * 0.5f;
    }
    return x > r.m_left && r.m_width > x && y > r.m_top && r.m_height > y;
}
void GUI_Obj::SetVisible(bool vis) {
    m_visible = vis;
}
RECT2 GUI_Obj::GetHeirarchyScissorRect() {
    RECT2 ret;
    auto y = GetY(), x = GetX(), p2 = m_padding + m_padding;
    if (m_parent) {
        auto r = m_parent->GetHeirarchyScissorRect();
        ret.m_top = fmaxf(y - m_padding - m_bordTop, r.m_top);
        ret.m_left = fmaxf(x - m_padding - m_bordLeft, r.m_left);
        ret.m_height = fminf(y + m_pos.m_height + p2 + m_bordTop + m_bordBottom, r.m_height);
        ret.m_width = fminf(x + m_pos.m_width + p2 + m_bordLeft + m_bordRight, r.m_width);
    } else {
        ret.m_left = x - m_padding - m_bordLeft;
        ret.m_top = y - m_padding - m_bordTop;
        ret.m_height = p2 + m_pos.m_height + m_bordTop + m_bordBottom + ret.m_top;
        ret.m_width = p2 + m_pos.m_width + m_bordLeft + m_bordRight + ret.m_left;
    }
    return ret;
}
void GUI_Obj::DrawDarkenedBackground() {
    GFX_PushUIScissor();
    GFX_SetScissorTestEnable(false);
    auto v0 = g_OrthoScalarH, v1 = g_OrthoScalarW;
    g_OrthoScalarH = 1.0f;
    g_OrthoScalarW = 1.0f;
    GFX_ActivateTexture(g_bgTex, -1, nullptr, TWM_REPEAT);
    GFX_SetTextureWrap(0, TWM_REPEAT, TWM_REPEAT);
    ZWIFT_SetupDeviceViewport();
    GFX_Draw2DQuad_720p(0.0f, 0.0f, 1280.0f, (float)VRAM_GetCurrentRT()->m_dw_height, 0.0f, 0.0f, 30.0f, 16.875f, -1, 0.0f, -1, 0);
    ZWIFT_SetupUIViewport();
    g_OrthoScalarH = v0;
    g_OrthoScalarW = v1;
    GFX_PopUIScissor();
}
void GUI_RemoveObject(GUI_Obj *) {
    //TODO
}
void GUI_RemoveDescendants(GUI_Obj *) {
    //TODO
}
void *GUI_GetTopmostDialog() {
    //TODO
    return nullptr;
}
GUI_Obj *GUI_CreateMessageBox(const char *capt, const char *msg, const char *b1, const char *b2, std::function<void(MessageBoxResults)> f, float w, float h, bool, float) {
    //TODO
    return nullptr;
}
GUI_Obj *GUI_BasicContainer::FindByID(const char *id) {
    //TODO
    return nullptr;
}
void UI_Refactor::temp::PairingListDialogConfirmFromNewHomeScreen(uint32_t) {
    //TODO
}
void OnFinishAreaText(UI_SendAreaTextDialog::DIALOG_RESULTS) {
    //TODO
}
void OnApplyPromoCode(UI_ApplyPromoCode::DIALOG_RESULTS) {
    //TODO
}
void CustomizationDialogConfirm() {
    //TODO
}
bool SelectBranch(uint32_t, bool, bool, bool, bool) {
    //TODO
    return false;
}
void GUI_MouseClick(int, int, float, float, float, float, int *) {
    //TODO
}
void GUI_MouseDoubleClick() {
    //TODO
}
