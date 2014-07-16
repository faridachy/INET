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

#include "UDPNatTable.h"
#include "NotifierConsts.h"

uint32 UDPNatTable::nextEntryNumber = 0;


Define_Module( UDPNatTable );


UDPNatTable::UDPNatTable()
{

}

UDPNatTable::~UDPNatTable()
{
    for (uint32 i=0; i<natEntries.size(); i++)
        delete natEntries[i];
}

UDPNatEntry* UDPNatTable::findNatEntryWhileSending(Address srcAddr, uint16 srcPrt)
{
    //Enter_Method_Silent();
    for (UDPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getSrcAddress()== srcAddr && (*i)->getSrcPort() == srcPrt)
            {
                return *i;
            }
    return NULL;
}

/*UDPNatEntry* UDPNatTable::findNatEntryWhileSending(Address srcAddr, uint16 srcPrt, Address destAddr, uint16 destPrt)
{
    //Enter_Method_Silent();
    for (UDPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getSrcAddress()== srcAddr && (*i)->getSrcPort() == srcPrt && (*i)->getDestAddress()== destAddr && (*i)->getDestPort()== destPrt)
            {
                return *i;
            }
    return NULL;
}*/

UDPNatEntry* UDPNatTable::findNatEntryWhileReceiving(Address nattedAddr, uint16 nattedPrt)
{
    //Enter_Method_Silent();
   for (UDPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getNattedAddress()== nattedAddr && (*i)->getNattedPort()== nattedPrt)
            {
                return *i;
            }
    return NULL;
}

bool UDPNatTable::foundNatPort(uint16 port){
    for (UDPNatEntryTable::iterator i = natEntries.begin(); i != natEntries.end(); ++i)
        if ((*i)->getNattedPort() == port) {
            return true;
        }
    return false;
}

void UDPNatTable::removeEntry(UDPNatEntry* entry)
{
    //Enter_Method_Silent();
    for (UDPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if (((*i)->getSrcAddress()==entry->getSrcAddress() && (*i)->getSrcPort()==entry->getSrcPort() &&
                (*i)->getNattedAddress()==entry->getNattedAddress() && (*i)->getNattedPort()==entry->getNattedPort())
                && (*i)->getDestAddress()==entry->getDestAddress() && (*i)->getDestPort()==entry->getDestPort())
        {
            natEntries.erase(i);
            return;
        }
}

void UDPNatTable::printNatTable()
{
    for (UDPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
    {
        std::cerr << "Source Address:" << (*i)->getSrcAddress() << "  Source Port:" << (*i)->getSrcPort() << "  Dest Address:" << (*i)->getDestAddress() << "  Dest Port:" << (*i)->getDestPort() << "  Natted Addr:" << (*i)->getNattedAddress() << "  Natted Port:" << (*i)->getNattedPort() << "\n";;
    }
}

UDPNatEntry::UDPNatEntry()
{
    srcAddress = Address("0.0.0.0");
    destAddress = Address("0.0.0.0");
    nattedAddress = Address("0.0.0.0");
    srcPort = 0;
    destPort = 0;
    nattedPort = 0;
}

UDPNatEntry::~UDPNatEntry()
{

}
