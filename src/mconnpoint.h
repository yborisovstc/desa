#ifndef __DESA_MCONNPOINT__
#define __DESA_MCONNPOINT__

#include <miface.h>

namespace desa {

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
	    virtual bool IsConnected() const = 0;
	    virtual const MBase* MConnPoint_Base() const { return nullptr;};
    };

    class MExtension
    {
	public:
	    virtual const MConnPoint& Orig() const = 0;
    };

} // namespace desa

#endif  //  __DESA_MCONNPOINT__
