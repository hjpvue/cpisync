//
// Created by Sean Brandenburg on 2019-06-10.
//
#include <cppunit/extensions/HelperMacros.h>
#include "Syncs/CPISync.h"
#include "CPISyncTest.h"
#include "Syncs/InterCPISync.h"
#include "TestAuxiliary.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CPISyncTest);

CPISyncTest::CPISyncTest() = default;

CPISyncTest::~CPISyncTest() = default;

void CPISyncTest::setUp() {
	const int SEED = 902;
	srand(SEED);
}

void CPISyncTest::tearDown() {}

//CPISync Test Cases

void CPISyncTest::testCPIAddDelElem() {
	// number of elems to add
	const int ITEMS = 50;
	CPISync cpisync(mBar, eltSizeSq, err,0);

	multiset<DataObject *, cmp<DataObject*>> elts;

	// check that add works
	for(int ii = 0; ii < ITEMS; ii++) {
		DataObject* item = new DataObject(randZZ());
		elts.insert(item);
		CPPUNIT_ASSERT(cpisync.addElem(item));
	}

	// check that elements can be recovered correctly through iterators
	multiset<DataObject *, cmp<DataObject*>> resultingElts;
	for(auto iter = cpisync.beginElements(); iter != cpisync.endElements(); ++iter) {
		resultingElts.insert(*iter);
	}

	vector<DataObject *> diff;
	rangeDiff(resultingElts.begin(), resultingElts.end(), elts.begin(), elts.end(), back_inserter(diff));
	CPPUNIT_ASSERT(diff.empty());

	// check that delete works
	for(auto dop : elts) {
		CPPUNIT_ASSERT(cpisync.delElem(dop));
	}

	CPPUNIT_ASSERT(cpisync.printElem().empty());
}

void CPISyncTest::CPISyncSetReconcileTest() {
		GenSync GenSyncServer = GenSync::Builder().
				setProtocol(GenSync::SyncProtocol::CPISync).
				setComm(GenSync::SyncComm::socket).
				setBits(eltSizeSq).
				setMbar(mBar).
				setErr(err).
				build();

		GenSync GenSyncClient = GenSync::Builder().
				setProtocol(GenSync::SyncProtocol::CPISync).
				setComm(GenSync::SyncComm::socket).
				setBits(eltSizeSq).
				setMbar(mBar).
				setErr(err).
				build();

		//(oneWay = false, probSync = false)
		CPPUNIT_ASSERT(syncTest(GenSyncClient, GenSyncServer, false, false));
}


void CPISyncTest::CPISyncMultisetReconcileTest() {
	GenSync GenSyncServer = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::CPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(mBar).
			setErr(err).
			setHashes(true).
			build();

	GenSync GenSyncClient = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::CPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(mBar).
			setErr(err).
			setHashes(true).
			build();

	//(oneWay = false, probSync = false, syncParamTest = false, Multiset = true)
	CPPUNIT_ASSERT(syncTest(GenSyncClient, GenSyncServer,false, false,false,true));
}

void CPISyncTest::ProbCPISyncSetReconcileTest() {
	GenSync GenSyncServer = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::ProbCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(mBar).
			setErr(err).
			build();

	GenSync GenSyncClient = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::ProbCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(mBar).
			setErr(err).
			build();

	//(oneWay = false, probSync = false)
	CPPUNIT_ASSERT(syncTest(GenSyncClient, GenSyncServer, false, false));
}

void CPISyncTest::ProbCPISyncMultisetReconcileTest() {
	GenSync GenSyncServer = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::ProbCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(mBar).
			setErr(err).
			setHashes(true).
			build();

	GenSync GenSyncClient = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::ProbCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(mBar).
			setErr(err).
			setHashes(true).
			build();

	//(oneWay = false, probSync = false, syncParamTest = false, Multiset = true)
	CPPUNIT_ASSERT(syncTest(GenSyncClient, GenSyncServer, false, false,false,true));
}

//InterCPISync Test Cases

void CPISyncTest::testInterCPIAddDelElem() {
	// number of elems to add
	const int ITEMS = 50;
	InterCPISync interCpiSync(mBar, eltSizeSq, err, partitions);

	multiset<DataObject *, cmp<DataObject*>> elts;

	// check that add works
	for(int ii = 0; ii < ITEMS; ii++) {
		DataObject* item = new DataObject(randZZ());
		elts.insert(item);
		CPPUNIT_ASSERT(interCpiSync.addElem(item));
	}

	// check that elements can be recovered correctly through iterators
	multiset<DataObject *, cmp<DataObject*>> resultingElts;
	for(auto iter = interCpiSync.beginElements(); iter != interCpiSync.endElements(); ++iter) {
		resultingElts.insert(*iter);
	}

	vector<DataObject *> diff;
	rangeDiff(resultingElts.begin(), resultingElts.end(), elts.begin(), elts.end(), back_inserter(diff));
	CPPUNIT_ASSERT(diff.empty());

	// check that delElem works
	for(auto dop : elts) {
		CPPUNIT_ASSERT(interCpiSync.delElem(dop));
	}

	CPPUNIT_ASSERT(interCpiSync.getNumElem() == 0);

}

void CPISyncTest::InterCPISyncSetReconcileTest() {
	//A small mBar so that InterCPISync is forced to recurse
	const int interCPImBar = 2;

	GenSync GenSyncServer = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::InteractiveCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(interCPImBar).
			setNumPartitions(numParts).
			build();

	GenSync GenSyncClient = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::InteractiveCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(interCPImBar).
			setNumPartitions(numParts).
			build();

	//(oneWay = false, probSync = false)
	CPPUNIT_ASSERT(syncTest(GenSyncClient, GenSyncServer, false, false));
}

//Error with very large negative number of bits being calculated in this equation (not sure why yet)
//bitNum = (long) 2 * bits + log(-1.0/log(1.0-pow(2.0,-epsilon-1.0)))/log(2) - 1;
void CPISyncTest::InterCPISyncMultisetReconcileTest() {
	//A small mBar so that InterCPISync is forced to recurse
	const int interCPImBar = 15;

	GenSync GenSyncServer = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::InteractiveCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(interCPImBar).
			setNumPartitions(numParts).
			setHashes(true).
			build();

	GenSync GenSyncClient = GenSync::Builder().
			setProtocol(GenSync::SyncProtocol::InteractiveCPISync).
			setComm(GenSync::SyncComm::socket).
			setBits(eltSizeSq).
			setMbar(interCPImBar).
			setNumPartitions(numParts).
			setHashes(true).
			build();

	//(oneWay = false, probSync = false, syncParamTest = false, Multiset = true)
	CPPUNIT_ASSERT(syncTest(GenSyncClient, GenSyncServer, false, false, false,true));
}