#ifndef __DESA_STATE__
#define __DESA_STATE__

#include "mdes.h"
#include "connpoint.h"
#include <vector>
#include <set>

namespace desa {

    using namespace std;

    template <typename T> class TState;

    /**
     * @brief Base state
     */
    class State: public Comp
    {
	public:
	    typedef set<MIface*> TIfSet; // Set of ifaces

	    friend class InputObserver;
	    friend class StateNotifier; 
	    static string type() { return "State";}
	    virtual MIface *DoGetObj(const char *aName) override;
	protected:
	    class InputObserver: public MInputObserver {
	    public:
		InputObserver(State& aHost): mHost(aHost) {};
		// From MStateObserver
		virtual void OnInputChanged() { mHost.HandleInputChanged();};
	    private:
		State& mHost;
	};
	    class StateNotifier: public MStateNotifier {
		public:
		    StateNotifier(State& aHost): mHost(aHost) {};
		    static const char* Type() { return "StateNotifier";}
		    virtual MIface *DoGetObj(const char *aName) override {if (aName == Type()) return this; else return nullptr;}
		    // From MStateNotifier
		    virtual void OnStateChangeHandled(MIface* aObserver) { mHost.HandleStateChangeHandled(aObserver);};
		private:
		    State& mHost;
	    };
	public:
	    State(const string& aName);
	    State(const string& aName, MOwner* aOwner);
	    virtual ~State();
	    template<typename T> inline operator const TState<T>&() const;
	    template<typename T> inline operator const T&() const;
	    ConnPointBase& Output() { return *mOutput;};
	    void EnableDanglingOutput(bool aEnable = true) { mEnableDanglingOutput = aEnable;};
	public:
	    virtual void Run();
	    // From MComp
	    virtual void Update();
	    virtual void Confirm();
	    virtual const MBase* base() const {  return dynamic_cast<const MBase*>(this);};
	    operator MInputObserver*() {return &mSobs;}
	protected:
	    virtual const std::string GetType() const { return type();}
	    virtual void Trans() {};
	    //virtual void* Conf() { return NULL;};
	    //virtual void* Upd() { return NULL;};
	    //virtual int Len() const { return 0;};
	    virtual bool IsChanged() const { return false;};
	    virtual void ApplyChange() {};
	    virtual void NotifyOutputs(ConnPoint* aOutput);
	    void HandleInputChanged();
	    void HandleStateChangeHandled(MIface* aObserver);
	    virtual void DoNotifyOutput(MIface* aObserver) = 0;
	    inline void NotifyCompConfirmed();
	    inline void NotifyCompActivated();
	    inline void NotifyCompUpdated();
	protected:
	    InputObserver mSobs;
	    StateNotifier mSntf;
	    vector<ConnPoint*> mInputs;
	    ConnPointBase* mOutput;
	    bool mIsActive;
	    bool mUninit; // Sign of state is not initialized yet
	    TIfSet mNotifUnconfirmed; // Set of notifiers not yet confirmed notification
	    bool mOutputsNotified;
	    bool mEnableDanglingOutput;
    };

    inline void State::NotifyCompActivated()
    {
	if (mOwner != NULL) {
	    mOwner->OnCompActivated(this);
	}
    }

    inline void State::NotifyCompConfirmed()
    {
	if (mOwner != NULL) {
	    mOwner->OnCompConfirmed(this);
	}
    }

    inline void State::NotifyCompUpdated()
    {
	if (mOwner != NULL) {
	    mOwner->OnCompUpdated(this);
	}
    }


    /**
     * @brief Customized state
     */
    template<typename T> class TState: public State
    {
	public:
	    // Disabling undefined value, ref uc_006 invalidated
	    TState(const string& aName, MOwner* aOwner, const T& aData):
		State(aName, aOwner), mConf(aData), mUpd(aData) {
		    mOutput = new StateOutput<T>("Out", this, mSntf);
		};
	    virtual ~TState() {};
	    inline operator const T&() const { return mConf;};
	protected:
	    //virtual void* Conf() { return &mConf;};
	    //virtual void* Upd() {return &mUpd;};
	    //virtual int Len() const { return sizeof(T); };
	    virtual bool IsChanged() const { return !(mConf == mUpd);};
	    virtual void ApplyChange() { mConf = mUpd;};
	    /*
	    virtual void NotifyOutputs(ConnPoint* aOutput) {
		int cnt = aOutput->Required().size();
		mOutputsNotified = false;
		for (auto& iobs : aOutput->Required()) {
		    mNotifUnconfirmed.insert(iobs.second);
		    if (--cnt == 0) {
			mOutputsNotified = true;
		    }
		    MStateObserver<T>* obs = *iobs.second;
		    obs->OnStateChanged(&mSntf, mConf);
		}
	    };
	    */
	    virtual void DoNotifyOutput(MIface* aObserver) {
		MStateObserver<T>* obs = dynamic_cast<MStateObserver<T>*>(aObserver);
		obs->OnStateChanged(&mSntf, mConf);
	  };
	protected:
	    T mConf; // Confirmed data
	    T mUpd;  // Updated data
    };


    template<typename T> inline State::operator const TState<T>&() const { return dynamic_cast<const TState<T>& >(*this);};

    template<typename T> inline State::operator const T&() const { const TState<T>& st = *this; return st.operator const T&(); };


} // namespace desa


#endif   //  __DESA_STATE__
