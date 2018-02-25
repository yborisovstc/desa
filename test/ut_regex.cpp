#include <stdlib.h>
#include "state.h"
#include "system.h"
#include <iostream>
#include <thread>
#include <chrono>

#include <cppunit/extensions/HelperMacros.h>

using namespace desa;

/**
 * @brief Unit test of regex engine
*/

class Ut_Regex : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Ut_Regex);
//    CPPUNIT_TEST(test_OneChar);
    CPPUNIT_TEST(test_ConcatTwoOneChar);
    CPPUNIT_TEST(test_Concat2);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_OneChar();
    void test_ConcatTwoOneChar();
    void test_Concat2();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Ut_Regex );

void Ut_Regex::setUp()
{
}

void Ut_Regex::tearDown()
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

//typedef struct { int pos; char symbol;} TSourceElem;
struct TSourceElem
{
    int pos; char symbol;
    TSourceElem(int aPos, char aSymbol): pos(aPos), symbol(aSymbol) {};
    TSourceElem(const TSourceElem& src): pos(src.pos), symbol(src.symbol) {};
    bool operator==(const TSourceElem& s) const {return pos == s.pos && symbol == s.symbol;};
};

/**
 * @brief Source of text being processed. This state is not "right" state because it contain "count" state data that is not exposed.
 */
class Source: public TState<TSourceElem>
{
    public:
	Source(const string& aName, const string& aContent): TState<TSourceElem>(aName, NULL, {0, 0}), mCount(0), mContent(aContent) {};
	virtual ~Source() {};
	virtual void Update() override;
	virtual void Confirm() override;
    private:
	int mCount;
	string mContent;
};

void Source::Update()
{
    mUpd = { mCount, mContent.at(mCount++)};
    cout << "Source::Update, [" << mUpd.pos << ", " << mUpd.symbol << "]" << endl;
    State::Update();
};

void Source::Confirm()
{
    State::Confirm();
    // Restore active till content is not earned
    if (mCount < (mContent.size() - 1)) {
	mIsActive = true;
	NotifyCompActivated();
    }
}

/**
 * @brief One char matching atom
 */
class AtomOneChar: public TState<TSourceElem>
{
    public:
	AtomOneChar(const string& aName, char aPattern): TState<TSourceElem>(aName, NULL, {-1, 0}), mInp("Inp", mSobs), mInpPresursor("InpPresursor", mSobs),
	    mPattern(aPattern) {};
	virtual void Trans() override;
    public:
	StateInput<TSourceElem> mInp;
	StateInput<TSourceElem> mInpPresursor;
	char mPattern;
};

void AtomOneChar::Trans()
{
    /*
    if (mInpPresursor.IsConnected()) {
	auto &inp = mInp.Data().begin()->second;
	auto &inpp = mInpPresursor.Data().begin()->second;
	if (inp.pos == inpp.pos +1 && inp.symbol == mPattern) {
	    mUpd = inp;
	}
    } else {
	auto &inp = mInp.Data().begin()->second;
	if (inp.symbol == mPattern) {
	    mUpd = inp;
	}
    }
    cout << Name() << " Update, [" << mUpd.pos << ", " << mUpd.symbol << "]" << endl;
    */
    if (mInpPresursor.IsConnected()) {
	auto &inp = mInp.Data().begin()->second;
	for (auto& inpd: mInpPresursor.Data()) {
	    auto &inpp = inpd.second;
	    if (inp.pos == inpp.pos + 1 && inp.symbol == mPattern) {
		mUpd = inp;
		break;
	    }
	}
    } else {
	auto &inp = mInp.Data().begin()->second;
	if (inp.symbol == mPattern) {
	    mUpd = inp;
	}
    }
    cout << Name() << " Update, [" << mUpd.pos << ", " << mUpd.symbol << "]" << endl;

}

/**
 * @brief Match handler base
 */
