#include "trans.h"
#include "connpoint.h"
#include <assert.h>
#include <iostream>
#include <functional>

using namespace desa;
using namespace std;

ConnPointBase::ConnPointBase(const string& aName, MConnPoint::TDir aDir, MBase* aCowner):
    MBase(aName, aCowner), mDir(aDir)
{
}

ConnPointBase::~ConnPointBase()
{
}

MIface *ConnPointBase::DoGetObj(const char *aName)
{
    if (aName == MConnPoint::type()) return this;
    else return nullptr;
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

#if 0
void ConnPointBase::doConnect(const MConnPoint& aCp, std::function<void (bool)>* aHandler)
{
    /*
    aCp.isCompatible();
    assert(IsCompatible(aCp) && aCp.IsCompatible(*this));
    for (auto& pair : mPairs) { if (pair == &aCp) { res = false; break; } }
    if (res) {
	mPairs.push_back(&aCp);
    }
    */
    (*aHandler)(true);
}
#endif

/*
using ctxOnIsCompatible = struct {ConnPointBase& self; const MConnPoint& cp; Tr<bool>& cb;};

class trOnIsCompatible2: public Trc<ctxOnIsCompatible, bool> { public:
    trOnIsCompatible2(ctxOnIsCompatible& a): Trc<ctxOnIsCompatible, bool>(a) {};
    static Tr<bool>* get(ctxOnIsCompatible& a) { return new trOnIsCompatible2(a);}
    virtual void tr(const bool& aT) override {
	bool res = true;
	for (auto& pair : m.self.mPairs) { if (pair == &m.cp) { res = false; break; } }
	if (res) {
	    m.self.mPairs.push_back(&m.cp);
	}
	m.cb(res);
    } };

class trOnIsCompatible: public Trc<ctxOnIsCompatible, bool> { public:
    trOnIsCompatible(ctxOnIsCompatible& a): Trc<ctxOnIsCompatible, bool>(a) {};
    static Tr<bool>* get(ctxOnIsCompatible& a) { return new trOnIsCompatible(a);}
    virtual void tr(const bool& aT) override {
	m.cp.isCompatible(m.self, false, *trOnIsCompatible2::get(m));
    } };
    */

class trOnIsCompatible2: public Stt<ConnPointBase, const MConnPoint, bool, bool> { public:
    trOnIsCompatible2(ConnPointBase* aC, const MConnPoint& aT, Tr<bool>& aN): Stt<ConnPointBase, const MConnPoint, bool, bool>(aC, aT, aN) {};
    virtual void tr(const bool& aT) override {
	bool res = true;
	for (auto& pair : c->mPairs) { if (pair == &t) { res = false; break; } }
	if (res) {
	    c->mPairs.push_back(&t);
	}
	n(res);
    } };

class trOnIsCompatible: public Stt<ConnPointBase, const MConnPoint, bool, bool> { public:
    trOnIsCompatible(ConnPointBase* aC, const MConnPoint& aT, Tr<bool>& aN): Stt<ConnPointBase, const MConnPoint, bool, bool>(aC, aT, aN) {};
    virtual void tr(const bool& aT) override {
	auto* cb = new trOnIsCompatible2(c, t, n);
	//t.isCompatible(*c, false, cb);
    } };


void ConnPointBase::doConnect(const MConnPoint& aCp, Tr<bool>* aNext)
{
    auto* cb = new trOnIsCompatible(this, aCp, *aNext);
    isCompatible(const_cast<MConnPoint*>(&aCp), false, cb);
}

bool ConnPointBase::Connect(const MConnPoint& aCp)
{
    bool res = DoConnect(aCp);
    if (res) {
	Notify();
    }
    return res;
}

void ConnPointBase::onDoConnect(bool aRes, std::function<void (bool)>* aHandler)
{
    if (aRes) {
	Notify();
    }
    (*aHandler)(true);
}

class trOnDoConnect: public St<ConnPointBase, bool, bool> { public: trOnDoConnect(ConnPointBase* aC, Tr<bool>& aN): St<ConnPointBase, bool, bool>(aC, aN) {}
    virtual void tr(const bool& aT) override {
	if (aT) {
	    c->Notify();
	}
	bool res = true;
	n(res);
    } };


void ConnPointBase::connect(const MConnPoint& aCp, Tr<bool>* aCb)
{
    auto* cb = new trOnDoConnect(this, *aCb);
    doConnect(aCp, cb);
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

void ConnPointBase::isCompatible(MConnPoint* aPair, bool aExtd, Tr<bool>* aCb)
{
    /* Removed for a while. TODO to ronsider to avoid using dir
    bool res = false;
    bool samedir = (aPair.Dir() == mDir);
    res = !aExtd && !samedir || aExtd && samedir;
    aCb(res);
    */
    (*aCb)(true);
}

void ConnPointBase::isCompatible2(MConnPoint* aPair, bool aExtd, Tr<bool>* aCb)
{
    (*aCb)(true);
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


// ConnPointBase, non-blocking


void ConnPointBase::dir(Tr<MConnPoint::TDir>* aCb) const
{
    (*aCb)(mDir);
};


// Extension


Extention::~Extention()
{
    if (mOrig != NULL) {
	delete mOrig;
    }
}

MIface *Extention::DoGetObj(const char *aName) {
    if (aName == MExtension::type())
	return dynamic_cast<MExtension*>(this);
    else
	return ConnPointBase::DoGetObj(aName);
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




void ConnPoint::SIsCompatible::operator()()
{
    //mThread = thread(&ConnPointBase::isCompatible, dynamic_cast<ConnPointBase*>(&mGc), &mPair, mExtd, &mStep1);
    //mThread.detach();
    thread tt(&ConnPointBase::isCompatible2, &mGc, &mPair, mExtd, &mStep1);
    //thread tt(&ConnPointBase::isCompatible, dynamic_cast<ConnPointBase*>(&mGc), &mPair, mExtd, &mStep1);
    tt.detach();
    //mGc.ConnPointBase::isCompatible(&mPair, mExtd, &mStep1);
}

void ConnPoint::SIsCompatible::St1::operator()(const bool& aI) {
    if (aI) {
	// Get MExtension from pair
	mOc.mPair.doGetObj(MExtension::type(), &mOc.mStep2);
    } else
	mOc.mCb(false);
}
void ConnPoint::SIsCompatible::St2::operator()(const MIface& aI) {
    auto& iface = const_cast<MIface&>(aI);
    if (&aI) { // Extension ?
	MExtension* ext = iface.GetObj(ext);
	assert(ext);
	// Get origin from extension
	//ext->orig(cb);
    } else { // Regular Cp
	// Get JointPr
	mOc.mPair.doGetObj(MJointPr::type(), &mOc.mStep3);
    }
}
void ConnPoint::SIsCompatible::St3::operator()(const MIface& aI) {
    if (&aI) {
	MJointPr* jp = const_cast<MIface&>(aI).GetObj(jp);
	assert(jp);
	mOc.mJp = jp;
	// Get provided type
	jp->providedType(&mOc.mStep4);
    } else {
	mOc.mCb(false);
    }
}
void ConnPoint::SIsCompatible::St4::operator()(const string& aI) {
    if (aI == mOc.mGc.mJointPr.RequiredType()) {
	// Get required type
	assert(mOc.mJp);
	mOc.mJp->requiredType(&mOc.mStep5);
    } else {
	mOc.mCb(false);
    }
}
void ConnPoint::SIsCompatible::St5::operator()(const string& aI) {
    if (aI == mOc.mGc.mJointPr.ProvidedType()) {
	mOc.mCb(true);
    } else {
	mOc.mCb(false);
    }
    delete &mOc;
}

MIface *ConnPoint::DoGetObj(const char *aName)
{
    MIface* res = nullptr;
    if (aName == MJointPr::type()) 
	res = &mJointPr;
    else
	res = ConnPointBase::DoGetObj(aName);
    return res;
}

bool ConnPoint::DoConnect(const MConnPoint& aCp)
{
    bool res =  ConnPointBase::DoConnect(aCp);
    if (res) {
	const Extention* ext = dynamic_cast<const Extention*>(&aCp);
	if (ext != NULL) {
	    MConnPoint& mpair = ext->Orig();
	    MJointPr* jp = mpair.GetObj(jp);
	    if (jp != NULL) {
		const MJointPr::TPairsIfaces& requireds = jp->Required();
		for (auto& rq : requireds) {
		    mRequired.insert(rq);
		}
	    } else {
		res = false;
	    }
	} else {
	    MJointPr* jp = const_cast<MConnPoint&>(aCp).GetObj(jp);
	    if (jp != NULL) {
		if (&jp->Provided() != NULL) {
		    mRequired.insert(MJointPr::TPairsIfacesElem(&aCp, &jp->Provided()));
		}
	    } else {
		res = false;
	    }
	}
    }
    return res;
}

class sOnCpbDoConnect_Cp5: public St<ConnPoint, bool, MIface> { public:
    sOnCpbDoConnect_Cp5(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MIface>(ac, an) {};
    virtual void tr(const MIface& aI) override {
	 }};

class sOnCpbDoConnect_Cp4: public St<ConnPoint, bool, MIface> { public:
    sOnCpbDoConnect_Cp4(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MIface>(ac, an) {};
    virtual void tr(const MIface& aI) override {
	if (&aI) {
	    MJointPr* jp = const_cast<MIface&>(aI).GetObj(jp);
	    assert(jp);
	    auto* cb = new sOnCpbDoConnect_Cp5(c, n);
	    jp->provided(cb);
	} else {
	    n(false);
	} }};

class sOnCpbDoConnect_Cp3: public St<ConnPoint, bool, MConnPoint> { public:
    sOnCpbDoConnect_Cp3(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MConnPoint>(ac, an) {};
    virtual void tr(const MConnPoint& aI) override {
	if (&aI) {
	    auto* cb = new sOnCpbDoConnect_Cp4(c, n);
	    // Get MJointPr from origin
	    MConnPoint& cp = const_cast<MConnPoint&>(aI);
	    cp.doGetObj(MJointPr::type(), cb);
	} else {
	    n(false);
	} }};


class sOnCpbDoConnect_Cp2: public St<ConnPoint, bool, MIface> { public:
    sOnCpbDoConnect_Cp2(ConnPoint* ac, Tr<bool>& an): St<ConnPoint, bool, MIface>(ac, an) {};
    virtual void tr(const MIface& aI) override {
	if (&aI) {
	    MExtension* ext = const_cast<MIface&>(aI).GetObj(ext);
	    assert(ext);
	    auto* cb = new sOnCpbDoConnect_Cp3(c, n);
	    // Get origin from extension
	    ext->orig(cb);
	} else {
	} }};

class sOnCpbDoConnect_Cp: public Stt<ConnPoint, MConnPoint, bool, bool> { public:
    sOnCpbDoConnect_Cp(ConnPoint* ac, MConnPoint& at, Tr<bool>& an): Stt<ConnPoint, MConnPoint, bool, bool>(ac, at, an) {};
    virtual void tr(const bool& aI) override {
	if (&aI) {
	    auto* cb = new sOnCpbDoConnect_Cp2(c, n);
	    // Get MExtension from pair
	    t.doGetObj(MExtension::type(), cb);
	} else {
	    n(aI);
	} }};


void ConnPoint::doConnect(const MConnPoint& aCp, Tr<bool>* aNext)
{
    auto* cb = new sOnCpbDoConnect_Cp(this, (MConnPoint&)aCp, *aNext);
    ConnPointBase::doConnect(aCp, cb);
}

bool ConnPoint::DoDisconnect(const MConnPoint& aCp)
{
    bool res =  ConnPointBase::DoDisconnect(aCp);
    if (res) {
	const ConnPoint* ccp = dynamic_cast<const ConnPoint*>(&aCp);
	MJointPr* jp = const_cast<MConnPoint&>(aCp).GetObj(jp);
	MIface* irq = &jp->Provided();
	for (MJointPr::TPairsIfaces::iterator it = mRequired.begin(); it != mRequired.end() && !res; it++) {
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
    if (res) {
	// Checking if pair is extension
	MConnPoint& gpair = const_cast<MConnPoint&>(aPair);
	MExtension* ext = gpair.GetObj(ext);
	MConnPoint* pair = (ext == nullptr) ? &gpair : const_cast<MConnPoint*>(&ext->Orig());
	MJointPr* jp = pair->GetObj(jp);
	bool extd = aExtd ^ (ext != NULL);
	if (extd) {
	    res = jp->RequiredType() == mJointPr.RequiredType() && jp->ProvidedType() == mJointPr.ProvidedType();
	} else {
	    res = jp->RequiredType() == mJointPr.ProvidedType() && jp->ProvidedType() == mJointPr.RequiredType();
	}
    }
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
    return ConnPointBase::IsConnected() && mJointPr.Required().size() != 0;
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
