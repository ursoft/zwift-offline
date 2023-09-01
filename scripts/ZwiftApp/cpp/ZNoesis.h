#pragma once
namespace NoesisLib {
struct NoesisGUI {
    void OnResize(int w, int h);
/*  void AddModalView(Noesis::UIElement *, Noesis::Visibility);
    void AddView(Noesis::UIElement *,char const*);
    void Exit(void);
    void FindNodeByName(std::string);
    void GetActiveChildCount(void);
    void GetMainWindowName(void);
    void GetViewIndex(Noesis::UIElement *);
    void HasActiveChild(void);
    void Init(char const*,int,int,Noesis::Ptr<Noesis::RenderDevice>,Noesis::Ptr<NoesisLib::LocalTextureProvider>);
    void IsActiveElement(Noesis::UIElement const*);
    void NoesisGUI(void);
    void OffscreenRender(void);
    void OnChar(uint);
    void OnCursorMove(double,double);
    void OnFocus(void);
    void OnKeyDown(int);
    void OnKeyUp(int);
    void OnMouseButtonDown(int,double,double);
    void OnMouseButtonUp(int,double,double);
    void OnMouseWheel(double,double,int,int);
    void OnTouchDown(double,double);
    void OnTouchMove(double,double);
    void OnTouchUp(double,double);
    void OnscreenRender(void);
    void RemoveModalView(uint);
    void RemoveView(uint);
    void RestoreKeyboardFocus(void);
    void RouteKeyEventToTopMostView(Noesis::Key,Noesis::RoutedEvent const*,Noesis::KeyStates);
    void SetFocus(std::string);
    void SetFocusToNode(Noesis::FrameworkElement *,bool);
    void SetGestureNavigation(float,ulong);
    void StoreAndClearKeyboardFocus(void);
    void StoreBackupKeyBoardFocus(std::string const&);
    void StoreBackupKeyboardFocus(void);
    void Update(double);
    void ~NoesisGUI();*/
};
}
inline std::weak_ptr<NoesisLib::NoesisGUI> g_pNoesisGUI;
