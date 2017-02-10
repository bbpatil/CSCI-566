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
 * - HttpBrowserBase needs `virtual handleDataMessage`
 *
 */

using namespace omnetpp;

class CDNBrowser : public inet::httptools::HttpBrowser {
public:
    virtual void handleMessage(cMessage *msg) override;
    void handleDataMessage(cMessage *msg);
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
};

void CDNBrowser::handleMessage(cMessage *msg) {
    inet::httptools::HttpRequestMessage *msg2 = dynamic_cast<inet::httptools::HttpRequestMessage *>(msg);
    if (msg2 == nullptr) {
        inet::httptools::HttpBrowser::handleMessage(msg);
    } else {
        this->sendRequestToServer(msg2);
    }
}

// Direct copy from HttpBrowser.h
void CDNBrowser::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) {
    EV << "NATE WE GOT A FRIGGEN RESPONSE!!!" << endl;

    EV_DEBUG << "Socket data arrived on connection " << connId << ": " << msg->getName() << endl;
    if (yourPtr == nullptr) {
        EV_ERROR << "socketDataArrivedfailure. Null pointer" << endl;
        return;
    }

    SockData *sockdata = (SockData *)yourPtr;
    inet::TCPSocket *socket = sockdata->socket;
    handleDataMessage(msg);

    if (--sockdata->pending == 0) {
        EV_DEBUG << "Received last expected reply on this socket. Issuing a close" << endl;
        socket->close();
    }
    // Message deleted in handler - do not delete here!
}

void CDNBrowser::handleDataMessage(cMessage *msg) {
     this->sendDirect(msg, this->getParentModule()->getSubmodule("tcpApp", 0), 0);
}


class CDNServerBase : public virtual inet::httptools::HttpServerBase {
    protected:
        inet::httptools::HttpReplyMessage* handleGetRequest(inet::httptools::HttpRequestMessage *request, std::string resource);
        cPacket *handleReceivedMessage(cMessage *msg);
};

class CDNServer : public virtual inet::httptools::HttpServer, public virtual CDNServerBase {
protected:
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
    virtual void handleMessage(cMessage *msg) override;
};

void CDNServer::handleMessage(cMessage *msg) {
    inet::httptools::HttpReplyMessage *msg2 = dynamic_cast<inet::httptools::HttpReplyMessage *>(msg);
    if (msg2 == nullptr) {
        inet::httptools::HttpServer::handleMessage(msg);
    } else {
        EV << "Figure out how to cache this fucker" << endl;
        delete msg;
    }
}

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

    inet::httptools::HttpRequestMessage *newRequest = new inet::httptools::HttpRequestMessage(*request);
    EV << "LISA 3.0!!!! " << res[1] << endl;
    char target[127];
    strcpy(target, ("origin.example.org" + res[1]).c_str());
    newRequest->setTargetUrl(target);
    EV << "LISA 4.0!!!! " << res[1] << " awww "<< this->getParentModule()->getSubmodule("tcpApp", 1)->getFullPath() << " me " << this->getFullName() << endl;
    this->sendDirect(newRequest, this->getParentModule()->getSubmodule("tcpApp", 1), 0);
//    this->fetcher->sendThisBitch(newRequest);
    EV << "LISA 5.0!!!! " << res[1] << endl;
    return generateErrorReply(request, 404);
}

inet::httptools::HttpReplyMessage* CDNServerBase::handleGetRequest(inet::httptools::HttpRequestMessage *request, std::string resource)
{
    EV_DEBUG << "Handling GET request CUSTOM " << request->getName() << " resource: " << resource << endl;
    return inet::httptools::HttpServerBase::generateErrorReply(request, 418);
}

Define_Module(CDNServer); // CRASH : ambiguous conversion from derived class 'CDNServer' to base class 'omnetpp::cModule'
Define_Module(CDNBrowser);
