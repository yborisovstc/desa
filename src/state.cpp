#include "state.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace desa;

// Not active on init, to be activated via notif from deps
State::State(const string& aName): Comp(aName), mSobs(*this), mSntf(*this), mOutput(NULL), mIsActive(false), mUninit(true)
{
}

State::State(const string& aName, MOwner* aOwner): Comp(aName, aOwner), mSobs(*this), mSntf(*this), mOutput(NULL), mIsActive(false),
    mUninit(true), mOutputsNotified(false), mEnableDanglingOutput(false)
{
}

State::~State()
{
    if (mOutput != NULL) {
	delete mOutput;
    }
};

MIface *State::DoGetObj(const char *aName)
{
    if (aName == MComp::type())
	return this;
    else
	return Comp::DoGetObj(aName);
}

void State::Update()
{
    Trans();
    NotifyCompUpdated();
    // Update has been done, reset active flag now
    mIsActive = false;
}

void State::Confirm()
{
    if (IsChanged() || mUninit) {
	ConnPoint* cp = dynamic_cast<ConnPoint*>(mOutput);
	assert(cp != NULL);
	ApplyChange(); // Upd to Conf
	if (mOutput->IsConnected()) {
	    NotifyOutputs(cp);
	} else if (!mEnableDanglingOutput) {
	    assert(false);
	} else {
	    NotifyCompConfirmed();
	}
	mUninit = false;
    } else {
	NotifyCompConfirmed();
    }
}

void State::Run()
{
    assert(mOwner == NULL);

    Confirm();
    while (mIsActive) {
	Update();
	Confirm();
    }
}

void State::HandleInputChanged()
{
    if (!mIsActive) {
	mIsActive = true;
	NotifyCompActivated();
    }
}

void State::HandleStateChangeHandled(MIface* aObserver)
{
    TIfSet::size_type res = mNotifUnconfirmed.erase(aObserver);
    assert(res == 1);
    if (mOutputsNotified && mNotifUnconfirmed.empty()) {
	NotifyCompConfirmed();
    }
}

void State::NotifyOutputs(ConnPoint* aOutput)
{
    MJointPr* out = aOutput->GetObj(out);
    int cnt = out->Required().size();
    mOutputsNotified = false;
    for (auto& iobs : out->Required()) {
	mNotifUnconfirmed.insert(iobs.second);
	if (--cnt == 0) {
	    mOutputsNotified = true;
	}
	DoNotifyOutput(iobs.second);
    }
}
