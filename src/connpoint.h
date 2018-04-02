#ifndef __DESA_CONNPOINT__
#define __DESA_CONNPOINT__

/**
 * Connection points
 */

#include <vector>
#include <map>
#include <mconnpoint.h>
#include "mdes.h"

namespace desa {

    class MState;


    /**
     * @brief Connection Point base class
     */
    class ConnPointBase: public MBase, public MConnPoint, public MCpClient
    {
	public:
	    friend class trOnDoConnect;
	public:
	    typedef vector<const MConnPoint*> TPairs;
	    static string type() { return "ConnPointBase";}
	    static const char* Type() { return "ConnPointBase";}
	    virtual MIface *DoGetObj(const char *aName) override;
	public:
	    ConnPointBase(const string& aName, TDir aDir, MBase* aCowner = nullptr);
	    virtual ~ConnPointBase();
	    // From MConnPoint, blocking
	    virtual TDir Dir() const { return mDir;};
	    virtual bool Connect(const MConnPoint& aCp);
	    virtual bool Disconnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	    virtual void OnPairChanged(MConnPoint* aPair);
	    virtual void OnMediatorChanged(MConnPoint* aMediator);
	    virtual bool IsConnected() const override;
	    virtual const MBase* MConnPoint_Base() const { return this;};
	    // From MConnPoint, non-blocking
	    virtual void dir(Tr<MConnPoint::TDir>* aTr) const override;
	    virtual void connect(const MConnPoint& aCp, Tr<bool>* aTr) override;
	    virtual void disconnect(const MConnPoint& aCp, MCpClient* aClient) override {};
	    virtual void isCompatible(const MConnPoint& aPair, bool aExtd, Tr<bool>* aCb) override;
	    //virtual void isCompatible(const MConnPoint& aPair, bool aExtd, void(*aCb)(bool)) const override;
	    virtual void onPairChanged(MConnPoint* aPair, MCpClient* aClient) override {};
	    virtual void onMediatorChanged(MConnPoint* aMediator, MCpClient* aClient) override {};
	    virtual void isConnected(MCpClient* aClient) const override {};
	    virtual void dump(MCpClient* aClient) const override {}
	    // From MCpClient
	    virtual void onDir(MConnPoint::TDir aDir) override {}
	    virtual void onConnected(bool aSuccess) override {}
	    virtual void onDisconnected(bool aSuccess) override {}
	    virtual void onIsCompatible(bool aCompatible) override {}
	    virtual void onIsConnected(bool aConnected) override {}
	    // From MBase
	    virtual const std::string GetType() const { return type();}
//	    virtual const std::string getUri() const;
	public:
	    // TODO To move to protected
	    virtual void Notify(MConnPoint* aExclude = NULL);
	protected:
	    virtual bool DoConnect(const MConnPoint& aCp);
	    virtual void doConnect(const MConnPoint& aCp, Tr<bool>* aNext);
	    //virtual void doConnect(const MConnPoint& aCp, std::function<void (bool)>* aHandler);
	    //void doConnect(const MConnPoint& aCp, void (aHandler)(ConnPointBase&, bool, std::function<void (bool)>*));
	    virtual bool DoDisconnect(const MConnPoint& aCp);
	    virtual void Dump() const override;
	    void onDoConnect(bool aRes, std::function<void (bool)>* aHandler);
	public:
	    TDir mDir;
	    TPairs mPairs;
    };

    /**
     * @brief Extention base
     *
     * This connection point allows to extend another connection point, for instance system's internal state
     * output to the system output.
     *
     */
    // TODO [YB] To desing templated Extention
    // TODO Extention with input direction has to support number of origins. It has acts as a coupler.
    class Extention: public ConnPointBase, public MExtension
    {
	public:
	    static string type() { return "Extention";}
	    Extention(const string& aName, MBase* aCowner, TDir aDir, ConnPointBase* aOrig):
		ConnPointBase(aName, aDir, aCowner), mOrig(aOrig) {}
	    virtual ~Extention();
	    //ConnPointBase& Orig() { return *mOrig; };
	    virtual const std::string GetType() const { return type();}
	    virtual MIface *DoGetObj(const char *aName) override;
	    // From MExtension
	    MConnPoint& Orig() const override { return *mOrig; };
	    virtual void orig(Tr<MConnPoint>*) const override {};
	    // From MConnPoint
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const override;
	protected:
	    // TODO Seems aExclude is not to be used. Remove it?
	    virtual void Notify(MConnPoint* aExclude = nullptr);
	    virtual void Dump() const override;
	protected:
	    ConnPointBase* mOrig; // Original conn point that is represented by extention, owned
    };