class MultiBase: public TState<TSourceElem>
{
    public:
	MultiBase(const string& aName): TState<TSourceElem>(aName, NULL, {-1, 0}), mInp("Inp", mSobs), mInpPresursor("InpPresursor", mSobs) {};
    public:
	StateInput<TSourceElem> mInp;
	StateInput<TSourceElem> mInpPresursor;
};

/**
 * @brief Multi One-or-More (\+)
 */
class MultiOneOrMore: public MultiBase
{
    public:
	MultiOneOrMore(const string& aName): MultiBase(aName) {};
	virtual void Trans() override {
	    auto &inp = mInp.Data().begin()->second;
	    mUpd = inp;
	    cout << Name() << " Update, [" << mUpd.pos << ", " << mUpd.symbol << "]" << endl;
	}
};

/**
 * @brief Main system that includes text source and test processing
 */
class MainSystem: public System
{
    public:
	MainSystem(const string& aName, MOwner* aOwner);
	virtual ~MainSystem();
	virtual void Update() override;
    protected:
	Source* mSource;
	AtomOneChar* mAtom1;
};


MainSystem::MainSystem(const string& aName, MOwner* aOwner): System(aName, aOwner)
{
    mSource = new Source("Source", "Test content one");
    mAtom1 = new AtomOneChar("Atom1", 'e');
    AddComp(mSource);
    AddComp(mAtom1);
    bool res = Connect(mAtom1->mInp, mSource->Output());
}

MainSystem::~MainSystem()
{
    delete mSource;
    delete mAtom1;
}

void MainSystem::Update()
{
    cout << "MainSystem::Update" << endl;
    System::Update();
}

void Ut_Regex::test_OneChar()
{
    cout << " === Test of one char atom" << endl;
    MainSystem syst("MainSystem", NULL);
    syst.Run();
    //CPPUNIT_ASSERT_MESSAGE("Incorrect data", data == 4);
}




/**
 * Concat 'e' then 'n' one-or-more times: en\+
 */
class Concat1: public System
{
    public:
	Concat1(const string& aName, MOwner* aOwner);
	virtual ~Concat1();
	virtual void Update() override;
    protected:
	Source* mSource;
	AtomOneChar* mAtom1;
	AtomOneChar* mAtom2;
};

Concat1::Concat1(const string& aName, MOwner* aOwner): System(aName, aOwner)
{
    mSource = new Source("Source", "Test contennnt one");
    mAtom1 = new AtomOneChar("Atom1", 'e');
    mAtom2 = new AtomOneChar("Atom2", 'n');
    AddComp(mSource);
    AddComp(mAtom1);
    AddComp(mAtom2);
    bool res = Connect(mAtom1->mInp, mSource->Output());
    res = Connect(mAtom2->mInp, mSource->Output());
    res = Connect(mAtom2->mInpPresursor, mAtom1->Output());
    res = Connect(mAtom2->Output(), mAtom2->mInpPresursor);
}

Concat1::~Concat1()
{
}

void Concat1::Update()
{
    cout << endl << "Concat1::Update" << endl;
    System::Update();
}


/**
 * Testing concat 'e' then 'n' one-or-more times: en\+
 */
void Ut_Regex::test_ConcatTwoOneChar()
{
    cout << " === Test of concat of two one char atom" << endl;
    Concat1 syst("Concat1", NULL);
    syst.Run();
}

/**
 * @brief Counter with reset on achieving defined threshold
 */
class Count: public TState<int>
{
    public:
	Count(const string& aName, int aMatchCnt): TState<int>(aName, NULL, 0), mMatchCnt(aMatchCnt), mInp("Inp", mSobs), mSelf("Self", mSobs) {};
	virtual void Trans() override {
	    auto &inp = mInp.Data().begin()->second.pos;
	    auto &self = mSelf.Data().begin()->second;
	    if (inp > -1 && self < mMatchCnt) {
		mUpd = self + 1;
	    }
	    cout << Name() << " Update, [" << mUpd  << "]" << endl;
	}
    public:
	StateInput<TSourceElem> mInp;
	StateInput<int> mSelf;
    protected:
	int mMatchCnt;
};

