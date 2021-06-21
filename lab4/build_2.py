#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.cli import CLI

class MultiSwitchTopo(Topo):
    "Multi switches connected to hosts."

    def build(self):
        s2 = self.addSwitch('s2')
        h3 = self.addHost('h3')
        h4 = self.addHost('h4')
        self.addLink(s2, h3,bw=10,loss=0,delay='5ms')
        self.addLink(s2, h4,bw=10,loss=0,delay='5ms')

def simpleTest():
    "Create and test a simple network"
    topo = MultiSwitchTopo()
    net = Mininet(topo, link=TCLink)
    net.start()
    CLI(net)
    net.stop()


if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    simpleTest()
