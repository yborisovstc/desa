#include "system.h"
#include "state.h"
#include <assert.h>


using namespace desa;

System::System(const string& aName, MOwner* aOwner): Comp(aName, aOwner), mOwnerImpl(*this)
{
}

System::~System()
{
    for (TCompRec cr: mComps) {
	delete cr.second;
    }
}

void System::HandleCompActivated(MComp* aComp)
{
}

void System::Update()
{
}

 void System::Confirm()
{
}

void System::AddComp(Comp* aComp)
{
    assert(mComps.find(aComp->Name()) == mComps.end());
    aComp->SetOwner(&mOwnerImpl);
    mComps.insert(TCompRec(aComp->Name(), aComp));
}

void System::Run()
{
    assert(mOwner == NULL);

    while (mIsActive) {
	Update();
	Confirm();
    }
}

