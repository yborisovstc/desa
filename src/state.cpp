#include "state.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace desa;

State::State(const string& aName): Comp(aName), mSobs(*this), mSntf(*this), mOutput(NULL), mIsActive(true), mUninit(true)
{
}

State::State(const string& aName, MOwner* aOwner): Comp(aName, aOwner), mSobs(*this), mSntf(*this), mOutput(NULL), mIsActive(true),
    mUninit(true), mOutputsNotified(false)
{
}

State::~State()
{
    if (mOutput != NULL) {
	delete mOutput;
    }
};

void State::Update()
{
    if (mOwner != NULL) {
	mOwner->OnCompUpdated(this);
    }
}

void State::Confirm()
{
    mIsActive = false;
    if (memcmp(Conf(), Upd(), Len()) || mUninit) {
	ConnPoint* cp = dynamic_cast<ConnPoint*>(mOutput);
	assert(cp != NULL);
	memcpy(Conf(), Upd(), Len()); // Upd to Conf
	NotifyOutputs(cp);
	mUninit = false;
    } else {
	if (mOwner != NULL) {
	    mOwner->OnCompConfirmed(this);
	}
    }
}

void State::Run()
{
    assert(mOwner == NULL);

    while (mIsActive) {
	Confirm();
	Update();
    }
}

void State::HandleInputChanged()
{
    if (!mIsActive) {
	mIsActive = true;
	if (mOwner != NULL) {
	    mOwner->OnCompActivated(this);
	}
    }
}

void State::HandleStateChangeHandled(MIface* aObserver)
{
    TIfSet::size_type res = mNotifUnconfirmed.erase(aObserver);
    assert(res == 1);
    if (mOutputsNotified && mNotifUnconfirmed.empty()) {
	if (mOwner != NULL) {
	    mOwner->OnCompConfirmed(this);
	}
    }
}

void State::NotifyOutputs(ConnPoint* aOutput)
{
    int cnt = aOutput->Required().size();
    mOutputsNotified = false;
    for (auto& iobs : aOutput->Required()) {
	mNotifUnconfirmed.insert(iobs.second);
	if (--cnt == 0) {
	    mOutputsNotified = true;
	}
	DoNotifyOutput(iobs.second);
    }
}
