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
	public:
	    virtual void Run();
	    // From MComp
	    virtual void Update();
	    virtual void Confirm();
	protected:
	    virtual void* Conf() { return NULL;};
	    virtual void* Upd() { return NULL;};
	    virtual int Len() const { return 0;};
	    virtual void NotifyOutputs(ConnPoint* aOutput);
	    void HandleInputChanged();
	    void HandleStateChangeHandled(MIface* aObserver);
	    virtual void DoNotifyOutput(MIface* aObserver) = 0;
	protected:
	    InputObserver mSobs;
	    StateNotifier mSntf;
	    vector<ConnPoint*> mInputs;
	    ConnPointBase* mOutput;
	    bool mIsActive;
	    bool mUninit; // Sign of state is not initialized yet
	    TIfSet mNotifUnconfirmed; // Set of notifiers not yet confirmed notification
	    bool mOutputsNotified;
    };


    /**
     * @brief Customized state
     */
    template<typename T> class TState: public State
    {
	friend class TData;
	protected:
	    class TData: public MData<T>
	{
	    public:
		TData(TState<T>& aHost): mHost(aHost) {};
		virtual const T& Data() const { return mHost.mConf;};
	    private:
		TState<T>& mHost;
	};
	public:
	    TState(const string& aName, MOwner* aOwner, const T& aData):
		State(aName, aOwner), mConf(aData), mUpd(aData), mData(*this) {
		    mOutput = new TConnPoint<MStateNotifier, MStateObserver<T>>("Out", MConnPoint::EOutput, mSntf);
		};
	    virtual ~TState() {};
	    inline operator const T&() const { return mConf;};
	protected:
	    virtual void* Conf() { return &mConf;};
	    virtual void* Upd() {return &mUpd;};
	    virtual int Len() const { return sizeof(T); };
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
		MStateObserver<T>* obs = *aObserver;
		obs->OnStateChanged(&mSntf, mConf);
	    };
	protected:
	    TData mData;
	    T mConf; // Confirmed data
	    T mUpd;  // Updated data
    };


    template<typename T> inline State::operator const TState<T>&() const { return dynamic_cast<const TState<T>& >(*this);};

    template<typename T> inline State::operator const T&() const { const TState<T>& st = *this; return st.operator const T&(); };


} // namespace desa


#endif   //  __DESA_STATE__
