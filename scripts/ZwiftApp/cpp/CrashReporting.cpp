#include "ZwiftApp.h" //READY for testing (omitted)
CrashReporting::CrashReporting(EventSystem *ptr) {
    //��� �������������� (EventSystem::SubscribeObj) ��������� ������� - ������, ������������� ��� ���������.
    //����� ��� ���������, ��� �� ������ ZwifApp, � ��������� ��������� �������.
    //��������, ���� �� ������� ����� ������ - ��������� ZwiftWindowsCrashHandler � �������� ��� ������� �������.
}
CrashReporting::~CrashReporting() {
    //������������� ������� �� ������������ � ������ ������� - �������, �� ������
}
void CrashReporting::Initialize(EventSystem *ptr) {
    g_CrashReportingUPtr.reset(new CrashReporting(ptr));
}
void CrashReporting::HandleEvent(EVENT_ID e, va_list va) {
    //OMIT
}