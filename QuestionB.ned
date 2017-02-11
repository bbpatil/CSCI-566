package csci_566_proj_1;
//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003 Ahmet Sekercioglu
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

import inet.applications.httptools.configurator.HttpController;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.applications.contract.ITCPApp;

channel ethernetline extends ned.DatarateChannel {
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

simple CDNServer like ITCPApp {
    parameters:
        string hostName = default("");                  // The domain name of the server
        int port = default(80);                         // The listening port number
        int httpProtocol = default(11);                 // The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        string logFile = default("");                   // Name of server log file. Events are appended, allowing sharing of file for multiple servers.
        string siteDefinition = default("");            // The site script file. Blank to disable.
        double activationTime @unit("s") = default(0s); // The initial activation delay. Zero to disable.
        xml config;                                     // The XML configuration file for random sites
    gates:
        input responses @directIn; // CDNBrowser calls back into this
        input tcpIn;
        output tcpOut;
}

simple CDNBrowser like ITCPApp {
    parameters:
        int httpProtocol = default(11);                 // The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        string logFile = default("");                   // Name of a browser log file. Browse events are appended, allowing sharing of file for multiple browsers.
        string scriptFile = default("");                // The browsing script file. Blank to disable.
        double activationTime @unit("s") = default(0s); // The initial activation delay. Zero to disable. This is the time at which the browser first comes to life in a simulation scenario. Note that this can be any random distribution which OMNeT++ supports.
        xml config;                                     // The XML config file
        string httpBrowserControllerModule = default("controller"); // the absolute path to the http browser controller, @see HttpController
    gates:
        input requests @directIn; // CDNServer makes requests into here
        input tcpIn;
        output tcpOut;
}

//simple HttpBrowserStats like ITCPApp {
//    parameters:
//        @signal[rcvdPk](type=cPacket);
//        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
//}

network QuestionB {
    parameters:
        double numclients @prompt("Number of clients") = default(1);
        @display("bgb=1000,600;bgl=4");

    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=50,50;i=block/cogwheel");
        }

        controller: HttpController {
            parameters:
                @display("p=100,50;i=block/cogwheel");
        }

        origin: StandardHost {
            parameters:
                @display("i=device/server_l;p=950,50");
        }

        router_3: Router {
            parameters:
                @display("i=abstract/router_l;p=500,50");
        }

        server1: StandardHost {
            parameters:
                @display("i=device/server_l;p=950,300");
        }

        server2: StandardHost {
            parameters:
                @display("i=device/server_l;p=50,300");
        }

        router_2: Router {
            parameters:
                @display("i=abstract/router_l;p=500,300");
        }

        router_1: Router {
            parameters:
                @display("i=abstract/router_l;p=500,550");
        }

        client[numclients]: StandardHost {
            parameters:
                @display("i=device/laptop_l;p=950,550");
        }

    connections:
        router_1.ethg++ <--> ethernetline <--> router_2.ethg++;
        router_2.ethg++ <--> ethernetline <--> router_3.ethg++;

        origin.ethg++ <--> ethernetline <--> router_3.ethg++;

        server1.ethg++ <--> ethernetline <--> router_2.ethg++;
        server2.ethg++ <--> ethernetline <--> router_2.ethg++;

        for i=0..numclients-1 {
            client[i].ethg++ <--> ethernetline <--> router_1.ethg++;
        }
}

