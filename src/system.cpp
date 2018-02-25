
#include <iostream>

#include "system.h"
#include "state.h"
#include <assert.h>


using namespace desa;

// Not active on init, to be activated via notif from comps
System::System(const string& aName, MOwner* aOwner): Comp(aName, aOwner), mOwnerImpl(*this),
    mIsActive(false), mState(ESt_Unknown)
{
    mStatusCur = &mStatus1;
    mStatusNext = &mStatus2;
}

System::~System()
{
    for (TCompRec cr: mComps) {
	delete cr.second;
    }
}

void System::HandleCompActivated(MComp* aComp)
{
    mStatusNext->push_back(aComp);
    if (!mIsActive) {
	mIsActive = true;
	if (mOwner != NULL) {
	    mOwner->OnCompActivated(this);
	}
    }
}

void System::HandleCompUpdated(MComp* aComp)
{
    assert(mState == ESt_UpdateRequesing);
    string::size_type ret = mRequested.erase(aComp);
    assert(ret == 1);
    if (mAllCompsUpdated && mRequested.empty()) {
	mState = ESt_Updated;
	if (mOwner != NULL) {
	    mOwner->OnCompUpdated(this);
	}
	mRq.unlock();
    }
}

void System::HandleCompConfirmed(MComp* aComp)
{
    assert(mState == ESt_ConfirmRequesting);
    string::size_type ret = mRequested.erase(aComp);
    assert(ret == 1);
    if (mAllCompsConfirmed && mRequested.empty()) {
	mState = ESt_Confirmed;
	if (mOwner != NULL) {
	    mOwner->OnCompConfirmed(this);
	}
	mRq.unlock();
    }

}

void System::Update()
{
    if (mState == ESt_Unknown || mState == ESt_Confirmed) {
	mState = ESt_UpdateRequesing;
    }
    assert(mRequested.empty());

    // After confirmation move to new step, swap status
    TActives* cur = mStatusCur;
    mStatusCur = mStatusNext;
    mStatusNext = cur;
    mStatusNext->clear();

    pair<set<MComp*>::iterator,bool> ret;
    mAllCompsUpdated = false;
    int cnt = mStatusCur->size();
    for (MComp* comp: *mStatusCur) {
	ret = mRequested.insert(comp);
	mAllCompsUpdated = (--cnt == 0);
	assert(ret.second);
	comp->Update();
    }
    // Update has been done, reset active flag now
    mIsActive = false;
}

void System::Confirm()
{
    if (mState == ESt_Unknown || mState == ESt_Updated) {
	mState = ESt_ConfirmRequesting;
    }
    // Comps can notify of status change during confirm, the status is collected for next step.
    assert(mRequested.empty());
    pair<set<MComp*>::iterator,bool> ret;
    mAllCompsConfirmed = false;
    int cnt = mStatusCur->size();
    for (MComp* comp: *mStatusCur) {
	ret = mRequested.insert(comp);
	mAllCompsConfirmed = (--cnt == 0);
	assert(ret.second);
	comp->Confirm();
    }
    // After confirmation move to new step, swap status
    /*
    TActives* cur = mStatusCur;
    mStatusCur = mStatusNext;
    mStatusNext = cur;
    mStatusNext->clear();
    */
}

void System::AddComp(Comp* aComp)
{
    assert(mComps.find(aComp->Name()) == mComps.end());
    aComp->SetOwner(&mOwnerImpl);
    mComps.insert(TCompRec(aComp->Name(), aComp));
    mStatusCur->push_back(aComp);
}

void System::Run()
{
    assert(mOwner == NULL);
    Confirm();
    while (mIsActive) {
	mRq.lock(); // Waiting for current cycle confirmation phase to be completed
	Update();
	mRq.lock(); // Waiting for previous cycle to be completed (update)
	Confirm();
    }
}

void System::DumpComps() const
{
    cout << "[" << Name() << "]" << " components:" << endl;
    for (auto &comp: mComps) {
	cout << &comp << ": " << comp.second->Name() << endl;
    }
    cout << endl;
}

void System::DumpActives() const
{
    cout << "[" << Name() << "]" << " actives:" << endl;
    cout << "Cur: ";
    for (auto &comp: *mStatusCur) {
	cout << "[" << dynamic_cast<Comp*>(comp)->Name() << "], ";
    }
    cout << endl;
    cout << "Next: ";
    for (auto &comp: *mStatusNext) {
	cout << "[" << dynamic_cast<Comp*>(comp)->Name() << "], ";
    }
    cout << endl;
}