    /**
     * @brief Simple Connection Point
     *
     * ConnPoint contains cache of required interfaces implementation, i.e. the state can get from
     * input ConnPoint the set of MState interfaces of all connected state outputs
     * ConnPoint has it's direct pair but this direct pair can be "proxy" conn point, like extension
     * In that case ConnPoint communicates to proxy in order to get the interfaces.
     * This appoach allows to change connections net dynamically and refresh interfaces properly.
     */
    class ConnPoint: public ConnPointBase
    {
	public:
	    class JointPr: public MJointPr {
		public:
		JointPr(ConnPoint& aHost): mHost(aHost) {}
		ConnPoint& mHost;
		// From MJointPr
		virtual MIface *DoGetObj(const char *aName) override { return aName == MJointPr::type() ? this : nullptr;}
		virtual std::string ProvidedType() const override { return mHost.mProvidedType;}
		virtual void providedType(Tr<std::string>* aCb) const override { (*aCb)(mHost.mProvidedType);}
		virtual std::string RequiredType() const override { return mHost.mRequiredType;}
		virtual void requiredType(Tr<std::string>* aCb) const override { (*aCb)(mHost.mRequiredType);}
		virtual MIface& Provided() { return *mHost.mProvided;}
		virtual const MIface& Provided() const { return *mHost.mProvided;}
		virtual void provided(Tr<MIface>* aCb) { (*aCb)(*mHost.mProvided); }
		virtual TPairsIfaces& Required() { return mHost.mRequired;} 
		virtual const TPairsIfaces& Required() const { return mHost.mRequired;} 
		virtual void required(Tr<TPairsIfaces>*) override {}
	    };
	public:
	    ConnPoint(const string& aName, MBase* aCowner, TDir aDir, MIface* aProvided = NULL):
		ConnPointBase(aName, aDir, aCowner), mProvided(aProvided), mJointPr{*this} {}
	    MJointPr& GetJointPr() { return mJointPr;}
	    virtual MIface *DoGetObj(const char *aName) override;
	    // From MConnPoint
	    virtual bool DoDisconnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	    virtual void isCompatible(const MConnPoint& aPair, bool aExtd, Tr<bool>* aCb) override {
		auto* handler = new SIsCompatible(this, aPair, aExtd, aCb);
		(*handler)();
	    }
	    virtual void OnPairChanged(MConnPoint* aPair);
	    virtual bool IsConnected() const override;
	protected:
	    virtual bool DoConnect(const MConnPoint& aCp);
	    virtual void doConnect(const MConnPoint& aCp, Tr<bool>* aNext);
	    virtual void Dump() const override;
	    
	protected: // Non blocking operations
	    class SIsCompatible {
		private:
		    template<typename T> class Trl: public Tr<T> { public: Trl(SIsCompatible* aHost): mOc(*aHost) {} SIsCompatible& mOc; };
		    struct St1: public Trl<bool> { St1(SIsCompatible* aHost): Trl<bool>(aHost){} void operator()(const bool& a) override;};
		    struct St2: public Trl<MIface> { St2(SIsCompatible* aHost): Trl<MIface>(aHost){} void operator()(const MIface&) override;};
		    struct St3: public Trl<MIface> { St3(SIsCompatible* aHost): Trl<MIface>(aHost){} void operator()(const MIface&) override;};
		    struct St4: public Trl<string> { St4(SIsCompatible* aHost): Trl<string>(aHost){} void operator()(const string&) override;};
		    struct St5: public Trl<string> { St5(SIsCompatible* aHost): Trl<string>(aHost){} void operator()(const string&) override;};
		public:
		    SIsCompatible(ConnPoint* aC, const MConnPoint& aPair, bool aExtd, Tr<bool>* aCb):
			mGc(*aC),  mPair(const_cast<MConnPoint&>(aPair)), mExtd(aExtd), mCb(*aCb), mJp(nullptr),
			mStep1(this), mStep2(this), mStep3(this), mStep4(this), mStep5(this){}
		    void operator()();
		private: // Context
		    ConnPoint& mGc; // Global Context
		    MConnPoint& mPair;
		    bool mExtd;
		    Tr<bool>& mCb;
		    MJointPr* mJp;
		private: St1 mStep1; St2 mStep2; St3 mStep3; St4 mStep4; St5 mStep5;
	    };

