#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "inet/common/ModuleAccess.h"
#include "inet/applications/httptools/server/HttpServer.h"
#include "inet/applications/httptools/browser/HttpBrowser.h"
#include "inet/applications/httptools/common/HttpMessages_m.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/httptools/browser/HttpBrowserBase.h"

using namespace omnetpp;

class CDNServerBase : public inet::httptools::HttpServer { // , protected inet::httptools::HttpBrowser
//    public:

//        CDNServerBase();

    protected:

        typedef std::deque<inet::httptools::HttpRequestMessage *> HttpRequestQueue;

        virtual void initialize(int stage) override;
        struct SockData
            {
                HttpRequestQueue messageQueue;    // Queue of pending messages.
                inet::TCPSocket *socket = nullptr;    // A reference to the socket object.
                int pending = 0;    // A counter for the number of outstanding replies.
            };

        virtual void handleMessage(cMessage *msg) override;
//        inet::httptools::HttpBrowser* browser;


        inet::httptools::HttpController *controller = nullptr;    // Reference to the central controller object
        void sendRequestToServer(inet::httptools::HttpRequestMessage *request);
        void submitToSocket(const char *moduleName, int connectPort, inet::httptools::HttpRequestMessage *msg);
        void submitToSocket(const char *moduleName, int connectPort, HttpRequestQueue& queue);
};

Define_Module(CDNServerBase);

//CDNServerBase::CDNServerBase() {
//    browser = new inet::httptools::HttpBrowser;
//}

void CDNServerBase::initialize(int stage)
{
    EV_DEBUG << "Initializing base CDN browser component -- stage " << stage << endl;
    inet::httptools::HttpServer::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        controller = inet::getModuleFromPar<inet::httptools::HttpController>(par("httpBrowserControllerModule"), this);
    }
}

void CDNServerBase::handleMessage(cMessage *msg) {
    inet::httptools::HttpRequestMessage *request = check_and_cast<inet::httptools::HttpRequestMessage *>(msg);

    inet::httptools::HttpRequestMessage *outbound = new inet::httptools::HttpRequestMessage();
    outbound->setTargetUrl("http://origin.example.com/shit_to_parse.later");
    outbound->setOriginatorUrl(request->targetUrl());

//    inet::httptools::HttpRequestMessage *outbound = new inet::httptools::HttpRequestMessage(request);
//    browser->sendRequestToServer(outbound);

    // TODO: check our cache

    // if exists, respond
    // otherwise forward request


    EV_DEBUG << "NATE!!! Fuck this" << endl;

    inet::httptools::HttpServer::handleMessage(msg);
}

void CDNServerBase::sendRequestToServer(inet::httptools::HttpRequestMessage *request)
{
    int connectPort;
    char szModuleName[127];

    if (controller->getServerInfo(request->targetUrl(), szModuleName, connectPort) != 0) {
        EV_ERROR << "Unable to get server info for URL " << request->targetUrl() << endl;
        delete request;
        return;
    }

    EV_DEBUG << "Sending request to server " << request->targetUrl() << " (" << szModuleName << ") on port " << connectPort << endl;
    submitToSocket(szModuleName, connectPort, request);
}


void CDNServerBase::submitToSocket(const char *moduleName, int connectPort, inet::httptools::HttpRequestMessage *msg)
{
    // Create a queue and push the single message
    HttpRequestQueue queue;
    queue.push_back(msg);
    // Call the overloaded version with the queue as parameter
    submitToSocket(moduleName, connectPort, queue);
}

void CDNServerBase::submitToSocket(const char *moduleName, int connectPort, HttpRequestQueue& queue)
{
    // Don't do anything if the queue is empty.
    if (queue.empty()) {
        EV_INFO << "Submitting to socket. No data to send to " << moduleName << ". Skipping connection." << endl;
        return;
    }

    EV_DEBUG << "Submitting to socket. Module: " << moduleName << ", port: " << connectPort << ". Total messages: " << queue.size() << endl;

    // Create and initialize the socket
    inet::TCPSocket *socket = new inet::TCPSocket();
    socket->setDataTransferMode(inet::TCP_TRANSFER_OBJECT);
    socket->setOutputGate(gate("tcpOut"));
    sockCollection.addSocket(socket);

    // Initialize the associated data structure
    SockData *sockdata = new SockData;
    sockdata->messageQueue = HttpRequestQueue(queue);
    sockdata->socket = socket;
    sockdata->pending = 0;
    socket->setCallbackObject(this, sockdata);

    // Issue a connect to the socket for the specified module and port.
    socket->connect(inet::L3AddressResolver().resolve(moduleName), connectPort);
}
