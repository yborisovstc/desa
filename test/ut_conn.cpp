#include <stdlib.h>
#include "state.h"
#include "trans.h"
#include "system.h"
#include <iostream>
#include <thread>
#include <chrono>

#include <cppunit/extensions/HelperMacros.h>

using namespace desa;


class Ut_Conn : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Ut_Conn);
    CPPUNIT_TEST(test_SimpleUbc);
    CPPUNIT_TEST(test_Stio);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Stio();
    void test_SimpleUbc();
};

CPPUNIT_TEST_SUITE_REGISTRATION( Ut_Conn );

void Ut_Conn::setUp()
{
}

void Ut_Conn::tearDown()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}



class SubcIncr: public TState<int>
{
    public:
	SubcIncr(const string& aName, const int& aData, int aLim): TState<int>(aName, nullptr, aData), mInp("Inp", this, mSobs), mLim(aLim) {}
	virtual void Trans() {
	    mUpd = 1;
	    for (auto inp: mInp.Data()) {
		mUpd += inp.second;
		if (mUpd > mLim) {
		    mUpd = mLim;
		    break;
		}
	    }
	}
	StateInput<int> mInp;
	int mLim;
};

/**
 * @brief State of incrementors connected
 * It is MNbCpClient so listens response from connpoints being connected. 
 * Change state to true as soon of positive response
 */
class SubcRootInitConn: public TState<bool>
{
    public:
	SubcRootInitConn(const string& aName, const bool& aData): TState<bool>(aName, nullptr, aData) {}
	void onConnected(bool aRes) {
	    cout << "onConnected, aRes = " << aRes << endl;
	    mUpd = true;
	}
	// Transition
	virtual void Trans() {
	}
};

class SubcRoot: public System
{
    public:
	class trOnConnect: public Tr1<SubcRoot, bool> { public: trOnConnect(SubcRoot& a0): Tr1<SubcRoot, bool>(a0) {}
	    static Tr<bool>* get(SubcRoot& a0) { return new trOnConnect(a0);}
	    virtual void tr(const bool& aRes) override {
		m0.mConn->onConnected(aRes);
	    } };

	class sOnConnect: public Stf<SubcRoot, bool> { public: sOnConnect(SubcRoot* ac): Stf<SubcRoot, bool>(ac) {};
	    virtual void tr(const bool& aI) override {
		c->mConn->onConnected(aI);
	    } };

    public:
	SubcRoot(const string& aName): System(aName, nullptr), mS1(nullptr), mS2(nullptr) {
	    AddComp(mConn = new SubcRootInitConn("Conn", false));
	    AddComp(mS1 = new SubcIncr("S1", 4, 30));
	    AddComp(mS2 = new SubcIncr("S2", 2, 60));
	    ConnPointBase& o1 = mS1->Output();
	    ConnPointBase& i2 = mS2->mInp;
	    //auto handler = [this] (bool aRes) {this->mConn->onConnected(aRes);};
	    //o1.connect(i2, new function<void (bool)>(handler));
	    //o1.connect(i2, trOnConnect::get(*this));
	    auto* cb = new sOnConnect(this);
	    o1.isCompatible(&i2, false, cb);
	    //o1.connect(i2, cb);
	}
	SubcRootInitConn* mConn;
	SubcIncr* mS1;
	SubcIncr* mS2;
};

void Ut_Conn::test_SimpleUbc()
{


    printf("\n === Test of simple unblocking connection\n");
    SubcRoot syst("Root");
    syst.Run();

    //CPPUNIT_ASSERT_MESSAGE("Fail to get tst1", tst2 != 0);
}


class StateIoIncr: public TState<int>
{
    public:
	StateIoIncr(const string& aName, const int& aData, int aLim): TState<int>(aName, nullptr, aData), mInp("Inp", this, mSobs), mLim(aLim) {}
	virtual void Trans() {
	    mUpd = 1;
	    for (auto inp: mInp.Data()) {
		mUpd += inp.second;
		if (mUpd > mLim) {
		    mUpd = mLim;
		    break;
		}
	    }
	}
	StateInput<int> mInp;
	int mLim;
};

class SystIo1: public System
{
    public:
	SystIo1(const string& aName): System(aName, nullptr), mCp("Inp", this, "B", "A") {
	    AddComp(mIncr = new StateIoIncr("Incr1", 0, 6));
	    bool res = Connect(mIncr->Output(), mCp.mOut->Orig());
	    if (!res) 
		Connect(mIncr->Output(), *mCp.mInp);
	    res = res && Connect(mIncr->mInp, mCp.mInp->Orig());
	    CPPUNIT_ASSERT_MESSAGE("SystIo1: failed connecting input/outp", res);
	}
    public:
	StateIoIncr* mIncr;
	StateIo<int, int> mCp;	
};

class SystIo2: public System
{
    public:
	SystIo2(const string& aName): System(aName, nullptr), mCp("Inp", this, "A", "B") {
	    AddComp(mIncr = new StateIoIncr("Incr1", 0, 6));
	    bool res = Connect(mIncr->Output(), mCp.mOut->Orig());
	    res = res && Connect(mIncr->mInp, mCp.mInp->Orig());
	    CPPUNIT_ASSERT_MESSAGE("SystIo2: failed connecting input/outp", res);
	}
    public:
	StateIoIncr* mIncr;
	StateIo<int, int> mCp;	
};


class RootSyst: public System
{
    public:
	RootSyst(const string& aName): System(aName, nullptr) {
	    AddComp(mS1 = new SystIo1("S1"));
	    AddComp(mS2 = new SystIo2("S2"));
	    bool res = Connect(mS1->mCp, mS2->mCp);
	    CPPUNIT_ASSERT_MESSAGE("Failed connecting mS1 an mS2", res);
	}
	SystIo1* mS1;
	SystIo2* mS2;
};

void Ut_Conn::test_Stio()
{
    printf("\n === Test of StateIO connection\n");
    RootSyst syst("Root");
    syst.Run();

    //CPPUNIT_ASSERT_MESSAGE("Fail to get tst1", tst2 != 0);
}

