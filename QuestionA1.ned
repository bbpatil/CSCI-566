package csci_566_proj_1;

simple TxcA1 {
    parameters:
        volatile double delayTime @unit(s); // Exercise 13 (Question 4)
        @display("i=block/routing");
    gates:
        inout port[];
}

network QuestionA1 {
    types:
        channel Channel extends ned.DelayChannel { delay = 100ms; }
    submodules:
        tic[6]: TxcA1;
    connections:
        tic[0].port++ <--> Channel <--> tic[1].port++;
        tic[1].port++ <--> Channel <--> tic[2].port++;
        tic[1].port++ <--> Channel <--> tic[4].port++;
        tic[3].port++ <--> Channel <--> tic[4].port++;
        tic[4].port++ <--> Channel <--> tic[5].port++;
}
