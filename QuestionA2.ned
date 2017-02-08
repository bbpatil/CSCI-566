package csci_566_proj_1;

simple TxcA2 {
    parameters:
        volatile double delayTime @unit(s); // delay before transmitting a new message
        @display("i=block/routing");
    gates:
        inout gate[];
}

network QuestionA2 {
    types:
        channel Channel extends ned.DelayChannel { delay = 100ms; }
    submodules:
        tic[6]: TxcA2;
    connections:
        tic[0].gate++ <--> Channel <--> tic[1].gate++;
        tic[1].gate++ <--> Channel <--> tic[2].gate++;
        tic[1].gate++ <--> Channel <--> tic[4].gate++;
        tic[3].gate++ <--> Channel <--> tic[4].gate++;
        tic[4].gate++ <--> Channel <--> tic[5].gate++;
}
