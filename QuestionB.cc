#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "inet/common/ModuleAccess.h"
#include "inet/applications/httptools/server/HttpServer.h"
#include "inet/applications/httptools/browser/HttpBrowser.h"
#include "inet/applications/httptools/common/HttpMessages_m.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"

/*
 *
 * DONT FORGET TO MAKE ALL THE INET THINGS VIRTUAL!!!
 *
 * - HttpServer needs `virtual HttpServerBase`
 *
 */

using namespace omnetpp;

//class CDNBrowser : public inet::httptools::HttpBrowser {
//public:
//    void sendThisBitch(inet::httptools::HttpRequestMessage msg);
//};
//
//void CDNBrowser::sendThisBitch(inet::httptools::HttpRequestMessage msg) {
//    inet::httptools::HttpBrowser::sendRequestToServer(msg);
//}


class CDNServerBase : public virtual inet::httptools::HttpServerBase {
//    public:
//        CDNServerBase();

    protected:
        inet::httptools::HttpReplyMessage* handleGetRequest(inet::httptools::HttpRequestMessage *request, std::string resource);
        cPacket *handleReceivedMessage(cMessage *msg);


};

//CDNServerBase::CDNServerBase() {
//
//}

class CDNServer : public virtual inet::httptools::HttpServer, public virtual CDNServerBase {
protected:
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
};

// Direct copy from HttpServer
void CDNServer::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) {
    EV << "NATE 2.0!!!" << endl;
    if (yourPtr == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }
    inet::TCPSocket *socket = (inet::TCPSocket *)yourPtr;

    // Should be a HttpReplyMessage
    EV_DEBUG << "Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;

    // call the message handler to process the message.
    cMessage *reply = handleReceivedMessage(msg);
    if (reply != nullptr) {
        socket->send(reply);    // Send to socket if the reply is non-zero.
    }
    delete msg;    // Delete the received message here. Must not be deleted in the handler!
}

cPacket *CDNServerBase::handleReceivedMessage(cMessage *msg) {
    EV << "LISA!!!!" << endl;
//    return inet::httptools::HttpServerBase::handleReceivedMessage(msg);
    inet::httptools::HttpRequestMessage *request = check_and_cast<inet::httptools::HttpRequestMessage *>(msg);
    // ASSUME EVERYTHING FUCKING WORKS!!!

    cStringTokenizer tokenizer = cStringTokenizer(request->heading(), " ");
    std::vector<std::string> res = tokenizer.asVector();

    EV << "LISA 2.0!!!! " << res[1] << endl;
    htmlDocsServed++;

    return generateErrorReply(request, 404);
}

inet::httptools::HttpReplyMessage* CDNServerBase::handleGetRequest(inet::httptools::HttpRequestMessage *request, std::string resource)
{
    EV_DEBUG << "Handling GET request CUSTOM " << request->getName() << " resource: " << resource << endl;
    return inet::httptools::HttpServerBase::generateErrorReply(request, 418);
}

Define_Module(CDNServer); // CRASH : ambiguous conversion from derived class 'CDNServer' to base class 'omnetpp::cModule'
