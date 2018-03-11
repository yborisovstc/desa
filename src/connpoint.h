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
     *
     */
    class ConnPointBase: public MBase, public MConnPoint
    {
	public:
	    typedef vector<const MConnPoint*> TPairs;
	    static string type() { return "ConnPointBase";}
	public:
	    ConnPointBase(const string& aName, TDir aDir, MBase* aCowner = nullptr);
	    virtual ~ConnPointBase();
	    // From MConnPoint
	    virtual TDir Dir() const { return mDir;};
	    virtual bool Connect(const MConnPoint& aCp);
	    virtual bool Disconnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	    virtual void OnPairChanged(MConnPoint* aPair);
	    virtual void OnMediatorChanged(MConnPoint* aMediator);
	    virtual bool IsConnected() const override;
	    virtual const MBase* MConnPoint_Base() const { return this;};
	    // From MBase
	    virtual const std::string getType() const { return type();}
//	    virtual const std::string getUri() const;
	protected:
	    virtual bool DoConnect(const MConnPoint& aCp);
	    virtual bool DoDisconnect(const MConnPoint& aCp);
	    virtual void Notify(MConnPoint* aExclude = NULL);
	    virtual void Dump() const override;
	protected:
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
	    ConnPointBase& Orig() { return *mOrig; };
	    virtual const std::string getType() const { return type();}
	    // From MConnPoint
	    const MConnPoint& Orig() const { return *mOrig; };
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
	typedef map<const MConnPoint*, MIface*> TPairsIfaces;
	typedef pair<const MConnPoint*, MIface*> TPairsIfacesElem;

	public:
	    ConnPoint(const string& aName, MBase* aCowner, TDir aDir, MIface* aProvided = NULL):
		ConnPointBase(aName, aDir, aCowner), mProvided(aProvided) {}
	    virtual MIface& Provided() { return *mProvided;};
	    virtual const MIface& Provided() const { return *mProvided;};
	    virtual TPairsIfaces& Required() { return mRequired;}; 
	    virtual const TPairsIfaces& Required() const { return mRequired;}; 
	    // From MConnPoint
	    virtual bool DoConnect(const MConnPoint& aCp);
	    virtual bool DoDisconnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	    virtual void OnPairChanged(MConnPoint* aPair);
	    virtual bool IsConnected() const override;
	protected:
	    virtual void Dump() const override;
	protected:
	    MIface* mProvided;
	    TPairsIfaces mRequired;
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
    template<typename _Provided, typename _Required>
    class TConnPoint: public ConnPoint
    {
	typedef _Required TRequired;
	typedef _Provided TProvided;

	public:
	    TConnPoint(const string& aName, MBase* aCowner, TDir aDir, TProvided* aProvided = NULL):
		ConnPoint(aName, aCowner, aDir, aProvided) {};
	    // From MConnPoint
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
    };

    template<typename _Provided, typename _Required> inline
	bool TConnPoint<_Provided, _Required>::IsCompatible(const MConnPoint& aPair, bool aExtd) const {
	    bool res = ConnPoint::IsCompatible(aPair, aExtd);
	    if (res) {
		/*
		const ConnPoint* cp = dynamic_cast<const ConnPoint*>(&aPair);
		if (cp != NULL) {
		    const TRequired* req = cp->Provided();
		    res = req != NULL;
		}
		*/
		// Checking if pair is extension
		const Extention* ext = dynamic_cast<const Extention*>(&aPair);
		const MConnPoint* pair = ext == NULL ? &aPair : &ext->Orig();
		bool extd = aExtd ^ (ext != NULL);
		if (extd) {
		    const TConnPoint<TProvided, TRequired>* cp = dynamic_cast<const TConnPoint<TProvided, TRequired>*>(pair);
		    res = cp != NULL;
		} else {
		    const TConnPoint<TRequired, TProvided>* cp = dynamic_cast<const TConnPoint<TRequired, TProvided>*>(pair);
		    res = cp != NULL;
		}
	    }
	    return res;
	}


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

    /**
     * @brief Connection point for state input
     *
     * Actually it doesn't represent some role of state - state delegates the role
     * to input. This role is to get the input data setter. 
     * So input is ConnPoint that provides role MStateObserver and this role is 
     * implemented by input itself 
     */
    template<typename T> class StateInput: public TConnPoint<MStateObserver<T>, MStateNotifier>, public MStateObserver<T>
    {
	public:
	    typedef pair<const MIface*, T> TDataElem;
	    typedef map<const MIface*, T> TData;
	public:
	    StateInput(const string& aName, MBase* aCowner, MInputObserver& aObserver):
		TConnPoint<MStateObserver<T>, MStateNotifier>(aName, aCowner, MConnPoint::EInput, this),
		mObserver(aObserver) {};
	    const TData& Data() { return mData;};
	    // From MStateObserver
	    virtual void OnStateChanged(MIface* aSource, const T& aData) {
		if (mData.count(aSource) == 0) {
		    mData.insert(TDataElem(aSource, aData));
		} else { mData.at(aSource) = aData; }
		mObserver.OnInputChanged();
		MStateNotifier* intf = dynamic_cast<MStateNotifier*>(aSource);
		intf->OnStateChangeHandled(this);
	    };
	protected:
	    TData mData;
	    MInputObserver& mObserver;
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
		TConnPoint<MStateNotifier, MStateObserver<T>>(aName, aCowner, aNotifier) {};
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
	    virtual const std::string getType() const { return type();}
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
	    virtual const std::string getType() const { return type();}
	    Extention* mInp;
	    Extention* mOut;
    };

} // namespace desa

#endif  //  __DESA_CONNPOINT__
