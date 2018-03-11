#include <stdlib.h>
#include "state.h"
#include "system.h"
#include <iostream>
#include <thread>
#include <chrono>

#include <cppunit/extensions/HelperMacros.h>

using namespace desa;


class Ut_Base : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Ut_Base);
    CPPUNIT_TEST(test_Cre);
    CPPUNIT_TEST(test_Incr);
    CPPUNIT_TEST(test_Syst1);
    CPPUNIT_TEST(test_Extention1);
    CPPUNIT_TEST(test_Extention2);
    CPPUNIT_TEST(test_Async1);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Cre();
    void test_Incr();
    void test_Syst1();
    void test_Async1();
    void test_Extention1();
    void test_Extention2();
};

CPPUNIT_TEST_SUITE_REGISTRATION( Ut_Base );

void Ut_Base::setUp()
{
}

void Ut_Base::tearDown()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void Ut_Base::test_Cre()
{
    printf("\n === Test of creation of hier\n");

    int init_data = 0;
    TState<int>* tst1 = new TState<int>("Test", NULL, init_data);
    MBase* nd1 = tst1;
    TState<int> *tst2 = dynamic_cast<TState<int>* >(nd1);
    State *st1 = dynamic_cast<State* >(nd1);
    const TState<int>& tst3 = *st1;
    const int& d1 = tst3;
    const int& d2 = *st1;

    CPPUNIT_ASSERT_MESSAGE("Fail to get tst1", tst2 != 0);
}

class IncrementorState: public TState<int>
{
    public:
	IncrementorState(const string& aName, const int& aData, MOwner* aOwner = NULL);
	ConnPointBase& Input() { return mInp;};
	virtual void Update();
	void SetInc(int aInc) { mInc = aInc;}
	void SetLim(int aLim) { mLim = aLim;}
    private:
	StateInput<int> mInp;
	int mInc;
	int mLim;
};


IncrementorState::IncrementorState(const string& aName, const int& aData, MOwner* aOwner): TState<int>(aName, aOwner, aData),
    mInp("Inp", this, mSobs), mInc(1), mLim(4)
{
}

void IncrementorState::Update()
{
    mUpd = mInc;
    for (StateInput<int>::TDataElem inp: mInp.Data()) {
	mUpd += inp.second;
	if (mUpd > mLim) {
	    mUpd = mLim;
	    break;
	}
    }
    State::Update();
}

/**
 * @brief Test of creation and running single state
 */

void Ut_Base::test_Incr()
{
    printf("\n === Test of ruinning single state (Iterator)\n");
    int init_data = 0;
    IncrementorState* state = new IncrementorState("Incr", init_data);
    bool res = Connect(state->Input(), state->Output());
    CPPUNIT_ASSERT_MESSAGE("Fail to connect inp to output", res);
    state->Run();
    int data = *state;
    CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
    delete state;
}

class Syst1: public System
{
    public:
	Syst1(const string& aName, MOwner* aOwner): System(aName, aOwner) {
	    int init_data = 0;
	    mIncr = new IncrementorState("Syst1_incr", init_data, NULL);
	    bool res = Connect(mIncr->Input(), mIncr->Output());
	    CPPUNIT_ASSERT_MESSAGE("Fail to connect inp to output", res);
	    AddComp(mIncr);
	};
	IncrementorState& Incr() { return *mIncr;};
    protected:
	IncrementorState* mIncr;
};

/**
 * @brief Test of creation simple system
 * The system contains just one state - incrementor, system doesn't have conn points
 * Incrementor cycling synchronously, its conn points are connected internally
 * System should run until Incrementor limit get achieved (state gets inactive) and
 * then leave running cycle.
 * 
 */
void Ut_Base::test_Syst1()
{
    Syst1 syst("Syst1", NULL);
    printf("\n === Test of creating simple system and running it\n");
    syst.Run();
    int data = syst.Incr();
    CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
}


class Syst2: public Syst1
{
    public:
	Syst2(const string& aName, MOwner* aOwner);
    public:
	ExtStateInp<int>* mInp;
	ExtStateOut<int>* mOut;
};

Syst2::Syst2(const string& aName, MOwner* aOwner): Syst1(aName, aOwner) {
    mInp = new ExtStateInp<int>("Inp", this); 
    mOut = new ExtStateOut<int>("Out", this); 
    bool res = Disconnect(mIncr->Input(), mIncr->Output());
    CPPUNIT_ASSERT_MESSAGE("Fail to disconnect inp to output", res);
    res = Connect(mIncr->Input(), mInp->Orig());
    CPPUNIT_ASSERT_MESSAGE("Fail to connect state inp to ext", res);
    res = Connect(mIncr->Output(), mOut->Orig());
    CPPUNIT_ASSERT_MESSAGE("Fail to connect state out to ext", res);
};

/**
 * @brief Simple test of extention
 * Simple system with single incrementor state, system includes 2 conn points -
 * extentions, inp and out
 * 
 */
void Ut_Base::test_Extention1()
{
    printf("\n === Test of extentions, system with incrementor, loop thru system inp-out\n");
    Syst2 syst("Syst2", NULL);
    bool res = Connect(*syst.mInp, *syst.mOut);
    CPPUNIT_ASSERT_MESSAGE("Fail to connect system out to inp", res);
    syst.Run();
    int data = syst.Incr();
    CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
}

