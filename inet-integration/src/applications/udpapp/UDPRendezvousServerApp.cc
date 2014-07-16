//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "UDPRendezvousServerApp.h"

#include "AddressResolver.h"
#include "InterfaceTableAccess.h"
#include "NodeOperations.h"
#include "UDPControlInfo_m.h"
#include <boost/tokenizer.hpp>



Define_Module(UDPRendezvousServerApp);

simsignal_t UDPRendezvousServerApp::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPRendezvousServerApp::rcvdPkSignal = registerSignal("rcvdPk");

UDPRendezvousServerApp::UDPRendezvousServerApp()
{
    selfMsg = NULL;
}

UDPRendezvousServerApp::~UDPRendezvousServerApp()
{
    cancelAndDelete(selfMsg);
}

void UDPRendezvousServerApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            error("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");
        //serverTable = new UDPRendezvousServerTable();
    }
}

void UDPRendezvousServerApp::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UDPRendezvousServerApp::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0])
    {
        IInterfaceTable *ift = InterfaceTableAccess().get(this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups)
        socket.joinLocalMulticastGroups();
}

Address UDPRendezvousServerApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isLinkLocal()) // KLUDGE for IPv6
    {
        const char *destAddrs = par("destAddresses");
        cStringTokenizer tokenizer(destAddrs);
        const char *token;

        for (int i = 0; i <= k; ++i)
            token = tokenizer.nextToken();
        destAddresses[k] = AddressResolver().resolve(token);
    }
    return destAddresses[k];
}

void UDPRendezvousServerApp::sendPacket()
{
    char msgName[32];
    sprintf(msgName, "UDPRendezvousServerAppData-%d", numSent);
    cPacket *payload = new cPacket("Hello World!");
    /*cMessage *msg = new cMessage("Hello World!");
    payload->;*/
    payload->setByteLength(par("messageLength").longValue());

    Address destAddr = chooseDestAddr();

    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;
}

