#include "ZwiftApp.h"
PlayerProfileCache::PlayerProfileCache(EventSystem *ev) : EventObject(ev) {
    ev->Subscribe(EV_RESET, this);
}
void PlayerProfileCache::Initialize(EventSystem *ev) { g_pPlayerProfileCache.reset(new PlayerProfileCache(ev)); }
void PlayerProfileCache::HandleEvent(EVENT_ID e, va_list) { if (e == EV_RESET) LoadCache(); }
PlayerProfileCache::~PlayerProfileCache() { 
    ClearCache(); 
    m_eventSystem->Unsubscribe(EV_RESET, this); 
    //TODO field28
}
void PlayerProfileCache::ClearCache() { DeallocatePlayerCacheList(&m_list); }
void PlayerProfileCache::ClearSavedCache() {
    auto i = BikeManager::Instance();
    if (i)
        i->m_pah->StoreValue("PLAYER_CACHE_BLOB", "000000");
}
void PlayerProfileCache::AllocateEnrolledFriendProfiles(std::list<CachedProfile *> *dest, uint32_t id) {
    DeallocatePlayerCacheList(dest);
    for (auto psrc : m_list) {
        if (psrc->m_trp_id == id) {
            auto copy = new CachedProfile();
            *copy = *psrc;
            dest->push_back(copy);
        }
    }
}
void PlayerProfileCache::DeallocatePlayerCacheList(std::list<CachedProfile *> *list) {
    for (auto psrc : *list) {
        if (psrc)
            delete psrc;
    }
    list->clear();
}
void PlayerProfileCache::LoadCache() {
    DeallocatePlayerCacheList(&m_list);
    PlayerProfileCache::DeallocatePlayerCacheList(&this->m_list);
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (!ZNETWORK_IsLoggedIn() || !mainBike)
        return;
    auto blob = BikeManager::Instance()->m_pah->GetStringValue( SIG_CalcCaseInsensitiveSignature("PLAYER_CACHE_BLOB"), "");
    uint32_t v14;
    std::stringstream sblob(blob);
    PrivateAttributesHelper::Parse(sblob, &v14);
    if (v14 == 7) {
        PrivateAttributesHelper::Parse(sblob, &m_socialUpdTime);
        uint8_t count;
        PrivateAttributesHelper::Parse(sblob, &count);
        if (count) {
            for (int i = 0; i < count; i++) {
                auto cp = new CachedProfile();
                cp->Parse(sblob);
                m_list.push_back(cp);
            }
        } else {
            m_socialUpdTime = 1;
        }
    } else {
        ClearSavedCache();
    }
    if (!m_socialUpdTime)
        m_socialUpdTime = 1;
}
void PlayerProfileCache::SaveCache() { //??? NOT FOUND in IDA
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (!mainBike) return;
    std::stringstream sblob;
    PrivateAttributesHelper::Serialize(sblob, (uint32_t)7LL);
    PrivateAttributesHelper::Serialize(sblob, m_socialUpdTime);
    uint8_t count = (uint8_t)std::min((size_t)100, m_list.size());
    PrivateAttributesHelper::Serialize(sblob, count);
    for (auto i : m_list) {
        i->Serialize(sblob);
        if (--count == 0)
            break;
    }
    if (ZNETWORK_IsLoggedIn())
        BikeManager::Instance()->m_pah->StoreValue("PLAYER_CACHE_BLOB", sblob.str().c_str());
}
void PlayerProfileCache::TestCache() {
    //TODO - not used unit test?
}
void PlayerProfileCache::UpdateCache(float dt) {
    if (ZNETWORK_IsLoggedIn()) {
        auto NetworkSyncedTimeGMT = ZNETWORK_GetNetworkSyncedTimeGMT();
        if (m_socialActual) {
            if (m_secsUntilSocialUpdate > 0.0) {
                m_secsUntilSocialUpdate -= dt;
                if (m_secsUntilSocialUpdate <= 0.0)
                    m_socialActual = false;
            } else {
                //TODO
                //v48[0] = 0i64;
                if (true/* TODO !sub_7FF658C9E0A0((__int64)&this_->field_28, v48)*/) {
                    int httpCode = -1; //TODO:inside struct
                    //TODO sub_7FF658A95670((__int64 *)&this_->field_28, &httpCode);// std::__assoc_state<std::shared_ptr<zwift_network::NetworkResponse<zwift::protobuf::PlayerProfiles> const>>::move
                    if (httpCode) {
                        if (httpCode == 404) {
                            m_socialActual = false;
                            m_socialUpdTime = NetworkSyncedTimeGMT;
                        } else {
                            m_secsUntilSocialUpdate = 600.0;
                        }
                    } else {
                        PlayerProfileCache::DeallocatePlayerCacheList(&m_list);
                        auto tph_crc = SIG_CalcCaseInsensitiveSignature("TRAINING_PLAN_HASH");
                        auto tps_crc = SIG_CalcCaseInsensitiveSignature("TRAINING_PLAN_START");
                        auto tpe_crc = SIG_CalcCaseInsensitiveSignature("TRAINING_PLAN_END");
                        protobuf::PlayerProfiles pps/* TODO (*(_QWORD *)(v49 + 40), v8, v9)*/;
                        for (auto &pp : pps.profiles()) {
                            int trp_id = 0;
                            uint64_t tps_time = 0, tpe_time = 0;
                            for (auto &pa : pp.private_attributes()) {
                                auto paid = pa.id();
                                if (paid == tph_crc) {
                                    trp_id = pa.number_value();
                                } else if (paid == tps_crc) {
                                    tps_time = pa.number_value();
                                } else if (paid == tpe_crc) {
                                    tpe_time = pa.number_value();
                                }
                            }
                            if (trp_id && tpe_time >= NetworkSyncedTimeGMT) {
                                auto cp = new CachedProfile();
                                //TODO
                                //v48[1] = (__int64)&v52;
                                //arr_str_construct(&v52, (std::string *)(v67 & 0xFFFFFFFFFFFFFFF8ui64));
                                //v20 = v19;
                                //arr_str_construct((std::string *)v53, (std::string *)(v66 & 0xFFFFFFFFFFFFFFF8ui64));
                                cp->m_str0 = "TODO str0"; // ??? plan name and description ???
                                cp->m_str20 = "TODO str20";
                                cp->m_trp_id = trp_id;
                                cp->m_trp_start = tps_time;
                                cp->m_trp_end = tpe_time;
                                m_list.push_back(cp);
                            }
                            m_socialActual = false;
                            m_socialUpdTime = ZNETWORK_GetNetworkSyncedTimeGMT();
                        }
                    }
                    /*v25 = v50;
                    if (v50 && _InterlockedExchangeAdd(v50 + 2, 0xFFFFFFFF) == 1)// std::__shared_weak_count::__release_weak
                    {
                        (**(void(__fastcall ***)(volatile signed __int32 *))v25)(v25);
                        if (_InterlockedExchangeAdd(v25 + 3, 0xFFFFFFFF) == 1)
                            (*(void(__fastcall **)(volatile signed __int32 *))(*(_QWORD *)v25 + 8i64))(v25);
                    }*/
                }
            }
        } else {
            auto mainBike = BikeManager::Instance()->m_mainBike;            
            if (/*TODO (_QWORD *)&m_mainBike->gap13F8[248]*/ true) {
                if (m_socialUpdTime) {
                    if (int(NetworkSyncedTimeGMT - m_socialUpdTime) > 86400) {
                        /* TODO v29 = *(_QWORD *)(*(_QWORD *)&m_mainBike->gapC65[107] + 280i64);
                        v30 = v29 < 0
                            ? (float)(v29 & 1 | (unsigned int)((unsigned __int64)v29 >> 1))
                            + (float)(v29 & 1 | (unsigned int)((unsigned __int64)v29 >> 1))
                            : (float)(int)v29;
                        if ((float)(v30 * 0.001) > 300.0 && *(int *)&m_mainBike[1].gap0[335] > 0) {
                            sub_7FF658A18C30((__int64)&v54);
                            if (*(int *)&m_mainBike[1].gap0[335] > 0) {
                                v31 = 0i64;
                                do {
                                    v32 = *(_QWORD *)(v31 + *(_QWORD *)&m_mainBike[1].gap0[343] + 8);
                                    v33 = &ProfileSocialFacts_default_instance_;
                                    if (*(_QWORD *)(v32 + 40))
                                        v33 = *(void ****)(v32 + 40);
                                    ProfileSocialFacts::ProfileSocialFacts((__int64)v60, (__int64)v33);
                                    this__ = v61;
                                    sub_7FF658A1BD00((__int64)&v54, (__int64)v51, (unsigned __int8 *)&this__);// std::__hash_table<long,std::hash<long>,std::equal_to<long>,std::allocator<long>>::__emplace_unique_key_args<long,long>
                                    ProfileSocialFacts_dtr(v60);
                                    ++v3;
                                    v31 += 8i64;
                                } while (v3 < *(_DWORD *)&m_mainBike[1].gap0[335]);
                            }
                            if (*(_QWORD *)p1) {
                                v34 = (__int64 *)zwift_network::profiles((__int64)&p_m_list, (__int64)&v54);
                                sub_7FF658A5ADE0((__int64 *)&this_->field_28, v34);
                                v37 = (__int64)p_m_list;
                                if (p_m_list && _InterlockedExchangeAdd((volatile signed __int32 *)&p_m_list->_Mysize, 0xFFFFFFFF) == 1) {
                                    v38 = *(void(__fastcall ****)(_QWORD, __int64))(v37 + 216);
                                    if (v38)
                                        (**v38)(*(_QWORD *)(v37 + 216), v37);
                                    else
                                        (**(void(__fastcall ***)(__int64, __int64))v37)(v37, 1i64);
                                }
                                m_socialActual = true;
                                Log("Player Cache: requesting %d profiles", p1[0], v35, v36);
                            }
                            sub_7FF658A16680(&v57);
                            std::list_dtr(&v55);
                        }*/
                    }
                }
            }
        }
    }
}
void CachedProfile::Clear() {
    m_trp_id = 0;
    m_trp_start = 0;
    m_trp_end = 0;
    m_str0.clear();
    m_str20.clear();
}
void CachedProfile::Parse(std::stringstream &stream) {
    Clear();
    PrivateAttributesHelper::Parse(stream, &m_str0);
    PrivateAttributesHelper::Parse(stream, &m_str20);
    PrivateAttributesHelper::Parse(stream, &m_trp_id);
    PrivateAttributesHelper::Parse(stream, &m_trp_start);
    PrivateAttributesHelper::Parse(stream, &m_trp_end);
}
void CachedProfile::Serialize(std::stringstream &dest) const {
    PrivateAttributesHelper::Serialize(dest, m_str0);
    PrivateAttributesHelper::Serialize(dest, m_str20);
    PrivateAttributesHelper::Serialize(dest, m_trp_id);
    PrivateAttributesHelper::Serialize(dest, m_trp_start);
    PrivateAttributesHelper::Serialize(dest, m_trp_end);
}
//void CachedProfile::Set(const CachedProfile &) {
    //TODO
