//
// Created by Bowen Song on 9/23/18.
//

#include <Exceptions.h>
#include "kshinglingSync.h"

kshinglingSync::kshinglingSync(GenSync::SyncProtocol set_sync_protocol, const size_t shingle_size,
        const char stop_word) : myKshingle(shingle_size, stop_word), setSyncProtocol(set_sync_protocol), shingleSize(shingle_size) {
    oneway = true;
    auto setProto_avil = {GenSync::SyncProtocol::IBLTSyncSetDiff, GenSync::SyncProtocol::CPISync,
                          GenSync::SyncProtocol::InteractiveCPISync};
    if (find(setProto_avil.begin(), setProto_avil.end(), set_sync_protocol) == setProto_avil.end())
        throw invalid_argument("Base Set Reconciliation Protocol not supported.");
}


//Alice
bool kshinglingSync::SyncClient(const shared_ptr<Communicant> &commSync, shared_ptr<SyncMethod> & setHost,
        DataObject &selfString, DataObject &otherString) {
    Logger::gLog(Logger::METHOD, "Entering kshinglingSync::SyncClient");
    bool syncSuccess = true;

    // call parent method for bookkeeping
    SyncMethod::SyncClient(commSync, setHost, selfString, otherString);
    // create kshingle

    // connect to server
    commSync->commConnect();
    // ensure that the kshingle size and stopword equal those of the server
    if (!commSync->establishKshingleSend(myKshingle.getElemSize(), myKshingle.getStopWord(), oneway)) {
        Logger::gLog(Logger::METHOD_DETAILS,
                     "Kshingle parameters do not match up between client and server!");
        syncSuccess = false;
    }

    // send cycNum
    if (!oneway){
        cycleNum = myKshingle.reconstructStringBacktracking().second;
        commSync->commSend(cycleNum);
    }
    cycleNum = commSync->commRecv_long();


    // estimate difference
    if (needEst()) {
        StrataEst est = StrataEst(myKshingle.getElemSize());

        for (auto item : myKshingle.getShingleSet_str()) {
            est.insert(new DataObject(item)); // Add to estimator
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        commSync->commSend(est.getStrata(), false);

        mbar = commSync->commRecv_long(); // cast long to long long

    }

    // reconcile difference + delete extra
    configurate(setHost, myKshingle.getSetSize());
    for (auto item : myKshingle.getShingleSet_str()) {
        setHost->addElem(new DataObject(item)); // Add to GenSync
    }
    // choose to send if not oneway (default is one way)
//

    return syncSuccess;
}

//Bob
bool kshinglingSync::SyncServer(const shared_ptr<Communicant> &commSync,  shared_ptr<SyncMethod> & setHost,
        DataObject &selfString, DataObject &otherString) {
    Logger::gLog(Logger::METHOD, "Entering kshinglingSync::SyncServer");
    bool syncSuccess = true;

    SyncMethod::SyncServer(commSync, setHost, selfString, otherString);

    commSync->commListen();
    if (!commSync->establishKshingleRecv(myKshingle.getElemSize(), myKshingle.getStopWord(), oneway)) {
        Logger::gLog(Logger::METHOD_DETAILS,
                     "Kshingle parameters do not match up between client and server!");
        syncSuccess = false;
    }

    // send cycNum
    cycleNum = myKshingle.reconstructStringBacktracking().second; // perform backtrack no matter what
    auto tmpcycleNum = cycleNum;
    if (!oneway) cycleNum = commSync->commRecv_long();
     commSync->commSend(tmpcycleNum);

    // estimate difference
    if (needEst()) {
        StrataEst est = StrataEst(myKshingle.getElemSize());

        for (auto item : myKshingle.getShingleSet_str()) {
            est.insert(new DataObject(item)); // Add to estimator
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        auto theirStarata = commSync->commRecv_Strata();
        mbar = (est -= theirStarata).estimate();
//        mbar = mbar + mbar / 2; // get an upper bound
        commSync->commSend(mbar); // Dangerous cast

    }

    // reconcile difference + delete extra
    configurate(setHost, myKshingle.getSetSize());
    for (auto item : myKshingle.getShingleSet_str()) {
        auto* tmp = new DataObject(item);
        setHost->addElem(tmp); // Add to GenSync
        delete tmp;
    }
    return syncSuccess;
}

void kshinglingSync::configurate(shared_ptr<SyncMethod>& setHost, idx_t set_size) {

    int err = 8;// negative log of acceptable error probability for probabilistic syncs

    if (setSyncProtocol == GenSync::SyncProtocol::CPISync) {
        eltSize = 14 + (myKshingle.getshinglelen_str() + 2) * 6;
        setHost = make_shared<ProbCPISync>(mbar, eltSize, err, true);
    } else if (setSyncProtocol == GenSync::SyncProtocol::InteractiveCPISync) {
        eltSize = 14 + (myKshingle.getshinglelen_str() + 2 ) * 6;
        int par = (ceil(log(set_size)) > 1) ?: 2;
        setHost = make_shared<InterCPISync>(5, eltSize, err, 3, true);
        //(ceil(log(set_size))>1)?:2;
    } else if (setSyncProtocol == GenSync::SyncProtocol::IBLTSyncSetDiff) {
        (mbar == 0) ? mbar = 10 : mbar;
        eltSize = myKshingle.getElemSize();
        setHost = make_shared<IBLTSync_SetDiff>(mbar, eltSize, true);
    }
}

bool kshinglingSync::reconstructString(DataObject* & recovered_string, const list<DataObject *> & Elems) {
    if (cycleNum != 0)
        myKshingle.clear_shingleSet();

        for (auto elem: Elems) {
            //change here - send pair
            myKshingle.updateShingleSet_str(ZZtoStr(elem->to_ZZ()));
        }
        recovered_string = new DataObject(myKshingle.reconstructStringBacktracking(cycleNum).first);
    return cycleNum != 0;
}

vector<DataObject*> kshinglingSync::addStr(DataObject* datum){
    // call parent add
    SyncMethod::addStr(datum);
    myKshingle.clear_shingleSet();

    myKshingle.inject(datum->to_string());
    vector<DataObject*> res;
    for (auto item : myKshingle.getShingleSet_str()){
        auto tmp = new DataObject(StrtoZZ(item));
        res.push_back(tmp);
    }
    return res;
}

long kshinglingSync::getVirMem(){
    return myKshingle.virtualMemUsed();
}


string kshinglingSync::getName(){ return "This is a kshinglingSync of string reconciliation";}