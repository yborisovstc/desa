#ifndef __DESA_IFACE__
#define __DESA_IFACE__

#include <string>

namespace desa {

    /*
     * Interface of interface. Support unified invocation of methods
     */

    class MIface
    {
	public:
	    template<typename T> operator T*() { return dynamic_cast<T*>(this);};
	    template<typename T> operator const T*() const { return dynamic_cast<const T*>(this);};
	    virtual void Call(const std::string& aSpec) {};

    };

}

#endif //  __DESA_IFACE__
