#include "system.h"
#include "state.h"
#include <assert.h>


using namespace desa;

System::System(const string& aName, MOwner* aOwner): Comp(aName, aOwner), mOwnerImpl(*this),
    mIsActive(true), mState(ESt_Unknown)
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
    assert(mState == ESt_UpdateRequesing || mState == ESt_Updating);
    string::size_type ret = mRequested.erase(aComp);
    assert(ret == 1);
    if (mAllCompsUpdated && mRequested.empty()) {
	bool updating = (mState == ESt_Updating);
	mState = ESt_Updated;
	if (mOwner != NULL) {
	    mOwner->OnCompUpdated(this);
	} else if (updating) {
	    //Confirm();
	}
	mRq.unlock();
    }
}

void System::HandleCompConfirmed(MComp* aComp)
{
    assert(mState == ESt_ConfirmRequesting || mState == ESt_Confirming);
    string::size_type ret = mRequested.erase(aComp);
    assert(ret == 1);
    if (mAllCompsConfirmed && mRequested.empty()) {
	bool confirming = (mState == ESt_Confirming);
	mState = ESt_Confirmed;
	if (mOwner != NULL) {
	    mOwner->OnCompConfirmed(this);
	} else if (confirming) {
	    //Update();
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
    pair<set<MComp*>::iterator,bool> ret;
    mAllCompsUpdated = false;
    int cnt = mStatusCur->size();
    for (MComp* comp: *mStatusCur) {
	ret = mRequested.insert(comp);
	mAllCompsUpdated = (--cnt == 0);
	assert(ret.second);
	comp->Update();
    }
    if (mState == ESt_UpdateRequesing) {
	mState = ESt_Updating;
    }
}

void System::Confirm()
{
    mIsActive = false;
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
    TActives* cur = mStatusCur;
    mStatusCur = mStatusNext;
    mStatusNext = cur;
    mStatusNext->clear();
    if (mState == ESt_ConfirmRequesting) {
	mState = ESt_Confirming;
    }
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
    while (mIsActive) {
	mRq.lock();
	Confirm();
	/*
	if (mState != ESt_Confirmed) {
	    // If state isn't Confirmed this means that acks from components are not collected in the frame of Confirm
	    // So running the system will be continued on ack from comp.
	    break;
	}
	*/
	mRq.lock();
	Update();
	/*
	if (mState != ESt_Updated) {
	    break;
	}
	*/
    }
}