void UDPRendezvousServerApp::processStart()
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != NULL) {
        Address result;
        AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV << "cannot resolve destination address: " << token << endl;
        else
            destAddresses.push_back(result);
    }

    if (!destAddresses.empty())
    {
        selfMsg->setKind(SEND);
        processSend();
    }
    else
    {
        if (stopTime >= SIMTIME_ZERO)
        {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

void UDPRendezvousServerApp::processSend()
{
    sendPacket();
    simtime_t d = simTime() + par("sendInterval").doubleValue();
    if (stopTime < SIMTIME_ZERO || d < stopTime)
    {
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
    else
    {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void UDPRendezvousServerApp::processStop()
{
    socket.close();
}

void UDPRendezvousServerApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START: processStart(); break;
            case SEND:  processSend(); break;
            case STOP:  processStop(); break;
            default: throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == 5)
        {
            // 5 is our defined packet type. 5 indicates that it is a registration packet.
            processRegistrationPacket(PK(msg));
        }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        //std::cout << "The message that I have received in else:" << msg->getDisplayString() << "\n";
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPRendezvousServerApp::processPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV << "Received packet in Process Packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    std::cerr << "Received packet in Process Packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    std::string packet = pk->getFullName();
    std::string tmpPacket = pk->getFullName();

    /*std::string numbers_str =
            "zero,one,two,three,four,five,six,seven,eight,nine,ten";*/
    std::vector<std::string> numbers; //we'll put all of the tokens in here
    std::string temp;

    while (packet.find(":", 0) != std::string::npos) { //does the string a comma in it?
        size_t pos = packet.find(":", 0); //store the position of the delimiter
        temp = packet.substr(0, pos);      //get the token
        packet.erase(0, pos + 1);          //erase it from the source
        numbers.push_back(temp);                //and put it into the array
    }

    numbers.push_back(packet);           //the last token is all alone

    /*for(std::vector<std::string>::iterator it = numbers.begin(); it != numbers.end(); ++it) {
        std::cerr << "String:" << *it << std::endl;
    }*/

    /*std::cerr << "String:" << numbers[0] << std::endl;
    std::cerr << "String:" << numbers[1] << std::endl;
    std::cerr << "String:" << numbers[2] << std::endl;
    std::cerr << "String:" << numbers[3] << std::endl;*/

    std::string regiStr= "Registration";
    if(strcasecmp(numbers[0].c_str(), regiStr.c_str()) == 0){
        //std::cerr << "This is a registration msg.\n";
        std::size_t pos2 = numbers[1].find("-", 0); //store the position of the delimiter
        std::string tempId = (numbers[1].substr(pos2 + 1)).c_str();

        std::size_t pos3 = numbers[2].find("-", 0); //store the position of the delimiter
        std::string tempIp = (numbers[2].substr(pos3 + 1)).c_str();

        std::size_t pos4 = numbers[3].find("-", 0); //store the position of the delimiter
        std::string tempPort = (numbers[3].substr(pos4 + 1)).c_str();

        std::cerr << "This is a registration msg. Id is:" << tempId << ". IP is:" << tempIp << ". Port is:" << tempPort <<  "\n";

        UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
        Address srcAddress = ctrl->getSrcAddr();

        int srcPort = ctrl->getSrcPort();

        Address privateAddr = Address(tempIp.c_str());

        if(!serverTable.findAllEntry(tempId, privateAddr, std::atoi(tempPort.c_str()), srcAddress, srcPort)){
            UDPRendezvousServerEntry *entry = new UDPRendezvousServerEntry();
            entry->setId(tempId);

            entry->setPrivateAddress(privateAddr);
            entry->setPrivatePort(std::atoi(tempPort.c_str()));
            entry->setPublicAddress(srcAddress);
            entry->setPublicPort(srcPort);
            serverTable.addEntry(entry);

            serverTable.printTable();
            /*cMessage *msg = new cMessage();
            msg->setKind(20);*/

            cPacket *returnPacket = new cPacket("Registration successful!");
            returnPacket->setKind(20);
            socket.sendTo(returnPacket, srcAddress, srcPort);
            //delete returnPacket;
        }
    }

    regiStr = "Query";
    if (strcasecmp(numbers[0].c_str(), regiStr.c_str()) == 0) {
        //std::cerr << "This is a registration msg.\n";
        std::size_t pos2 = numbers[1].find("-", 0); //store the position of the delimiter
        std::string tempId = (numbers[1].substr(pos2 + 1)).c_str();

        //std::cerr << "This is a registration msg. Id is:" << tempId << ". IP is:" << tempIp << ". Port is:" << tempPort <<  "\n";

        UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
        Address srcAddress = ctrl->getSrcAddr();

        int srcPort = ctrl->getSrcPort();

        UDPRendezvousServerEntry* tempEntry = serverTable.findEntryWithId(tempId);

        if (tempEntry != NULL) {

            std::stringstream ss;
            ss << "Query:private address-" << tempEntry->getPrivateAddress() << ":private port-" << tempEntry->getPrivatePort() << ":public address-" << tempEntry->getPublicAddress() << ":public port-" << tempEntry->getPublicPort();
            const char *payloadString = ss.str().c_str();

            /*cMessage *msg = new cMessage(payloadString);
            msg->setKind(25);*/

            //cPacket *returnPacket = PK(msg);
            cPacket *returnPacket = new cPacket(payloadString);
            returnPacket->setKind(25);
            socket.sendTo(returnPacket, srcAddress, srcPort);
            //delete returnPacket;
        }
    }

    //std::cout << "Number " << 3 << " is " << numbers[3] << std::endl;

    //std::cerr << "The message that I have received in process packet:" << packet << "\n";
    delete pk;
    numReceived++;


    //serverTable.printTable();
    /*// determine its source address/port
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
    Address srcAddress = ctrl->getSrcAddr();
    int srcPort = ctrl->getSrcPort();
    delete ctrl;
    delete pk;

    // send back
    socket.sendTo(pk, srcAddress, srcPort);

    if (ev.isGUI()){
        char buf[40];
        sprintf(buf, "rcvd: pkts and now sending back...");
        getDisplayString().setTagArg("t", 0, buf);
    }*/
}

void UDPRendezvousServerApp::processRegistrationPacket(cMessage *msg)
{
    std::cout << "The message that I have received in processRegistrationPacket:" << msg->getDisplayString() << "\n";

    cPacket *pk = PK(msg);
    emit(rcvdPkSignal, pk);
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;

    // determine its source address/port
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
    Address publicAddress = ctrl->getSrcAddr();
    int publicPort = ctrl->getSrcPort();

    delete ctrl;
    delete pk;


    // send back
    /*socket.sendTo(pk, srcAddress, srcPort);

    if (ev.isGUI()){
        char buf[40];
        sprintf(buf, "rcvd: pkts and now sending back...");
        getDisplayString().setTagArg("t", 0, buf);
    }*/
}

bool UDPRendezvousServerApp::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))
    {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
    return true;
}

bool UDPRendezvousServerApp::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPRendezvousServerApp::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

