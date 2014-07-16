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


#include "UDPHolePunchingApp.h"

#include "AddressResolver.h"
#include "InterfaceTableAccess.h"
#include "NodeOperations.h"
#include "UDPControlInfo_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include <sstream>


Define_Module(UDPHolePunchingApp);

simsignal_t UDPHolePunchingApp::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPHolePunchingApp::rcvdPkSignal = registerSignal("rcvdPk");

UDPHolePunchingApp::UDPHolePunchingApp()
{
    selfMsg = NULL;
}

UDPHolePunchingApp::~UDPHolePunchingApp()
{
    cancelAndDelete(selfMsg);
}

void UDPHolePunchingApp::initialize(int stage)
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
        id = par("givenId");
        startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            error("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");
    }
}

void UDPHolePunchingApp::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UDPHolePunchingApp::setSocketOptions()
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

Address UDPHolePunchingApp::chooseDestAddr()
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

void UDPHolePunchingApp::sendPacket()
{
    char msgName[32];
    std::stringstream ss;

    sprintf(msgName, "UDPHolePunchingAppData-%d", numSent);

    IInterfaceTable* ift = InterfaceTableAccess().get(this);
    localIP = ift->getInterface(1)->getNetworkAddress();

    /*cModuleType *moduleType = cModuleType::get("UDPTester_Sc2");

    if(moduleType == NULL ) std::cerr << "Module type is null\n";
    else std::cerr << "Module type is not null\n";*/

    //std::cerr << "My IP Address is:" << localIP << "\n";

    std::cout << "My IP Address is:" << localIP << " and my ID is:" << id << "\n";

    ss << "Registration:"  << "id-" << id << ":ip-" << localIP << ":port-" << localPort;
    const char *payloadString = ss.str().c_str();

    //std::cerr << payloadString << "\n";

    /*cMessage *msg = new cMessage("Hello World!");
    payload->;*/

    cPacket *payload = new cPacket(payloadString);
    //cPacket *payload = new cPacket("Hello World");
    payload->setByteLength(par("messageLength").longValue());

    Address destAddr = chooseDestAddr();

    std::cout << "My IP Address is:" << localIP << " and my ID is:" << id << " and the dest address:" << destAddr << "\n";

    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;
}

void UDPHolePunchingApp::processStart()
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

void UDPHolePunchingApp::processSend()
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

void UDPHolePunchingApp::processStop()
{
    socket.close();
}

void UDPHolePunchingApp::handleMessageWhenUp(cMessage *msg)
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
    }else if (msg->getKind() == 20)
    {
        // 20 indicates that the server has registered the peer successfully. So process it now.
        processRegistrationSuccessfulPacket(PK(msg));
    }else if (msg->getKind() == 25)
    {
        // 25 indicates that the server has responded against a query request. So process it now.
        processQueryPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPHolePunchingApp::processPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    std::string packet = pk->getFullName();

    std::string str = "Registration successful";

    std::size_t found = packet.find(str);
    if (found != std::string::npos) {
        processRegistrationSuccessfulPacket(pk);
    } else {
        str = "Query";
        found = packet.find(str);
        if (found != std::string::npos) {
            processQueryPacket(pk);
        }
    }


    //delete pk;
    //numReceived++;
}

void UDPHolePunchingApp::processRegistrationSuccessfulPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV << "Received Registration Successful Packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

    IInterfaceTable* ift = InterfaceTableAccess().get(this);
    localIP = ift->getInterface(1)->getNetworkAddress();

    //std::cerr << "My IP Address is:" << localIP << "\n";

    std::stringstream ss;
    if(strcasecmp(id, "A") == 0){
        ss << "Query:"  << "id-B";
    }
    if(strcasecmp(id, "B") == 0){
        ss << "Query:"  << "id-A";
    }

    const char *payloadString = ss.str().c_str();

    //std::cerr << payloadString << "\n";

    /*cMessage *msg = new cMessage("Hello World!");
     payload->;*/

    cPacket *payload = new cPacket(payloadString);
    //cPacket *payload = new cPacket("Hello World");
    payload->setByteLength(par("messageLength").longValue());

    Address destAddr = chooseDestAddr();

    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);

    //delete pk;
}

void UDPHolePunchingApp::processQueryPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV << "Received Query Response Packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

    std::string packet = pk->getFullName();


    std::vector<std::string> numbers; //we'll put all of the tokens in here
    std::string temp;

    while (packet.find(":", 0) != std::string::npos) { //does the string a comma in it?
        size_t pos = packet.find(":", 0); //store the position of the delimiter
        temp = packet.substr(0, pos);      //get the token
         packet.erase(0, pos + 1);          //erase it from the source
         numbers.push_back(temp);                //and put it into the array
    }

    numbers.push_back(packet);

    std::string regiStr= "Query";
    if(strcasecmp(numbers[0].c_str(), regiStr.c_str()) == 0){

        std::size_t pos4 = numbers[3].find("-", 0); //store the position of the delimiter
        std::string destAddrString = (numbers[3].substr(pos4 + 1)).c_str();

        pos4 = numbers[4].find("-", 0); //store the position of the delimiter
        std::string destPortString = (numbers[4].substr(pos4 + 1)).c_str();

        Address destAddrReceived = Address(destAddrString.c_str());
        int destPort = atoi(destPortString.c_str());

        cPacket *payload = NULL;
        if (strcasecmp(id, "A") == 0) {
            payload = new cPacket("Hello World from A");
        }
        else if (strcasecmp(id, "B") == 0) {
            payload = new cPacket("Hello World from B");
        }

        emit(sentPkSignal, payload);
        socket.sendTo(payload, destAddrReceived, destPort);


        //simtime_t start = std::max(startTime, simTime());
        //scheduleAt(start+60, payload);
        //delete pk;
    }

}

bool UDPHolePunchingApp::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))
    {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
    return true;
}

bool UDPHolePunchingApp::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPHolePunchingApp::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

