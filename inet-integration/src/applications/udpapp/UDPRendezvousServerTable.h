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

#ifndef __UDPRendezvousServerTable_H
#define __UDPRendezvousServerTable_H

#include <vector>
#include <omnetpp.h>
#include "Address.h"


class INET_API UDPRendezvousServerEntry : public cPolymorphic
{
  protected:
    Address privateAddress;
    Address publicAddress;
    uint16 privatePort;
    uint16 publicPort;
    std::string id;

  public:
    UDPRendezvousServerEntry();
    ~UDPRendezvousServerEntry();

    cMessage* NatTimer;
    void setPrivateAddress(Address addr) {privateAddress = addr;};
    void setPublicAddress(Address addr) {publicAddress = addr;};
    void setPrivatePort(uint16 port) {privatePort = port;};
    void setPublicPort(uint16 port) {publicPort = port;};

    Address getPrivateAddress() {return privateAddress;};
    Address getPublicAddress() {return publicAddress;};
    uint16 getPrivatePort() {return privatePort;};
    uint16 getPublicPort() {
        return publicPort;
    }
    std::string getId() const {
        return id;
    }

    void setId(const std::string i) {
        id = i;
    }

    ;
};


class INET_API UDPRendezvousServerTable : public cSimpleModule
{
  public:

    typedef std::vector<UDPRendezvousServerEntry*> UDPRendezvousServerEntryTable;

    UDPRendezvousServerEntryTable rServerEntries;

    UDPRendezvousServerTable();

    ~UDPRendezvousServerTable();


    UDPRendezvousServerEntry* findEntry(Address privateAddr, uint16 privatePrt);
    bool findAllEntry(std::string id, Address privateAddr, uint16 privatePrt, Address publicAddr, uint16 publicPrt);

    UDPRendezvousServerEntry* findEntryWithId(std::string id);

    bool foundNatPort(uint16 port);

    void addEntry(UDPRendezvousServerEntry* entry);

    void removeEntry(UDPRendezvousServerEntry* entry);

    void printTable();

};

#endif
