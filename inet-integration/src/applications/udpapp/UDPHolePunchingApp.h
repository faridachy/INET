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


#ifndef __INET_UDPHOLEPUNCHINGPP_H
#define __INET_UDPHOLEPUNCHINGPP_H

#include <vector>

#include "INETDefs.h"

#include "ApplicationBase.h"
#include "UDPSocket.h"
#include "IPv4Address.h"

/**
 * UDP Hole Punching application. See NED for more info.
 */
class INET_API UDPHolePunchingApp : public ApplicationBase
{
  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };

    UDPSocket socket;
    int localPort, destPort;
    std::vector<Address> destAddresses;
    simtime_t startTime;
    simtime_t stopTime;
    cMessage *selfMsg;
    Address localIP;
    const char *id;

    // statistics
    int numSent;
    int numReceived;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

    // chooses random destination address
    virtual Address chooseDestAddr();
    virtual void sendPacket();
    virtual void processPacket(cPacket *msg);
    virtual void processRegistrationSuccessfulPacket(cPacket *msg);
    virtual void processQueryPacket(cPacket *msg);
    virtual void setSocketOptions();

  public:
    UDPHolePunchingApp();
    ~UDPHolePunchingApp();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessageWhenUp(cMessage *msg);
    virtual void finish();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();
};

#endif

