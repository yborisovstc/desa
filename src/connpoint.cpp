#include "connpoint.h"
#include <assert.h>
#include <iostream>

using namespace desa;
using namespace std;

ConnPointBase::ConnPointBase(const string& aName, MConnPoint::TDir aDir, MBase* aCowner):
    MBase(aName, aCowner), mDir(aDir)
{
}

ConnPointBase::~ConnPointBase()
{
}

void ConnPointBase::Notify(MConnPoint* aExclude)
{
    // TODO We have actually point-to-point topology, so pairs are not dependent one to another
    // Thus there is no need to notify pairs. To remove?
    /*
    for (auto& pair : mPairs) {
	if (pair != aExclude) {
	    MConnPoint* pp = const_cast<MConnPoint*>(pair);
	    pp->OnPairChanged(this);	
	}
    } */
}

bool ConnPointBase::DoConnect(const MConnPoint& aCp)
{
    bool res = true;
    assert(IsCompatible(aCp) && aCp.IsCompatible(*this));
    for (auto& pair : mPairs) { if (pair == &aCp) { res = false; break; } }
    if (res) {
	mPairs.push_back(&aCp);
    }
    return res;
}

bool ConnPointBase::Connect(const MConnPoint& aCp)
{
    bool res = DoConnect(aCp);
    if (res) {
	Notify();
    }
    return res;
}

bool ConnPointBase::DoDisconnect(const MConnPoint& aCp)
{
    bool res = false;
    for (TPairs::iterator it = mPairs.begin(); it != mPairs.end() && !res; it++) {
	if (*it == &aCp) {
	    res = true; mPairs.erase(it);
	}
    }
    return res;
}

bool ConnPointBase::Disconnect(const MConnPoint& aCp)
{
    bool res = DoDisconnect(aCp);
    if (res) {
	Notify();
    }
    return res;
}

bool ConnPointBase::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    bool res = false;
    bool samedir = (aPair.Dir() == mDir);
    res = !aExtd && !samedir || aExtd && samedir;
    return res;
}

void ConnPointBase::OnMediatorChanged(MConnPoint* aMediator)
{
    // TODO Preliminary solution, to re-design
    MExtension* ext = dynamic_cast<MExtension*>(aMediator);
    if (ext) {
	const MConnPoint& md = ext->Orig();
	const MExtension* mdext = dynamic_cast<const MExtension*>(&md);
	const MConnPoint* mdp = mdext ? &mdext->Orig() : &md;
    }
}

void ConnPointBase::OnPairChanged(MConnPoint* aPair)
{
    // Refresh connection
    DoDisconnect(*aPair);
    DoConnect(*aPair);
    Notify(aPair);
}

bool ConnPointBase::IsConnected() const
{
    return !mPairs.empty();
}

/*
const string ConnPointBase::getUri() const
{
    const MBase* owner = nullptr;
    std::string uri = (owner != nullptr) ? owner->getUri(): "?";
    uri.append("/"); uri.append(Name());
    return uri;
};
*/

void ConnPointBase::Dump() const
{
    cout << "ConnPointBase [" << Name() << "]" << endl;
    cout << "Dir: " << ((mDir == EInput)?"Inp":"Out") << endl;
    cout << "Pairs: ";
    for (auto& pair: mPairs) {
	const MBase* pbase = pair->MConnPoint_Base();
	cout << pair << "-" << pbase->getUri() << ", ";
    }
    cout << endl;

}


// Extension


Extention::~Extention()
{
    if (mOrig != NULL) {
	delete mOrig;
    }
}

void Extention::Notify(MConnPoint* aExclude)
{
    mOrig->OnPairChanged(this);
}

void Extention::Dump() const
{
    ConnPointBase::Dump();
    cout << "Extention" << endl;
    cout << "Orig: " << mOrig << endl;
    cout << endl;
}

bool Extention::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    bool res = false;
    const Extention* pext = dynamic_cast<const Extention*>(&aPair);
    if (pext == NULL) {
	res = mOrig->IsCompatible(aPair, !aExtd);
    } else {
	res = mOrig->IsCompatible(pext->Orig(), aExtd);
    }
    return res;
}




// ConnPoint providing/requiring

