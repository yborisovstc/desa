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
	static string type() { return "System";}

	enum TDesState {
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
		    static const char* Type() { return "OwnerImpl";}
		    virtual MIface *DoGetObj(const char *aName) override {if (aName == Type()) return this; else return nullptr;}
		    // From MOwner
		    virtual void OnCompActivated(MComp* aComp) { mHost.HandleCompActivated(aComp);};
		    virtual void OnCompUpdated(MComp* aComp) { mHost.HandleCompUpdated(aComp);};
		    virtual void OnCompConfirmed(MComp* aComp) { mHost.HandleCompConfirmed(aComp);};
		    // From MIface
		    virtual const MBase* base() const {  return dynamic_cast<const MBase*>(&mHost);};
		protected:
		    System& mHost;
	    };
	public:
	    System(const string& aName, MOwner* aOwner);
	    virtual ~System();
	    virtual MIface *DoGetObj(const char *aName) override;
	    void AddComp(Comp* aComp);
	    void Run();
	protected:
	    void HandleCompActivated(MComp* aComp);
	    void HandleCompUpdated(MComp* aComp);
	    void HandleCompConfirmed(MComp* aComp);
	    virtual const std::string GetType() const { return type();}
	    // From MComp
	    virtual void Update();
	    virtual void Confirm();
	protected:
	    void DumpComps() const;
	    void DumpActives() const;
	protected:
	    OwnerImpl mOwnerImpl;
	    TComps mComps; // Register of the components
	    TActives mStatus1; // Component's statuses
	    TActives mStatus2;
	    bool mIsActive; // TODO [YB] to move to Comp
	    TActives* mStatusCur; // Component's status on the current step
	    TActives* mStatusNext; // Component's status on the next step
	    TCompsSet mRequested; // Components requested for update/confirm
	    TDesState mState; // DES state
	    bool mAllCompsConfirmed;
	    bool mAllCompsUpdated;
	    mutex mRq;
    };


} // namespace desa


#endif // __DESA_SYSTEM__
