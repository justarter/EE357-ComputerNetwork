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
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        h4 = self.addHost('h4')
        self.addLink(s1,s2)
        self.addLink(s1,s3)
        self.addLink(s1, h4)
        self.addLink(s1, h1)
        self.addLink(s3, h3)
        self.addLink(s2, h2)

def simpleTest():
    "Create and test a simple network"
    topo = MultiSwitchTopo()
    net = Mininet(topo)
    net.start()
    CLI(net)
    net.stop()


if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    simpleTest()
