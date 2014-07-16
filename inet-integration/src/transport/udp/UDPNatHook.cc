#include "IPv4ControlInfo.h"
#include "IPv4.h"
#include "UDPNatHook.h"
#include "IPv4RoutingTableAccess.h"
#include "IPv4InterfaceData.h"
#include "UDPPacket_m.h"

Define_Module(UDPNatHook);

void UDPNatHook::initialize()
{
    IPv4RoutingTableAccess routingTableAccess;
    InterfaceTableAccess interfaceTableAccess;

    ipLayer = check_and_cast<IPv4*>(getParentModule()->getSubmodule("networkLayer")->getSubmodule("ip"));
    rt = routingTableAccess.get();
    ift = interfaceTableAccess.get();
    natTable = new UDPNatTable();
    nattedPackets = 0;
    ipLayer->registerHook(0, this);
}


INetfilter::IHook::Result UDPNatHook::datagramForwardHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr)
{
    std::cerr << "AT UDPNatHook::datagramForwardHook..." << "\n";
    std::cout << "AT UDPNatHook::datagramForwardHook..." << "\n";
    //std::cerr<<"Inner IP Address::" << inIE->ipv4Data()->getIPAddress() <<", Outer IP Address:" << outIE->ipv4Data()->getIPAddress() <<"\n";

    IPv4Datagram *dgram;
    UDPNatEntry* entry;
    UDPPacket* udpPacket;
    HolePunched* holePunched = NULL;

    //HolePunched *holePunched;

    dgram = dynamic_cast<IPv4Datagram *>(datagram);

    if(dgram->getTransportProtocol() == IP_PROT_ICMP) {

    } else {
        udpPacket = check_and_cast<UDPPacket*>(dgram->getEncapsulatedPacket());

            //std::cerr<<"Datagram Source IP Address::" << dgram->getSourceAddress() <<", Source Port:" << udpPacket->getSourcePort() <<"\n";
            //std::cerr<<"Datagram Destination IP Address::" << dgram->getDestAddress() <<", Destination Port:" << udpPacket->getDestinationPort() <<"\n";

            //holePunched = new HolePunched();
            /*Address dAddress = dgram->getDestAddress();
            std::string tempString = "" + dAddress.toIPv4().str(false);
            if(!checkDestIpPort(tempString, udpPacket->getDestinationPort())) {
                holePunched->setDestAddr(tempString);
                holePunched->setDestPort(udpPacket->getDestinationPort());
                holePunchedList.push_back(holePunched);
            }*/

            if(detectPrivateIP(dgram->getSourceAddress().toIPv4().str(false))){
                std::cerr << "local IP Address Detected..." << "\n";

            Address dAddress = dgram->getDestAddress();
            std::string tempString = dAddress.toIPv4().str(false);
            if (!checkDestIpPort(tempString, udpPacket->getDestinationPort())) {
                holePunched = new HolePunched();
                if (holePunched != NULL) {
                    std::cerr << "Adding dest address:" << tempString << " and dest port:" << udpPacket->getDestinationPort() << "\n";
                    std::cout << "Adding dest address:" << tempString << " and dest port:" << udpPacket->getDestinationPort() << "\n";
                    holePunched->setDestAddr(tempString);
                    holePunched->setDestPort(udpPacket->getDestinationPort());
                    holePunchedList.push_back(holePunched);
                }

            }

                bool local = rt->isLocalAddress(dgram->getDestAddress());

                if (!local) {
                    /*entry = natTable->findNatEntryWhileSending(dgram->getSrcAddress(),
                            udpPacket->getSourcePort(), dgram->getDestAddress(),
                            udpPacket->getDestinationPort());*/
                    entry = natTable->findNatEntryWhileSending(dgram->getSrcAddress(), udpPacket->getSourcePort());
                    if (entry != NULL) {
                        //std::cerr << "Entry found in the NAT Table..." << "\n";
                        dgram->setSourceAddress(entry->getNattedAddress().toIPv4());
                        udpPacket->setSourcePort(entry->getNattedPort());
                    } else {
                            //std::cerr
                              //  << "Entry not found in the NAT Table, so creating a new entry in the NAT table.."
                               // << "\n";
                            entry = new UDPNatEntry();
                            uint16 natPort = NULL;

                            natPort = rand() % (5000 - 1025 + 1) + 1025;

                            while (natTable->foundNatPort(natPort)) {
                                natPort = rand() % (5000 - 1025 + 1) + 1025;
                            }
                            //std::cerr << "Generated Natted Port:" << natPort << "\n";

                            entry->setDestAddress(dgram->getDestinationAddress());
                            entry->setDestPort(udpPacket->getDestinationPort());
                            entry->setSrcAddress(dgram->getSourceAddress());
                            entry->setSrcPort(udpPacket->getSourcePort());
                            entry->setNattedPort(natPort);
                            //entry->setNattedPort(udpPacket->getSourcePort());
                            entry->setNattedAddress(outIE->ipv4Data()->getIPAddress());
                            natTable->natEntries.push_back(entry);

                            dgram->setSourceAddress(outIE->ipv4Data()->getIPAddress());
                            udpPacket->setSourcePort(natPort);
                            //udpPacket->setSourcePort(udpPacket->getSourcePort());
                        }
                 }
            } else {
                entry = natTable->findNatEntryWhileReceiving(dgram->getDestAddress(), udpPacket->getDestinationPort());
                if (entry != NULL) {
                    //std::cerr << "Entry found in the NAT Table while receiving..." << "\n";
                    dgram->setDestinationAddress(entry->getNattedAddress().toIPv4());
                    udpPacket->setDestinationPort(entry->getNattedPort());
                }
            }
    }


    /*UDPNatEntry* entry;
    IPv4Datagram *dgram;

    dgram = dynamic_cast<IPv4Datagram *>(datagram);
    UDPPacket* udpPacket = check_and_cast<UDPPacket*>(dgram->getEncapsulatedPacket());

    bool local = rt->isLocalAddress(dgram->getDestAddress());

        if(!local){
            entry = natTable->findNatEntryWhileSending(dgram->getSrcAddress(),udpPacket->getSourcePort(),dgram->getDestAddress(),udpPacket->getDestinationPort());
            if(entry != NULL){
                std::cerr<<"Entry found in the NAT Table..."<<"\n";
                dgram->setSourceAddress(entry->getNattedAddress().toIPv4());
                udpPacket->setSourcePort(entry->getNattedPort());
            } else {
                std::cerr<<"Entry not found in the NAT Table, so creating a new entry in the NAT table.."<<"\n";
                entry = new UDPNatEntry();
                uint16 natPort = NULL;

                natPort = rand()%(65535-1025 + 1) + 1025;

                while(natTable->foundNatPort(natPort)){
                    natPort = rand()%(65535-1025 + 1) + 1025;
                }
                entry->setDestAddress(dgram->getDestinationAddress());
                entry->setDestPort(udpPacket->getDestinationPort());
                entry->setSrcAddress(dgram->getSourceAddress());
                entry->setSrcPort(udpPacket->getSourcePort());
                entry->setNattedPort(natPort);
                entry->setNattedAddress(outIE->ipv4Data()->getIPAddress());

                dgram->setSourceAddress(outIE->ipv4Data()->getIPAddress());
                udpPacket->setSourcePort(natPort);
            }
        }*/
        /*entry = natTable->findNatEntryWhileReceiving(dgram->getDestinationAddress(),
                udpPacket->getDestinationPort());
        if (entry != NULL) {
            std::cerr << "Entry found in the NAT Table..." << "\n";
            dgram->setDestinationAddress(entry->getSrcAddress().toIPv4());
            udpPacket->setSourcePort(entry->getSrcPort());
        } else {
            std::cerr
                    << "Entry not found while receiving, so resume as normal...."
                    << "\n";
        }*/

    natTable->printNatTable();

	return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result UDPNatHook::datagramPreRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr)
{
    std::cerr<<"AT UDPNatHook::datagramPreRoutingHook..."<<"\n";
    std::cout<<"AT UDPNatHook::datagramPreRoutingHook..."<<"\n";

    //std::cerr<<"Inner IP Address::" << inIE->ipv4Data()->getIPAddress() << "\n";

    IPv4Datagram *dgram;
    UDPNatEntry* entry;


    dgram = dynamic_cast<IPv4Datagram *>(datagram);
    UDPPacket* udpPacket = check_and_cast<UDPPacket*>(dgram->getEncapsulatedPacket());

    //std::cerr<<"Datagram Source IP Address::" << dgram->getSourceAddress() <<", Source Port:" << udpPacket->getSourcePort() <<"\n";
    //std::cerr<<"Datagram Destination IP Address::" << dgram->getDestAddress() <<", Destination Port:" << udpPacket->getDestinationPort() <<"\n";

    if(detectPrivateIP(dgram->getSourceAddress().toIPv4().str(false))){
        //holePunched = new HolePunched();

    } else {
        Address dAddress = dgram->getSourceAddress();
        std::string tempString = dAddress.toIPv4().str(false);

        if (!checkDestIpPort(tempString, udpPacket->getSourcePort())) {
            //dropAndDelete(dgram);
            return INetfilter::IHook::DROP;
        }
        entry = natTable->findNatEntryWhileReceiving(dgram->getDestAddress(), udpPacket->getDestinationPort());
        //std::cerr << "Now look at the NAT table for the nat traversal..." << "\n";
        if (entry != NULL) {
            //std::cerr << "Entry found in the NAT Table while sending..." << "\n";
            dgram->setDestinationAddress(entry->getSrcAddress().toIPv4());
            udpPacket->setDestinationPort(entry->getSrcPort());
        }
    }

    /*Address dAddress = dgram->getDestAddress();
    std::string tempString = "" + dAddress.toIPv4().str(false);

    if(!checkDestIpPort(tempString, udpPacket->getDestinationPort())) {
        dropAndDelete(dgram);
    }*/

    /*UDPNatEntry* entry;
    IPv4Datagram *dgram;

    dgram = dynamic_cast<IPv4Datagram *>(datagram);

    UDPPacket* udpPacket = check_and_cast<UDPPacket*>(dgram->getEncapsulatedPacket());

    bool local = rt->isLocalAddress(dgram->getDestAddress());

    if(!local){
        entry = natTable->findNatEntryWhileSending(dgram->getSrcAddress(),udpPacket->getSourcePort(),dgram->getDestAddress(),udpPacket->getDestinationPort());
        if(entry != NULL){
            std::cerr<<"Entry found in the NAT Table..."<<"\n";
            dgram->setSourceAddress(entry->getNattedAddress().toIPv4());
            udpPacket->setSourcePort(entry->getNattedPort());
        } else {
            std::cerr<<"Entry not found in the NAT Table, so creating a new entry in the NAT table.."<<"\n";
            entry = new UDPNatEntry();
            uint16 natPort = NULL;

            natPort = rand()%(65535-1025 + 1) + 1025;

            while(natTable->foundNatPort(natPort)){
                natPort = rand()%(65535-1025 + 1) + 1025;
            }
            entry->setDestAddress(dgram->getDestinationAddress());
            entry->setDestPort(udpPacket->getDestinationPort());
            entry->setSrcAddress(dgram->getSourceAddress());
            entry->setSrcPort(udpPacket->getSourcePort());
            entry->setNattedPort(natPort);

            std::cerr<<"Inner IP Address::" << inIE->ipv4Data()->getIPAddress() << "\n";

            entry->setNattedAddress(outIE->ipv4Data()->getIPAddress());

            dgram->setSourceAddress(outIE->ipv4Data()->getIPAddress());
            udpPacket->setSourcePort(natPort);
        }
    }*/

    //natTable->printNatTable();

    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result UDPNatHook::datagramPostRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr)
{
    //std::cerr<<"AT UDPNatHook::datagramPostRoutingHook..."<<"\n";

    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result UDPNatHook::datagramLocalInHook(INetworkDatagram* datagram, const InterfaceEntry* inIE)
{
    //std::cerr<<"AT UDPNatHook::datagramLocalInHook..."<<"\n";

    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result UDPNatHook::datagramLocalOutHook(INetworkDatagram* datagram, const InterfaceEntry*& outIE, Address& nextHopAddr)
{
    //std::cerr<<"AT UDPNatHook::datagramLocalOutHook..."<<"\n";

    return INetfilter::IHook::ACCEPT;
}

bool UDPNatHook::detectPrivateIP(std::string ip){
    std::string str1 = ip.substr(0, 3), str2 = ip.substr(0, 4);
    //std::cerr<<"Detecting IP Address for:" << ip << ", str1::" << str1 << ", str2::" << str2 << "\n";
    if (str1.compare("10.") == 0 || str2.compare("172.") == 0 || str2.compare("192.") == 0) {
        // contains
        //std::cerr<<"Packet coming from local network..."<<"\n";
        return true;
    }
    return false;
}

bool UDPNatHook::checkDestIpPort(std:: string destAddr, int port){
    for (HolePunchedList::iterator i=holePunchedList.begin(); i!=holePunchedList.end(); ++i){
        std::cerr << "In table dest addr:" << (*i)->getDestAddr()<< " and dest port:" << (*i)->getDestPort() << ", passed dest addr:"  << destAddr << " and dest port:" << port << "\n";
        if ((*i)->getDestAddr() == destAddr && (*i)->getDestPort() == port)
        {
            return true;
        }
    }
    return false;
}


void UDPNatHook::finish()
{
    if (ipLayer)
        ipLayer->unregisterHook(0, this);
    ipLayer = NULL;
    delete natTable;
    std::cout<<getFullPath()<<": Natted packets: "<<nattedPackets<<"\n";
}
