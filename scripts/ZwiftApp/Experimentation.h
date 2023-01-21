#pragma once

enum FeatureID { FID_PPDATA=0x1D, FID_RLOG=0x1E, FID_FQUERY=0x1F, FID_VID_CAP=0x3D, FID_SERV_PC=0x65, FID_HIDE_HW=0x2E, FID_HIDE_HD=0x2F, FID_FIXLEAD=0x30, FID_REND_P=0x31, 
    FID_AVOIDAB=0x32, FID_INTEGRA=0x33, FID_USERELA=0x34, FID_IGNOREP=0x35, FID_CLEARPO=0x37, FID_ENABLEA=0x36, FID_WNS=0x64, FID_ENABLEJ=0x62, FID_MRB_V2 =0x67, FID_NINETYD=0, FID_DRAFTLO=1,
    FID_PPV=2, FID_ASSERTD=3, FID_NOESISE=4, FID_SETTING=5, FID_HOMESCR=6, FID_PERFANA=7, FID_FORYOUC=8, FID_SYSTEMA=9, FID_BLEALT=0xA, FID_TDFFEM=0xB, FID_BLEDISA=0xC, FID_NEWWELC=0xD, FID_HOLIDAY=0xE, 
    FID_PACKDY=0xF, FID_OSXMON=0x10, FID_DEFAUL=0x11, FID_USETABB=0x12, FID_HWEXPER=0x13, FID_PAIREDD=0x14, FID_REALTIM=0x15, FID_BLEDATA=0x16, FID_MIX=0x17, FID_ADDDEVI=0x18, FID_CLICKAB=0x19, 
    FID_ENABLE=0x1A, FID_HUDHIG=0x1B, FID_GPUDEP=0x1C, FID_FTMS=0x20, FID_BLEDLLV=0x21, FID_ALLOWUT=0x22, FID_FTMSSP=0x23, FID_FTMSBIK=0x24, FID_LOG_BLE=0x25, FID_WAHOOD=0x26, FID_WAP=0x27, 
    FID_WBE=0x28, FID_UNIVERS=0x29, FID_BLE_EXC=0x2A, FID_DWOW=0x2B, FID_DRWC=0x2C, FID_PART=0x2D, FID_URR=0x38, FID_WDCERRO=0x39, FID_EXTENDE=0x3A, FID_FLATGRA=0x3B, 
    FID_ANDROID=0x3C, FID_WD=0x3E, FID_CLUBATT=0x3F, FID_FIXTRAI=0x40, FID_EVENTSU=0x41, FID_BLEMIDD=0x42, FID_MEMORAB=0x43, FID_ATVREMO=0x44, FID_HARDWAR=0x45, FID_GOALNOT=0x46, FID_HUBFIRM=0x47, 
    FID_FSF=0x48, FID_DYNAMIC=0x49, FID_BOUT=0x4A, FID_KOUT=0x4B, FID_GETSTAR=0x4C, FID_EVENTCO=0x4D, FID_FTMSROA=0x4E, FID_WT2=0x4F, FID_TRAININ=0x50, 
    FID_HANDCYC=0x51, FID_EVENTCA=0x52, FID_SETTHRE=0x53, FID_EVENTQU=0x54, FID_BLECYCL=0x55, FID_NEWLOCA=0x56, FID_FIXSTUC=0x57, FID_CLIENTR=0x58, FID_GWP=0x59, FID_LEVEL60=0x5A, 
    FID_PLAYERH=0x5B, FID_POS_G=0x5C, FID_POS_L=0x5D, FID_LO_WM=0x5E, FID_PACKDYN=0x5F, FID_CAMPAIG=0x60, FID_ENABLES=0x61, FID_SETDEFA=0x63, FID_PD4=0x66, FID_CNT=104 };

struct FeatureMetadata { //40 bytes
	FeatureID m_id;
	const char *m_name;
	int32_t m_unk[5];
	//int m_notused; always 0
};
extern const FeatureMetadata g_featureMetadata[];
struct Feature {
	FeatureID m_id;
	FeatureID ToIndex() { return m_id; }
	const char *c_str();
	static const char *c_str(FeatureID id);
};

class ZwiftDispatcher { //size=8
	char data[8];
public:
	ZwiftDispatcher();
	void Assert(bool bPredicate);
	void Assert(bool bPredicate, const char *errMsg);
};
class FeatureStateMachine { //size=72
public:
	template<class T> T GetFeatureVariable(const std::string &name) {
	}
};

class Experimentation : public EventObject { //sizeof=0x1E48
	FeatureStateMachine m_fsms[FID_CNT];
public:
	static void Initialize(EventSystem *ev);
	Experimentation(EventSystem *ev);
	void HandleEvent(EVENT_ID e, va_list args) override;
	virtual ~Experimentation() { /*todo*/ }
	bool IsEnabled(FeatureID) { return false; }
	//todo bool IsEnabled(FeatureID, Experiment::Variant)
    //todo bool IsEnabled(Experiment::Feature, std::function<void()(Experiment::Variant)>)
	void HandleLogout();
	//todo void Unregister(Experiment::CallbackID<Experiment::Feature>)
	bool GetBool(FeatureID, std::string const &);
	int64_t GetInt64(FeatureID, std::string const &);
	double GetDouble(FeatureID, std::string const &);
	std::string GetString(FeatureID, std::string const &);
	void SetEventTypeAttribute(std::string_view);
};

extern std::unique_ptr<Experimentation> g_sExperimentationUPtr;