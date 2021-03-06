#ifndef __DESA_MCONNPOINT__
#define __DESA_MCONNPOINT__

#include <miface.h>

namespace desa {

    class MConnPoint
    {
	public:
	    enum TDir {
		EInput = 1,
		EOutput = 2
	    };
	public:
	    virtual TDir Dir() const = 0;
	    virtual bool Connect(const MConnPoint& aCp) = 0;
	    virtual bool IsCompatible(const MConnPoint& aPair, bool aExtd = false) const = 0;
    };

} // namespace desa

#endif  //  __DESA_MCONNPOINT__
