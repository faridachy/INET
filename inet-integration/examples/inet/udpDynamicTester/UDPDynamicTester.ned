package inet.examples.inet.udpDynamicTester;

import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import ned.DatarateChannel;
import inet.nodes.ethernet.Eth100G;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.ethernet.Eth1G;
import inet.transport.udp.UDPNatRouter;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;


channel Dynamic extends DatarateChannel
{
    datarate = 100 Mbps;
}

//
// Several hosts on a NAT LAN consiting of a NAT Router...
//

network UDPDynamicTester
{
    parameters:
        int columns = default(1);
        int rows = default(2);
    types:
        module LAN
        {
            parameters:
                int numHosts = default(2);
                @display("i=misc/cloud;is=s");
            gates:
                inout ethg[];
                inout pppg[];
            submodules:
                natRouter: UDPNatRouter {
                    @display("i=block/join");
                }
                host[numHosts]: StandardHost {
                    @display("is=s");
                }
            connections:
                for i=0..numHosts-1 {
                    host[i].pppg++ <--> Dynamic <--> natRouter.pppg++;
                }
                //for i=0..sizeof(pppg) - 1 {
                //    natRouter.pppg++ <--> pppg++;
                //}
                natRouter.pppg++ <--> pppg++;
        }

        module Area
        {
            parameters:
                int numAreas = default(1);
                @display("i=misc/cloud;is=s");
            gates:
                //inout ethg[];
                inout pppg[];
            submodules:
                lan[numAreas]: LAN;
                server: StandardHost {
                    parameters:
                        @display("p=290,30;i=device/server");                    
                }
                router[numAreas]: Router {
                    @display("is=s");
                }
            connections:
                for i=0..numAreas-1 {
                    router[i].pppg++ <--> Dynamic <--> router[(i+1)%numAreas].pppg++ if numAreas > 1;
                    router[i].pppg++ <--> Dynamic <--> lan[i].pppg++;
                }
                router[0].pppg++ <--> pppg++;
                router[0].pppg++ <--> Dynamic <--> server.pppg++;
        }

    submodules:
        configurator: IPv4NetworkConfigurator {
            @display("p=37,31");
            config = xmldoc("config.xml");
        }
        router: Router {
            @display("p=115,103;is=s");
        }
        area[columns*rows]: Area;
    connections:
        //for i=0..rows-1, for j=0..columns-1 {
        //    router[i*columns+j].ethg++ <--> Eth100G <--> router[(i+1)*columns+j].ethg++ if i!=rows-1;
        //    router[i*columns+j].ethg++ <--> Eth100G <--> router[i*columns+j+1].ethg++ if j!=columns-1;
        //}
        
        for i=0..rows*columns-1 {
            router.pppg++ <--> Dynamic <--> area[i].pppg++;
        }
}
