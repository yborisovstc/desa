#ifndef __DESA_CONNPOINT__
#define __DESA_CONNPOINT__

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
	public:
	    ConnPointBase(const string& aName, TDir aDir);
	    virtual ~ConnPointBase();
	    // From MConnPoint
	    virtual TDir Dir() const { return mDir;};
	    virtual bool Connect(const MConnPoint& aCp);
	    virtual bool Disconnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	    virtual void OnPairChanged(MConnPoint* aPair);
	    virtual bool IsConnected() const override;
	protected:
	    virtual bool DoConnect(const MConnPoint& aCp);
	    virtual bool DoDisconnect(const MConnPoint& aCp);
	    void Notify(MConnPoint* aExclude = NULL);
	    virtual void Dump() const;
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
    class Extention: public ConnPointBase
    {
	public:
	    Extention(const string& aName, TDir aDir, ConnPointBase* aOrig): ConnPointBase(aName, aDir), mOrig(aOrig) {};
	    virtual ~Extention();
	    ConnPointBase& Orig() { return *mOrig; };
	    const ConnPointBase& Orig() const { return *mOrig; };
	    // From MConnPoint
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	protected:
	    ConnPointBase* mOrig; // Original conn point that is represented by extention, owned
    };


    /**
     * @brief Simple Connection Point
     *
     * ConnPoint contains cache of required interfaces implementation, i.e. the state can get from
     * input ConnPoint the set of MState interfaces of all connected state outputs
     */
    class ConnPoint: public ConnPointBase
    {
	typedef map<const MConnPoint*, MIface*> TPairsIfaces;
	typedef pair<const MConnPoint*, MIface*> TPairsIfacesElem;

	public:
	    ConnPoint(const string& aName, TDir aDir, MIface* aProvided = NULL);
	    virtual MIface& Provided() { return *mProvided;};
	    virtual const MIface& Provided() const { return *mProvided;};
	    virtual TPairsIfaces& Required() { return mRequired;}; 
	    virtual const TPairsIfaces& Required() const { return mRequired;}; 
	    // From MConnPoint
	    virtual bool Connect(const MConnPoint& aCp);
	    virtual bool Disconnect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	    virtual void OnPairChanged(MConnPoint* aPair);
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
//	    TConnPoint(const string& aName, TDir aDir, MIface& aProvided): ConnPoint(aName, aDir, aProvided) {};
	    TConnPoint(const string& aName, TDir aDir, TProvided* aProvided = NULL): ConnPoint(aName, aDir, aProvided) {};
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
	    StateInput(const string& aName, MInputObserver& aObserver):
		TConnPoint<MStateObserver<T>, MStateNotifier>(aName, MConnPoint::EInput, this),
		mObserver(aObserver) {};
	    const TData& Data() { return mData;};
	    // From MStateObserver
	    virtual void OnStateChanged(MIface* aSource, const T& aData) {
		if (mData.count(aSource) == 0) {
		    mData.insert(TDataElem(aSource, aData));
		} else { mData.at(aSource) = aData; }
		mObserver.OnInputChanged();
		MStateNotifier* intf = *aSource;
		intf->OnStateChangeHandled(this);
	    };
	protected:
	    TData mData;
	    MInputObserver& mObserver;
    };

    template<typename T> class StateOutput: public TConnPoint<MStateNotifier, MStateObserver<T>>
    {
	public:
	    StateOutput(const string& aName, MStateNotifier& aNotifier):
		TConnPoint<MStateNotifier, MStateObserver<T>>(aName, aNotifier) {};
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
	    ExtStateInp(const string& aName): Extention(aName, EInput,
		    new TConnPoint<MStateNotifier, MStateObserver<T>>("Int", EOutput)) {
	    }
    };

    /**
     * @brief Extention of StateOutput
     */
    template<typename T> class ExtStateOut: public Extention
    {
	public:
	    ExtStateOut(const string& aName): Extention(aName, EOutput,
		    mOrig = new TConnPoint<MStateObserver<T>, MStateNotifier>("Int", EInput)) {
			    }
    };



} // namespace desa

#endif  //  __DESA_CONNPOINT__
