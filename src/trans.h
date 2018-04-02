#ifndef __DESA_TRANS__
#define __DESA_TRANS__

#include <functional>

/**
 * @brief Transitions framework
 * provides lightweight solution for async state/transition
 */

namespace desa {

    template<typename T> class Tr {
	public:
	    virtual ~Tr() {}
	    virtual void operator()(const T& a) {
		tr(a);
		delete this;
	    }
	    //virtual void operator()(const T&) = 0; 
	    virtual void tr(const T&) {}; 
    };

    /**
     * @brief Base of state
     * TC - context, persistent inputs, e.g. master class kept the state
     * TT - transient inputs - produced by states
     * TI - main input type
     * TN - notification to input of state connected to output, next part of movement,
     * also can be simply considered as next state input
     */
    template<typename TC, typename TT, typename TN, typename TI> class Stt: public Tr<TI>
    {
	public:
	    Stt(TC* aC, TT& aT, Tr<TN>& aN): c(aC), t(aT), n(aN) {}
	    TC* c;
	    TT& t;
	    Tr<TN>& n;
    };

    /**
     * @brief Base of state
     * TC - context, persistent inputs, e.g. master class kept the state
     * TI - main input type
     * TN - notification to input of state connected to output, next part of movement,
     * also can be simply considered as next state input
     */
    template<typename TC, typename TN, typename TI> class St: public Tr<TI>
    {
	public:
	    St(TC* aC, Tr<TN>& aN): c(aC), n(aN) {}
	    TC* c;
	    Tr<TN>& n;
    };

    /**
     * @brief Base of state
     * TC - context, persistent inputs, e.g. master class kept the state
     * TI - main input type
     */
    template<typename TC, typename TI> class Stf: public Tr<TI>
    {
	public:
	    Stf(TC* aC): c(aC) {}
	    TC* c;
    };

    template<typename C0, typename T> class Trc: public Tr<T>
    {
	public:
	    Trc(C0& a0): m(a0) {}
	    C0 m;
    };

    template<typename C0, typename T> class Tr1: public Tr<T>
    {
	public:
	    Tr1(C0& a0): m0(a0) {}
	    C0& m0;
    };

    template<typename C0, typename C1, typename T> class Tr2: public Tr<T>
    {
	public:
	    Tr2(C0& a0, C1& a1): m0(a0), m1(a1) {}
	    C0& m0;
	    C1& m1;
    };

//    template<typename T> using Tf = std::functional<void(const T&)>;

} // namespace desa


#endif // __DESA_TRANS__
