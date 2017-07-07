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
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	protected:
	    TDir mDir;
	    TPairs mPairs;
    };

    /**
     * @brief Simple Connection Point
     *
     * ConnPoint contains cache of required interfaces implementation, i.e. the state can get from
     * input ConnPoint the set of MState interfaces of all connected state outputs
     */
    class ConnPoint: public ConnPointBase
    {
	typedef std::vector<MIface*> TIfaces;

	public:
	    ConnPoint(const string& aName, TDir aDir, MIface& aProvided);
	    virtual MIface& Provided() { return mProvided;};
	    virtual const MIface& Provided() const { return mProvided;};
	    virtual TIfaces& Required() { return mRequired;}; 
	    virtual const TIfaces& Required() const { return mRequired;}; 
	    // From MConnPoint
	    virtual bool Connect(const MConnPoint& aCp);
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	protected:
	    MIface& mProvided;
	    TIfaces mRequired;
    };

    /**
     * @brief Simple Connection Point only providing iface, no required iface needed
     */
    class ConnPointP: public ConnPointBase
    {
	public:
	    ConnPointP(const string& aName, TDir aDir, MIface& aProvided);
	    virtual MIface& Provided() { return mProvided;};
	    virtual const MIface& Provided() const { return mProvided;};
	protected:
	    MIface& mProvided;
    };


    template<typename _Provided, typename _Required>
    class TConnPoint: public ConnPoint
    {
	typedef _Required TRequired;
	typedef _Provided TProvided;

	public:
//	    TConnPoint(const string& aName, TDir aDir, MIface& aProvided): ConnPoint(aName, aDir, aProvided) {};
	    TConnPoint(const string& aName, TDir aDir, TProvided& aProvided): ConnPoint(aName, aDir, aProvided) {};
	    // From MConnPoint
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
    };

    template<typename _Provided, typename _Required> inline
	bool TConnPoint<_Provided, _Required>::IsCompatible(const MConnPoint& aPair, bool aExtd) const {
	    bool res = ConnPoint::IsCompatible(aPair, aExtd);
	    if (res) {
		const ConnPoint* cp = dynamic_cast<const ConnPoint*>(&aPair);
		if (cp != NULL) {
		    const TRequired* req = cp->Provided();
		    res = req != NULL;
		}
	    }
	    return res;
	}


    template<typename _Provided>
	class TConnPointP: public ConnPointP
    {
	typedef _Provided TProvided;

	public:
	TConnPointP(const string& aName, TDir aDir, MIface& aProvided): ConnPoint(aName, aDir, aProvided) {};
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
	    Extention(const string& aName, TDir aDir, ConnPointBase* aSrc): ConnPointBase(aName, aDir), mSrc(aSrc) {};
	    virtual ~Extention();
	    // From MConnPoint
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const;
	protected:
	    ConnPointBase* mSrc; // Source conn point that is represented by extention, owned
    };


    /**
     * @brief Extention of connection point TConnPoint
     *
     * This connection point allows to extend another connection point, for instance system's internal state
     * output to the system output.
     *
     */
    template<typename _Provided, typename _Required>
    class  TConnPointExt: public Extention
    {
	public:
	    TConnPointExt(const string& aName, TDir aDir): Extention(aName, aDir,
		    new TConnPoint<_Provided, _Required>("Int", aDir, NULL)) {};
    };

    /**
     * @brief Connection point for state input
     *
     * Actually it doesn't represent some role of state - state delegates the role
     * to input. This role is to get the input data setter. 
     * So input is ConnPoint that provides role MDataSetter and this role is 
     * implemented by input itself 
     */
    template<typename T>
    class StateInput: public TConnPointP<MDataSetter<T>>, public MDataSetter<T>
    {
	public:
	    typedef pair<const MIface*, T> TDataElem;
	    typedef map<const MIface*, T> TData;
	public:
	    StateInput(const string& aName, MStateObserver& aObserver):
		TConnPointP<MDataSetter<T>>(aName, MConnPoint::EInput, *this),
		mObserver(aObserver) {};
	    const TData& Data() { return mData;};
	    // From MDataSetter
	    virtual void Set(const MIface* aSource, const T& aData) {
		if (mData.count(aSource) == 0) {
		    mData.insert(TDataElem(aSource, aData));
		} else { mData.at(aSource) = aData; }
		mObserver.OnSourceChanged();
	    };
	protected:
	    TData mData;
	    MStateObserver& mObserver;
    };

} // namespace desa

#endif  //  __DESA_CONNPOINT__
