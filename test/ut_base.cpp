#include <stdlib.h>
#include "state.h"
#include "system.h"

#include <cppunit/extensions/HelperMacros.h>

using namespace desa;


class Ut_Base : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Ut_Base);
    CPPUNIT_TEST(test_Cre);
    CPPUNIT_TEST(test_Incr);
    CPPUNIT_TEST(test_Syst1);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Cre();
    void test_Incr();
    void test_Syst1();
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
	IncrementorState(const int& aData, MOwner* aOwner = NULL);
	ConnPoint& Input() { return mInp;};
	virtual void Update();
    private:
	TConnPoint<MStateObserver, MData<int> > mInp;
};


IncrementorState::IncrementorState(const int& aData, MOwner* aOwner): TState("Incr", aOwner, aData),
    mInp("Inp", MConnPoint::EInput, mSobs)
{
    mOutput = new TConnPointP<MData<int>>("Out", MConnPoint::EOutput, mData);
}

void IncrementorState::Update()
{
    for (MIface* req: mInp.Required()) {
	MData<int>* tr = *req;
	int inp = tr->Data();
	if (mUpd < 4) {
	    mUpd = inp + 1;
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
    IncrementorState* state = new IncrementorState(init_data);
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
	Syst1(): System("Syst1", NULL) {
	    int init_data = 0;
	    mIncr = new IncrementorState(init_data, NULL);
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
    Syst1 syst;
    printf("\n === Test of creating simple system and running it\n");
    syst.Run();
    int data = syst.Incr();
    CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
}


