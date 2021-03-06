/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelAccess.h
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#ifndef __INET_SIMPLIFIEDRADIOCHANNELACCESS_H
#define __INET_SIMPLIFIEDRADIOCHANNELACCESS_H

#include <list>
#include <limits>

#include "RadioBase.h"
#include "ISimplifiedRadioChannel.h"

// Forward declarations
class SimplifiedRadioFrame;

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with SimplifiedRadioChannel to handle
 * the dynamically created gates. This means EVERY SnrEval (the lowest
 * layer in a host) has to be derived from this class!!!! And please
 * follow the instructions on how to declare a physical layer in a
 * .ned file in "The Design of a Mobility Framework in OMNeT++"
 * paper.
 *
 * Please don't touch this class.
 *
 * @author Marc Loebbers
 * @ingroup radioChannel
 * @ingroup phyLayer
 */
class INET_API SimplifiedRadioChannelAccess : public RadioBase, protected cListener
{
  protected:
    ISimplifiedRadioChannel* cc;  // Pointer to the SimplifiedRadioChannel module
    ISimplifiedRadioChannel::RadioRef myRadioRef;  // Identifies this radio in the SimplifiedRadioChannel module
    cModule *hostModule;    // the host that contains this radio model
    Coord radioPos;  // the physical position of the radio (derived from display string or from mobility models)
    bool positionUpdateArrived;

  public:
    SimplifiedRadioChannelAccess() : cc(NULL), myRadioRef(NULL), hostModule(NULL) {}
    virtual ~SimplifiedRadioChannelAccess();

    /**
     * @brief Called by the signalling mechanism to inform of changes.
     *
     * ChannelAccess is subscribed to position changes.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    /** Finds the radio channel module in the network */
    static ISimplifiedRadioChannel *getSimplifiedRadioChannel();

  protected:
    /** Sends a message to all radios in range */
    virtual void sendToChannel(SimplifiedRadioFrame *msg);

    virtual cPar& getRadioChannelPar(const char *parName) { return dynamic_cast<cModule *>(cc)->par(parName); }
    const Coord& getRadioPosition() const { return radioPos; }
    cModule *getHostModule() const { return hostModule; }

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
};

#endif