/**
 * @brief Switch fixing match on exact matching count and defined match number
 */
class MultiExactSwitch: public TState<TSourceElem>
{
    public:
	MultiExactSwitch(const string& aName, int aMatchCnt): TState<TSourceElem>(aName, NULL, {-1, 0}), mMatchCnt(aMatchCnt),
	    mInp("Inp", mSobs), mInpCount("InpCount", mSobs) {};
	virtual void Trans() override {
	    auto &inp = mInp.Data().begin()->second;
	    auto &cnt = mInpCount.Data().begin()->second;
	    if (cnt >= mMatchCnt) {
		mUpd = inp;
	    }
	    cout << Name() << " Update, [" << mUpd.pos << ", " << mUpd.symbol << "]" << endl;
	}
    public:
	int mMatchCnt;
	StateInput<TSourceElem> mInp;
	StateInput<int> mInpCount;
};

/**
 * Mutli with exact matching n times
 */
class MultiExact: public System
{
    public:
	MultiExact(const string& aName, MOwner* aOwner, int aMatchCnt);
	virtual ~MultiExact();
    public:
	ExtStateInp<TSourceElem> mInp;
	ExtStateOut<TSourceElem> mOut;
    protected:
	int mMatchCnt;
	Count* mCount;
	MultiExactSwitch* mSwitch;
}; 

MultiExact::MultiExact(const string& aName, MOwner* aOwner, int aMatchCnt): System(aName, aOwner), mMatchCnt(aMatchCnt), 
    mInp("Inp"), mOut("Out")
{
    AddComp(mCount = new Count("Count", mMatchCnt));
    AddComp(mSwitch = new MultiExactSwitch("Switch", mMatchCnt));
    mSwitch->EnableDanglingOutput();
    bool res = Connect(mSwitch->mInp, mInp.Orig());
    res = res && Connect(mInp.Orig(), mCount->mInp);
    res = res && Connect(mSwitch->Output(), mOut.Orig());
    res = res && Connect(mCount->Output(), mCount->mSelf);
    res = res && Connect(mCount->Output(), mSwitch->mInpCount);
    assert(res);
}

MultiExact::~MultiExact()
{
}


/**
 * Concat 'e' then 'n' exact n times: en{n}
 */
class Concat2: public System
{
    public:
	Concat2(const string& aName, MOwner* aOwner);
	virtual ~Concat2();
	virtual void Update() override;
    protected:
	Source* mSource;
	AtomOneChar* mAtom1;
	AtomOneChar* mAtom2;
	MultiExact* mMultiExact;
};

Concat2::Concat2(const string& aName, MOwner* aOwner): System(aName, aOwner)
{
    mSource = new Source("Source", "Test contennnt one");
    mAtom1 = new AtomOneChar("Atom1", 'e');
    mAtom2 = new AtomOneChar("Atom2", 'n');
    AddComp(mMultiExact = new MultiExact("MultiExact", NULL, 3));
    AddComp(mSource);
    AddComp(mAtom1);
    AddComp(mAtom2);
    bool res = Connect(mAtom1->mInp, mSource->Output());
    res = res && Connect(mAtom2->mInp, mSource->Output());
    res = res && Connect(mAtom2->mInpPresursor, mAtom1->Output());
    res = res && Connect(mAtom2->Output(), mAtom2->mInpPresursor);
    res = res && Connect(mAtom2->Output(), mMultiExact->mInp);
    CPPUNIT_ASSERT_MESSAGE("Fail to connect inp to output", res);
}

Concat2::~Concat2()
{
}

void Concat2::Update()
{
    cout << endl << "Concat2::Update" << endl;
    System::Update();
}

/**
 * Testing concat#2 'e' then 'n' exact 3 times: en\{3\}
 */
void Ut_Regex::test_Concat2()
{
    cout << endl << " === Test of concat#2 'e' then 'n' exact 3 times: en{3}" << endl;
    Concat2 syst("Concat2", NULL);
    syst.Run();
}

