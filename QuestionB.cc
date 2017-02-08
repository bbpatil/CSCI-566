#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "inet/applications/httptools/server/HttpServerBase.h"

using namespace omnetpp;

class CDNServerBase : public inet::httptools::HttpServerBase {
    protected:
        virtual void handleMessage(cMessage *msg) override;
};

Define_Module(CDNServerBase);

void CDNServerBase::handleMessage(cMessage *msg) {
    // TODO: check our cache

    // if exists, respond
    // otherwise forward request

    inet::httptools::HttpServerBase::handleMessage(msg);
}
