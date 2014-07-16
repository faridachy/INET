//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

simple Txc5
{
    parameters:
        bool sendMsgOnInit = default(false); // whether the module should send out a message on initialization
        int limit = default(2);   // another parameter with a default value
        @display("i=routing");
    gates:
        input in;
        output out;
}

simple Tic5 extends Txc5
{
    parameters:
      @display("i=,cyan");    
      sendMsgOnInit = true; // Tic modules should send a message on init
}

simple Toc5 extends Txc5 
{
    parameters:
      @display("i=,gold");
      sendMsgOnInit = false; // Toc modules should NOT send a message on init
}


network Tictoc5
{
    submodules:
        tic:Tic5; 
        toc:Toc5;
    connections:
        tic.out --> {  delay = 100ms; } --> toc.in;
        tic.in <-- {  delay = 100ms; } <-- toc.out;
}


