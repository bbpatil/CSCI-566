package csci_566_proj_1;

simple Txc {
    parameters:
        @display("i=block/routing");
    gates:
        inout port[];
}

network QuestionA {
    types:
        channel Channel extends ned.DelayChannel { delay = 100ms; }
    submodules:
        tic[6]: Txc;
    connections:
        tic[0].port++ <--> Channel <--> tic[1].port++;
        tic[1].port++ <--> Channel <--> tic[2].port++;
        tic[1].port++ <--> Channel <--> tic[4].port++;
        tic[3].port++ <--> Channel <--> tic[4].port++;
        tic[4].port++ <--> Channel <--> tic[5].port++;
}
