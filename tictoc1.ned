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

channel ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

//
// Two instances (tic and toc) of Txc1 connected both ways.
// Tic and toc will pass messages to one another.
//
network Tictoc1
{
    parameters:
        double numclients @prompt("Number of clients") = default(10);

        @display("bgb=812.5,534.3;bgl=3");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                //                moduleTypes = "Router StandardHost";
                //                nonIPModuleTypes = "";
                @display("p=36.96,33.11;i=block/cogwheel");
        }

        controller: HttpController {
            parameters:
                @display("p=46,140;i=block/cogwheel");
        }

        origin: StandardHost {
            parameters:
                @display("i=device/server_l;p=421.96,30");
        }

        router_3: Router {
            parameters:
                @display("i=abstract/router_l;p=238.7,30");
        }

        server1: StandardHost {
            parameters:
                @display("i=device/server_l;p=421.96,150");
        }

        server2: StandardHost {
            parameters:
                @display("i=device/server_l;p=36.96,150");
        }

        router_2: Router {
            parameters:
                @display("i=abstract/router_l;p=238.7,150");
        }

        router_1: Router {
            parameters:
                @display("i=abstract/router_l;p=238.7,300");
        }

        client[numclients]: StandardHost {
            parameters:
                @display("i=device/laptop_l;p=421,300");
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

