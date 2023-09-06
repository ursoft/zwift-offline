#pragma once
inline CFont2D *g_pGUI_GlobalFont;
inline void GUI_SetDefaultFont(CFont2D *v) { g_pGUI_GlobalFont = v; }
bool GUI_Key(int key, int mod);
struct GUI_DragSource;
struct GUI_Obj;
struct GUI_DragTarget {
    virtual void OnDrop(GUI_DragSource *) {}
    virtual void OnHover(GUI_DragSource *) {}
};
struct GUI_Button;
struct I_GUINavigationGraphDelegate;
struct GUINavigationGraph {
    //TODO
    enum LinkType { LT_0 };
    struct Node {
        Node(GUI_Button *);
    };
    void AddHorizontal(GUI_Button *, GUI_Button *);
    void AddLink(GUINavigationGraph::LinkType, GUI_Button *, GUI_Button *);
    void AddNode(GUI_Button *);
    void AddVertical(GUI_Button *, GUI_Button *);
    void FindNode(GUI_Button *);
    GUINavigationGraph(I_GUINavigationGraphDelegate *);
    GUINavigationGraph();
    void HandleClick();
    void HandleNavigation(GUINavigationGraph::LinkType);
    void OnKey(int, int);
    void SetSelected(GUI_Button *, bool);
    ~GUINavigationGraph();
};
struct I_GUINavigationGraphDelegate {
    virtual void Selected(GUI_Button *) = 0;
    virtual void Unselected(GUI_Button *) = 0;
    virtual void Clicked(GUI_Button *) = 0;
    virtual void Invalid(GUI_Button *, GUINavigationGraph::LinkType) = 0;
};
struct I_GUI_BreadCrumbsDelegate {
    virtual void ToState(uint32_t) = 0;
};
struct I_GUI_GridsDelegate {
    enum NavigationDirection { ND_0 };
    virtual void GridsInFocus(GUI_Obj *) = 0;
    virtual void GridsOutFocus(GUI_Obj *) = 0;
    virtual void InvalidNavigation(GUI_Obj *, I_GUI_GridsDelegate::NavigationDirection, void *) = 0;
};
struct I_GUI_ScrollBarDelegate {
    virtual void UpbuttonClicked() = 0;
    virtual void DnButtonClicked() = 0;
    virtual void ScrollTo(float, bool) = 0;
};
struct I_GUI_EditBoxDelegate {
    virtual void KeyInput(const uint16_t *, GUI_Obj *) = 0;
    virtual void KeyDelete(const uint16_t *, GUI_Obj *) = 0;
    virtual void EditDone(const uint16_t *, GUI_Obj *) = 0;
};
struct I_GUI_ListBoxDelegate {
    virtual void ListBoxInFocus(GUI_Obj *) = 0;
    virtual void Selected(GUI_Obj *, uint32_t, void *) = 0;
    virtual void MultiSelected(GUI_Obj *, const std::vector<int> &, void *) = 0;
    virtual void Hover(GUI_Obj *, uint32_t, void *) = 0;
    virtual void ListBoxOutFocus(GUI_Obj *) = 0;
    virtual void ListItemDoubleClicked(int, void *, GUI_Obj *) = 0;
    virtual void ListItemSwiped(GUI_Obj *, int) = 0;
    virtual void ListItemFinishedSwipe(GUI_Obj *, int) = 0;
    virtual void ListBoxEscaped(GUI_Obj *) = 0;
};
struct I_GUI_SliderDelegate {
    virtual void Release(GUI_Obj *, float) = 0;
    virtual void SliderClick(GUI_Obj *) = 0;
    virtual void SliderInFocus(GUI_Obj *) = 0;
};
struct GUI_CheckBox;
struct I_GUI_CheckBoxDelegate {
    virtual void CheckBoxInFocus(GUI_Obj *) = 0;
    virtual void Checked(GUI_CheckBox *, bool, void *) = 0;
};
struct intUINotificationHandler {
    virtual bool IsFinished() = 0;
    virtual void ForceClose() = 0;
};
enum GUI_BorderStyle { BS_0 };
struct GUI_Obj { //0xF0 bytes
    enum Flag { F };
    virtual void Destroy();
    static void DrawDarkenedBackground();
    const char *m_mouseOverSid, *m_toggleOnSid, *m_toggleOffSid, *m_selectSid;
    GUI_Obj();
    GUI_BorderStyle m_borderStyle = BS_0;
    int m_field_C8 = 0, m_field_D8 = 0;
    virtual GUI_BorderStyle GetBorderStyle() { return m_borderStyle; }
    virtual float GetCustomLength(bool ish);
    virtual RECT2 GetHeirarchyScissorRect();
    //void GetUIText();
    //void GetUITextStringWidth();
    //void GetUIWTextStringWidth();
    virtual float GetX();
    virtual float GetY();
    virtual bool HitTest(float, float);
    virtual void InFocus() {}
    virtual bool IsDisabled();
    virtual bool IsVisible();
    virtual bool IsVisibleWithScissoring(float, float, bool);
    virtual bool OnDoubleSwipe(int, VEC2, VEC2, float, float) { return false; } //[9]
    //not found virtual void OnEscape();
    virtual bool OnExecuteAction() { return false; } //[10]
    virtual bool OnKey(int, int) { return false; } //[11]
    virtual bool OnMouseDoubleClick(float, float) { return false; } //[6]
    virtual bool OnMouseDown(float, float) { return false; } //[4]
    virtual void OnMouseMove(float, float) {} //[3]
    virtual bool OnMouseUp(float, float) { return false; } //[5]
    virtual bool OnMouseWheel(int, float, float) { return false; } //[7]
    virtual bool OnSwipe(int, VEC2, VEC2, float, float) { return false; } //[8]
    virtual void OutFocus() {}
    virtual void PostRender() {} //[14]
    virtual void PreRender(float) {} //[12]
    virtual void Render() {} //[13]
    virtual void SetBorderStyle(GUI_BorderStyle);
    virtual void SetDisabled(bool);
    virtual void SetFlag(GUI_Obj::Flag, bool); //[0]
    CFont2D *m_font;
    virtual void SetFont(CFont2D *); //[2]
    virtual void SetHeight(float);
    GUI_Obj *m_parent = nullptr;
    virtual void SetParent(GUI_Obj *par) { m_parent = par; }
    std::wstring m_txt;
    int m_flag = 0;
    bool m_disabled = false, m_hasText = false, m_field_70 = true, m_field_71 = false, m_visible = true, m_field_74 = false, m_field_75 = false, m_field_76 = false;
    virtual void SetUIText(const char *txt);
    virtual void SetUIWText(const UChar *txt);
    virtual void SetVisible(bool);
    RECT2 m_pos{};
    virtual void SetWHCentered(float w, float h);
    virtual void SetWidth(float);
    virtual void SetX(float);
    float m_xvo = 0.0f, m_yvo = 0.0f, m_padding = 10.0f, m_bordTop = 0.0f, m_bordBottom = 0.0f, m_bordLeft = 0.0f, m_bordRight = 0.0f;
    virtual void SetXVisualOffset(float);
    virtual void SetY(float);
    virtual void SetYVisualOffset(float);
    virtual void SetupScissoring(bool);
    virtual void SetupScissoringRespectingParent();
    virtual void Update(float) {} //[15]
    virtual ~GUI_Obj(); //[1]
};
struct WorkoutCanvas : public GUI_DragTarget, public GUI_Obj, public I_GUI_EditBoxDelegate, public I_GUI_ListBoxDelegate {
    //TODO
};
struct GUIOBJ_TrainingPlanViewer : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
    struct CallbackListener {
        //TODO
    };
};
struct GUI_TimePicker : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct GUI_TreeListElement : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct GUI_AccessoryWidget : public GUI_Obj {
    //TODO
};
struct GUI_StylesheetConsumer : public GUI_Obj {
    //TODO
};
struct GUI_BasicContainer : public GUI_StylesheetConsumer {
    //TODO
    GUI_Obj *FindByID(const char *id);
};
struct GUI_AlignmentObject : public GUI_BasicContainer {
    //TODO
};
struct GUI_Button : public GUI_Obj {
    //TODO
};
struct GUI_Caption : public GUI_StylesheetConsumer {
    //TODO
};
struct GUI_CategoryWidget : public GUI_Obj {
    //TODO
};
struct GUI_CheckBox : public GUI_Obj {
    //TODO
};
struct GUI_ColorSchemeWidget : public GUI_Obj {
    //TODO
};
struct GUI_CustomizationDialogV2 : public GUI_Obj, public I_GUI_GridsDelegate, public I_GUI_SliderDelegate {
    //TODO
};
struct GUI_DragSource : public GUI_Obj {
    //TODO
};
struct GUI_ListBox : public GUI_Obj, public I_GUI_ScrollBarDelegate {
    //TODO
};
struct GUI_DropDownBox : public GUI_ListBox {
    //TODO
};
struct XUX_Object : public GUI_BasicContainer {
    //TODO
};
struct XUXDrivenHUDComponent : public XUX_Object {
    //TODO
};
struct GUI_DropsIntro : public XUX_Object {
    //TODO
};
struct GUI_EditBox : public GUI_Obj, public I_GUI_ScrollBarDelegate {
    void SetText(char const *);
    void SetText(const std::string &);
    void SetText(const UChar *);
    void SetTextBuffer(const UChar *);
    void SetCaretPosition(int, int);
    /*GUI_EditBox::AddChar(int,int,bool &)
GUI_EditBox::CalcCursorPositionByIndex(float)
GUI_EditBox::CalculateCursorIndexFromPosition(float)
GUI_EditBox::CalculateCursorOffsetForAlignment(CFont2D::FontAlignment,ushort const*,uint,float)
GUI_EditBox::ClearLastWord(void)
GUI_EditBox::ClearSelection(void)
GUI_EditBox::DeleteSelection(void)
GUI_EditBox::DnButtonClicked(void)
GUI_EditBox::ExtendSelection(int)
GUI_EditBox::GUI_EditBox(float,float,float,float,uint,GUI_Obj *,I_GUI_EditBoxDelegate *)
GUI_EditBox::GUI_EditBox(float,float,float,ushort const*,CFont2D::FontAlignment,GUI_Obj *,I_GUI_EditBoxDelegate *)
GUI_EditBox::GetCharCountForLine(int)
GUI_EditBox::GetColorFromHex(void)
GUI_EditBox::GetFlag(GUI_EditBox::EDITBox_Flags)
GUI_EditBox::GetLineFromIndex(int)
GUI_EditBox::GetLineLengthFromIndex(int)
GUI_EditBox::GetLineStartIndex(int)
GUI_EditBox::GetScrollbarWidth(void)
GUI_EditBox::GetSelectionEnd(void)
GUI_EditBox::GetSelectionLength(void)
GUI_EditBox::GetSelectionStart(void)
GUI_EditBox::GetText(void)
GUI_EditBox::GetTextHeight(void)
GUI_EditBox::GetTextMB(void)
GUI_EditBox::GetTextScale(ushort const*,uint)
GUI_EditBox::HexByteSwap(uint)
GUI_EditBox::MoveCursorTo(int,bool)
GUI_EditBox::OnButtonPress(void *)
GUI_EditBox::OnKey(int,int)
GUI_EditBox::OnMouseDoubleClick(float,float)
GUI_EditBox::OnMouseDown(float,float)
GUI_EditBox::OnMouseMove(float,float)
GUI_EditBox::OnMouseUp(float,float)
GUI_EditBox::OnMouseUpCalcCusorIndex(void)
GUI_EditBox::OnMouseWheel(int,float,float)
GUI_EditBox::ParseDecimal(char const*,uint *)
GUI_EditBox::ParseDecimal(ushort const*,uint *)
GUI_EditBox::ParseHex(char const*,uint *)
GUI_EditBox::ParseHex(ushort const*,uint *)
GUI_EditBox::Render(void)
GUI_EditBox::RenderSelectionHighlight(int,int,int,ushort const*,int)
GUI_EditBox::ScrollIncremental(float)
GUI_EditBox::ScrollTo(float,bool)
GUI_EditBox::SelectAll(void)
GUI_EditBox::SetDisabled(bool)
GUI_EditBox::SetFlag(GUI_EditBox::EDITBox_Flags,bool)
GUI_EditBox::SetFont(CFont2D *)
GUI_EditBox::SetHintText(char const*)
GUI_EditBox::SetHintText(ushort const*)
GUI_EditBox::SetMaxIntegerValue(uint)
GUI_EditBox::SetParent(GUI_Obj *)
GUI_EditBox::SetRequired(bool,CFont2D *)
GUI_EditBox::SetSliderConstraints(float,float)
GUI_EditBox::SetTitleText(char const*,CFont2D *)
GUI_EditBox::SetTitleText(ushort const*,CFont2D *)
GUI_EditBox::SetToolTip(char const*)
GUI_EditBox::SetValueWithinConstraints(float)
GUI_EditBox::SetVisible(bool)
GUI_EditBox::SetupGenericButton(ushort const*,void (*)(GUI_EditBox*))
GUI_EditBox::ToDecimal(uint)
GUI_EditBox::ToHex(uint)
GUI_EditBox::UpbuttonClicked(void)
GUI_EditBox::Update(float)
GUI_EditBox::UpdateScrollbar(void)
GUI_EditBox::UpdateTextHeight(void)
GUI_EditBox::~GUI_EditBox()
*/
};
struct GUI_Grids : public GUI_Obj, public I_GUI_ScrollBarDelegate {
    //TODO
};
struct GUI_GroupEventChatMessage : public GUI_Obj {
    //TODO
};
struct GUI_Image : public GUI_Obj {
    //TODO
};
struct GUI_ManufactureWidget : public GUI_Obj {
    //TODO
};
struct GUI_MessageBox : public GUI_Obj {
    //TODO
};
struct GUI_NewPling : public GUI_Obj {
    //TODO
};
struct GUI_PurchaseConfirmation : public GUI_Obj {
    //TODO
};
struct GUI_RowContainer : public GUI_BasicContainer {
    //TODO
};
struct GUI_ScreenshotSelectorWidget : public GUI_Obj {
    //TODO
};
struct GUI_ScrollBar : public GUI_Obj {
    //TODO
};
struct GUI_SegmentSplitWidget : public GUI_Obj {
    //TODO
};
struct GUI_Shape : public GUI_StylesheetConsumer {
    //TODO
};
struct GUI_Slider : public GUI_Obj {
    //TODO
};
struct GUI_SportSelectWidget : public GUI_Obj {
    //TODO
};
struct GUI_StreamedImage : public GUI_Image {
    //TODO
};
struct GUI_TabHeader : public GUI_Obj {
    //TODO
};
struct GUI_TimeSplitWidget : public GUI_Obj {
    //TODO
};
struct GUI_TitledContainer : public GUI_BasicContainer {
    //TODO
};
struct GUI_TrainingPlanBlockWidget : public GUI_RowContainer {
    //TODO
};
struct GUI_ZwiftShop : public GUI_Obj, public I_GUI_GridsDelegate, public I_GUI_BreadCrumbsDelegate, public I_GUINavigationGraphDelegate {
    void ToState(uint32_t) override { //I_GUI_BreadCrumbsDelegate
        //TODO
    }
    //TODO
};
struct UI_GroupEventChat : public GUI_Obj, public I_GUI_EditBoxDelegate, public I_GUI_ScrollBarDelegate {
    bool m_field_72 = false;
    static inline UI_GroupEventChat *g_GroupChat;
    void AddChatMessage(const UChar *, const UChar *, int64_t, bool);
    /*::
    UI_GroupEventChat::AddCustomMessage(ushort const *, uint, ushort const *, uint, uint, uint, CFont2D::FontAlignment, int)
    UI_GroupEventChat::AddNotification(ushort const *, bool)
    UI_GroupEventChat::BuildData(void)
    UI_GroupEventChat::ClearHistory(void)
    UI_GroupEventChat::ClearMessages(void)
    UI_GroupEventChat::DnButtonClicked(void)
    UI_GroupEventChat::EditBoxInFocus(GUI_Obj *)
    UI_GroupEventChat::EditDone(char const *, GUI_Obj *)
    UI_GroupEventChat::KeyDelete(char const *, GUI_Obj *)
    UI_GroupEventChat::KeyInput(char const *, GUI_Obj *)
    UI_GroupEventChat::OnKey(int, int)
    UI_GroupEventChat::OnMouseDown(float, float)
    UI_GroupEventChat::OnMouseMove(float, float)
    UI_GroupEventChat::OnMouseUp(float, float)
    UI_GroupEventChat::OnMouseWheel(int, float, float)
    UI_GroupEventChat::OnRulesViewButton(void *)
    UI_GroupEventChat::OnUserJoin(BikeEntity *)
    UI_GroupEventChat::PageDown(void)
    UI_GroupEventChat::PageUp(void)
    UI_GroupEventChat::Render(void)
    UI_GroupEventChat::RenderHeader(int, char const *, char const *, int, float, float, int, int, int, char const *, zwift::protobuf::EventPaceType, UI_GroupEventChat::EventLengthType, uint)
    UI_GroupEventChat::RenderNotificationBar(void)
    UI_GroupEventChat::ScrollTo(float, bool)
    UI_GroupEventChat::SendChat(ushort const *)
    UI_GroupEventChat::UI_GroupEventChat(void)
    UI_GroupEventChat::UpbuttonClicked(void)
    UI_GroupEventChat::Update(float)
    UI_GroupEventChat::UpdateNotificationBar(float)
    UI_GroupEventChat::~UI_GroupEventChat() */ 
};
void GUI_TakeFocus(GUI_Obj *);
void GUI_RemoveObject(GUI_Obj *);
void GUI_RemoveDescendants(GUI_Obj *);
enum MessageBoxResults { MBR_BUTTON0 };
GUI_Obj *GUI_CreateMessageBox(const char *capt, const char *msg, const char *b1, const char *b2, std::function<void(MessageBoxResults)> f, float w, float h, bool, float);
void *GUI_GetTopmostDialog();
/*
GUI_AccessoryWidget::EquipAccessory(void)
GUI_AccessoryWidget::GUI_AccessoryWidget(void)
GUI_AccessoryWidget::GetUnlockLevel(void)
GUI_AccessoryWidget::Init(GUI_CustomizationDialogV2 *,float,float,Accessory const*,Accessory const*)
GUI_AccessoryWidget::Init(GUI_CustomizationDialogV2 *,float,float,BikePresetConfig const*,int)
GUI_AccessoryWidget::Init(GUI_CustomizationDialogV2 *,float,float,int)
GUI_AccessoryWidget::IsAccessable(void)
GUI_AccessoryWidget::IsEntitlementUnlocked(void)
GUI_AccessoryWidget::IsEqual(Accessory const*,Accessory const*,BikePresetConfig const*)
GUI_AccessoryWidget::IsLevelLocked(void)
GUI_AccessoryWidget::IsMissionItem(void)
GUI_AccessoryWidget::IsPriceLocked(void)
GUI_AccessoryWidget::IsViewable(void)
GUI_AccessoryWidget::ParseName(char const*,char const*)
GUI_AccessoryWidget::ParsePrice(int)
GUI_AccessoryWidget::Render(void)
GUI_AccessoryWidget::RenderColor(void)
GUI_AccessoryWidget::RenderIconWithAlpha(bool)
GUI_AccessoryWidget::RenderIconWithoutAlpha(bool)
GUI_AccessoryWidget::RenderLogoBackground(int,float,float,float,float)
GUI_AccessoryWidget::RenderPurchasableComponents(bool,bool)
GUI_AccessoryWidget::RenderPurchasableThumbnailAccessory(float,float)
GUI_AccessoryWidget::RenderPurchasableThumbnailBikeFrame(float,float)
GUI_AccessoryWidget::RenderSelectedAccessoryName(void)
GUI_AccessoryWidget::Update(float)
GUI_AccessoryWidget::sRenderBikeThumbnailWithWheel(bool,BikePresetConfig const*,int,uint,float,float,float,float,float,float,float,float,float)
GUI_AccessoryWidget::~GUI_AccessoryWidget()
GUI_AddObj(GUI_Obj *)
GUI_AlignmentObject::Render(void)
GUI_AlignmentObject::~GUI_AlignmentObject()
GUI_AreMessageBoxesActive(void)
GUI_BasicContainer::ApplyStylesheet(GUI_Stylesheet *,bool)
GUI_BasicContainer::FindByID(char const*)
GUI_BasicContainer::FindByID(uint)
GUI_BasicContainer::GUI_Animation::GUI_Animation(GUI_BasicContainer*,tinyxml2::XMLElement *)
GUI_BasicContainer::GUI_Animation::Track::SetAnimationPosition(GUI_BasicContainer*,float)
GUI_BasicContainer::GUI_Animation::Track::Track(GUI_BasicContainer::GUI_Animation*,tinyxml2::XMLElement *)
GUI_BasicContainer::GUI_BasicContainer(void)
GUI_BasicContainer::OnMouseDown(float,float)
GUI_BasicContainer::OnMouseMove(float,float)
GUI_BasicContainer::OnMouseUp(float,float)
GUI_BasicContainer::PostRender(void)
GUI_BasicContainer::Render(void)
GUI_BasicContainer::Update(float)
GUI_BasicContainer::UpdatedStyle(char const*)
GUI_BasicContainer::UpdatedStyle(uint)
GUI_BasicContainer::~GUI_BasicContainer()
GUI_Button::FlipHorizontally(void)
GUI_Button::FlipVertically(void)
GUI_Button::GUI_Button(float,float,float,float,GUI_Obj *,bool,void (*)(void *),GUI_Obj *)
GUI_Button::GUI_Button(float,float,float,float,char const*,void (*)(void *),GUI_Obj *)
GUI_Button::GUI_Button(float,float,float,float,float *,int,void (*)(void *),GUI_Obj *)
GUI_Button::GUI_Button(float,float,float,float,int,void (*)(void *),GUI_Obj *)
GUI_Button::GUI_Button(float,float,float,float,void (*)(GUI_Button*),void (*)(void *),GUI_Obj *)
GUI_Button::GenericInit(float,float,float,float,void (*)(void *),GUI_Obj *)
GUI_Button::GetButtonFlag(GUI_Button::ButtonFlags)
GUI_Button::GetOrigX(void)
GUI_Button::GetOrigY(void)
GUI_Button::HitTest(float,float)
GUI_Button::InFocus(void)
GUI_Button::OnExecuteAction(void)
GUI_Button::OnKey(int,int)
GUI_Button::OnMouseDown(float,float)
GUI_Button::OnMouseMove(float,float)
GUI_Button::OnMouseUp(float,float)
GUI_Button::OutFocus(void)
GUI_Button::Render(void)
GUI_Button::SetButtonFlag(GUI_Button::ButtonFlags,bool)
GUI_Button::SetCornerScale(float)
GUI_Button::SetDisabled(bool)
GUI_Button::SetHeight(float)
GUI_Button::SetIconHeight(float)
GUI_Button::SetIconWidth(float)
GUI_Button::SetImage(GUI_Obj *,bool)
GUI_Button::SetStyle(buttonStyle)
GUI_Button::SetToolTip(char const*)
GUI_Button::SetWidth(float)
GUI_Button::SetX(float)
GUI_Button::SetY(float)
GUI_Button::Update(float)
GUI_Button::~GUI_Button()
GUI_Caption::ApplyStylesheet(GUI_Stylesheet *,bool)
GUI_Caption::GUI_Caption(char const*,char const*,uint,float,float,float,float,CFont2D *,CFont2D::FontAlignment,GUI_Obj *)
GUI_Caption::GUI_Caption(char const*,uint,float,char const*,CFont2D *,GUI_Obj *,CFont2D::FontAlignment)
GUI_Caption::GUI_Caption(char const*,uint,float,float,float,CFont2D *,char const*,GUI_Obj *,CFont2D::FontAlignment)
GUI_Caption::Render(void)
GUI_Caption::SetTimeValue(double)
GUI_Caption::UpdateWidth(void)
GUI_Caption::~GUI_Caption()
GUI_CategoryWidget::GUI_CategoryWidget(void)
GUI_CategoryWidget::Init(GUI_CustomizationDialogV2 *,float,float,uint,void (*)(void *))
GUI_CategoryWidget::Render(void)
GUI_CategoryWidget::Update(float)
GUI_CategoryWidget::~GUI_CategoryWidget()
GUI_CheckBox::GUI_CheckBox(float,float,float,bool *,char const*,GUI_Obj *,I_GUI_CheckBoxDelegate *,void *)
GUI_CheckBox::GUI_CheckBox(float,float,float,bool,char const*,GUI_Obj *,I_GUI_CheckBoxDelegate *,void *)
GUI_CheckBox::GUI_CheckBox(float,float,float,float,bool *,char const*,GUI_Obj *,I_GUI_CheckBoxDelegate *,void *)
GUI_CheckBox::GUI_CheckBox(float,float,float,float,bool,char const*,GUI_Obj *,I_GUI_CheckBoxDelegate *,void *)
GUI_CheckBox::HitTest(float,float)
GUI_CheckBox::InFocus(void)
GUI_CheckBox::Initialize(float,float,float,bool,char const*,GUI_Obj *,I_GUI_CheckBoxDelegate *,void *)
GUI_CheckBox::OnKey(int,int)
GUI_CheckBox::OnMouseDown(float,float)
GUI_CheckBox::OnMouseMove(float,float)
GUI_CheckBox::OnMouseUp(float,float)
GUI_CheckBox::OutFocus(void)
GUI_CheckBox::Render(void)
GUI_CheckBox::SetDisabled(bool)
GUI_CheckBox::SetName(char const*)
GUI_CheckBox::SetToolTip(char const*)
GUI_CheckBox::ToggleChecked(void)
GUI_CheckBox::Update(float)
GUI_CheckBox::~GUI_CheckBox()
GUI_ColorSchemeWidget::GUI_ColorSchemeWidget(void)
GUI_ColorSchemeWidget::Init(GUI_CustomizationDialogV2 *,float,float,CUSTOMIZATION_CATEGORY,ColorScheme *)
GUI_ColorSchemeWidget::Init(GUI_CustomizationDialogV2 *,float,float,CUSTOMIZATION_CATEGORY,uint)
GUI_ColorSchemeWidget::IsEqual(uint)
GUI_ColorSchemeWidget::IsHueShiftable(void)
GUI_ColorSchemeWidget::IsViewable(void)
GUI_ColorSchemeWidget::Render(void)
GUI_ColorSchemeWidget::Update(float)
GUI_ColorSchemeWidget::UpdateProfile(void)
GUI_ColorSchemeWidget::~GUI_ColorSchemeWidget()
GUI_CreateAchievementNotification(UI_AchievementNotification::Icon,char *,char *,void *,AchievementInstance::AchievementType)
GUI_CreateAchievementsList(void *,void *)
GUI_CreateAuthoritativeDeviceDataView(void (*)(UI_AuthoritativeDeviceDataView *))
GUI_CreateBonusNotificationPopup(UI_BonusNotificationPopup::BonusType,int)
GUI_CreateCalibrateRunSensorController(void (*)(UI_CalibrateRunSensorController::CALIBRATESENSOR_DIALOG_RESULTS))
GUI_CreateCalibrateSpinDownSensorController(void (*)(UI_CalibrateSpinDownSensorController::CALIBRATESENSOR_DIALOG_RESULTS))
GUI_CreateCalibrateSteeringSensorController(void (*)(UI_CalibrateSteeringSensorController::CALIBRATESENSOR_DIALOG_RESULTS))
GUI_CreateChallengeScreenDialog(char const*,void (*)(void))
GUI_CreateConfigScreenDialog(char const*,void (*)(int))
GUI_CreateCustomTargetConfigure(void (*)(UI_CustomTargetConfigure::CUSTOMCONFIGURE_DIALOG_RESULTS,DAILY_TARGET_TYPE,int))
GUI_CreateCustomizationDialogV2(void (*)(void))
GUI_CreateDailyTargetConfigure(void (*)(UI_DailyTargetConfigure::TARGETCONFIGURE_DIALOG_RESULTS,DAILY_TARGET_TYPE,int))
GUI_CreateDeviceListDialog(char const*,char const*,char const*,void (*)(uint),DeviceComponent::ComponentType,VirtualBikeComputer::ZSPORT)
GUI_CreateDropInDialog(char const*,char const*,void (*)(long long))
GUI_CreateDropsIntro(uint,uint,bool)
GUI_CreateEULAView(void (*)(void *))
GUI_CreateExtendWorkoutConfirmDialog(void (*)(UI_ExtendWorkoutConfirmDialog::DIALOG_RESULTS,int))
GUI_CreateFirmwareUpdatePromptDialog(sDeviceFirmwareInfo *)
GUI_CreateGenderDialog(char const*,void (*)(void))
GUI_CreateIOSLoginDialog(void *)
GUI_CreateIOSQuitScreen(void *)
GUI_CreateIntroSlideShowDialog(char const*,void (*)(int))
GUI_CreateLoginDialog(void (*)(UI_LoginDialog::DIALOG_RESULTS,char const*,char const*),bool,void *)
GUI_CreateMessageBox(char const*,char const*,float,float,float,std::function<void ()(MessageBoxResults)>)
GUI_CreateMessageBoxWithVariableButtonWidth(char const*,char const*,char const*,char const*,std::function<void ()(MessageBoxResults)>)
GUI_CreateMiniProfileScreenDialog(char const*,void (*)(void))
GUI_CreateNextupTrainingViewDialog(void (*)(UI_TrainingPlan_StartActivity::DIALOG_RESULTS))
GUI_CreateNotificationDialog(UI_DIALOGS,char const*,char const*,char const*,float)
GUI_CreatePacerBotScreen(void)
GUI_CreatePairingDialog(char const*,char const*,char const*,void (*)(uint),VirtualBikeComputer::ZSPORT)
GUI_CreatePauseDialog(void (*)(void))
GUI_CreatePerformanceIncreaseNotification(void)
GUI_CreatePowerUserLevelingSelector(void)
GUI_CreateProfileScreenDialog(char const*,void (*)(void))
GUI_CreatePromoCodeDialog(void (*)(UI_ApplyPromoCode::DIALOG_RESULTS))
GUI_CreateQuitDialog(void (*)(UI_QuitDialog::DIALOG_RESULTS))
GUI_CreateRouteSelectionDialog(char const*,void (*)(uint))
GUI_CreateSelectRouteListDialog(char const*,char const*,char const*,void (*)(uint))
GUI_CreateSendAreaTextDialog(void (*)(UI_SendAreaTextDialog::DIALOG_RESULTS))
GUI_CreateSingleTextEntryDialog(float,float,float,float,uint,char const**,uint,char const**,char const*,char *,void (*)(std::vector<GUI_EditBox *> const&,std::vector<GUI_CheckBox *> const&,void *),void *)
GUI_CreateSocialPreview(void (*)(UI_SocialPreview::DIALOG_RESULTS),facebookDisplay *)
GUI_CreateTrainerSelectionDialog(char const*,void (*)(int))
GUI_CreateTrialEndDialog(void (*)(UI_TrialNagScreen::TRIALNAG_DIALOG_RESULTS))
GUI_CreateTrialNagDialog(void (*)(UI_TrialNagScreen::TRIALNAG_DIALOG_RESULTS))
GUI_CreateTutorialScreens(void *)
GUI_CreateWorkoutCreator(void)
GUI_CreateWorkoutSelectionDialog(char const*,void (*)(Workout *),uint,bool)
GUI_CreateZCAdvertIfNeeded(void)
GUI_CreateZwiftShop(GUI_CustomizationDialogV2 *)
GUI_CustomizationDialogV2::AccessoriesTypeMustHaveHueShiftOffset(void)
GUI_CustomizationDialogV2::ChangeCategoryTabViewDuringOnboarding(GUI_CustomizationDialogV2*,GUI_CustomizationDialogV2::OnboardingSteps)
GUI_CustomizationDialogV2::CleanupAfterTransitionDone(std::vector<std::vector<UIStackElement *>> &)
GUI_CustomizationDialogV2::Clicked(GUI_Obj *,uint,uint,void *)
GUI_CustomizationDialogV2::CreateStoreLinkText(ColorScheme const*,void const*,bool)
GUI_CustomizationDialogV2::CreateStoreLinkText(void const*,bool)
GUI_CustomizationDialogV2::Dragging(GUI_Obj *,float)
GUI_CustomizationDialogV2::EquipHelmetForNewRiders(void)
GUI_CustomizationDialogV2::EquipItem(void)
GUI_CustomizationDialogV2::EquipSocksForNewRiders(void)
GUI_CustomizationDialogV2::FinishOnboardingCustomization(GUI_CustomizationDialogV2*)
GUI_CustomizationDialogV2::ForceTransitionComplete(std::vector<std::vector<UIStackElement *>> &)
GUI_CustomizationDialogV2::GUI_CustomizationDialogV2(float,float,float,float,void (*)(void))
GUI_CustomizationDialogV2::GUI_CustomizationDialogV2(void)
GUI_CustomizationDialogV2::GetAccessoryUnlockLevel(Accessory const*,Accessory const*)
GUI_CustomizationDialogV2::GetCurrentBikeHueOffset(void)
GUI_CustomizationDialogV2::GetDisplayAccessories(uint,Accessory::Type,std::vector<Accessory*> const*,std::vector<Accessory*>*,std::vector<Accessory*>*,Accessory const**,Accessory const**)
GUI_CustomizationDialogV2::GetDisplayBikes(uint,std::vector<BikePresetConfig *> const*,std::vector<BikePresetConfig *>*)
GUI_CustomizationDialogV2::GetHueOffsetFromSelectedWidget(void)
GUI_CustomizationDialogV2::GetSelectedWidget(void)
GUI_CustomizationDialogV2::GetSliderValueForBikeFrameColor(float)
GUI_CustomizationDialogV2::GetWidgetFromGridCategory(CUSTOMIZATION_CATEGORY)
GUI_CustomizationDialogV2::HideNonEssentialUI(void)
GUI_CustomizationDialogV2::InitAccessories(Accessory::Type,uint,uint,float,float)
GUI_CustomizationDialogV2::InitBikeFrames(uint,uint,float,float)
GUI_CustomizationDialogV2::InitCategory(VirtualBikeComputer::ZSPORT)
GUI_CustomizationDialogV2::InitColorSchemes(void const*,bool,uint)
GUI_CustomizationDialogV2::InitHairColorSchemes(Accessory const*)
GUI_CustomizationDialogV2::InitTransitionDirection(std::vector<UIStackElement *> *,GUI_CustomizationDialogV2::UI_Direction,bool)
GUI_CustomizationDialogV2::InitTransitionHelper(std::vector<std::vector<UIStackElement *>> *,std::vector<UIStackElement *>*,GUI_CustomizationDialogV2::UI_Direction,bool)
GUI_CustomizationDialogV2::InitialState(void)
GUI_CustomizationDialogV2::InitializeUIIfOnboarding(bool)
GUI_CustomizationDialogV2::InvalidNavigation(GUI_Obj *,I_GUI_GridsDelegate::NavigationDirection,void *)
GUI_CustomizationDialogV2::IsCurrentTabUnderAvatarCustomizationCategory(void)
GUI_CustomizationDialogV2::LeftTransitionFinished(void)
GUI_CustomizationDialogV2::ModifySliderHueShiftProperties(void)
GUI_CustomizationDialogV2::OnEscape(void)
GUI_CustomizationDialogV2::OnKey(int,int)
GUI_CustomizationDialogV2::OnMouseDown(float,float)
GUI_CustomizationDialogV2::OnMouseMove(float,float)
GUI_CustomizationDialogV2::OnMouseUp(float,float)
GUI_CustomizationDialogV2::PostRender(void)
GUI_CustomizationDialogV2::Render(void)
GUI_CustomizationDialogV2::RenderLevel(float,float)
GUI_CustomizationDialogV2::RenderPreviewOverlay(void)
GUI_CustomizationDialogV2::ReselectGrid(void)
GUI_CustomizationDialogV2::ResetCachedColorScheme(void)
GUI_CustomizationDialogV2::ResetCachedItems(void)
GUI_CustomizationDialogV2::RightTransitionFinished(void)
GUI_CustomizationDialogV2::Selected(GUI_Obj *,uint,uint,void *)
GUI_CustomizationDialogV2::SetAccessoryTransition(CUSTOMIZATION_CATEGORY)
GUI_CustomizationDialogV2::SetBikeFramHue(float)
GUI_CustomizationDialogV2::SetItemDescription(char const*)
GUI_CustomizationDialogV2::SetItemName(char const*)
GUI_CustomizationDialogV2::SetPreviewOverlay(std::string,std::string)
GUI_CustomizationDialogV2::SetUnlockOverlay(GUI_Obj *)
GUI_CustomizationDialogV2::SetupSupplementaryUI(float,bool)
GUI_CustomizationDialogV2::ShowBikesDuringOnBoarding(GUI_CustomizationDialogV2*)
GUI_CustomizationDialogV2::TransitionCompleted(UIStackElement **)
GUI_CustomizationDialogV2::TransitionIn(std::vector<UIStackElement *> *,std::vector<UIStackElement *> *)
GUI_CustomizationDialogV2::TransitionOut(bool,bool)
GUI_CustomizationDialogV2::TransitionOutHelper(std::vector<std::vector<UIStackElement *>> *,bool)
GUI_CustomizationDialogV2::Update(float)
GUI_CustomizationDialogV2::UpdateConditionalControls(void)
GUI_CustomizationDialogV2::UpdateNetworkFunction(void)
GUI_CustomizationDialogV2::UpdatePlayerLevel(void)
GUI_CustomizationDialogV2::UpdatePreviewImageDimension(float)
GUI_CustomizationDialogV2::UpdateProductNameUI(bool)
GUI_CustomizationDialogV2::UpdateTransition(float)
GUI_CustomizationDialogV2::UpdateTransitionHelper(float,std::vector<std::vector<UIStackElement *>> &)
GUI_CustomizationDialogV2::UpdateTranstionUIPosition(float *,float *,float *,float *,GUI_Obj *,std::vector<std::vector<UIStackElement *>> *)
GUI_CustomizationDialogV2::UpdateUI(float)
GUI_CustomizationDialogV2::UpdateUIPlayerMovementStateChanged(void)
GUI_CustomizationDialogV2::ValidateColorScheme(Accessory const*,uint &)
GUI_CustomizationDialogV2::ValidateColorScheme(BikePresetConfig const*,uint &)
GUI_CustomizationDialogV2::ValidateColorScheme(std::vector<ColorScheme *> const&,uint &)
GUI_CustomizationDialogV2::cbAvatarCategoryTabClicked(void *)
GUI_CustomizationDialogV2::cbBackButtonClicked(void *)
GUI_CustomizationDialogV2::cbBackButtonOnboardingClicked(void *)
GUI_CustomizationDialogV2::cbCategoryAccessoriesClicked(void *)
GUI_CustomizationDialogV2::cbCategoryAvatarMeClicked(void *)
GUI_CustomizationDialogV2::cbCategoryPurchasablesClicked(void *)
GUI_CustomizationDialogV2::cbDoneButtonClicked(void *)
GUI_CustomizationDialogV2::cbNextButtonOnboardingClicked(void *)
GUI_CustomizationDialogV2::cbOkayButtonClicked(void *)
GUI_CustomizationDialogV2::cbSkipButtonOnboardingClicked(void *)
GUI_CustomizationDialogV2::cbSprayPaintButtonClicked(void *)
GUI_CustomizationDialogV2::cbStoreButtonClicked(void *)
GUI_CustomizationDialogV2::sGetProductName(Accessory::Type)
GUI_CustomizationDialogV2::sInitializeAnimationCountDown(GUI_AccessoryWidget *,int)
GUI_CustomizationDialogV2::sSetupAnimationCountdown(GUI_AccessoryWidget *,int,int,float)
GUI_CustomizationDialogV2::sTransitionSelectedAccessoryWidget(GUI_AccessoryWidget **,GUI_Grids *)
GUI_CustomizationDialogV2::~GUI_CustomizationDialogV2()
GUI_DragSource::EndDrag(void)
GUI_DragSource::GetGrabOffsetFromCenter(void)
GUI_DragSource::IsDragging(void)
GUI_DragSource::OnDroppedOnNothing(void)
GUI_DragSource::OnMouseDown(float,float)
GUI_DragSource::OnMouseMove(float,float)
GUI_DragSource::OnMouseUp(float,float)
GUI_DragSource::PostRender(void)
GUI_DragSource::StartDrag(void)
GUI_DragSource::Update(float)
GUI_DragSource::~GUI_DragSource()
GUI_DragTarget::OnDrop(GUI_DragSource *)
GUI_DragTarget::OnHover(GUI_DragSource *)
GUI_DropDownBox::CloseDropDownBox(void)
GUI_DropDownBox::CreateDropDownArrow(char const*)
GUI_DropDownBox::DnButtonClicked(void)
GUI_DropDownBox::FinishedAddItems(void)
GUI_DropDownBox::GUI_DropDownBox(float,float,float,float,float,char const*,GUI_Obj *,I_GUI_ListBoxDelegate *)
GUI_DropDownBox::GUI_DropDownBox(float,float,float,float,float,char const*,GUI_Obj *,I_GUI_ListBoxDelegate *,void (*)(GUI_DropDownBox*))
GUI_DropDownBox::InFocus(void)
GUI_DropDownBox::OnEscape(void)
GUI_DropDownBox::OnKey(int,int)
GUI_DropDownBox::OnMouseDown(float,float)
GUI_DropDownBox::OnMouseMove(float,float)
GUI_DropDownBox::OnMouseUp(float,float)
GUI_DropDownBox::OpenDropDownBox(void)
GUI_DropDownBox::OutFocus(void)
GUI_DropDownBox::PositionDropDownArrow(void)
GUI_DropDownBox::PostRender(void)
GUI_DropDownBox::PrepSelectedTextDisplay(void)
GUI_DropDownBox::Render(void)
GUI_DropDownBox::ScrollToSelected(int)
GUI_DropDownBox::Select(uint)
GUI_DropDownBox::SetBoxPadding(float,float,float,float)
GUI_DropDownBox::SetDisabled(bool)
GUI_DropDownBox::SetDropDownBoxFlag(GUI_DropDownBox::GUI_DropDownBoxStyles,bool)
GUI_DropDownBox::SetTitleText(char const*)
GUI_DropDownBox::SetVisible(bool)
GUI_DropDownBox::SilentSelect(uint)
GUI_DropDownBox::ToggleDropDownBox(void)
GUI_DropDownBox::UpbuttonClicked(void)
GUI_DropDownBox::Update(float)
GUI_DropDownBox::~GUI_DropDownBox()
GUI_DropsIntro::GUI_DropsIntro(uint,uint,bool)
GUI_DropsIntro::Render(void)
GUI_DropsIntro::Update(float)
GUI_DropsIntro::~GUI_DropsIntro()
GUI_GetFocusedObject(void)
GUI_GetOutlineAnimationAlpha(void)
GUI_GetOutlineAnimationAlphaNormalized(void)
GUI_GetOutlineAnimationAlphaWithLowerCap(float)
GUI_GetPreviousFocusedObject(void)
GUI_GetSelectedObject(void)
GUI_GetTopLayer(void)
GUI_GetTopmostElement(void)
GUI_Grids::AddObjectAt(GUI_Obj *,int,int,bool,GUI_Obj **)
GUI_Grids::AddObjectNextAvailableSpace(GUI_Obj *)
GUI_Grids::DeleteAllObjects(void)
GUI_Grids::DnButtonClicked(void)
GUI_Grids::FindObjectAt(int,int)
GUI_Grids::GUI_Grids(float,float,float,float,GUI_Obj *,I_GUI_GridsDelegate *)
GUI_Grids::GUI_Grids(void)
GUI_Grids::GetTotalHeight(void)
GUI_Grids::GetTotalWidth(bool)
GUI_Grids::NullAllOjbects(void)
GUI_Grids::OnKey(int,int)
GUI_Grids::OnMouseDown(float,float)
GUI_Grids::OnMouseMove(float,float)
GUI_Grids::OnMouseUp(float,float)
GUI_Grids::OnMouseWheel(int,float,float)
GUI_Grids::OnSwipe(int,VEC2,VEC2,float,float)
GUI_Grids::ReCentering(int,int)
GUI_Grids::Render(void)
GUI_Grids::ResetSelected(void)
GUI_Grids::ReturnValidObject(int &,int &,bool)
GUI_Grids::ScrollTo(float,bool)
GUI_Grids::SetBehaviorFlags(GUI_Grids::BEHAVIOR_FLAGS,bool)
GUI_Grids::SetDisabled(bool)
GUI_Grids::SetGridDimension(int,int)
GUI_Grids::SetHeight(float)
GUI_Grids::SetHorizontalGridSpacing(float)
GUI_Grids::SetHorizontalGutting(float,float)
GUI_Grids::SetSelected(GUI_Obj *,bool)
GUI_Grids::SetSelected(int,int,bool)
GUI_Grids::SetSideGutting(float)
GUI_Grids::SetVerticalGridSpacing(float)
GUI_Grids::SetVisible(bool)
GUI_Grids::SetWidgetHeight(float)
GUI_Grids::SetWidgetWidth(float)
GUI_Grids::SetWidth(float)
GUI_Grids::UpbuttonClicked(void)
GUI_Grids::Update(float)
GUI_Grids::UpdateScrollBarsPosition(void)
GUI_Grids::~GUI_Grids()
GUI_GroupEventChatMessage::GUI_GroupEventChatMessage(ushort const*,uint,int,ushort const*,uint,uint,uint,CFont2D::FontAlignment,float)
GUI_GroupEventChatMessage::Render(void)
GUI_GroupEventChatMessage::~GUI_GroupEventChatMessage()
GUI_HandlePaste(void)
GUI_INTERNAL_PlaySound(char const*)
GUI_Image::GUI_Image(float,float,float,float,char const*,GUI_Obj *)
GUI_Image::GUI_Image(float,float,float,float,float *,char const*,GUI_Obj *)
GUI_Image::GUI_Image(float,float,float,float,int,GUI_Obj *)
GUI_Image::GUI_Image(float,float,float,float,int,GUI_Obj *,CFont2D *,char const*,float,float,uint,float,uint)
GUI_Image::GUI_Image(float,float,float,float,void (*)(GUI_Image*,float,float,float,float,void *),void *,GUI_Obj *)
GUI_Image::OnMouseMove(float,float)
GUI_Image::OnMouseUp(float,float)
GUI_Image::PostRender(void)
GUI_Image::Render(void)
GUI_Image::SetCustomUV(float *)
GUI_Image::Update(float)
GUI_Image::~GUI_Image()
GUI_Initialize(void (*)(char const*),bool)
GUI_Internal_HandleUnselectableListBox(int,GUI_ListBox *)
GUI_IsObjAdded(GUI_Obj *)
GUI_Key(int,int)
GUI_ListBox::AddCustomItem(float,float,void (*)(GUI_Image *,float,float,float,float,void *),void *,void *,bool,float)
GUI_ListBox::AddIconItem(float,float,GUI_Obj *,char const*,void *,bool,bool,bool,bool,bool,float)
GUI_ListBox::AddIconItem(float,float,char const*,char const*,void *,bool,bool,bool,float)
GUI_ListBox::AddIconItem(float,float,int,char const*,void *,bool,bool,bool,float)
GUI_ListBox::AddImageItem(float,float,char const*,void *,bool,float)
GUI_ListBox::AddItem(char const*,void *,bool,float)
GUI_ListBox::AddMultiSelection(int)
GUI_ListBox::CalcComplexListScrollToSelectDist(bool)
GUI_ListBox::ChangeItemPosition(uint,uint)
GUI_ListBox::ClearMouseState(void)
GUI_ListBox::ClearMultiSelection(void)
GUI_ListBox::DisableIncDecButtons(void)
GUI_ListBox::DnButtonClicked(void)
GUI_ListBox::EnableIncDecButtons(float,float,float,float,int,void (*)(void *),void (*)(void *),float *,float *)
GUI_ListBox::FindChildrenGUIObjIndex(GUI_Obj *)
GUI_ListBox::FindIndexByUserdata(void *)
GUI_ListBox::FinishedAddItems(void)
GUI_ListBox::GUI_ListBox(float,float,float,float,GUI_Obj *,I_GUI_ListBoxDelegate *,int)
GUI_ListBox::GetCurrentSelectedImage(void)
GUI_ListBox::GetCurrentSelectedName(void)
GUI_ListBox::GetCurrentSelectedUserData(void)
GUI_ListBox::GetCustomElementHeight(uint)
GUI_ListBox::GetElementAbsoluteY(int)
GUI_ListBox::GetElementHeight(uint)
GUI_ListBox::GetElementY(int)
GUI_ListBox::GetIndexFromUserdata(void *)
GUI_ListBox::GetItemName(uint)
GUI_ListBox::GetItemUserData(uint)
GUI_ListBox::GetListSpan(void)
GUI_ListBox::GetNumberOfItems(void)
GUI_ListBox::GetNumberOfUnlockedItems(void)
GUI_ListBox::HandleEndOfListBehavior(void)
GUI_ListBox::HandleListBounds(void)
GUI_ListBox::HideIncDecButtons(bool)
GUI_ListBox::HitTest(float,float)
GUI_ListBox::InFocus(void)
GUI_ListBox::IsComplexList(void)
GUI_ListBox::IsMultiSelected(int)
GUI_ListBox::ItemHitTest(float,float)
GUI_ListBox::OnEscape(void)
GUI_ListBox::OnMouseDoubleClick(float,float)
GUI_ListBox::OnMouseDown(float,float)
GUI_ListBox::OnMouseMove(float,float)
GUI_ListBox::OnMouseUp(float,float)
GUI_ListBox::OnMouseWheel(int,float,float)
GUI_ListBox::OutFocus(void)
GUI_ListBox::PostRender(void)
GUI_ListBox::RemoveAllItems(bool)
GUI_ListBox::RemoveImageItem(GUI_Obj const*)
GUI_ListBox::RemoveItem(int)
GUI_ListBox::RemoveMultiSelection(int)
GUI_ListBox::Render(void)
GUI_ListBox::RenderBgRing(void)
GUI_ListBox::ScrollTo(float,bool)
GUI_ListBox::ScrollToSelected(int)
GUI_ListBox::Select(uint)
GUI_ListBox::SetDisabled(bool)
GUI_ListBox::SetMultiSelected(std::vector<int>)
GUI_ListBox::SetParent(GUI_Obj *)
GUI_ListBox::SetSelectionIdx(int)
GUI_ListBox::SetVisible(bool)
GUI_ListBox::ShouldScrollListBox(void)
GUI_ListBox::SilentSelect(uint)
GUI_ListBox::StopScrolling(void)
GUI_ListBox::UpbuttonClicked(void)
GUI_ListBox::Update(float)
GUI_ListBox::UpdateAndDrawElement(float,float,float,float,int,bool)
GUI_ListBox::defaultOnClickDownArrow(void *)
GUI_ListBox::defaultOnClickUpArrow(void *)
GUI_ListBox::getAutoScrollOnHoverShift(int)
GUI_ListBox::resetAutoScrollOnHover(void)
GUI_ListBox::simulateOnClickDownArrow(GUI_ListBox*)
GUI_ListBox::simulateOnClickUpArrow(GUI_ListBox*)
GUI_ListBox::updateAutoScrollOnHover(float)
GUI_ListBox::~GUI_ListBox()
GUI_ManufactureWidget::GUI_ManufactureWidget(void)
GUI_ManufactureWidget::Init(GUI_ZwiftShop *,uint,uint,float,float,int,char const*)
GUI_ManufactureWidget::Render(void)
GUI_ManufactureWidget::Update(float)
GUI_ManufactureWidget::~GUI_ManufactureWidget()
GUI_MessageBox::GUI_MessageBox(float,float,float,float,char const*,char const*,char const*,char const*,std::function<void ()(MessageBoxResults)>,float)
GUI_MessageBox::OnButton1(void *)
GUI_MessageBox::OnButton2(void *)
GUI_MessageBox::OnKey(int,int)
GUI_MessageBox::Render(void)
GUI_MessageBox::SetFontScale(float)
GUI_MessageBox::Update(float)
GUI_MessageBox::~GUI_MessageBox()
GUI_MouseClick(int,int,float,float,float,float,int &)
GUI_MouseDoubleClick(float,float)
GUI_MouseMove(float,float,float,float)
GUI_MouseWheel(int,float,float)
GUI_NewPling::GUI_NewPling(float,float,GUI_Obj *,uint)
GUI_NewPling::Render(void)
GUI_NewPling::Update(float)
GUI_NewPling::~GUI_NewPling()
GUI_OnEscape(void)
GUI_PostRender(void)
GUI_PurchaseConfirmation::DisplayLackingFundMessage(void)
GUI_PurchaseConfirmation::GUI_PurchaseConfirmation(float,float,float,float,GUI_PurchaseConfirmation::PurchaseItemInfo *,GUI_ZwiftShop *,void (*)(GUI_ZwiftShop *,bool))
GUI_PurchaseConfirmation::OnEscape(void)
GUI_PurchaseConfirmation::OnKey(int,int)
GUI_PurchaseConfirmation::ProcessItemPurchase(void)
GUI_PurchaseConfirmation::PurchaseItemInfo::PurchaseItemInfo(void)
GUI_PurchaseConfirmation::Render(void)
GUI_PurchaseConfirmation::SetDisabled(bool)
GUI_PurchaseConfirmation::Update(float)
GUI_PurchaseConfirmation::cbBtnCancel(void *)
GUI_PurchaseConfirmation::cbBtnConfirmEquip(void *)
GUI_PurchaseConfirmation::cbBtnConfirmPurchase(void *)
GUI_PurchaseConfirmation::~GUI_PurchaseConfirmation()
GUI_Render(void)
GUI_ResortList(void)
GUI_ReturnTopMostModalDialog(GUI_Obj **)
GUI_RevZSort(GUI_Obj *,GUI_Obj *)
GUI_RowContainer::ApplyStylesheet(GUI_Stylesheet *,bool)
GUI_RowContainer::Render(void)
GUI_RowContainer::Update(float)
GUI_RowContainer::~GUI_RowContainer()
GUI_ScreenshotSelectorWidget::GUI_ScreenshotSelectorWidget(QueuedScreenshotUpload *,UI_ScreenshotSelectorController *,float,GUI_ListBox *)
GUI_ScreenshotSelectorWidget::OnMouseUp(float,float)
GUI_ScreenshotSelectorWidget::OnPrimaryPressed(void *)
GUI_ScreenshotSelectorWidget::OnToggleUIPressed(void *)
GUI_ScreenshotSelectorWidget::Render(void)
GUI_ScreenshotSelectorWidget::RenderPrimaryButton(GUI_Button *)
GUI_ScreenshotSelectorWidget::RenderToggleUIButton(GUI_Button *)
GUI_ScreenshotSelectorWidget::Update(float)
GUI_ScreenshotSelectorWidget::~GUI_ScreenshotSelectorWidget()
GUI_ScrollBar::DnButtonClicked(void *)
GUI_ScrollBar::GUI_ScrollBar(float,float,float,float,int,I_GUI_ScrollBarDelegate *,GUI_Obj *)
GUI_ScrollBar::GUI_ScrollBar(float,float,float,float,int,uint,int,int,char const*,char const*,char const*,I_GUI_ScrollBarDelegate *,GUI_Obj *)
GUI_ScrollBar::GetButtonScrollAmount(void)
GUI_ScrollBar::GetScaledScrollAmount(void)
GUI_ScrollBar::GetScrollJogSize(void)
GUI_ScrollBar::Init(float,float,float,float,int,uint,int,int,char const*,char const*,char const*,I_GUI_ScrollBarDelegate *,GUI_Obj *)
GUI_ScrollBar::OnMouseDown(float,float)
GUI_ScrollBar::OnMouseMove(float,float)
GUI_ScrollBar::OnMouseUp(float,float)
GUI_ScrollBar::PageDown(void)
GUI_ScrollBar::PageUp(void)
GUI_ScrollBar::Render(void)
GUI_ScrollBar::SetDisabled(bool)
GUI_ScrollBar::SetFlag(GUI_Obj::Flag,bool)
GUI_ScrollBar::SetLength(float)
GUI_ScrollBar::SetScrollPercent(float)
GUI_ScrollBar::SetSinglePageSize(float)
GUI_ScrollBar::SetTotalSize(float)
GUI_ScrollBar::SetupScrollButtonHeight(float,float,ulong long)
GUI_ScrollBar::SyncScrollBarPosition(float)
GUI_ScrollBar::UpbuttonClicked(void *)
GUI_ScrollBar::Update(float)
GUI_ScrollBar::UpdateScrollbar(void)
GUI_ScrollBar::UpdateScrollbarSizes(void)
GUI_ScrollBar::~GUI_ScrollBar()
GUI_SegmentSplitWidget::GUI_SegmentSplitWidget(uint,SegmentPath const*,uint)
GUI_SegmentSplitWidget::Render(void)
GUI_SegmentSplitWidget::RenderTrendLine(void)
GUI_SegmentSplitWidget::SegmentTrendData::sortByWorldTime(GUI_SegmentSplitWidget::SegmentTrendData const&,GUI_SegmentSplitWidget::SegmentTrendData const&)
GUI_SegmentSplitWidget::Update(float)
GUI_SegmentSplitWidget::UpdateData(void)
GUI_SegmentSplitWidget::~GUI_SegmentSplitWidget()
GUI_SetDefaultFont(CFont2D *)
GUI_Shape::GUI_Shape(float,float,float,float,uint,GUI_Obj *,GUI_Shape::Shapes)
GUI_Shape::OnMouseUp(float,float)
GUI_Shape::Render(void)
GUI_Shape::Update(float)
GUI_Shape::~GUI_Shape()
GUI_ShowEventFullNotification(GroupEvents::SubgroupState *)
GUI_Slider::GUI_Slider(float,float,float,float,GUI_Obj *,I_GUI_SliderDelegate *)
GUI_Slider::GetSliderAtCurrentValue(VEC2 &,VEC2 &)
GUI_Slider::GetSliderFlag(GUI_Slider::GUISliderFlags)
GUI_Slider::HitTest(float,float)
GUI_Slider::MouseXTofValue(float)
GUI_Slider::MouseYTofValue(float)
GUI_Slider::OnMouseDown(float,float)
GUI_Slider::OnMouseMove(float,float)
GUI_Slider::OnMouseUp(float,float)
GUI_Slider::Render(void)
GUI_Slider::SetRenderBackground(bool)
GUI_Slider::SetSliderFlag(GUI_Slider::GUISliderFlags,bool)
GUI_Slider::Update(float)
GUI_Slider::fValueToMouseX(float)
GUI_Slider::fValueToMouseY(float)
GUI_Slider::~GUI_Slider()
GUI_SportSelectWidget::Destroy(void)
GUI_SportSelectWidget::GUI_SportSelectWidget(UI_SportSelectController *,char const*,int,float,float,float,float)
GUI_SportSelectWidget::GetSelected(void)
GUI_SportSelectWidget::OnMouseDown(float,float)
GUI_SportSelectWidget::OnSelect(bool)
GUI_SportSelectWidget::OnSelectButton(void *)
GUI_SportSelectWidget::Render(void)
GUI_SportSelectWidget::SelectThis(void)
GUI_SportSelectWidget::SetSelected(bool)
GUI_SportSelectWidget::Update(float)
GUI_SportSelectWidget::~GUI_SportSelectWidget()
GUI_StreamedImage::GUI_StreamedImage(float,float,float,float,char const*,GUI_Obj *)
GUI_StreamedImage::OnStreamComplete(char const*)
GUI_StreamedImage::SetImagePath(char const*)
GUI_StreamedImage::~GUI_StreamedImage()
GUI_StylesheetConsumer::AddStyle(char const*)
GUI_StylesheetConsumer::AddStyle(uint)
GUI_StylesheetConsumer::ApplyStyleClass(char const*)
GUI_StylesheetConsumer::ApplyStyleClass(uint)
GUI_StylesheetConsumer::ApplyStylesheet(GUI_Stylesheet *,bool)
GUI_StylesheetConsumer::OnBlur(void)
GUI_StylesheetConsumer::OnFocus(void)
GUI_StylesheetConsumer::RemoveStyle(char const*)
GUI_StylesheetConsumer::RemoveStyle(uint)
GUI_StylesheetConsumer::Render(void)
GUI_StylesheetConsumer::Update(float)
GUI_StylesheetConsumer::UpdatedStyle(char const*)
GUI_StylesheetConsumer::UpdatedStyle(uint)
GUI_StylesheetConsumer::~GUI_StylesheetConsumer()
GUI_Swipe(int,VEC2,VEC2,float,float,int)
GUI_TabHeader::GUI_TabHeader(float,float,float,float,int,char const**,void (*)(void *),GUI_Obj *)
GUI_TabHeader::OnMouseDown(float,float)
GUI_TabHeader::OnMouseMove(float,float)
GUI_TabHeader::OnMouseUp(float,float)
GUI_TabHeader::Render(void)
GUI_TabHeader::Update(float)
GUI_TabHeader::~GUI_TabHeader()
GUI_TimePicker::Close(void)
GUI_TimePicker::GUI_TimePicker(float,float,float,float,GUI_Obj *,GUI_TimePicker::TimePickerModes,ulong long)
GUI_TimePicker::GetTimeString(char *,int)
GUI_TimePicker::OnMouseDown(float,float)
GUI_TimePicker::OnMouseUp(float,float)
GUI_TimePicker::Open(void)
GUI_TimePicker::Render(void)
GUI_TimePicker::Selected(GUI_Obj *,uint,void *)
GUI_TimePicker::SetHours(int)
GUI_TimePicker::SetMinutes(int)
GUI_TimePicker::SetSeconds(int)
GUI_TimePicker::SetTime(float)
GUI_TimePicker::Update(float)
GUI_TimePicker::onOkayClicked(void *)
GUI_TimePicker::~GUI_TimePicker()
GUI_TimeSplitWidget::GUI_TimeSplitWidget(uint,UI_TimeSplitView::SplitType,std::vector<float> *)
GUI_TimeSplitWidget::Render(void)
GUI_TimeSplitWidget::Update(float)
GUI_TimeSplitWidget::UpdateData(void)
GUI_TimeSplitWidget::~GUI_TimeSplitWidget()
GUI_TitledContainer::ApplyStylesheet(GUI_Stylesheet *,bool)
GUI_TitledContainer::FindByID(char const*)
GUI_TitledContainer::FindByID(uint)
GUI_TitledContainer::GUI_TitledContainer(GUI_Obj *,GUI_Obj *)
GUI_TitledContainer::GUI_TitledContainer(GUI_Obj *,char const*)
GUI_TitledContainer::OnMouseDown(float,float)
GUI_TitledContainer::OnMouseMove(float,float)
GUI_TitledContainer::OnMouseUp(float,float)
GUI_TitledContainer::Render(void)
GUI_TitledContainer::Update(float)
GUI_TitledContainer::~GUI_TitledContainer()
GUI_ToolTip::GUI_ToolTip(GUI_Obj *,float,float,CFont2D *,char const*)
GUI_ToolTip::OnMouseMove(float,float)
GUI_ToolTip::OnMouseWheel(int,float,float)
GUI_ToolTip::Render(void)
GUI_ToolTip::ScaleIn(float)
GUI_ToolTip::SetText(char const*)
GUI_ToolTip::Update(float)
GUI_ToolTip::~GUI_ToolTip()
GUI_TrainingPlanBlockWidget::GUI_TrainingPlanBlockWidget(TrainingPlan::PlanBlock *,TrainingPlanEnrollmentInstance *,GUI_Obj *,GUIOBJ_TrainingPlanViewer *,bool)
GUI_TrainingPlanBlockWidget::OnMouseUp(float,float)
GUI_TrainingPlanBlockWidget::Render(void)
GUI_TrainingPlanBlockWidget::SetDaySelected(int)
GUI_TrainingPlanBlockWidget::Update(float)
GUI_TrainingPlanBlockWidget::~GUI_TrainingPlanBlockWidget()
GUI_TreeListElement::Contract(void)
GUI_TreeListElement::Expand(void)
GUI_TreeListElement::GUI_TreeListElement(int,float,float,float,float,float,char const*,char const*,CFont2D *,GUI_ListBox *,void *,uint,bool)
GUI_TreeListElement::GetCustomLength(bool)
GUI_TreeListElement::Hover(GUI_Obj *,uint,void *)
GUI_TreeListElement::InFocus(void)
GUI_TreeListElement::ListBoxEscaped(GUI_Obj *)
GUI_TreeListElement::ListBoxInFocus(GUI_Obj *)
GUI_TreeListElement::ListBoxOutFocus(GUI_Obj *)
GUI_TreeListElement::ListItemDoubleClicked(int,void *,GUI_Obj *)
GUI_TreeListElement::OnKey(int,int)
GUI_TreeListElement::OnMouseDoubleClick(float,float)
GUI_TreeListElement::OnMouseDown(float,float)
GUI_TreeListElement::OnMouseMove(float,float)
GUI_TreeListElement::OnMouseUp(float,float)
GUI_TreeListElement::OutFocus(void)
GUI_TreeListElement::Render(void)
GUI_TreeListElement::Selected(GUI_Obj *,uint,void *)
GUI_TreeListElement::SetAddRemoveCallbackFunctions(void (*)(void *),void (*)(void *))
GUI_TreeListElement::Update(float)
GUI_TreeListElement::~GUI_TreeListElement()
GUI_Update(float)
GUI_ZSort(GUI_Obj *,GUI_Obj *)
GUI_ZwiftShop::AutoSelectButton(void)
GUI_ZwiftShop::CachedItems(CUSTOMIZATION_CATEGORY)
GUI_ZwiftShop::ClearSelect(void)
GUI_ZwiftShop::Clicked(GUI_Obj *,uint,uint,void *)
GUI_ZwiftShop::EquipPurchaseItem(CUSTOMIZATION_CATEGORY,uint,uint)
GUI_ZwiftShop::FilterDisplayableAccessories(std::vector<Accessory *> const*,std::vector<Accessory *> const*,std::vector<Accessory *>*,std::vector<Accessory *>*,int,uint)
GUI_ZwiftShop::FilterDisplayableBikes(std::vector<BikePresetConfig *> const*,std::vector<BikePresetConfig *>*,uint,uint)
GUI_ZwiftShop::GUI_ZwiftShop(float,float,float,float,GUI_CustomizationDialogV2 *)
GUI_ZwiftShop::GetShopDisplayableFlag(void)
GUI_ZwiftShop::HasPurchasableColorScheme(std::vector<ColorScheme *> const&)
GUI_ZwiftShop::HideAll(void)
GUI_ZwiftShop::HidePurchasingUI(void)
GUI_ZwiftShop::InitManufactures(std::vector<Accessory *> const*)
GUI_ZwiftShop::InitManufactures(std::vector<BikePresetConfig *> const*)
GUI_ZwiftShop::InitManufacturesGrids(std::vector<uint> &,Accessory::Type)
GUI_ZwiftShop::InitPurchasables(uint)
GUI_ZwiftShop::InitPurchasablesGrids(std::vector<Accessory *> const*,std::vector<Accessory *> const*)
GUI_ZwiftShop::InitPurchasablesGrids(std::vector<BikePresetConfig *> const*)
GUI_ZwiftShop::Invalid(GUI_Button *,GUINavigationGraph::LinkType)
GUI_ZwiftShop::NavigateByShopLink(std::string &)
GUI_ZwiftShop::OnEscape(void)
GUI_ZwiftShop::OnKey(int,int)
GUI_ZwiftShop::PostRender(void)
GUI_ZwiftShop::PurchaseDone(void)
GUI_ZwiftShop::Render(void)
GUI_ZwiftShop::RenderDrops(float,float)
GUI_ZwiftShop::RenderEquippingAnimation(void)
GUI_ZwiftShop::RenderTextualInformation(void)
GUI_ZwiftShop::ResetCachedItems(void)
GUI_ZwiftShop::Selected(GUI_Obj *,uint,uint,void *)
GUI_ZwiftShop::SetInitialShopState(void)
GUI_ZwiftShop::SetPurchaseInfo(GUI_Obj *,Accessory const*,Accessory const*,char const*)
GUI_ZwiftShop::SetPurchaseInfo(GUI_Obj *,BikePresetConfig const*,char const*)
GUI_ZwiftShop::SetPurchaseInfo(GUI_Obj *,ColorScheme const*)
GUI_ZwiftShop::SetPurchaseInfo(uint,uint,CUSTOMIZATION_CATEGORY,int,int,int,char const*,VEC4 *,GUI_Obj *)
GUI_ZwiftShop::SetShopTransition(CUSTOMIZATION_CATEGORY)
GUI_ZwiftShop::SetTargetGold(int)
GUI_ZwiftShop::ToState(uint,void *)
GUI_ZwiftShop::Update(float)
GUI_ZwiftShop::UpdateButtonBehavior(bool,bool)
GUI_ZwiftShop::UpdateDropsAmount(float,float &,int &,int const&,std::string &,float *)
GUI_ZwiftShop::UpdateLinkNavigateAnimation(float)
GUI_ZwiftShop::UpdateLinkNavigateAtColorSchemeSelection(float)
GUI_ZwiftShop::UpdateLinkNavigateAtLandingPage(float)
GUI_ZwiftShop::UpdateLinkNavigateAtManufacturesPage(float)
GUI_ZwiftShop::UpdateLinkNavigateAtProductPage(float)
GUI_ZwiftShop::UpdateUIPlayerMovementStateChanged(bool)
GUI_ZwiftShop::cbBackButtonClicked(void *)
GUI_ZwiftShop::cbFrameShopButtonClicked(void *)
GUI_ZwiftShop::cbPurchaseButtonClicked(void *)
GUI_ZwiftShop::cbPurchaseComplete(GUI_ZwiftShop*,bool)
GUI_ZwiftShop::cbRunningShoeShopButtonClicked(void *)
GUI_ZwiftShop::cbToEquip(void *)
GUI_ZwiftShop::cbWheelShopButtonClicked(void *)
GUI_ZwiftShop::sPurchaseButtonRenderer(GUI_Button *)
GUI_ZwiftShop::~GUI_ZwiftShop()
*/
struct UI_FriendsList : public GUI_Obj, public I_GUI_ListBoxDelegate { //1280 bytes
    bool m_field_428 = false, m_changeCamera = false, m_fanView = false;
    void FanView(BikeEntity *be, bool);
    BikeEntity *m_bikeEntity = nullptr;
    /*UI_FriendsList::AddToFriendlist(BikeEntity *,BikeEntity *,std::vector<friendsListBike> &,float)
UI_FriendsList::AutoFanView(bool)
UI_FriendsList::GetDisplayName(RiderName *,friendsListBike const*,bool)
UI_FriendsList::GetFocusBikeRideonButton(void)
UI_FriendsList::Hover(GUI_Obj *,uint,void *)
UI_FriendsList::IsActive(void)
UI_FriendsList::IsInFanView(void)
UI_FriendsList::ListBoxEscaped(GUI_Obj *)
UI_FriendsList::ListBoxInFocus(GUI_Obj *)
UI_FriendsList::ListBoxOutFocus(GUI_Obj *)
UI_FriendsList::ObserveRandomPerson(bool,int)
UI_FriendsList::OnKey(int,int)
UI_FriendsList::OnMouseDown(float,float)
UI_FriendsList::OnSwipe(int,VEC2,VEC2,float,float)
UI_FriendsList::RemovePassingRiders(void)
UI_FriendsList::Render(void)
UI_FriendsList::ResetBotFollowTime(void)
UI_FriendsList::Selected(GUI_Obj *,uint,void *)
UI_FriendsList::SendFollowData(std::optional<ulong>)
UI_FriendsList::SendFriendlistToPhone(void)
UI_FriendsList::SetDisabled(bool)
UI_FriendsList::SetNearestPacerBot(void)
UI_FriendsList::UI_FriendsList(void)
UI_FriendsList::Update(float)
UI_FriendsList::UpdateBotFollowTime(friendsListBike const&)
UI_FriendsList::UpdateNearbyPacerBotList(friendsListBike const&)
UI_FriendsList::rideonButtonPressed(void *,void *,float,float)
UI_FriendsList::~UI_FriendsList()*/
};
struct UI_AudioControl : public GUI_Obj, public I_GUI_SliderDelegate { //256 (0x100) bytes
    UI_AudioControl(uint32_t playId) {
        //TODO
    }
    virtual ~UI_AudioControl() {
        //TODO
    }
    void Dragging(GUI_Obj *, float) {}
    void HUD_cbDrawAudioControl(float, float, float, float, void *);
    bool OnMouseUp(float, float) override {
        //TODO
        return false;
    }
    void OnPressedVolumeButton(void *) {}
    void Release(GUI_Obj *, float) override {
        //TODO
    }
    void SliderInFocus(GUI_Obj *) override {}
    void SliderClick(GUI_Obj *) override {}
    void Update(float) override {
        //TODO
    }
};
struct UI_AchievementNotification : public GUI_Obj, public intUINotificationHandler {
    //TODO
};
struct UI_AchievementsList : public GUI_Obj, public I_GUI_ScrollBarDelegate {
    //TODO
};
struct UI_ActionKeys : public GUI_Obj {
    GUI_Obj *m_field_1D8[7 /*TODO*/]{};
    int m_field_318 = 0, m_field_C4 = 0;
    void HideActionKeys(bool) {
        //TODO
    }
    void MoveToNextKey(bool) {
        //TODO
    }
    void ShowActionKeys(bool) {
        //TODO
    }
        //TODO
    /*UI_ActionKeys::ChangeCamera(void)
UI_ActionKeys::DecSelectedButton(void)
UI_ActionKeys::GUI_MAK_BtnHolder::Render(void)
UI_ActionKeys::GUI_MAK_BtnHolder::~GUI_MAK_BtnHolder()
UI_ActionKeys::IncSelectedButton(void)
UI_ActionKeys::KeepMenuOpen(void)
UI_ActionKeys::OnLeftArrowButton(void *)
UI_ActionKeys::OnMouseDown(float,float)
UI_ActionKeys::OnMouseMove(float,float)
UI_ActionKeys::OnMouseUp(float,float)
UI_ActionKeys::OnRightArrowButton(void *)
UI_ActionKeys::OnSwipe(int,VEC2,VEC2,float,float)
UI_ActionKeys::Prep(void)
UI_ActionKeys::ReInitConstants(void)
UI_ActionKeys::Render(void)
UI_ActionKeys::ResetAutoCloseTimer(void)
UI_ActionKeys::SelectCurrentKey(void)
UI_ActionKeys::StartScreenCaptureDelay(void)
UI_ActionKeys::UI_ActionKeys(void)
UI_ActionKeys::Update(float)
UI_ActionKeys::UpdateAnimation(float)
UI_ActionKeys::UpdateScreenCapDelay(float)
UI_ActionKeys::UpdateState(float)
UI_ActionKeys::~UI_ActionKeys()*/
    struct GUI_MAK_BtnHolder : public GUI_Obj {
        //TODO
    };
};
inline UI_ActionKeys *g_pActionKeys = nullptr;
struct UI_ActivityCalendar : public GUI_Obj, public I_GUI_EditBoxDelegate {
    //TODO
};
struct UI_ApplyPromoCode : public GUI_Obj {
    enum DIALOG_RESULTS { DR_0 };
    //TODO
};
struct UI_AuthoritativeDeviceDataView : public GUI_Obj {
    //TODO
};
struct UI_BonusNotificationPopup : public GUI_Obj {
    //TODO
};
struct UI_BranchSelector : public GUI_Obj {
    //TODO
};
struct UI_CalibrateRunSensorController : public GUI_Obj {
    //TODO
};
struct UI_CalibrateSpinDownSensorController : public GUI_Obj {
    //TODO
};
struct UI_CalibrateSteeringSensorController : public GUI_Obj {
    //TODO
};
struct UI_Challenge : public GUI_Obj {
    //TODO
};
struct UI_ChallengeScreen : public GUI_Obj, public I_GUI_ScrollBarDelegate {
    //TODO
};
struct UI_ChallengeUnlockNotification : public GUI_Obj, public intUINotificationHandler {
    //TODO
};
struct UI_ConfigScreen : public GUI_Obj, public I_GUI_CheckBoxDelegate, public I_GUI_SliderDelegate {
    //TODO
};
struct ConnectionSubscriber {
    //TODO
};
struct UI_ConnectionNotifications : public ConnectionSubscriber, public GUI_Obj {
    //TODO
};
struct UI_CreateUserDialog : public GUI_Obj {
    //TODO
};
struct UI_CustomTargetConfigure : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_DailyTargetConfigure : public GUI_Obj, public I_GUI_CheckBoxDelegate {
    //TODO
};
struct UI_DeleteAccount : public GUI_Obj {
    //TODO
};
struct UI_DetectedDeviceList : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_DevicePairingWidget : public GUI_Obj {
    //TODO
};
struct UI_DeviceStatusWidget : public GUI_Obj {
    //TODO
};
struct UI_DropInDialog : public GUI_Obj, public I_GUI_ListBoxDelegate {
    void RefreshGroupList();
    /*
        void BuildData(void);
        void CanSelectRoute(uint);
        void CanSelectWorld(int);
        void CheckBoxInFocus(GUI_Obj *);
        void ClearActiveWorkout(void);
        void ConfigurePlayersRouteSettings(void);
        void Destroy(void);
        void ForceCloseDialog(void *);
        void FriendModel::FriendModel(UI_DropInDialog::FriendModel const&);
        void GetDialogState(void);
        void GetTargetWorldFull(zwift::protobuf::DropInWorld const&);
        void InitDeviceStatusWidgets(void);
        void IsRouteCompatibleWithActiveWorkout(uint);
        void ListBoxInFocus(GUI_Obj *);
        void ListBoxOutFocus(GUI_Obj *);
        void MessageBoxCallback(MessageBoxResults);
        void OnButton1(void *);
        void OnButton2(void *);
        void OnButtonJoinZwift(void *);
        void OnButtonRestore(void *);
        void OnButtonUpdateMacDriver(void *);
        void OnChangeRideType(void *);
        void OnChangeRouteType(void *);
        void OnChooseGuestWorld(void *);
        void OnChooseMainWorld(void *);
        void OnChooseThirdWorld(void *);
        void OnEscape(void);
        void OnExitButton(void *);
        void OnKey(int,int);
        void OnMouseMove(float,float);
        void OnSelectRouteFromDialog(uint);
        void OnSelectWorkoutFromDialog(Workout *);
        void OnSignedUpForGroup(int);
        void OnViewPlan(void *);
        void PostRender(void);
        void RefreshRiderListForSelectedWorld(void);
        void Render(void);
        void RenderDeviceStatusInfo(float,float,float,float);
        void RenderGuestWorldButton(GUI_Button *);
        void RenderJoinZwifterInfo(float,float,float,float,char const*);
        void RenderMainWorldButton(GUI_Button *);
        void RenderRideTypeInfo(float,float,float,float);
        void RenderRouteInfo(float,float,float,float);
        void RenderThirdWorldButton(GUI_Button *);
        void RenderTrainingPlanInfo(float,float,float,float,TrainingPlan *,TrainingPlanEnrollmentInstance *);
        void RenderWorldButton(float,float,float,float,int,bool);
        void RenderWorldInfo(float,float,float,float);
        void RequestPacePartnerPlayerStates(long long,long long);
        void RequestPacePartnerProfiles(zwift::protobuf::DropInWorld const&);
        void SelectRoute(uint);
        void SelectWorld(int);
        void Selected(GUI_Obj *,uint,void *);
        void SetDisabled(bool);
        void SetDropInData(int);
        void UI_DropInDialog(float,float,float,float,char const*,char const*,void (*)(long long));
        void Update(float);
        void onPairingContainerPressed(void *);
        void ~UI_DropInDialog()*/;
};
struct UI_EULAView : public GUI_Obj {
    //TODO
};
struct UI_EventFinished : public GUI_Obj {
    //TODO
};
struct UI_ExtendWorkoutConfirmDialog : public GUI_Obj {
    //TODO
};
struct UI_FirmwareUpdateDialog : public GUI_Obj {
    //TODO
};
struct UI_FollowRiderWidget : public GUI_Obj {
    //TODO
};
struct UI_GenderSelectScreen : public GUI_Obj {
    //TODO
};
struct UI_GoalMetNotification : public GUI_Obj, public intUINotificationHandler {
    //TODO
};
struct UI_Goals : public GUI_Obj {
    //TODO
};
struct UI_GroupEvent_Entry : public GUI_Obj, public NoCopy {
    //TODO
};
struct UI_GroupEvents : public GUI_Obj, public I_GUI_ListBoxDelegate, public EventObject, public NoCopy {
    //TODO
};
struct UI_GroupRideFenceFlyerHUD : public XUX_Object {
    //TODO
};
struct UI_GroupRideFenceHUD : public XUX_Object {
    //TODO
};
struct UI_Leaderboards : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_LoginDialog : public GUI_Obj {
    //TODO
};
struct UI_Map3DBase : public GUI_Obj {
    //TODO
};
struct UI_Map3DControl : public UI_Map3DBase {
    //TODO
};
struct UI_Map3DPath : public UI_Map3DBase {
    //TODO
};
struct UI_MapControl : public GUI_Obj {
    //TODO
};
struct UI_MiniProfileScreen : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_NewGoalWindow : public GUI_Obj, public I_GUI_CheckBoxDelegate {
    //TODO
};
struct UI_NoHUDIntro : public GUI_Obj {
    //TODO
};
struct UI_NotableMomentWidget : public GUI_Obj {
    //TODO
};
struct UI_NotificationView : public GUI_Obj {
    //TODO
};
struct UI_NotificationWidget : public GUI_Obj {
    //TODO
};
struct UI_OnRoadTutorial : public GUI_Obj {
    //TODO
};
struct UI_OnboardingCustomizationView : public GUI_Obj {
    //TODO
};
struct UI_OnboardingGoals : public GUI_Obj {
    //TODO
};
struct UI_Onboarding_ContinueYourTraining : public XUX_Object {
    //TODO
};
struct UI_Onboarding_IntroToTrainingPlans : public XUX_Object {
    //TODO
};
struct UI_Onboarding_ZCAadvert : public XUX_Object {
    //TODO
};
struct UI_PacerBot : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_RatingSubmissionController : public GUI_Obj, public I_GUI_EditBoxDelegate {
    //TODO
};
struct UI_PacerBotSurvey : public UI_RatingSubmissionController {
    //TODO
};
struct UI_PairingScreen : public GUI_Obj, public I_GUI_CheckBoxDelegate, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_PauseMinimize : public GUI_Obj {
    //TODO
};
struct UI_PauseScreenDialog : public GUI_Obj, public I_GUINavigationGraphDelegate {
    //TODO
};
struct UI_PerformanceIncreaseLineItem : public GUI_Obj {
    //TODO
};
struct UI_PerformanceIncreaseNotification : public GUI_Obj {
    //TODO
};
struct UI_PostRideStats : public GUI_Obj, public I_GUI_CheckBoxDelegate, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_PowerUserLevelingSelector : public GUI_Obj {
    //TODO
};
struct UI_ProfileScreen : public GUI_Obj, public I_GUI_CheckBoxDelegate, public I_GUI_ListBoxDelegate, public I_GUI_EditBoxDelegate {
    //TODO
    struct Tint : public GUI_Obj {
        //TODO
    };
};
struct UI_QuitDialog : public GUI_Obj, public I_GUI_CheckBoxDelegate {
    enum DIALOG_RESULTS { DR_OK /*TODO*/ };
    GUI_EditBox *m_editBox;
    void (*m_onCancel)(int64_t) = nullptr;
    inline static bool s_bQuitFromZC;
    void FinalizeDontSaveAndQuit(/*void **/);
    //TODO
    /*
UI_QuitDialog::BuildData(void)
UI_QuitDialog::CheckBoxInFocus(GUI_Obj *)
UI_QuitDialog::Checked(GUI_CheckBox *,bool,void *)
UI_QuitDialog::GetDefaultHeight(void)
UI_QuitDialog::GetDefaultWidth(void)
UI_QuitDialog::InitWidget(void)
UI_QuitDialog::IsReturnToDropInEnabled(bool)
UI_QuitDialog::OnButtonCancel(void *)
UI_QuitDialog::OnButtonDontSaveAndQuit(void *)
UI_QuitDialog::OnButtonLocalSaveAndReturn(void *)
UI_QuitDialog::OnButtonSaveAndQuit(void *)
UI_QuitDialog::OnConfirmDontSaveAndQuit(MessageBoxResults)
UI_QuitDialog::OnKey(int,int)
UI_QuitDialog::OnMouseDown(float,float)
UI_QuitDialog::OnMouseMove(float,float)
UI_QuitDialog::OnMouseUp(float,float)
UI_QuitDialog::OnPrivacyPressed(void *)
UI_QuitDialog::Render(void)
UI_QuitDialog::RenderPrivacyButton(GUI_Button *)
UI_QuitDialog::RenderServices(GUI_Image *,float,float,float,float,void *)
UI_QuitDialog::UI_QuitDialog(float,float,void (*)(UI_QuitDialog::DIALOG_RESULTS))
UI_QuitDialog::Update(float)
UI_QuitDialog::~UI_QuitDialog()*/
};
void OnQuit(UI_QuitDialog::DIALOG_RESULTS);
struct UI_IOS_QuitScreen : public GUI_Obj, public I_GUI_EditBoxDelegate {
    static void OnSavePressed(void *quitDialog);
    /*UI_IOS_QuitScreen::EditBoxInFocus(GUI_Obj *)
UI_IOS_QuitScreen::EditDone(char const*,GUI_Obj *)
UI_IOS_QuitScreen::InitializePartnerConnections(void)
UI_IOS_QuitScreen::KeyDelete(char const*,GUI_Obj *)
UI_IOS_QuitScreen::KeyInput(char const*,GUI_Obj *)
UI_IOS_QuitScreen::OnCancelPressed(void *)
UI_IOS_QuitScreen::OnConfirmDontSaveAndQuit(MessageBoxResults)
UI_IOS_QuitScreen::OnDiscardConfirmed(void *)
UI_IOS_QuitScreen::OnDiscardPressed(void *)
UI_IOS_QuitScreen::OnLocalSaveAndReturnPressed(void *)
UI_IOS_QuitScreen::OnPrivacyPressed(void *)
UI_IOS_QuitScreen::Render(void)
UI_IOS_QuitScreen::SetPrivacy(UI_IOS_QuitScreen::PRIVACY_SETTING)
UI_IOS_QuitScreen::SetShouldPlayTutorial(void)
UI_IOS_QuitScreen::UI_IOS_QuitScreen(void (*)(UI_IOS_QuitScreen::DIALOG_RESULTS))
UI_IOS_QuitScreen::Update(float)
UI_IOS_QuitScreen::~UI_IOS_QuitScreen()*/
};
struct UI_RPERatingController : public GUI_Obj {
    //TODO
};
struct UI_RaceResults : public GUI_Obj, public I_GUI_ListBoxDelegate, public I_GUI_CheckBoxDelegate, public I_GUI_ScrollBarDelegate {
    //TODO
    struct RaceResultEntry : public GUI_Obj {
        //TODO
    };
};
struct UI_RaceRulesView : public GUI_Obj {
    //TODO
};
struct UI_RepackRidgeMenu : public GUI_Obj {
    //TODO
};
struct UI_RouteWidget : public GUI_Obj {
    //TODO
};
struct UI_RunFitTestResults : public XUX_Object {
    //TODO
};
struct UI_RunningPaceControl : public GUI_Obj, public I_GUI_EditBoxDelegate {
    //TODO
};
struct UI_Scotty : public GUI_Obj {
    //TODO
};
struct UI_ScottyCallbackReceiver {
    //TODO
};
struct UI_ScottyPromptSystem : public GUI_Obj, public UI_ScottyCallbackReceiver {
    //TODO
};
struct UI_ScreenshotSelectorController : public GUI_Obj {
    //TODO
};
struct UI_SegmentSplitView : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_SelectRouteList : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_SendAreaTextDialog : public GUI_Obj {
    enum DIALOG_RESULTS { DR_0 };
    //TODO
};
struct UI_SkillSelect : public GUI_Obj {
    //TODO
};
struct UI_SkillSelectListItem : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_SlideShow : public GUI_Obj {
    //TODO
};
struct UI_SportSelectController : public GUI_Obj {
    //TODO
};
struct UI_SteeringControls : public GUI_Obj {
    //TODO
};
struct UI_SteeringPairingWidget : public UI_DevicePairingWidget {
    //TODO
};
struct UI_SteeringSurvey : public UI_RatingSubmissionController {
    //TODO
};
struct UI_SummaryBlock : public GUI_Obj {
    //TODO
};
struct UI_TechnicalTerminologyHelpBox : public GUI_Obj {
    //TODO
};
struct UI_TimeSplitView : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
};
struct UI_TrainerSelection : public GUI_Obj, public I_GUI_ScrollBarDelegate {
    //TODO
};
struct TrainingPlan;
struct UI_TrainingPlanEnrollPopup : public GUI_Obj, public I_GUI_SliderDelegate {
    //TODO
    struct Callbacks {
        //TODO
    };
    UI_TrainingPlanEnrollPopup(TrainingPlan *, UI_TrainingPlanEnrollPopup::Callbacks *) {
        //TODO
    }
    void Release(GUI_Obj *, float) {
        //TODO
    }
    void SliderClick(GUI_Obj *) {
        //TODO
    }
    void SliderInFocus(GUI_Obj *) {
        //TODO
    }
};
struct UI_TrainingPlanEntry : public GUI_Obj, public I_GUI_ListBoxDelegate, public UI_TrainingPlanEnrollPopup::Callbacks {
    //TODO
};
struct UI_TrainingPlanSummaryCard : public GUI_Obj {
    //TODO
};
struct UI_TrainingPlan_Nextup : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
    struct NextupListEntry : public GUI_Obj {
        //TODO
    };
    struct NextupWeekSeparator : public GUI_Obj {
        //TODO
    };
};
struct UI_TrainingPlan_StartActivity : public GUI_Obj, public I_GUI_ListBoxDelegate {
    //TODO
    struct StartActivityListEntry : public GUI_Obj {
        //TODO
    };
    struct StartActivity_EventSubstitution_ListEntry : public XUX_Object {
        //TODO
    };
};
struct UI_TrainingPlan_View : public GUI_Obj, public I_GUI_ListBoxDelegate, public I_GUI_ScrollBarDelegate, public GUIOBJ_TrainingPlanViewer::CallbackListener {
    //TODO
};
struct UI_TrialNagScreen : public GUI_Obj {
    //TODO
};
struct UI_TrialEndScreen : public UI_TrialNagScreen {
    //TODO
};
struct UI_TutorialScreens : public GUI_Obj {
    //TODO
};
struct UI_TwoButtonsDialog : public GUI_Obj {
    enum class DIALOG_RESULTS { OK, CANCEL };
    //TODO
};
void kickCallback(UI_TwoButtonsDialog::DIALOG_RESULTS dr);
void GUI_CreateTwoButtonsDialog(const char *, const char *, const char *, const char *, const char *, float, std::function<void(UI_TwoButtonsDialog::DIALOG_RESULTS)>, float, float, bool, float, bool);
void GUI_CreateTwoButtonsDialog(const char *, const char *, const char *, const char *, const char *, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS), float, float, bool, float, bool);
void GUI_CreateTwoButtonsDialog(const char *, const char *, const char *, int, const char *, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS), float, float, bool, float, bool);
void GUI_CreateTwoButtonsDialog(float, float, const char *, const char *, const char *, const char *, float, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS));
void GUI_CreateTwoButtonsDialogWithHyperlink(const char *, void (*)(void *), const char *, const char *, const char *, const char *, const char *, float, void (*)(UI_TwoButtonsDialog::DIALOG_RESULTS), float, float, bool, float, bool);
struct UI_HyperlinkDialog : public UI_TwoButtonsDialog {
    //TODO
};
void GUI_Initialize(/*void (*func)(const char *),*/ bool testServer);
struct UI_UnlockNotification : public GUI_Obj, public intUINotificationHandler {
    //TODO
};
struct UI_VideoPlayer : public GUI_Obj {
    //TODO
};
struct UI_WorkoutCreator : public GUI_Obj, public I_GUI_ListBoxDelegate, public I_GUI_CheckBoxDelegate {
    //TODO
};
struct UI_WorkoutSelection : public GUI_Obj, public I_GUI_ListBoxDelegate, public I_GUI_SliderDelegate, public I_GUI_EditBoxDelegate, public GUIOBJ_TrainingPlanViewer::CallbackListener, public UI_TrainingPlanEnrollPopup::Callbacks {
    //TODO
};
struct UI_ZMLAdvertView : public GUI_Obj, public I_GUI_CheckBoxDelegate {
    //TODO
};
struct UI_ZwiftDropInWidget : public GUI_Obj {
    //TODO
};
struct UI_ZwiftProgramsItem : public GUI_Obj {
    //TODO
};
inline std::unique_ptr<UI_AudioControl> g_pAudioController;
inline UI_FriendsList *g_friendsListGUIObj;
struct strUID_EVENT_FINISHED {
    std::string m_field_0;
    void *m_field_20;
    int m_20;
    double m_field_30;
    float m_field_38;
    int m_field_3C;
    float m_field_40;
};
inline float g_3DTVEnabledDisplayNotificationTimer;
namespace UI_Refactor {
namespace temp {
    void PairingListDialogConfirmFromNewHomeScreen(uint32_t);
}
}
void OnFinishAreaText(UI_SendAreaTextDialog::DIALOG_RESULTS);
void OnApplyPromoCode(UI_ApplyPromoCode::DIALOG_RESULTS);
void CustomizationDialogConfirm();
inline bool g_bDropInAfterCustomization = true;
bool SelectBranch(uint32_t, bool, bool, bool, bool);
struct UI_NotificationQueue {
    static void QueueGoalMilestone(uint32_t) { //TODO
    }
        /*OnNotificationFinished(intUINotificationHandler *)
    QueueAchievement(uint)
    QueueChallengeComplete(UI_Challenge *)
    QueueUnlock(uint)
    QueueUnlockChallenge(UI_Challenge *)
    UI_NotificationQueue(void)
    Update(float)
    ~UI_NotificationQueue()*/
};
