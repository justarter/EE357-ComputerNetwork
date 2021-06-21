#!/bin/sh
ovs-ofctl add-flow s2 "in_port=s2-eth3,actions=output:s2-eth2"
ovs-ofctl add-flow s3 "in_port=s3-eth3,actions=output:s3-eth2"
