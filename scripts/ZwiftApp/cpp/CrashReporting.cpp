#include "ZwiftApp.h" //READY for testing (omitted)
CrashReporting::CrashReporting(EventSystem *ptr) {
    //Тут регистрируются (EventSystem::SubscribeObj) несколько событий - видимо, крашрепортеру это интересно.
    //Также ему интересно, что за версия ZwifApp, и некоторые параметры системы.
    //Наверное, одна из функций этого класса - запустить ZwiftWindowsCrashHandler и снабдить его всякими данными.
}
CrashReporting::~CrashReporting() {
    //Дерегистрация событий из конструктора и прочая требуха - надеюсь, не нужная
}
void CrashReporting::Initialize(EventSystem *ptr) {
    g_CrashReportingUPtr.reset(new CrashReporting(ptr));
}
void CrashReporting::HandleEvent(EVENT_ID e, va_list va) {
    //OMIT
}