//}
//void CachedProfile::Set(const std::string &str0, const std::string &str20, uint32_t id, uint64_t field_48, uint64_t field_50) {
//    m_str0 = str0;
//    m_str20 = str20;
//    m_trp_id = id;
//    m_field_48 = field_48;
//    m_trp_end = field_50;
//}
bool PrivateAttributesHelper::ClearValue(const char *name) { return ClearValue(SIG_CalcCaseInsensitiveSignature(name)); }
bool PrivateAttributesHelper::ClearValue(uint32_t nameCrc) {
    for (auto it = m_be->m_profile.private_attributes().begin(); it != m_be->m_profile.private_attributes().end(); it++) {
        if (it->id() == nameCrc) {
            m_be->m_profile.mutable_private_attributes()->erase(it);
            return true;
        }
    }
    return false;
}
float PrivateAttributesHelper::GetFloatValue(const char *name, float def) { return GetFloatValue(SIG_CalcCaseInsensitiveSignature(name), def); }
float PrivateAttributesHelper::GetFloatValue(uint32_t nameCrc, float def) {
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc)
            return i.float_value();
    }
    return def;
}
int64_t PrivateAttributesHelper::GetIntValue(const char *name, int64_t def) { return GetIntValue(SIG_CalcCaseInsensitiveSignature(name), def); }
int64_t PrivateAttributesHelper::GetIntValue(uint32_t nameCrc, int64_t def) {
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc)
            return i.number_value();
    }
    return def;
}
std::string PrivateAttributesHelper::GetStringValue(const char *name, const std::string &def) { return GetStringValue(SIG_CalcCaseInsensitiveSignature(name), def); }
std::string PrivateAttributesHelper::GetStringValue(uint32_t nameCrc, const std::string &def) {
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc)
            return i.string_value();
    }
    return def;
}
bool PrivateAttributesHelper::IsValuePresent(const char *name) {
    auto nameCrc = SIG_CalcCaseInsensitiveSignature(name);
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc)
            return true;
    }
    return false;
}
bool PrivateAttributesHelper::IsValuePresent(const char *name, float *dest) {
    auto nameCrc = SIG_CalcCaseInsensitiveSignature(name);
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc) {
            *dest = i.float_value();
            return true;
        }
    }
    return false;
}
bool PrivateAttributesHelper::IsValuePresent(const char *name, int64_t *dest) {
    auto nameCrc = SIG_CalcCaseInsensitiveSignature(name);
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc) {
            *dest = i.number_value();
            return true;
        }
    }
    return false;
}
bool PrivateAttributesHelper::IsValuePresent(const char *name, std::string *dest) {
    auto nameCrc = SIG_CalcCaseInsensitiveSignature(name);
    for (auto &i : m_be->m_profile.private_attributes()) {
        if (i.id() == nameCrc) {
            *dest = i.string_value();
            return true;
        }
    }
    return false;
}
void PrivateAttributesHelper::Parse(std::stringstream &ss, std::string *s) {
    char nptr[3] = {};
    ss.read(nptr, 2);
    auto d = strtol(nptr, 0, 16);
    if (d) {
        auto str = (char *)calloc(d + 1, 1);
        ss.read(str, d);
        *s = str;
        free(str);
    }
}
void PrivateAttributesHelper::Parse(std::stringstream &s, uint8_t *d) {
    char nptr[3] = {};
    s.read(nptr, 2);
    *d = strtol(nptr, 0, 16);
}
void PrivateAttributesHelper::Parse(std::stringstream &s, uint32_t *d) {
    char buf[9] = {};
    s.read(buf, 8);
    *d = strtol(buf, 0, 16);
}
void PrivateAttributesHelper::Parse(std::stringstream &s, uint64_t *d) {
    char buf[17] = {};
    s.read(buf, 16);
    sscanf_s(buf, "%llx", d);
}
void PrivateAttributesHelper::Parse(std::stringstream &s, uint16_t *d) {
    char buf[5] = {};
    s.read(buf, 4);
    *d = strtol(buf, 0, 16);
}
void PrivateAttributesHelper::Serialize(std::stringstream &s, const std::string &d) {
    int stringSize = std::min(255, (int)d.length());
    Serialize(s, uint8_t(stringSize));
    s.write(d.c_str(), stringSize);
}
void PrivateAttributesHelper::Serialize(std::stringstream &s, uint8_t d) {
    char c = d / 16;
    if (c < 10) c += '0'; else c += ('A' - 10);
    s << c;
    c = d % 16;
    if (c < 10) c += '0'; else c += ('A' - 10);
    s << c;
}
void PrivateAttributesHelper::Serialize(std::stringstream &s, uint32_t d) {
    char buf[9] = {};
    sprintf_s(buf, "%08X", d);
    s.write(buf, 8);
}
void PrivateAttributesHelper::Serialize(std::stringstream &s, uint64_t d) {
    char buf[17] = {};
    sprintf_s(buf, "%16llX", d);
    s.write(buf, 16);
}
void PrivateAttributesHelper::Serialize(std::stringstream &s, uint16_t d) {
    char buf[5] = {};
    sprintf_s(buf, "%04X", d);
    s.write(buf, 4);
}
void PrivateAttributesHelper::StoreValue(const char *n, const char *v) { StoreValue(SIG_CalcCaseInsensitiveSignature(n), v); }
void PrivateAttributesHelper::StoreValue(const char *n, float v) { StoreValue(SIG_CalcCaseInsensitiveSignature(n), v); }
void PrivateAttributesHelper::StoreValue(const char *n, int64_t v) { StoreValue(SIG_CalcCaseInsensitiveSignature(n), v); }
void PrivateAttributesHelper::StoreValue(uint32_t nameCrc, const char *v) {
    for (auto &i : *m_be->m_profile.mutable_private_attributes()) {
        if (i.id() == nameCrc) {
            i.set_string_value(v);
            return;
        }
    }
}
void PrivateAttributesHelper::StoreValue(uint32_t nameCrc, float v) {
    for (auto &i : *m_be->m_profile.mutable_private_attributes()) {
        if (i.id() == nameCrc) {
            i.set_float_value(v);
            return;
        }
    }
}
void PrivateAttributesHelper::StoreValue(uint32_t nameCrc, int64_t v) {
    for (auto &i : *m_be->m_profile.mutable_private_attributes()) {
        if (i.id() == nameCrc) {
            i.set_number_value(v);
            return;
        }
    }
}
