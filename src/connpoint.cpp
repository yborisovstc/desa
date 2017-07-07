#include "connpoint.h"
#include <assert.h>

using namespace desa;

ConnPointBase::ConnPointBase(const string& aName, MConnPoint::TDir aDir):
    MBase(aName), mDir(aDir)
{
}

ConnPointBase::~ConnPointBase()
{
}

bool ConnPointBase::Connect(const MConnPoint& aCp)
{
    bool res = true;
    assert(IsCompatible(aCp) && aCp.IsCompatible(*this));
    for (auto& pair : mPairs) { if (pair == &aCp) { res = false; break; } }
    if (res) {
	mPairs.push_back(&aCp);
    }
    return res;
}

bool ConnPointBase::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    bool res = false;
    res = aPair.Dir() != mDir;
    return res;
}



// ConnPoint providing/requiring

ConnPoint::ConnPoint(const string& aName, MConnPoint::TDir aDir, MIface& aProvided):
    ConnPointBase(aName, aDir), mProvided(aProvided)
{
}

bool ConnPoint::Connect(const MConnPoint& aCp)
{
    bool res =  ConnPointBase::Connect(aCp);
    if (res) {
	const ConnPoint* ccp = dynamic_cast<const ConnPoint*>(&aCp);
	ConnPoint* cp = const_cast<ConnPoint*>(ccp);
	if (cp != NULL) {
	    mRequired.push_back(cp->Provided());	
	} else {
	    res = false;
	}
    }
    return res;
}

bool ConnPoint::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    bool res = ConnPointBase::IsCompatible(aPair, aExtd);
    return res;
}

// ConnPoint providing only

ConnPointP::ConnPointP(const string& aName, MConnPoint::TDir aDir, MIface& aProvided):
    ConnPointBase(aName, aDir), mProvided(aProvided)
{
}


// Extention


Extention::~Extention()
{
    if (mSrc != NULL) {
	delete mSrc;
    }
}

bool Extention::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    return mSrc->IsCompatible(aPair, !aExtd);
}

