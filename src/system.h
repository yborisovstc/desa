#ifndef __DESA_SYSTEM__
#define __DESA_SYSTEM__

#include "miface.h"
#include "mdes.h"
#include  <vector>
#include  <map>

namespace desa {

    using namespace std;

    class State;

    /**
     * @brief System representing the vector of states
     *
     */
    class System: public Comp
    {
	typedef pair<string, Comp*> TCompRec;
	typedef map<string, Comp*> TComps;
	friend class Owner;
	friend class Comp;
	protected:
	    class OwnerImpl: public MOwner {
		public:
		    OwnerImpl(System& aHost): mHost(aHost) {};
		    // From MOwner
		    virtual void OnCompActivated(MComp* aComp) { mHost.HandleCompActivated(aComp);};
		protected:
		    System& mHost;
	    };
	public:
	    System(const string& aName, MOwner* aOwner);
	    virtual ~System();
	    void AddComp(Comp* aComp);
	    void Run();
	protected:
	    void HandleCompActivated(MComp* aComp);
	    // From MComp
	    virtual void Update();
	    virtual void Confirm();
	protected:
	    OwnerImpl mOwnerImpl;
	    TComps mComps;
	    bool mIsActive;
    };


} // namespace desa


#endif // __DESA_SYSTEM__
