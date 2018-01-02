#ifndef __DESA_MSTATE__
#define __DESA_MSTATE__

#include <miface.h>
#include <string>
#include <assert.h>

namespace desa {

    using namespace std;

    /**
     * @brief Base, support naming
     */
    class MBase
    {
	public:
	    MBase(const string& aName): mName(aName) {};
	    virtual ~MBase() {};
	    virtual const std::string& Name() const { return mName;};
	    virtual const std::string GetUri() const { return std::string();};
	protected:
	    string mName;
    };

    /**
     * @brief Interface of node-component in hierarchy tree
     */
    class MComp: public MIface
    {
	public:
	    virtual void Update() = 0;
	    virtual void Confirm() = 0;
    };

    /**
     * @brief Interface of node-owner in hierarchy tree
     */
    class MOwner: public MIface
    {
	public:
	    virtual void OnCompActivated(MComp* aComp) = 0;
	    virtual void OnCompUpdated(MComp* aComp) = 0;
	    virtual void OnCompConfirmed(MComp* aComp) = 0;
    };

    /**
     *
     */
    class MState: public MIface
    {
	public:
	    virtual const void* Data() const = 0;
	    template<typename T> const T& GetData() const;
    };

    /**
     * @brief Observer of state
     *
     * Normally observable is state connected to input
     */
    template<typename T> class MStateObserver: public MIface
    {
	public:
	    virtual void OnStateChanged(MIface* aSource, const T& aStateData) = 0;

    };

    template<typename T> class MData: public MIface
    {
	public:
	    virtual const T& Data() const = 0;
    };

    template<typename T> class MDataSetter: public MIface
    {
	public:
	    virtual void Set(const MIface* aSource, const T& aData) = 0;
    };

    class MStateNotifier: public MIface
    {
	public:
	    virtual void OnStateChangeHandled(MIface* aObserver) = 0;
    };

    class MInputObserver
    {
	public:
	    virtual void OnInputChanged() = 0;
    };

    /**
     * @brief Named component
     *
     */
    // TODO To move to base.h
    class Comp: public MBase, public MComp
    {
	public:
	    Comp(const string& aName): MBase(aName), mOwner(NULL) {};
	    Comp(const string& aName, MOwner* aOwner): MBase(aName), mOwner(aOwner) {};
	    void SetOwner(MOwner* aOwner) { assert(mOwner == NULL); mOwner = aOwner;};
	    virtual const std::string GetUri() const override {
		if (mOwner == nullptr) return "/";
		const MBase* owner = mOwner->GetBase();
		std::string uri = (owner != nullptr) ? owner->GetUri(): "?";
		uri.append("/"); uri.append(Name());
		return uri;
	    };
	protected:
	    MOwner* mOwner;
    };


} // namespace desa

#endif  //  __DESA_MSTATE__