/** 
 * @brief Test of system's states async interacting 
 * There are system with two states:
 * first is incrementor, i.e. state adding some input value to the states value
 * second is the events generator of input value for incrementor. This state is async to the incrementor
 */

class EventsGenerator: public TState<int>
{
    public:
	EventsGenerator(const int& aData, MOwner* aOwner = NULL):
	    TState<int>("Evg", aOwner, aData), mInp("Inp", this, mSobs), mInpCount("InpCount", this, mSobs) {};
	ConnPointBase& Input() { return mInp;};
	ConnPointBase& InpCount() { return mInpCount;};
	virtual void Update();
	virtual void Confirm();
    private:
	void DoUpdate();
    private:
	StateInput<int> mInp;
	StateInput<int> mInpCount;
};

void Pause(int aDelay) {
    this_thread::sleep_for(chrono::seconds(aDelay));
}

void EventsGenerator::DoUpdate()
{
    Pause(1);
    mUpd = 1;
    State::Update();
}

void EventsGenerator::Update() {
    assert(mInp.Data().size() == 1);
    int data = mInp.Data().begin()->second;
    int count = mInpCount.Data().begin()->second;
    cout << "EventsGenerator::Update, data: " << data << ", count: " << count << endl;
    if (count < 4) {
	if (data == 1) {
	    mUpd = 0;
	    State::Update();
	} else {
	    thread tt(&EventsGenerator::DoUpdate, this);
	    tt.detach();
	}
    } else {
	State::Update();
    }
};

void EventsGenerator::Confirm()
{
    //cout << "EventsGenerator::Confirm" << endl;
    State::Confirm();
}

class Incr2: public TState<int>
{
    public:
	Incr2(const int& aData, MOwner* aOwner = NULL): TState<int>("Incr", aOwner, aData), mInp("Inp", this, mSobs) {};
	ConnPointBase& Input() { return mInp;};
	virtual void Update();
	virtual void Confirm();
    private:
	StateInput<int> mInp;
};

void Incr2::Update()
{
    mUpd = 0;
    for (StateInput<int>::TDataElem inp: mInp.Data()) {
	if (mUpd < 4) {
	    mUpd += inp.second;
	}
    }
    //cout << "Incr2::Update, " << mUpd << " -> " << mConf << endl;
    State::Update();
}

void Incr2::Confirm()
{
    //cout << "Incr2::Confirm" << endl;
    State::Confirm();
}

class Syst3: public System
{
    public:
	Syst3(const string& aName, MOwner* aOwner);
	Incr2& Incr() { return *mIncr;};
	virtual void Update();
	virtual void Confirm();
    protected:
	Incr2* mIncr;
	EventsGenerator* mEvg;
};

Syst3::Syst3(const string& aName, MOwner* aOwner): System(aName, aOwner) {
    int init_data = 0;
    mIncr = new Incr2(init_data, NULL);
    mEvg = new EventsGenerator(init_data, NULL);
    bool res = Connect(mIncr->Input(), mIncr->Output());
    CPPUNIT_ASSERT_MESSAGE("Fail to connect incr inp to output", res);
    res = Connect(mEvg->Input(), mEvg->Output());
    res = Connect(mIncr->Input(), mEvg->Output());
    res = Connect(mEvg->InpCount(), mIncr->Output());
    CPPUNIT_ASSERT_MESSAGE("Fail to connect incr inp to evg output", res);
    AddComp(mIncr);
    AddComp(mEvg);
};

void Syst3::Update()
{
    //cout << "Syst3::Update" << endl;
    System::Update();
}

void Syst3::Confirm()
{
    //cout << "Syst3::Confirm" << endl;
    System::Confirm();
}

void Ut_Base::test_Async1()
{
    printf("\n === Test of system states async interacting\n");
    Syst3 syst("Syst3", NULL);
    syst.Run();
    int data = syst.Incr();
    CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
}




/**
 * @brief System containig two sybsystems.
 *
 * Checking connection via extentions
 */
class Syst4: public System
{
    public:
	Syst4(const string& aName);
    public:
	IncrementorState* mIncr;
	Syst2* mS1;
	//Syst2* mS2;
};

Syst4::Syst4(const string& aName): System(aName, NULL)
{
    int init_data = 0;
    AddComp(mIncr = new IncrementorState("Incr", init_data));
    AddComp(mS1 = new Syst2("S1", NULL));
    mS1->Incr().SetInc(0);
    mS1->Incr().SetLim(100);
    //AddComp(mS2 = new Syst2("S2", NULL));
    bool res = Connect(mIncr->Input(), mIncr->Output());
    res = res && Connect(mS1->Incr().Input(), mS1->Incr().Output());
    res = res && Connect(mIncr->Output(), *mS1->mInp);
    //res = res && Connect(*mS1->mOut, *mS2->mInp);
    CPPUNIT_ASSERT_MESSAGE("Fail to connect incr inp to output", res);
}


/**
 * @brief Simple test of extention
 * State output to system input (extention)
 * 
 */
void Ut_Base::test_Extention2()
{
    cout << endl <<  "=== Test of extentions, State output to system input (extention)" << endl;
    Syst4 syst("Syst4");
    syst.Run();
    int data = syst.mS1->Incr();
    cout << "data: " << data << endl;
    //CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
}
