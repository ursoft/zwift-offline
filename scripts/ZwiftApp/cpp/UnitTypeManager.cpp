#include "ZwiftApp.h"
UnitTypeManager::UnitTypeManager(EventSystem *ev, bool readonly) : EventObject(ev), m_readonly(readonly) {
    ev->Subscribe(EV_33, this);
}
void UnitTypeManager::SetUseMetric(bool m) {
    m_unitType = m ? UT_METRIC : UT_IMPERIAL;
    if (!m_readonly)
        g_UserConfigDoc.SetBool("ZWIFT\\CONFIG\\METRIC", m, true);
    if (BikeManager::Instance()) {
        auto mainBike = BikeManager::Instance()->m_mainBike;
        if (mainBike) {
            if (mainBike->m_metricUnits != m) {
                mainBike->m_changeFlags |= BCH_UNITS;
                mainBike->m_metricUnits = m;
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
            SetUseMetric(mainBike->m_metricUnits);
    }
}
UnitTypeManager::~UnitTypeManager() { m_eventSystem->Unsubscribe(EV_33, this); }
