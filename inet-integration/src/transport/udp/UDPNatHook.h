#ifndef __INET_UDPNATHOOK_H
#define __INET_UDPNATHOOK_H

#include "INetfilter.h"
#include "UDPNatTable.h"
#include "INETDefs.h"
#include "InterfaceTableAccess.h"
#include <map>

//class IPv4;

class INET_API HolePunched : public cPolymorphic
{
  protected:
    std::string destAddr;
    short unsigned int destPort;

  public:
    /*HolePunched();
    ~HolePunched();*/

    const std::string getDestAddr() const {
        return destAddr;
    }

    void setDestAddr(const std::string destAddr) {
        this->destAddr = destAddr;
    }

    const unsigned short int getDestPort() const {
        return destPort;
    }

    void setDestPort(unsigned short int destPort) {
        this->destPort = destPort;
    }
};

class INET_API UDPNatHook : public cSimpleModule, INetfilter::IHook
{
    protected:
        IPv4 *ipLayer;      // IPv4 module
        UDPNatTable* natTable;
        IRoutingTable *rt;
        IInterfaceTable *ift;
        uint64 nattedPackets;
        void initialize();
        void finish();
        bool detectPrivateIP(std::string ip);
        bool checkDestIpPort(std::string destAddr, int port);

        typedef std::vector<HolePunched*> HolePunchedList ;
        HolePunchedList holePunchedList;


    public:
      virtual IHook::Result datagramPreRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      virtual IHook::Result datagramForwardHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      virtual IHook::Result datagramPostRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      virtual IHook::Result datagramLocalInHook(INetworkDatagram* datagram, const InterfaceEntry* inIE);
      virtual IHook::Result datagramLocalOutHook(INetworkDatagram* datagram, const InterfaceEntry*& outIE, Address& nextHopAddr);

};

#endif
