#include <stdlib.h>
#include "state.h"
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

class SubcRoot: public System
{
    public:
	SubcRoot(const string& aName): System(aName, nullptr) {
	    AddComp(mS1 = new SubcIncr("S1", 4, 30));
	    AddComp(mS2 = new SubcIncr("S2", 2, 60));
	}
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

