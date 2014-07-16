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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>

#include "UDPRendezvousServerTable.h"
#include "NotifierConsts.h"

//uint32 UDPNatTable::nextEntryNumber = 0;

Define_Module( UDPRendezvousServerTable );


UDPRendezvousServerTable::UDPRendezvousServerTable()
{

}

UDPRendezvousServerTable::~UDPRendezvousServerTable()
{
    for (uint32 i=0; i<rServerEntries.size(); i++)
        delete rServerEntries[i];
}

UDPRendezvousServerEntry* UDPRendezvousServerTable::findEntry(Address privateAddr, uint16 privatePrt)
{
    //Enter_Method_Silent();
    for (UDPRendezvousServerEntryTable::iterator i=rServerEntries.begin(); i!=rServerEntries.end(); ++i)
        if ((*i)->getPrivateAddress() == privateAddr && (*i)->getPrivatePort() == privatePrt)
            {
                return *i;
            }
    return NULL;
}

UDPRendezvousServerEntry* UDPRendezvousServerTable::findEntryWithId(std::string id)
{
    //Enter_Method_Silent();
    for (UDPRendezvousServerEntryTable::iterator i=rServerEntries.begin(); i!=rServerEntries.end(); ++i)
        if ((*i)->getId() == id)
            {
                return *i;
            }
    return NULL;
}

bool UDPRendezvousServerTable::findAllEntry(std::string id, Address privateAddr, uint16 privatePrt, Address publicAddr, uint16 publicPrt)
{
    //Enter_Method_Silent();
    for (UDPRendezvousServerEntryTable::iterator i=rServerEntries.begin(); i!=rServerEntries.end(); ++i)
        if ((*i)->getId() == id && (*i)->getPrivatePort() == privatePrt &&
                (*i)->getPrivateAddress() == privateAddr && (*i)->getPublicAddress() == publicAddr &&
                (*i)->getPublicPort() == publicPrt)
            {
                return true;
            }
    return false;
}

void UDPRendezvousServerTable::addEntry(UDPRendezvousServerEntry* entry)
{
    //Enter_Method_Silent();
    rServerEntries.push_back(entry);
}

/*UDPRendezvousServerEntry* UDPRendezvousServerTable::findEntryWhileReceiving(Address privateAddr, uint16 privatePrt)
{
    //Enter_Method_Silent();
    for (UDPRendezvousServerEntryTable::iterator i=rServerEntries.begin(); i!=rServerEntries.end(); ++i)
        if ((*i)->getPrivateAddress()== privateAddr && (*i)->getPrivatePort()== privatePrt)
            {
                return *i;
            }
    return NULL;
}*/

bool UDPRendezvousServerTable::foundNatPort(uint16 port){
    /*for (UDPRendezvousServerTable::iterator i = rServerEntries.begin(); i != rServerEntries.end(); ++i)
        if ((*i)->getNattedPort() == port) {
            return true;
        }*/
    return false;
}

void UDPRendezvousServerTable::removeEntry(UDPRendezvousServerEntry* entry)
{
    //Enter_Method_Silent();
    for (UDPRendezvousServerEntryTable::iterator i=rServerEntries.begin(); i!=rServerEntries.end(); ++i)
        if (((*i)->getPrivateAddress()==entry->getPrivateAddress() && (*i)->getPrivatePort()==entry->getPrivatePort() &&
                (*i)->getPublicAddress()==entry->getPublicAddress() && (*i)->getPublicPort()==entry->getPublicPort()))
        {
            rServerEntries.erase(i);
            return;
        }
}

void UDPRendezvousServerTable::printTable()
{
    for (UDPRendezvousServerEntryTable::iterator i=rServerEntries.begin(); i!=rServerEntries.end(); ++i)
    {
        std::cerr << "ID:" << (*i)->getId() << ", Private Address:" << (*i)->getPrivateAddress() << "  Private Port:" << (*i)->getPrivatePort() << "  Public Address:" << (*i)->getPublicAddress() << "  Public Port:" << (*i)->getPublicPort() << "\n";
    }
}

UDPRendezvousServerEntry::UDPRendezvousServerEntry()
{
    privateAddress = Address("0.0.0.0");
    publicAddress = Address("0.0.0.0");
    privatePort = 0;
    publicPort = 0;
}

UDPRendezvousServerEntry::~UDPRendezvousServerEntry()
{

}