bool ConnPoint::DoConnect(const MConnPoint& aCp)
{
    bool res =  ConnPointBase::DoConnect(aCp);
    if (res) {
	const Extention* ext = dynamic_cast<const Extention*>(&aCp);
	if (ext != NULL) {
	    const MConnPoint& mpair = ext->Orig();
	    const ConnPoint* ccp = dynamic_cast<const ConnPoint*>(&mpair);
	    ConnPoint* cp = const_cast<ConnPoint*>(ccp);
	    if (cp != NULL) {
		const TPairsIfaces& requireds = cp->Required();
		for (auto& rq : requireds) {
		    mRequired.insert(rq);
		}
	    } else {
		res = false;
	    }
	} else {
	    const ConnPoint* ccp = dynamic_cast<const ConnPoint*>(&aCp);
	    ConnPoint* cp = const_cast<ConnPoint*>(ccp);
	    if (cp != NULL) {
		if (&cp->Provided() != NULL) {
		    mRequired.insert(TPairsIfacesElem(&aCp, &cp->Provided()));
		}
	    } else {
		res = false;
	    }
	}
    }
    return res;
}

bool ConnPoint::DoDisconnect(const MConnPoint& aCp)
{
    bool res =  ConnPointBase::DoDisconnect(aCp);
    if (res) {
	const ConnPoint* ccp = dynamic_cast<const ConnPoint*>(&aCp);
	ConnPoint* cp = const_cast<ConnPoint*>(ccp);
	MIface* irq = &cp->Provided();
	for (TPairsIfaces::iterator it = mRequired.begin(); it != mRequired.end() && !res; it++) {
	    if (it->second == irq) {
		res = true; mRequired.erase(it);
	    }
	}
    }
    return res;
}

bool ConnPoint::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    bool res = ConnPointBase::IsCompatible(aPair, aExtd);
    return res;
}

void ConnPoint::OnPairChanged(MConnPoint* aPair)
{
    ConnPointBase::OnPairChanged(aPair);
    /*
    // Refresh connection
    DoDisconnect(*aPair);
    DoConnect(*aPair);
    Notify(aPair);
    */
}

bool ConnPoint::IsConnected() const
{
    return ConnPointBase::IsConnected() && Required().size() != 0;
}

void ConnPoint::Dump() const
{
    ConnPointBase::Dump();
    cout << "ConnPoint [" << Name() << "]" << endl;
    cout << "Provided: " << mProvided << endl;
    cout << "Required: "; for (auto& elem: mRequired) { cout << "[" << elem.first << "-" << elem.second << "] ";}; cout << endl;
}



// ConnPoint providing only

ConnPointP::ConnPointP(const string& aName, MConnPoint::TDir aDir, MIface* aProvided):
    ConnPointBase(aName, aDir), mProvided(aProvided)
{
}



Socket::~Socket()
{
    for (auto comp : mPins) {
	ConnPointBase* pin = comp.second;
	delete pin;
    }
    mPins.clear();
}

bool Socket::IsCompatible(const MConnPoint& aPair, bool aExtd) const
{
    bool res = true;
    const MExtension* ext = dynamic_cast<const MExtension*>(&aPair);
    const Socket* pair = dynamic_cast<const Socket*>(ext ? &ext->Orig() : &aPair);
    bool isext = ext ? !aExtd : aExtd;
    if (pair != nullptr) {
	for (auto comp : mPins) {
	    if (pair->pins().count(comp.first) == 0) {
		res = false; break;
	    } else {
		ConnPointBase* ppin = pair->pins().at(comp.first);
		res = comp.second->IsCompatible(*ppin, isext);
	    }
	    if (!res) {
		break;
	    }
	}
    } else {
	res = false;
    }
    return res;
}

bool Socket::DoConnect(const MConnPoint& aCp)
{
    bool res = true;
    const Extention* ext = dynamic_cast<const Extention*>(&aCp);
    const Socket* psock = dynamic_cast<const Socket*>(ext ? &ext->Orig() : &aCp);
    if (psock) {
	for (auto rpin: mPins) {
	    MConnPoint* ppair = psock->pinAt(rpin.first);
	    res = rpin.second->Connect(*ppair);
	    if (!res) break;
	}
    }
    res = res && ConnPointBase::DoConnect(*psock);
    return res;
}
