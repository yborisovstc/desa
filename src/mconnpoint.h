#ifndef __DESA_MCONNPOINT__
#define __DESA_MCONNPOINT__

#include <miface.h>

namespace desa {


    /**
     * @brief Connection points mediator, ref ds_conn_chain
     */
    class MMediator
    {
	public:

    };


    /**
     * @brief Connection point interface. Blocking.
     */
    class MConnPoint
    {
	public:
	    enum TDir {
		EInput = 1,
		EOutput = 2,
		EBidir = 3
	    };
	public:
	    virtual TDir Dir() const = 0;
	    virtual bool Connect(const MConnPoint& aCp) = 0;
	    virtual bool Disconnect(const MConnPoint& aCp) = 0;
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const = 0;
	    virtual void OnPairChanged(MConnPoint* aPair) = 0;
	    virtual void OnMediatorChanged(MConnPoint* aMediator) = 0;
	    virtual bool IsConnected() const = 0;
	    virtual const MBase* MConnPoint_Base() const { return nullptr;};
	    virtual void Dump() const = 0;
    };

    class MExtension
    {
	public:
	    virtual const MConnPoint& Orig() const = 0;
    };

    /**
     * @brief Connection point interface. Unblocking.
     */
    class MUbConnPoint
    {
	public:
	    virtual void Connect(const MConnPoint& aCp) = 0;
	    virtual void Disconnect(const MConnPoint& aCp) = 0;
	    virtual void IsCompatible(const MConnPoint& aPair, bool aExtd = false) const = 0;
	    virtual void OnPairChanged(MConnPoint* aPair) = 0;
	    virtual void OnMediatorChanged(MConnPoint* aMediator) = 0;
	    virtual void IsConnected() const = 0;
	    virtual void Dump() const = 0;
    };

    /**
     * @brief Unblicking Connection point controller interface
     */
    class MUbCpController
    {
	public:
	    virtual void onConnected(bool aSuccess) = 0;
	    virtual void onDisconnected(bool aSuccess) = 0;
	    virtual void onIsCompatible(bool aCompatible) = 0;
	    virtual void onIsConnected(bool aConnected) = 0;
    };

} // namespace desa

#endif  //  __DESA_MCONNPOINT__
