#ifndef __DESA_STATE__
#define __DESA_STATE__

#include "mdes.h"
#include "connpoint.h"
#include <vector>

namespace desa {

    using namespace std;

    template <typename T> class TState;

    /**
     * @brief Base state
     */
    class State: public Comp
    {
	friend class StateObserver;
	protected:
	    class StateObserver: public MStateObserver
	{
	    public:
		StateObserver(State& aHost): mHost(aHost) {};
		// From MStateObserver
		virtual void OnSourceChanged() { mHost.HandleSourceChanged();};
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
	    void HandleSourceChanged();
	protected:
	    StateObserver mSobs;
	    vector<ConnPoint*> mInputs;
	    ConnPointBase* mOutput;
	    bool mIsActive;
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
		    mOutput = new TConnPointP<MData<T>>("Out", MConnPoint::EOutput, mData);
		};
	    virtual ~TState() {};
	    inline operator const T&() const { return mConf;};
	protected:
	    virtual void* Conf() { return &mConf;};
	    virtual void* Upd() {return &mUpd;};
	    virtual int Len() const { return sizeof(T); };
	protected:
	    TData mData;
	    T mConf; // Confirmed data
	    T mUpd;  // Updated data
    };


    template<typename T> inline State::operator const TState<T>&() const { return dynamic_cast<const TState<T>& >(*this);};

    template<typename T> inline State::operator const T&() const { const TState<T>& st = *this; return st.operator const T&(); };


} // namespace desa


#endif   //  __DESA_STATE__
