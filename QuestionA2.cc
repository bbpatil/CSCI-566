#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

#include "QuestionA2_m.h"

class TxcA2 : public cSimpleModule {
  protected:
    virtual QuestionA2Msg *generateMessage();
    virtual void forwardMessage(QuestionA2Msg *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(TxcA2);

void TxcA2::initialize() {
    QuestionA2Msg *msg = generateMessage();

    if (getIndex() == 0) {
        scheduleAt(0.0, msg);
    } else {
        scheduleAt(par("delayTime"), msg);
    }
}

void TxcA2::handleMessage(cMessage *msg) {
    // If message is ours, schedule another one to come down the pipe later
    if (msg->isSelfMessage()) {
        EV << "Enquing another message: ";
        QuestionA2Msg *newmsg = generateMessage();
        EV << newmsg << endl;
        scheduleAt(simTime() + par("delayTime"), newmsg);
    }

    QuestionA2Msg *ttmsg = check_and_cast<QuestionA2Msg *>(msg);
    if (ttmsg->getDestination() == getIndex()) {
        // Message arrived.
        EV << "Message " << ttmsg << " arrived after " << ttmsg->getHopCount() << " hops.\n";
        bubble("ARRIVED, starting new one!");
        delete ttmsg;
    } else {
        // We need to forward the message.
        forwardMessage(ttmsg);
    }
}

QuestionA2Msg *TxcA2::generateMessage() {
    // Produce source and destination addresses.
    int src = getIndex();  // our module index
    int n = getVectorSize();  // module vector size
    int dest = intuniform(0, n-2);
    if (dest >= src)
        dest++;

    char msgname[20];
    sprintf(msgname, "tic-%d-to-%d", src, dest);

    // Create message object and set source and destination field.
    QuestionA2Msg *msg = new QuestionA2Msg(msgname);
    msg->setSource(src);
    msg->setDestination(dest);
    return msg;
}

void TxcA2::forwardMessage(QuestionA2Msg *msg) {
    msg->setHopCount(msg->getHopCount()+1);

    // Same routing as before: random gate.
    int n = gateSize("gate");
    int k = intuniform(0, n-1);

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    send(msg, "gate$o", k);
}
