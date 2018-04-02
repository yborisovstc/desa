#ifndef __DESA_MCONNPOINT__
#define __DESA_MCONNPOINT__

#include <map>
#include <functional>
#include <miface.h>
#include "mdes.h"
#include "trans.h"

namespace desa {

    /**
     * @brief Connection points mediator, ref ds_conn_chain
     */
    class MMediator
    {
	public:

    };

    class MCpClient;

    /**
     * @brief Connection point interface. Blocking.
     */
    class MConnPoint: public MIface
    {
	public:
	    /**
	     * @brief The direction - simplest attribute stated compatiblity of CPs.
	     * It allows to create complex topology of connectins following the rules, e.g.
	     * many undef to many 1 input to many out, 1 bidir to 1 bidir 
	     * TODO to consider other simple compatibility mechanisms
	     */
	    enum TDir {
		EUndefined = 0,
		EInput = 1,
		EOutput = 2,
		EBidir = 3
	    };
	public:
	    static const char* type() { return "MConnPoint";}
	    // Blocking
	    virtual TDir Dir() const = 0;
	    virtual bool Connect(const MConnPoint& aCp) = 0;
	    virtual bool Disconnect(const MConnPoint& aCp) = 0;
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const = 0;
	    virtual void OnPairChanged(MConnPoint* aPair) = 0;
	    virtual void OnMediatorChanged(MConnPoint* aMediator) = 0;
	    virtual bool IsConnected() const = 0;
	    virtual const MBase* MConnPoint_Base() const { return nullptr;};
	    virtual void Dump() const = 0;
	    // Non blocking
	    virtual void dir(Tr<MConnPoint::TDir>* aTr) const = 0;
	    virtual void connect(const MConnPoint& aCp, Tr<bool>* aTr) = 0;
	    virtual void disconnect(const MConnPoint& aCp, MCpClient* aClient) = 0;
	    virtual void isCompatible(const MConnPoint& aPair, bool aExtd, Tr<bool>* aCb) = 0;
	    //virtual void isCompatible(const MConnPoint& aPair, bool aExtd, void(*aCb)(bool)) const = 0; // To use std::function
	    virtual void onPairChanged(MConnPoint* aPair, MCpClient* aClient) = 0;
	    virtual void onMediatorChanged(MConnPoint* aMediator, MCpClient* aClient) = 0;
	    virtual void isConnected(MCpClient* aClient) const = 0;
	    virtual void dump(MCpClient* aClient) const = 0;
    };

    /**
     * @brief Non-blicking Connection point client interface
     */
    class MCpClient
    {
	public:
	    virtual void onDir(MConnPoint::TDir aDir) = 0;
	    virtual void onConnected(bool aSuccess) = 0;
	    virtual void onDisconnected(bool aSuccess) = 0;
	    virtual void onIsCompatible(bool aCompatible) = 0;
	    virtual void onIsConnected(bool aConnected) = 0;
    };

    /**
     * @brief Extension interface. Blocking.
     */
    class MExtension: public MIface
    {
	public:
	    static const char* type() { return "MExtension";}
	    virtual MConnPoint& Orig() const = 0;
	    virtual void orig(Tr<MConnPoint>*) const = 0;
    };


    /**
     * @brief joint via provided/required interfaces 
     */
    class MJointPr: public MIface
    {
	public:
	typedef std::map<const MConnPoint*, MIface*> TPairsIfaces;
	typedef std::pair<const MConnPoint*, MIface*> TPairsIfacesElem;
	public:
	static const char* type() { return "MJointPr";}
	virtual std::string ProvidedType() const = 0;
	virtual void providedType(Tr<std::string>*) const = 0;
	virtual std::string RequiredType() const = 0;
	virtual void requiredType(Tr<std::string>*) const = 0;
	virtual MIface& Provided() = 0;
	virtual void provided(Tr<MIface>*) = 0;
	virtual const MIface& Provided() const = 0;
	virtual TPairsIfaces& Required() = 0; 
	virtual const TPairsIfaces& Required() const = 0; 
	virtual void required(Tr<TPairsIfaces>*) = 0;
    };

} // namespace desa

#endif  //  __DESA_MCONNPOINT__