	protected:
	    string mProvidedType;
	    string mRequiredType;
	    MIface* mProvided;
	    MJointPr::TPairsIfaces mRequired;
	    JointPr mJointPr;
    };

    /**
     * @brief Simple Connection Point only providing iface, no required iface needed
     */
    class ConnPointP: public ConnPointBase
    {
	public:
	    ConnPointP(const string& aName, TDir aDir, MIface* aProvided);
	    virtual MIface& Provided() { return *mProvided;};
	    virtual const MIface& Provided() const { return *mProvided;};
	protected:
	    MIface* mProvided;
    };


    /**
     * @brief Templated connection point with types of provided and required ifaces
     *
     */
    // TODO YB Can we use templated iface for it
    template<typename _Provided, typename _Required> class TConnPoint: public ConnPoint
    {
	typedef _Required TRequired;
	typedef _Provided TProvided;

	public:
	TConnPoint(const string& aName, MBase* aCowner, TDir aDir, TProvided* aProvided = NULL):
	    ConnPoint(aName, aCowner, aDir, aProvided) {
		this->mProvidedType =_Provided::type();
		this->mRequiredType = _Required::type();
	    }
    };

    class sIsCompatible_Cp6: public Stt<ConnPoint, MJointPr, bool, std::string> { public:
	sIsCompatible_Cp6(ConnPoint* ac, MJointPr& at, Tr<bool>& an): Stt<ConnPoint, MJointPr, bool, std::string>(ac, at, an) {};
	virtual void tr(const std::string& aI) override {
	    if (aI == c->GetJointPr().ProvidedType()) {
		n(true);
	    } else {
		n(false);
	    }
	}};

    class sIsCompatible_Cp5: public Stt<ConnPoint, MJointPr, bool, std::string> { public:
	sIsCompatible_Cp5(ConnPoint* ac, MJointPr& at, Tr<bool>& an): Stt<ConnPoint, MJointPr, bool, std::string>(ac, at, an) {};
	virtual void tr(const std::string& aI) override {
	    if (aI == c->GetJointPr().RequiredType()) {
		auto* cb = new sIsCompatible_Cp6(c, t, n);
		t.requiredType(cb);
	    } else {
		n(false);
	    }
	}};

    class sIsCompatible_Cp4: public St<ConnPoint, bool, MIface> { public:
	sIsCompatible_Cp4(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MIface>(ac, an) {};
	virtual void tr(const MIface& aI) override {
	    if (&aI) {
		MJointPr* jp = const_cast<MIface&>(aI).GetObj(jp);
		assert(jp);
		auto* cb = new sIsCompatible_Cp5(c, *jp, n);
		jp->providedType(cb);
	    } else {
		n(false);
	    }
	}};


    class sIsCompatible_Cp3: public St<ConnPoint, bool, MConnPoint> { public:
	sIsCompatible_Cp3(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MConnPoint>(ac, an) {};
	virtual void tr(const MConnPoint& aI) override {
	    if (&aI) {
		auto* cb = new sIsCompatible_Cp4(c, n);
		// Get MJointPr from origin
		MConnPoint& cp = const_cast<MConnPoint&>(aI);
		cp.doGetObj(MJointPr::type(), cb);
	    } else {
		n(false);
	    } }};


    class sIsCompatible_Cp2: public St<ConnPoint, bool, MIface> { public:
	sIsCompatible_Cp2(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MIface>(ac, an) {};
	virtual void tr(const MIface& aI) override {
	    if (&aI) {
		MExtension* ext = const_cast<MIface&>(aI).GetObj(ext);
		assert(ext);
		auto* cb = new sIsCompatible_Cp3(c, n);
		// Get origin from extension
		ext->orig(cb);
	    } else {
	    } }};


    class sIsCompatible_Cp1: public Stt<ConnPoint, MConnPoint, bool, bool> { public:
	sIsCompatible_Cp1(ConnPoint* ac, MConnPoint& at, Tr<bool>& an): Stt<ConnPoint, MConnPoint, bool, bool>(ac, at, an) {}
	virtual void tr(const bool& aI) override {
	    if (aI) {
		auto* cb = new sIsCompatible_Cp2(c, n);
		// Get MExtension from pair
		t.doGetObj(MExtension::type(), cb);
	    } else
		n(false);
	}};


