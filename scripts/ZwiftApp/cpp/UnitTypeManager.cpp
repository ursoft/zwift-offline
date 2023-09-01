#include "ZwiftApp.h" //READY for testing
UnitTypeManager::UnitTypeManager(EventSystem *ev, bool readonly) : EventObject(ev), m_readonly(readonly) {
    ev->Subscribe(EV_33, this);
}
void UnitTypeManager::SetUseMetric(bool m) {
    m_unitType = m ? UT_METRIC : UT_IMPERIAL;
    if (!m_readonly)
        g_UserConfigDoc.SetBool("ZWIFT\\CONFIG\\METRIC", m);
    if (BikeManager::Instance()) {
        auto mainBike = BikeManager::Instance()->m_mainBike;
        if (mainBike) {
            if (mainBike->m_profile.use_metric() != m) {
                mainBike->m_profile.set_use_metric(m);
                if (!m_readonly && mainBike->m_writable)
                    mainBike->SaveProfile(true, false);
            }
        }
    }
    m_eventSystem->TriggerEvent(EV_SENS_RECONN, 0);
}
void UnitTypeManager::Initialize(EventSystem *ev, bool b) { g_pUnitTypeManagerUPtr.reset(new UnitTypeManager(ev, b)); }
void UnitTypeManager::HandleEvent(EVENT_ID e, va_list) {
    if (e == EV_33 && BikeManager::Instance()) {
        auto mainBike = BikeManager::Instance()->m_mainBike;
        if (mainBike)
            SetUseMetric(mainBike->m_profile.use_metric());
    }
}
UnitTypeManager::~UnitTypeManager() { m_eventSystem->Unsubscribe(EV_33, this); }
