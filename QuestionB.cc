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


/*
 * CDNBrowser Implementation
 */

class CDNBrowser : public inet::httptools::HttpBrowser {
public:
    virtual void handleMessage(cMessage *msg) override;
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
    if (yourPtr == nullptr) { EV_ERROR << "socketDataArrivedfailure. Null pointer" << endl; return; }
    EV_DEBUG << "CDNBrowser: Socket data arrived on connection " << connId << ": " << msg->getName() << endl;

    SockData *sockdata = (SockData *)yourPtr;
    inet::TCPSocket *socket = sockdata->socket;

    // Send client data to consumer
    this->sendDirect(msg, this->getParentModule()->getSubmodule("tcpApp", 0), 0);

    if (--sockdata->pending == 0) {
        EV_DEBUG << "Received last expected reply on this socket. Issuing a close" << endl;
        socket->close();
    }
    // Message deleted in handler - do not delete here!
}


/*
 * CDNServer Implementation
 */

class CDNServer : public inet::httptools::HttpServer {
public:
    CDNServer();
    ~CDNServer();

protected:
    inet::TCPSocketMap sockCollection;

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
    virtual void handleMessage(cMessage *msg) override;
};

CDNServer::CDNServer() {};
CDNServer::~CDNServer() { sockCollection.deleteSockets(); };

void CDNServer::handleMessage(cMessage *msg) {
    inet::httptools::HttpReplyMessage *msg2 = dynamic_cast<inet::httptools::HttpReplyMessage *>(msg);
    if (msg2 == nullptr) {
        inet::httptools::HttpServer::handleMessage(msg);
    } else {
        EV << "NATE!!! Figure out how to cache this" << endl;
        inet::TCPSocket *socket = sockCollection.findSocketFor(msg2);
        if (socket == nullptr) {
            EV_ERROR << "WHOA!!!" << endl;
            return;
        }
        socket->send(msg2);
        delete msg;
    }
};

void CDNServer::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) {
    if (yourPtr == nullptr) { EV_ERROR << "Socket establish failure. Null pointer" << endl; return; }
    EV_DEBUG << "CDNServer: Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;
    inet::TCPSocket *socket = (inet::TCPSocket *)yourPtr;

    // Process message
    inet::httptools::HttpRequestMessage *request = check_and_cast<inet::httptools::HttpRequestMessage *>(msg);
    cStringTokenizer tokenizer = cStringTokenizer(request->heading(), " ");
    std::vector<std::string> res = tokenizer.asVector();

    // Create Origin Lookup Request
    inet::httptools::HttpRequestMessage *newRequest = new inet::httptools::HttpRequestMessage(*request);
    char target[127];
    strcpy(target, ("origin.example.org" + res[1]).c_str());
    newRequest->setTargetUrl(target);
    this->sendDirect(newRequest, this->getParentModule()->getSubmodule("tcpApp", 1), 0);
    htmlDocsServed++;

    sockCollection.addSocket(socket);

//    // Fire Response
//    cMessage *reply = generateErrorReply(request, 418);
//    if (reply != nullptr) {
//        socket->send(reply);
//    }
    delete msg; // Delete the received message here. Must not be deleted in the handler!
};

//// HttpBrowserStats extends a basic HttpBrowser with received packet stats
//class HttpBrowserStats : public inet::httptools::HttpBrowser {
//protected:
//    static simsignal_t rcvdPkSignal;
//};
//
//simsignal_t HttpBrowserStats::rcvdPkSignal = registerSignal("rcvdPk");
// emit(rcvdPkSignal, msg);

Define_Module(CDNServer); // CRASH : ambiguous conversion from derived class 'CDNServer' to base class 'omnetpp::cModule'
Define_Module(CDNBrowser);
