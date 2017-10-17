#ifndef __DESA_SYSTEM__
#define __DESA_SYSTEM__

#include "miface.h"
#include "mdes.h"
#include  <vector>
#include  <map>
#include  <set>
#include  <mutex>

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
	typedef vector<MComp*> TActives;
	typedef set<MComp*> TCompsSet;
	friend class Owner;
	friend class Comp;

	enum TState {
	    ESt_Unknown = 0,
	    ESt_UpdateRequesing,
	    ESt_Updated,
	    ESt_ConfirmRequesting,
	    ESt_Confirmed
	};
	protected:
	    class OwnerImpl: public MOwner {
		public:
		    OwnerImpl(System& aHost): mHost(aHost) {};
		    // From MOwner
		    virtual void OnCompActivated(MComp* aComp) { mHost.HandleCompActivated(aComp);};
		    virtual void OnCompUpdated(MComp* aComp) { mHost.HandleCompUpdated(aComp);};
		    virtual void OnCompConfirmed(MComp* aComp) { mHost.HandleCompConfirmed(aComp);};
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
	    void HandleCompUpdated(MComp* aComp);
	    void HandleCompConfirmed(MComp* aComp);
	    // From MComp
	    virtual void Update();
	    virtual void Confirm();
	protected:
	    OwnerImpl mOwnerImpl;
	    TComps mComps; // Register of the components
	    TActives mStatus1; // Component's statuses
	    TActives mStatus2;
	    bool mIsActive; // TODO [YB] to move to Comp
	    TActives* mStatusCur; // Component's status on the current step
	    TActives* mStatusNext; // Component's status on the next step
	    TCompsSet mRequested; // Components requested for update/confirm
	    TState mState; // DES state
	    bool mAllCompsConfirmed;
	    bool mAllCompsUpdated;
	    mutex mRq;
    };


} // namespace desa


#endif // __DESA_SYSTEM__
