#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "inet/common/ModuleAccess.h"
#include "inet/applications/httptools/server/HttpServer.h"
#include "inet/applications/httptools/common/HttpMessages_m.h"

using namespace omnetpp;

class CDNServerBase : public virtual inet::httptools::HttpServerBase {
    protected:
        inet::httptools::HttpReplyMessage* handleGetRequest(inet::httptools::HttpRequestMessage *request, std::string resource);
};

class VirtualHttpServer : public virtual inet::httptools::HttpServer {};

class CDNServer : public virtual VirtualHttpServer, public virtual CDNServerBase {
protected:
    virtual void handleMessage(cMessage *msg) override;
};

void CDNServer::handleMessage(cMessage *msg) {
    inet::httptools::HttpServer::handleMessage(msg);
}

inet::httptools::HttpReplyMessage* CDNServerBase::handleGetRequest(inet::httptools::HttpRequestMessage *request, std::string resource)
{
    EV_DEBUG << "Handling GET request CUSTOM " << request->getName() << " resource: " << resource << endl;
    return inet::httptools::HttpServerBase::generateErrorReply(request, 418);
}

Define_Module(CDNServer); // CRASH : ambiguous conversion from derived class 'CDNServer' to base class 'omnetpp::cModule'
