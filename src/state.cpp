#include "state.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace desa;

State::State(const string& aName): Comp(aName), mSobs(*this), mOutput(NULL), mIsActive(true)
{
}

State::State(const string& aName, MOwner* aOwner): Comp(aName, aOwner), mSobs(*this), mOutput(NULL), mIsActive(true)
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
}

void State::Confirm()
{
    mIsActive = false;
    if (memcmp(Conf(), Upd(), Len()))
    {
	ConnPoint* cp = dynamic_cast<ConnPoint*>(mOutput);
	assert(cp != NULL);
	for (MIface* iobs : cp->Required()) {
	    MStateObserver* obs = *iobs;
	    obs->OnSourceChanged();
	}
    }
    memcpy(Conf(), Upd(), Len()); // Upd to Conf
}

void State::Run()
{
    assert(mOwner == NULL);

    while (mIsActive) {
	Update();
	Confirm();
    }
}

void State::HandleSourceChanged()
{
    mIsActive = true;
    if (mOwner != NULL) {
	mOwner->OnCompActivated(this);
    }
}
