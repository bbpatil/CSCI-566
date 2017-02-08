#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TxcA1 : public cSimpleModule {
  protected:
    virtual void forwardMessage(cMessage *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(TxcA1);

void TxcA1::initialize() {
    if (getIndex() == 0) {
        char msgname[20];
        sprintf(msgname, "tic-%d", getIndex());
        cMessage *msg = new cMessage(msgname);
        scheduleAt(0.0, msg);
    }
}

void TxcA1::handleMessage(cMessage *msg) {
    if (getIndex() == 3) {
        EV << "Message " << msg << " arrived.\n";
        delete msg;
    } else {
        forwardMessage(msg);
    }
}

void TxcA1::forwardMessage(cMessage *msg) {
    int n = gateSize("port");

    // START: Exercise 10 (Question 3) If randomly generated number is same as input gate, generate again
    cGate *arrival = msg->getArrivalGate();
    int k;
    do {
        k = intuniform(0, n-1);
    } while (arrival != nullptr && n > 1 && k == arrival->getIndex());
    // STOP: Exercise 10 (Question 3)

    EV << "Forwarding message " << msg << " on port out[" << k << "]\n";
    send(msg, "port$o", k);
}
