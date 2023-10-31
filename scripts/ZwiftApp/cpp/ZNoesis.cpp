#include "ZwiftApp.h"

namespace NoesisLib {
NoesisGUI::NoesisGUI() {
    //TODO
}
void NoesisGUI::OnResize(int w, int h) {
    //TODO
}
bool NoesisGUI::OnMouseButtonUp(const VEC2 &pos) {
    //LATER
    return false;
}
void NoesisGUI::OnCursorMove(double x, double y) {
    //LATER
}
bool NoesisGUI::HasActiveChild() {
    //LATER
    return false;
}
bool NoesisGUI::OnMouseButtonDown(const VEC2 &pos) {
    /*LATER auto name = m_app->GetMainWindow()->FindName(GetMainWindowName());
    Noesis::BaseFreezableCollection behs;// = NoesisApp::Interaction::GetBehaviors(name);
    auto cnt = behs.Count();
    for(auto v15 = 0u; v15 < cnt; ++v15) {
        auto c = behs.GetComponent(v15);
        c->Release();
        auto cc = dynamic_cast<UI_Refactor::KeyboardNavigationBehavior> c;
        if (cc)
            v18[240] = 1;
    }
    auto x = pos.m_data[0];
    auto y = pos.m_data[1];
    double _x, _y;
    if (x >= 0.0f)
        _x = x + 0.5;
    else
        _x = x - 0.5;
    if (y >= 0.0f)
        _y = y + 0.5;
    else
        _y = y - 0.5;
    return ((*(unsigned __int8(__fastcall **)(void *, _QWORD, _QWORD, _QWORD))(*(_QWORD *)Ptr[2].vptr + 168i64))(
        Ptr[2].vptr,
        (unsigned int)(int)v20,
        (unsigned int)(int)v19,
        0i64))
    */
    return true;
}
}