    /*
    template<typename _Provided, typename _Required>
	void TConnPoint<_Provided, _Required>::isCompatible(const MConnPoint& aPair, bool aExtd, Tr<bool>* aCb) const {
	    auto* cb = new sIsCompatible_Cp1(this, aPair, *aCb);
	    ConnPoint::isCompatible(aPair, aExtd, cb);
	}
	*/




    /**
     * @brief Templated connection point with just provided iface
     *
     */
    template<typename _Provided>
	class TConnPointP: public ConnPointP
    {
	typedef _Provided TProvided;

	public:
	TConnPointP(const string& aName, TDir aDir, MIface* aProvided): ConnPoint(aName, aDir, aProvided) {};
	// From MConnPoint
	virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
    };

    template<typename _Provided> inline
	bool TConnPointP<_Provided>::IsCompatible(const MConnPoint& aPair, bool aExtd) const {
	    bool res = false;
	    const ConnPoint* cp = dynamic_cast<const ConnPoint*>(&aPair);
	    res =  (cp != NULL);
	    return res;
	}

    inline bool Connect(MConnPoint& aCp1, MConnPoint& aCp2) { return aCp1.Connect(aCp2) && aCp2.Connect(aCp1); }

    inline bool Disconnect(MConnPoint& aCp1, MConnPoint& aCp2) { return aCp1.Disconnect(aCp2) && aCp2.Disconnect(aCp1); }

    template<typename T> class MStateObserver_Imd: public MStateObserver<T>
    {
	public:
	    virtual MIface *MStateObserver_DoGetObj(const char* aTypeName) = 0;
	    virtual MIface *DoGetObj(const char* aTypeName) { return MStateObserver_DoGetObj(aTypeName);}
    };

    /**
     * @brief Connection point for state input
     *
     * Actually it doesn't represent some role of state - state delegates the role
     * to input. This role is to get the input data setter. 
     * So input is ConnPoint that provides role MStateObserver and this role is 
     * implemented by input itself 
     */
    template<typename T> class StateInput: public TConnPoint<MStateObserver<T>, MStateNotifier>
    {
	protected:
	    class StateObserver: public MStateObserver<T> {
		public:
		    StateObserver(StateInput& aHost): mHost(aHost) {}
		    // From MStateObserver
		    virtual void OnStateChanged(MIface* aSource, const T& aData) {
			if (mHost.mData.count(aSource) == 0) {
			    mHost.mData.insert(TDataElem(aSource, aData));
			} else {mHost. mData.at(aSource) = aData; }
			mHost.mObserver.OnInputChanged();
			MStateNotifier* intf = dynamic_cast<MStateNotifier*>(aSource);
			intf->OnStateChangeHandled(this);
		    };
		    StateInput& mHost;
	    };
	    friend StateObserver;
	public:
	    typedef pair<const MIface*, T> TDataElem;
	    typedef map<const MIface*, T> TData;
	public:
	    StateInput(const string& aName, MBase* aCowner, MInputObserver& aObserver):
		TConnPoint<MStateObserver<T>, MStateNotifier>(aName, aCowner, MConnPoint::EInput, &mStateObserver),
		mObserver(aObserver), mStateObserver(*this) {};
	    virtual MIface *DoGetObj(const char *aName) override {
		return aName == MStateObserver<T>::type() ? &mStateObserver : ConnPoint::DoGetObj(aName);}
	    const TData& Data() { return mData;};
	protected:
	    TData mData;
	    MInputObserver& mObserver;
	    StateObserver mStateObserver;
    };

