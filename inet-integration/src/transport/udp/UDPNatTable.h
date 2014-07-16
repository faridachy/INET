//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __UDPNATTABLE_H
#define __UDPNATTABLE_H

#include <vector>
#include <omnetpp.h>
#include "Address.h"
//#include "UDPAssociation.h" //The association is for UDP


class INET_API UDPNatEntry : public cPolymorphic
{
  protected:
    uint32 entryNumber;
    Address srcAddress;
    Address destAddress;
    Address nattedAddress;
    uint16 srcPort;
    uint16 destPort;
    uint16 nattedPort;

  public:
    UDPNatEntry();
    ~UDPNatEntry();

    cMessage* NatTimer;
    void setSrcAddress(Address addr) {srcAddress = addr;};
    void setDestAddress(Address addr) {destAddress = addr;};
    void setNattedAddress(Address addr) {nattedAddress = addr;};
    void setSrcPort(uint16 port) {srcPort = port;};
    void setDestPort(uint16 port) {destPort = port;};
    void setNattedPort(uint16 port) {nattedPort = port;};
    void setEntryNumber(uint32 number) {entryNumber = number;};

    Address getSrcAddress() {return srcAddress;};
    Address getDestAddress() {return destAddress;};
    Address getNattedAddress() {return nattedAddress;};
    uint16 getSrcPort() {return srcPort;};
    uint16 getDestPort() {return destPort;};
    uint16 getNattedPort() {return nattedPort;};
};


class INET_API UDPNatTable : public cSimpleModule
{
  public:

    typedef std::vector<UDPNatEntry*> UDPNatEntryTable;

    UDPNatEntryTable natEntries;

    UDPNatTable();

    ~UDPNatTable();

    static uint32 nextEntryNumber;

    //void addNatEntry(UDPNatEntry* entry);

    //UDPNatEntry* findNatEntryWhileSending(Address srcAddr, uint16 srcPrt, Address destAddr, uint16 destPrt);
    UDPNatEntry* findNatEntryWhileSending(Address srcAddr, uint16 srcPrt);

    UDPNatEntry* findNatEntryWhileReceiving(Address nattedAddr, uint16 nattedPrt);

/*    UDPNatEntry* getEntry(Address globalAddr, uint16 globalPrt, Address nattedAddr, uint16 nattedPrt, uint32 localVtag);

    UDPNatEntry* getSpecialEntry(Address globalAddr, uint16 globalPrt, Address nattedAddr, uint16 nattedPrt);

    UDPNatEntry* getLocalInitEntry(Address globalAddr, uint16 localPrt, uint16 globalPrt);

    UDPNatEntry* getLocalEntry(Address globalAddr, uint16 localPrt, uint16 globalPrt, uint32 localVtag);*/

    bool foundNatPort(uint16 port);

    void removeEntry(UDPNatEntry* entry);

    void printNatTable();

    static uint32 getNextEntryNumber() {return nextEntryNumber++;};
};

#endif
