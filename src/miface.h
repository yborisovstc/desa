#ifndef __DESA_IFACE__
#define __DESA_IFACE__

#include <string>
#include "trans.h"

namespace desa {

    class MIface;

    class MIfProvider
    {
	public:
	    template <class T> T* GetObj(T* aInst) {return aInst = dynamic_cast<T*>(DoGetObj(aInst->type()));};
	    // TODO to make abstract. undefined ref atm
	    // TODO to have const version
	    virtual MIface *DoGetObj(const char* aTypeName) { return nullptr;}
	    void doGetObj(const char* aTypeName, Tr<MIface>* aNext) {
		MIface* res = DoGetObj(aTypeName);
		(*aNext)(*res);
	    }
	    /*
	    void doGetObj(const char* aTypeName, std::function<void(MIface*)>* aNext) {
		MIface* res = DoGetObj(aTypeName);
		(*aNext)(res);
	    }
	    */
    };


    class MIface: public MIfProvider
    {
	public:
	    static const char* type() {return "MIface";}
	    virtual const std::string GetIfType() const {return type();}
	    virtual void getIfType(Tr<std::string>* aCb) const { (*aCb)(type());}
	    virtual void Call(const std::string& aSpec) {};

    };

}

#endif //  __DESA_IFACE__
