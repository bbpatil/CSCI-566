#include <unordered_map>
#include <omnetpp.h>
#include "inet/applications/httptools/server/HttpServer.h"
#include "inet/applications/httptools/browser/HttpBrowser.h"
#include "lru_h.hpp"

namespace omnetpp {

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

protected:
    enum State { CDN, ORIGIN };

    struct Remember {
        inet::TCPSocket *sock;
        std::string slug;
        State state;
        inet::httptools::HttpRequestMessage* req;
    };

    // TODO: improve cache by providing content type too
    struct CacheEntry {
        std::string payload;
        int contentType;
    };

    // DATA STRUCTURES
    std::map<int, Remember> sockMap;
    cache::lru_cache<std::string, std::string> lru;

    // OVERRIDES
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) override;
    virtual void handleMessage(cMessage *msg) override;

    // INTERNAL METHODS
    void requestContent(inet::httptools::HttpRequestMessage *req, int connID, State state);
    inet::httptools::HttpReplyMessage* genCacheResponse(inet::httptools::HttpRequestMessage *request, std::string resource);
};

CDNServer::CDNServer() : lru(30) {}; // TODO: parse 2 from model parameters

void CDNServer::handleMessage(cMessage *msg) {
    inet::httptools::HttpReplyMessage *msg2 = dynamic_cast<inet::httptools::HttpReplyMessage *>(msg);
    if (msg2 == nullptr) {
        inet::httptools::HttpServer::handleMessage(msg);
    } else {
        auto i = sockMap.find(msg2->serial());
        if (i == sockMap.end()) {
            EV_INFO << "TODO: figure out how we are doubling up here!!" << endl;
            delete msg;
            return;
        }

        Remember memory = i->second;

        // Content not found on neighbor CDN, request from origin
        if (memory.state == CDN && msg2->result() != 200) {
            EV_INFO << "CDNServer: Neighbor CDN didn't have content, requesting from Origin: " << memory.req->heading() << endl;
            memory.state = ORIGIN;
            requestContent(memory.req, msg2->serial(), ORIGIN);
            delete msg;
            return;
        }

        inet::httptools::HttpReplyMessage *res = new inet::httptools::HttpReplyMessage(*msg2);
        res->setOriginatorUrl(hostName.c_str());
        res->setSerial(msg2->serial());
        EV_INFO << "CDNServer: Returning Content 1 '" << msg2->targetUrl() << "' '" << msg2->originatorUrl() << "'" << msg2->heading() << endl;
        EV_INFO << "CDNServer: Returning Content 2 '" << res->targetUrl() << "' '" << res->originatorUrl() << "'" << endl;
        memory.sock->send(res);
        char payload[127];
        strcpy(payload, msg2->payload());
        lru.put(memory.slug, payload);
        EV_INFO << "CDNServer: Caching Resource" << memory.slug << endl;
        sockMap.erase(msg2->serial());
        EV_INFO << "CDNServer: Remaining active memory: " << sockMap.size() << endl;
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
    std::string origin = request->originatorUrl();

    if (lru.exists(resource)) {
        // Directly respond
        socket->send(genCacheResponse(request, resource));

    } else if (origin == "cdn1.example.org" || origin == "cdn2.example.org") {
        // If it's a request from the other CDN server and we don't have it
        socket->send(generateErrorReply(request, 400));
        badRequests++;
        delete msg; // Return here because we don't want statistics
        return;

    } else {
        // Create CDN Lookup Request
        int id = simTime().raw(); // random number that kind-of guarantees this won't duplicate
        sockMap[id].slug = resource;
        sockMap[id].sock = socket;
        sockMap[id].state = CDN;
        sockMap[id].req = new inet::httptools::HttpRequestMessage(*request);
        requestContent(request, id, CDN);
    }

    // Update service statistics
    switch (inet::httptools::getResourceCategory(inet::httptools::parseResourceName(resource))) {
        case inet::httptools::CT_HTML: htmlDocsServed++; break;
        case inet::httptools::CT_TEXT: textResourcesServed++; break;
        case inet::httptools::CT_IMAGE: imgResourcesServed++; break;
        default: EV_WARN << "CDNServer: Received Unknown request type: " << resource << endl; break;
    };
    delete msg;
};

void CDNServer::requestContent(inet::httptools::HttpRequestMessage *req, int id, State state) {
    inet::httptools::HttpRequestMessage *newRequest = new inet::httptools::HttpRequestMessage(*req);
    if (state == ORIGIN) {
        newRequest->setTargetUrl("origin.example.org");
    } else if (hostName == "cdn1.example.org") {
        newRequest->setTargetUrl("cdn2.example.org");
    } else {
        newRequest->setTargetUrl("cdn1.example.org");
    }
    newRequest->setOriginatorUrl(hostName.c_str());
    newRequest->setSerial(id);
    EV_INFO << "CDNServer: Requesting Content: Target:'" << newRequest->targetUrl() << "'; Origin:'" << newRequest->originatorUrl() << "'; Heading "<< req->heading() << endl;
    this->sendDirect(newRequest, this->getParentModule()->getSubmodule("tcpApp", 1), 0);
};

inet::httptools::HttpReplyMessage* CDNServer::genCacheResponse(inet::httptools::HttpRequestMessage *request, std::string resource) {
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
    return replymsg;
};


/*
 * StatsBrowser Implementation
 */

class StatsBrowser : public inet::httptools::HttpBrowser {
    protected:
        static simsignal_t rcvdPkSignal;

//        virtual void handleDataMessage(cMessage *msg) override;
};

simsignal_t StatsBrowser::rcvdPkSignal = registerSignal("rcvdPk");


//void StatsBrowser::handleDataMessage(cMessage *msg) {
//    EV << "NATE!!!" << endl;
//    inet::httptools::HttpBrowser::handleDataMessage(msg);
//};

// emit(rcvdPkSignal, msg);

Define_Module(CDNServer);
Define_Module(CDNBrowser);
Define_Module(StatsBrowser);

};
