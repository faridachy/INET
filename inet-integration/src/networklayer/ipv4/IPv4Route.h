//
// Copyright (C) 2008 Andras Varga
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

#ifndef __INET_IPv4ROUTE_H
#define __INET_IPv4ROUTE_H


#include "INETDefs.h"

#include "IPv4Address.h"
#include "IRoute.h"

class InterfaceEntry;
class IIPv4RoutingTable;


/**
 * IPv4 unicast route in IIPv4RoutingTable.
 *
 * @see IIPv4RoutingTable, IPv4RoutingTable
 */
class INET_API IPv4Route : public cObject, public IRoute
{
  public:
    /** Cisco like administrative distances */
    enum RouteAdminDist
    {
        dDirectlyConnected = 0,
        dStatic = 1,
        dEIGRPSummary = 5,
        dBGPExternal = 20,
        dEIGRPInternal = 90,
        dIGRP = 100,
        dOSPF = 110,
        dISIS = 115,
        dRIP = 120,
        dEGP = 140,
        dODR = 160,
        dEIGRPExternal = 170,
        dBGPInternal = 200,
        dDHCPlearned = 254,
        dUnknown = 255
    };

  private:
    IIPv4RoutingTable *rt;    ///< the routing table in which this route is inserted, or NULL
    IPv4Address dest;     ///< Destination
    IPv4Address netmask;  ///< Route mask
    IPv4Address gateway;  ///< Next hop
    InterfaceEntry *interfacePtr; ///< interface
    SourceType sourceType;   ///< manual, routing prot, etc.
    unsigned int adminDist; ///< Cisco like administrative distance
    int metric;           ///< Metric ("cost" to reach the destination)
    cObject *source;   ///< Object identifying the source
    cObject *protocolData; ///< Routing Protocol specific data

  private:
    // copying not supported: following are private and also left undefined
    IPv4Route(const IPv4Route& obj);
    IPv4Route& operator=(const IPv4Route& obj);

  protected:
    void changed(int fieldCode);

  public:
    IPv4Route() : rt(NULL), interfacePtr(NULL), sourceType(MANUAL), adminDist(dUnknown),
                  metric(0), source(NULL), protocolData(NULL) {}
    virtual ~IPv4Route();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    bool operator==(const IPv4Route& route) const { return equals(route); }
    bool operator!=(const IPv4Route& route) const { return !equals(route); }
    bool equals(const IPv4Route& route) const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(IIPv4RoutingTable *rt) {this->rt = rt;}
    IIPv4RoutingTable *getRoutingTable() const {return rt;}

    /** test validity of route entry, e.g. check expiry */
    virtual bool isValid() const { return true; }

    virtual void setDestination(IPv4Address _dest)  { if (dest != _dest) {dest = _dest; changed(F_DESTINATION);} }
    virtual void setNetmask(IPv4Address _netmask)  { if (netmask != _netmask) {netmask = _netmask; changed(F_PREFIX_LENGTH);} }
    virtual void setGateway(IPv4Address _gateway)  { if (gateway != _gateway) {gateway = _gateway; changed(F_NEXTHOP);} }
    virtual void setInterface(InterfaceEntry *_interfacePtr)  { if (interfacePtr != _interfacePtr) {interfacePtr = _interfacePtr; changed(F_IFACE);} }
    virtual void setSourceType(SourceType _source)  { if (sourceType != _source) {sourceType = _source; changed(F_SOURCE);} }
    virtual void setAdminDist(unsigned int _adminDist)  { if (adminDist != _adminDist) { adminDist = _adminDist; changed(F_ADMINDIST);} }
    virtual void setMetric(int _metric)  { if (metric != _metric) {metric = _metric; changed(F_METRIC);} }

    /** Destination address prefix to match */
    IPv4Address getDestination() const {return dest;}

    /** Represents length of prefix to match */
    IPv4Address getNetmask() const {return netmask;}

    /** Next hop address */
    IPv4Address getGateway() const {return gateway;}

    /** Next hop interface */
    InterfaceEntry *getInterface() const {return interfacePtr;}

    /** Convenience method */
    const char *getInterfaceName() const;

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    SourceType getSourceType() const {return sourceType;}

    /** Route source specific preference value */
    unsigned int getAdminDist() const  { return adminDist; }

    /** "Cost" to reach the destination */
    int getMetric() const {return metric;}

    void setSource(cObject *_source) { if (source != _source) {source = _source; changed(F_SOURCE);}}
    cObject *getSource() const { return source; }

    cObject *getProtocolData() const { return protocolData; }
    void setProtocolData(cObject *protocolData) { this->protocolData = protocolData; }

    virtual IRoutingTable *getRoutingTableAsGeneric() const;
    virtual void setDestination(const Address& dest) {setDestination(dest.toIPv4());}
    virtual void setPrefixLength(int len) {setNetmask(IPv4Address::makeNetmask(len));}
    virtual void setNextHop(const Address& nextHop) {setGateway(nextHop.toIPv4());}  //TODO rename IPv4 method

    virtual Address getDestinationAsGeneric() const {return getDestination();}
    virtual int getPrefixLength() const {return getNetmask().getNetmaskLength();}
    virtual Address getNextHopAsGeneric() const {return getGateway();} //TODO rename IPv4 method

};

