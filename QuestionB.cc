#include <unordered_map>
#include <omnetpp.h>
#include "inet/applications/httptools/server/HttpServer.h"
#include "inet/applications/httptools/browser/HttpBrowser.h"
#include "lru_h.hpp"

using namespace omnetpp;

/*
 * Modified for debugging!
 * - HttpServerBase.cc::updateDisplay()
 *   Modified sprintf to be as follows:
 *   sprintf(buf, "htm: %ld, img: %ld, txt: %ld", htmlDocsServed, imgResourcesServed, textResourcesServed);
 */

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
    struct Remember {
        inet::TCPSocket *sock;
        std::string slug;
    };

    std::map<int, Remember> sockMap;
    cache::lru_cache<std::string, std::string> lru;

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
    virtual void handleMessage(cMessage *msg) override;
};

CDNServer::CDNServer() : lru(2) {}; // TODO: parse 2 from model parameters
CDNServer::~CDNServer() {
    // This is crashing when closing the Simulation
//    for (auto & elem : sockMap) delete elem.second;
//    sockMap.clear();
};

void CDNServer::handleMessage(cMessage *msg) {
    inet::httptools::HttpReplyMessage *msg2 = dynamic_cast<inet::httptools::HttpReplyMessage *>(msg);
    if (msg2 == nullptr) {
        inet::httptools::HttpServer::handleMessage(msg);
    } else {

        auto i = sockMap.find(msg2->serial());
        if (i == sockMap.end()) {
            EV_ERROR << "Figure out how we are hitting this!!!" << endl;
            delete msg;
            return;
        }
        Remember memory = i->second;
        if (memory.sock->getState() != inet::TCPSocket::CONNECTED) {
            EV_WARN << "Figure out why we hit this!!!" << endl;
            sockMap.erase(msg2->serial());
            delete msg;
            return;
        }

        EV_INFO << "FOUND SOCKET!!!" << msg2->serial() << " " << memory.sock->getState() << endl;
        inet::httptools::HttpReplyMessage *res = new inet::httptools::HttpReplyMessage(*msg2);
         res->setOriginatorUrl(hostName.c_str());
        EV_INFO << "CDNServer: Returning Content 1 '" << msg2->targetUrl() << "' '" << msg2->originatorUrl() << "'" << msg2->heading() << endl;
        EV_INFO << "CDNServer: Returning Content 2 '" << res->targetUrl() << "' '" << res->originatorUrl() << "'" << endl;
        memory.sock->send(res);
        lru.put(memory.slug, msg2->payload());
        EV_INFO << "CDNServer: Caching Resource" << memory.slug << endl;
        delete msg;
    }
};

void CDNServer::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) {
    if (yourPtr == nullptr) { EV_ERROR << "Socket establish failure. Null pointer" << endl; return; }
    EV_DEBUG << "CDNServer: Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;
    inet::TCPSocket *socket = (inet::TCPSocket *)yourPtr;

    // Process message
    inet::httptools::HttpRequestMessage *request = check_and_cast<inet::httptools::HttpRequestMessage *>(msg);
    std::string resource = cStringTokenizer(request->heading(), " ").asVector()[1];

    // Search Cache
    if (!lru.exists(resource)) {
        // Create Origin Lookup Request
        inet::httptools::HttpRequestMessage *newRequest = new inet::httptools::HttpRequestMessage(*request);
        newRequest->setTargetUrl("origin.example.org");
        newRequest->setSerial(socket->getConnectionId());
        EV_INFO << "CDNServer: Requesting Content 1'" << request->targetUrl() << "' '" << request->originatorUrl() << "'"<< request->heading() << endl;
        EV_INFO << "CDNServer: Requesting Content 2'" << newRequest->targetUrl() << "' '" << newRequest->originatorUrl() << "'" << request->heading() << endl;
        this->sendDirect(newRequest, this->getParentModule()->getSubmodule("tcpApp", 1), 0);
        sockMap[connId].sock = socket;
        sockMap[connId].slug = resource;
    } else {
        // Directly respond
        EV_INFO << "CDNServer: Found Resource " << resource << endl;
        char szReply[512];
        sprintf(szReply, "HTTP/1.1 200 OK (%s)", resource.c_str());
        inet::httptools::HttpReplyMessage *replymsg = new inet::httptools::HttpReplyMessage(szReply);
        replymsg->setHeading("HTTP/1.1 200 OK");
        replymsg->setOriginatorUrl(hostName.c_str());
        replymsg->setTargetUrl(request->originatorUrl());
        replymsg->setProtocol(request->protocol());
        replymsg->setSerial(request->serial());
        replymsg->setResult(200);
        replymsg->setContentType(inet::httptools::CT_HTML);    // Emulates the content-type header field
        replymsg->setKind(HTTPT_RESPONSE_MESSAGE);
        std::string body = lru.get(resource);
        replymsg->setPayload(body.c_str());
        replymsg->setByteLength(body.length());
        socket->send(replymsg);

    }

    // Update service stats
    switch (inet::httptools::getResourceCategory(inet::httptools::parseResourceName(resource))) {
        case inet::httptools::CT_HTML: htmlDocsServed++; break;
        case inet::httptools::CT_TEXT: textResourcesServed++; break;
        case inet::httptools::CT_IMAGE: imgResourcesServed++; break;
        default: EV_WARN << "CDNServer: Received Unknown request type: " << resource << endl; break;
    };
    delete msg; // Delete the received message here. Must not be deleted in the handler!
};


/*
 * StatsBrowser Implementation
 */

class StatsBrowser : public inet::httptools::HttpBrowser {};
//// HttpBrowserStats extends a basic HttpBrowser with received packet stats
//class HttpBrowserStats : public inet::httptools::HttpBrowser {
//protected:
//    static simsignal_t rcvdPkSignal;
//};
//
//simsignal_t HttpBrowserStats::rcvdPkSignal = registerSignal("rcvdPk");
// emit(rcvdPkSignal, msg);

Define_Module(CDNServer);
Define_Module(CDNBrowser);
Define_Module(StatsBrowser);