    /**
     * @brief Connection point for state multilcient input
     *
     * Improved StateInput supporting multiple obsesrvers, ref uc_007
     */
    template<typename T> class StateMcInp: public TConnPoint<MStateObserver<T>, MStateNotifier>, public MStateObserver<T>
    {
	public:
	    typedef pair<const MIface*, T> TDataElem;
	    typedef map<const MIface*, T> TData;
	public:
	    StateMcInp(const string& aName, MBase* aCowner, MInputObserver& aObserver):
		TConnPoint<MStateObserver<T>, MStateNotifier>(aName, aCowner, MConnPoint::EInput, this) {};
	    const TData& Data() { return mData;};
	    void attach(MInputObserver* aObs) {
		for (auto obs: mObservers) assert(obs != aObs);
		mObservers.push_back(aObs);
	    }
	    void deattach(MInputObserver* aObs) {
		for (auto it = mObservers.begin(); it < mObservers.end(); it++)
		    if (*it == aObs) { mObservers.erase(it); return; }
		assert(true);
	    }
	    // From MStateObserver
	    virtual void OnStateChanged(MIface* aSource, const T& aData) {
		if (mData.count(aSource) == 0) {
		    mData.insert(TDataElem(aSource, aData));
		} else { mData.at(aSource) = aData; }
		for (auto obs: mObservers)
		    obs->OnInputChanged();
		MStateNotifier* intf = *aSource;
		intf->OnStateChangeHandled(this);
	    };
	protected:
	    TData mData;
	    vector<MInputObserver*> mObservers;
    };


    template<typename T> class StateOutput: public TConnPoint<MStateNotifier, MStateObserver<T>>
    {
	public:
	    StateOutput(const string& aName, MBase* aCowner, MStateNotifier& aNotifier):
		TConnPoint<MStateNotifier, MStateObserver<T>>(aName, aCowner, MConnPoint::EOutput, &aNotifier) {};
    };


    /**
     * @brief Extention of connection point TConnPoint
     *
     * This connection point allows to extend another connection point, for instance system's internal state
     * output to the system output.
     *
     */
    // TODO [YB] Should extension implement templated connpoint iface? In that case the logic of compatibility and
    // connection will become simple.
    template<typename _Provided, typename _Required>
	class  TConnPointExt: public Extention
    {
	public:
	    TConnPointExt(const string& aName, TDir aDir): Extention(aName, aDir,
		    new TConnPoint<_Provided, _Required>("Int", aDir, NULL)) {};
    };

    /**
     * @brief Extention of StateInput
     * TODO [YB] To define via typedef as TConnPointExt<MStateNotifier, MStateObserver<T>>
     */
    template<typename T> class ExtStateInp: public Extention
    {
	public:
	    ExtStateInp(const string& aName, MBase* aCowner): Extention(aName, aCowner, EInput,
		    new TConnPoint<MStateNotifier, MStateObserver<T>>("Int", this, EOutput)) {
	    }
    };

    /**
     * @brief Extention of StateOutput
     */
    template<typename T> class ExtStateOut: public Extention
    {
	public:
	    ExtStateOut(const string& aName, MBase* aCowner): Extention(aName, aCowner, EOutput,
		    mOrig = new TConnPoint<MStateObserver<T>, MStateNotifier>("Int", this, EInput)) {}
    };


    /**
     * @brief Socket
     */

    class Socket: public ConnPointBase
    {
	public:
	    typedef pair<string, ConnPointBase*> TPinsElem;
	    typedef map<string, ConnPointBase*> TPins;
	    static string type() { return "Socket";}
	public:
	    Socket(const string& aName, MBase* aCowner, TDir aDir): ConnPointBase(aName, aDir, aCowner) {}
	    virtual ~Socket();
	    virtual const std::string GetType() const { return type();}
	    // From MConnPoint
	    virtual bool DoConnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const override;
	protected:
	    MConnPoint* pinAt(const string& key) const { return mPins.at(key);}
	    const TPins& pins() const { return mPins;}
	    void addPin(const string& aName, ConnPointBase* aPin) { mPins.insert(TPinsElem(aName, aPin));}
	protected:
	    TPins mPins;
    };

    /**
     * @brief Socket containing just two pins. One pin compatible to state input, another to output
     * Note that both pins are extenstions. This is because we need double-ended pin in orider to get
     * point-to-point connection between state's inputs and outputs.
     */
    template <class inp_type, class out_type>
	class StateIo: public Socket
    {
	public:
	    typedef inp_type TInpType;
	    typedef out_type TOutType;
	    static string type() { return "StateIo";}

	    StateIo(const string& aName, MBase* aCowner, const string& aInpName, const string& aOutName): Socket(aName, aCowner, EBidir) {
		addPin(aInpName, mInp = new ExtStateInp<TInpType>("Inp", this));
		addPin(aOutName, mOut = new ExtStateOut<TOutType>("Out", this));
	    }
	    virtual const std::string GetType() const { return type();}
	    Extention* mInp;
	    Extention* mOut;
    };

} // namespace desa

#endif  //  __DESA_CONNPOINT__