/**
 * IPv4 multicast route in IIPv4RoutingTable.
 * Multicast routing protocols may extend this class to store protocol
 * specific fields.
 *
 * Multicast datagrams are forwarded along the edges of a multicast tree.
 * The tree might depend on the multicast group and the source (origin) of
 * the multicast datagram.
 *
 * The forwarding algorithm chooses the route according the to origin
 * (source address) and multicast group (destination address) of the received
 * datagram. The route might specify a prefix of the origin and a multicast
 * group to be matched.
 *
 * Then the forwarding algorithm copies the datagrams arrived on the parent
 * (upstream) interface to the child interfaces (downstream). If there are no
 * downstream routers on a child interface (i.e. it is a leaf in the multicast
 * routing tree), then the datagram is forwarded only if there are listeners
 * of the multicast group on that link (TRPB routing).
 *
 * @see IIPv4RoutingTable, IPv4RoutingTable
 */
class INET_API IPv4MulticastRoute : public cObject, public IMulticastRoute
{
  private:
    IIPv4RoutingTable *rt;             ///< the routing table in which this route is inserted, or NULL
    IPv4Address origin;            ///< Source network
    IPv4Address originNetmask;     ///< Source network mask
    IPv4Address group;             ///< Multicast group, if unspecified then matches any
    InInterface *inInterface;      ///< In interface (upstream)
    OutInterfaceVector outInterfaces; ///< Out interfaces (downstream)
    SourceType sourceType;            ///< manual, routing prot, etc.
    cObject *source;               ///< Object identifying the source
    int metric;                    ///< Metric ("cost" to reach the source)

  public:
    // field codes for changed()
    enum {F_ORIGIN, F_ORIGINMASK, F_MULTICASTGROUP, F_IN, F_OUT, F_SOURCE, F_METRIC, F_LAST};

  protected:
    void changed(int fieldCode);

  private:
    // copying not supported: following are private and also left undefined
    IPv4MulticastRoute(const IPv4MulticastRoute& obj);
    IPv4MulticastRoute& operator=(const IPv4MulticastRoute& obj);

  public:
    IPv4MulticastRoute() : rt(NULL), inInterface(NULL), sourceType(MANUAL), metric(0) {}
    virtual ~IPv4MulticastRoute();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(IIPv4RoutingTable *rt) {this->rt = rt;}
    IIPv4RoutingTable *getRoutingTable() const {return rt;}

    /** test validity of route entry, e.g. check expiry */
    virtual bool isValid() const { return true; }

    virtual bool matches(const IPv4Address &origin, const IPv4Address &group) const
    {
        return (this->group.isUnspecified() || this->group == group) &&
                IPv4Address::maskedAddrAreEqual(origin, this->origin, this->originNetmask);
    }

    virtual void setOrigin(IPv4Address _origin)  { if (origin != _origin) {origin = _origin; changed(F_ORIGIN);} }
    virtual void setOriginNetmask(IPv4Address _netmask)  { if (originNetmask != _netmask) {originNetmask = _netmask; changed(F_ORIGINMASK);} }
    virtual void setMulticastGroup(IPv4Address _group)  { if (group != _group) {group = _group; changed(F_MULTICASTGROUP);} }
    virtual void setInInterface(InInterface *_inInterface);
    virtual void clearOutInterfaces();
    virtual void addOutInterface(OutInterface *outInterface);
    virtual bool removeOutInterface(const InterfaceEntry *ie);
    virtual void removeOutInterface(unsigned int i);
    virtual void setSourceType(SourceType _source)  { if (sourceType != _source) {sourceType = _source; changed(F_SOURCE);} }
    virtual void setMetric(int _metric)  { if (metric != _metric) {metric = _metric; changed(F_METRIC);} }

    /** Source address prefix to match */
    IPv4Address getOrigin() const {return origin;}

    /** Represents length of prefix to match */
    IPv4Address getOriginNetmask() const {return originNetmask;}

    /** Multicast group address */
    IPv4Address getMulticastGroup() const {return group;}

    /** In interface */
    InInterface *getInInterface() const {return inInterface;}

    /** Number of out interfaces */
    unsigned int getNumOutInterfaces() const {return outInterfaces.size();}

    /** The kth out interface */
    OutInterface *getOutInterface(unsigned int k) const {return outInterfaces.at(k); }

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    SourceType getSourceType() const {return sourceType;}

    /** "Cost" to reach the destination */
    int getMetric() const {return metric;}

    void setSource(cObject *_source) { source = _source; }
    cObject *getSource() const { return source; }

    virtual IRoutingTable *getRoutingTableAsGeneric() const;
    virtual void setEnabled(bool enabled) {/*TODO: setEnabled(enabled);*/}
    virtual void setOrigin(const Address& origin) {setOrigin(origin.toIPv4());}
    virtual void setPrefixLength(int len) {setOriginNetmask(IPv4Address::makeNetmask(len));} //TODO inconsistent naming
    virtual void setMulticastGroup(const Address& group) {setMulticastGroup(group.toIPv4());}

    virtual bool isEnabled() const {return true; /*TODO: isEnabled();*/}
    virtual bool isExpired() const {return !isValid();}  //TODO rename IPv4 method
    virtual Address getOriginAsGeneric() const {return getOrigin();}
    virtual int getPrefixLength() const {return getOriginNetmask().getNetmaskLength();} //TODO inconsistent naming
    virtual Address getMulticastGroupAsGeneric() const {return getMulticastGroup();}
};
#endif // __INET_IPv4ROUTE_H

