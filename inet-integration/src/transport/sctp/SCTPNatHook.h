#ifndef __INET_SCTPNATHOOK_H
#define __INET_SCTPNATHOOK_H

#include "INetfilter.h"
#include "SCTPNatTable.h"
#include "INETDefs.h"
#include "InterfaceTableAccess.h"

class IPv4;

class INET_API SCTPNatHook : public cSimpleModule, INetfilter::IHook
{
    protected:
        IPv4 *ipLayer;      // IPv4 module
        SCTPNatTable* natTable;
        IRoutingTable *rt;
        IInterfaceTable *ift;
        uint64 nattedPackets;
        void initialize();
        void finish();
    
    public:
      virtual IHook::Result datagramPreRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      virtual IHook::Result datagramForwardHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      virtual IHook::Result datagramPostRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      virtual IHook::Result datagramLocalInHook(INetworkDatagram* datagram, const InterfaceEntry* inIE);
      virtual IHook::Result datagramLocalOutHook(INetworkDatagram* datagram, const InterfaceEntry*& outIE, Address& nextHopAddr);
      void sendBackError(IPv4Datagram* dgram);
};

#endif