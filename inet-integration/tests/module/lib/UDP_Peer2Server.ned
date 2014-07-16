import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import ned.DatarateChannel;
import inet.transport.udp.UDPNatRouter;


network UDP_Peer2Server
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
                routingTable.routingFile = "../../lib/P2S_Peer_1.mrt";
                @display("p=35,201;i=device/laptop");
            gates:
                pppg[1];
        }
        Server: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/P2S_Server.mrt";
                @display("p=504,156;i=device/server");
            gates:
                pppg[1];
        }

        Nat_1_1: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/P2S_Nat_1_1.mrt";
                @display("p=179,83;i=block/join");
            gates:
                pppg[2];
        }

        Nat_1_2: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/P2S_Nat_1_2.mrt";
                @display("p=179,238;i=block/join");
            gates:
                pppg[1];
        }

        Router_1: Router {
            parameters:
                routingTable.routingFile = "../../lib/P2S_R_1.mrt";
                @display("p=337,83;i=abstract/router");
            gates:
                pppg[3];
        }

    connections:
        Peer_1.pppg[0] <--> natline <--> Nat_1_1.pppg[0];

        Nat_1_1.pppg[1] <--> natline <--> Router_1.pppg[0];

        Router_1.pppg[1] <--> natline <--> Nat_1_2.pppg[0];
        Router_1.pppg[2] <--> natline <--> Server.pppg[0];


}

channel natline extends DatarateChannel
{
    datarate = 100 Mbps;
}
