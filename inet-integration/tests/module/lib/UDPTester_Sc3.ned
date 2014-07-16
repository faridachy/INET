import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import ned.DatarateChannel;
import inet.transport.udp.UDPNatRouter;


network UDPTester_Sc3
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
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_peer1.mrt";
                @display("p=71,279;i=device/laptop");
            gates:
                pppg[1];
        }

        Peer_2: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_peer2.mrt";
                @display("p=311,279;i=device/laptop");
            gates:
                pppg[1];
        }

        Server1: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_server1.mrt";
                @display("p=206,23;i=device/server");
            gates:
                pppg[1];
        }

        Server2: StandardHost {
            parameters:
                IPForward = false;
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_server2.mrt";
                @display("p=441,23;i=device/server");
            gates:
                pppg[1];
        }

        Router1: Router {
            parameters:
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_router1.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
                @display("p=206,131;i=abstract/router");
            gates:
                pppg[4];
        }

        Router2: Router {
            parameters:
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_router2.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
                @display("p=441,131;i=abstract/router");
            gates:
                pppg[3];
        }

        NAT_1: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_nat_router1.mrt";
                @display("p=71,166;i=block/join");
            gates:
                pppg[2];
        }

        NAT_2: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_nat_router2.mrt";
                @display("p=311,183;i=block/join");
            gates:
                pppg[2];
        }

        NAT_3: UDPNatRouter {
            parameters:
                routingTable.routingFile = "../../lib/udpTester-sc3/UDPtest_sc3_nat_router3.mrt";
                @display("p=566,183;i=block/join");
            gates:
                pppg[1];
        }

    connections:
        Peer_1.pppg[0] <--> testline1 <--> NAT_1.pppg[0];
        NAT_1.pppg[1] <--> testline1 <--> Router1.pppg[0];
        NAT_2.pppg[0] <--> testline1 <--> Peer_2.pppg[0];
        NAT_2.pppg[1] <--> testline1 <--> Router1.pppg[1];
        Router1.pppg[2] <--> testline1 <--> Server1.pppg[0];
        Router1.pppg[3] <--> testline1 <--> Router2.pppg[2];
        Router2.pppg[0] <--> testline1 <--> NAT_3.pppg[0];
        Router2.pppg[1] <--> testline1 <--> Server2.pppg[0];

}
channel testline1 extends DatarateChannel
{
    datarate = 100 Mbps;
}
