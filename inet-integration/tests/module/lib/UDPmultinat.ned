import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import ned.DatarateChannel;
import inet.transport.udp.UDPNatRouter;


network UDPmultinat
{
    parameters:
        volatile double testTimeout;
        bool testing;
        **.networkLayer.configurator.networkConfiguratorModule = "";
        @display("bgb=607,318");
    submodules:
        Peer_1: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/udp_nat_peer1.mrt";
                @display("p=35,201;i=device/laptop");
            gates:
                pppg[1];
        }
        Peer_2: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/udp_nat_peer2.mrt";
                @display("p=496,218;i=device/laptop");
            gates:
                pppg[1];
        }
        Nat_1_1: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/udp_nat1_1.mrt";
                @display("p=179,83;i=block/join");
            gates:
                pppg[2];
        }

        Nat_1_2: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/udp_nat1_2.mrt";
                @display("p=426,83;i=block/join");
            gates:
                pppg[2];
        }

    connections:
        Peer_1.pppg[0] <--> natline <--> Nat_1_1.pppg[0];

        Nat_1_1.pppg[1] <--> natline <--> Nat_1_2.pppg[0];

        Nat_1_2.pppg[1] <--> natline <--> Peer_2.pppg[0];

}

channel natline extends DatarateChannel
{
    datarate = 100 Mbps;
}
