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
        s1 = self.addSwitch('s1')
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        self.addLink(s1, h1,bw=10,loss=0,delay='5ms')
        self.addLink(s1, h2,bw=10,loss=0,delay='5ms')

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
