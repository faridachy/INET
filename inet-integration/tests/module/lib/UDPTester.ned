import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import ned.DatarateChannel;
import inet.transport.udp.UDPNatRouter;


network UDPTester
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
                routingTable.routingFile = "../../lib/test_nat_peer1.mrt";
                @display("p=35,201;i=device/laptop");
            gates:
                pppg[1];
        }
        Server: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/test_nat_peer2.mrt";
                @display("p=496,218;i=device/server");
            gates:
                pppg[1];
        }
        
        Nat_1_1: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/test_nat_router.mrt";
                @display("p=249,94;i=block/join");
            gates:
                pppg[2];
        }
    connections:
        Peer_1.pppg[0] <--> testline <--> Nat_1_1.pppg[0];

        Nat_1_1.pppg[1] <--> testline <--> Server.pppg[0];
}

channel testline extends DatarateChannel
{
    datarate = 100 Mbps;
}
