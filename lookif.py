#!/usr/bin/env python2
# -*- coding: utf-8 -*-

# lookif.py
# This file is part of epol/snmp
#
# Copyright (C) 2017 - Enrico Polesel
#
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import easysnmp
import argparse
import sys

parser = argparse.ArgumentParser()
parser.add_argument("-H", "--hostname", help="hostname",required=True)
parser.add_argument("-C", "--community", help="SNMP community",required=True)
parser.add_argument("-i", "--interface", help="interface name",required=True)
parser.add_argument("-w", "--warning", type=int, help="Warning limit for the interface speed",default=0)
parser.add_argument("-c", "--critical", type=int, help="Critical limit for the interface speed",default=0)

args = parser.parse_args()

ifName = args.interface
hostname = args.hostname
community = args.community
warningSpeed = args.warning
criticalSpeed = args.critical

s = easysnmp.Session(hostname=hostname,community=community, version=2)
try:
    ifNames = s.walk('ifName')
except:
    print("UNKNOWN - unable to enumerate interfaces, is the host reachable and the community correct?")
    sys.exit(3)

ifIndexes = [ entry.oid_index  for entry in ifNames if entry.value==ifName ]

if (len(ifIndexes)==0):
    print ("UNKNOWN - unable to find interface {ifName}".format(ifName=ifName))
    sys.exit(3)
elif (len(ifIndexes)>1):
    print ("UNKNOWN - more than one interface named {ifName}".format(ifName=ifName))
    sys.exit(3)
else:
    ifIndex = ifIndexes[0]

try:
    ifOperStatus = s.get(('ifOperStatus',ifIndex)).value
except:
    print ("UNKNOWN - unable to get interface {ifName} status".format(ifName=ifName))

if ifOperStatus == '1':
    pass
elif ifOperStatus == '2':
    print ("CRITICAL - link {ifName} is down".format(ifName=ifName))
    sys.exit(2)
elif ifOperStatus == '3':
    print ("CRITICAL - link {ifName} is testing".format(ifName=ifName))
    sys.exit(2)
else:
    print ("UNKNOWN - link {ifName} in unknown state {ifOperStatus}".format(ifName=ifName,ifOperStatus=ifOperStatus))
    sys.exit(3)

try:
    ifHCInOctets = s.get(('ifHCInOctets',ifIndex)).value
except:
    print ("UNKNOWN - unable to get interface {ifName} IN octects".format(ifName=ifName))
    sys.exit(3)

try:
    ifHCOutOctets = s.get(('ifHCOutOctets',ifIndex)).value
except:
    print ("UNKNOWN - unable to get interface {ifName} OUT octects".format(ifName=ifName))
    sys.exit(3)


try:
    ifHighSpeed = int(s.get(('ifHighSpeed',ifIndex)).value)
except:
    print ("UNKNOWN - unable to get interface {ifName} speed".format(ifName=ifName))
    sys.exit(3)


if ifHighSpeed < criticalSpeed:
    returnState = "CRITICAL"
    exitCode = 2
elif ifHighSpeed < warningSpeed:
    returnState = "WARNING"
    exitCode = 1
else:
    returnState = "OK"
    exitCode = 0

print ("{returnState} - link {ifName} is up at {ifHighSpeed} Mbit/s| inOctets={ifHCInOctets} outOctets={ifHCOutOctets}".format(returnState=returnState, ifName=ifName,ifHighSpeed=ifHighSpeed,ifHCInOctets = ifHCInOctets,ifHCOutOctets = ifHCOutOctets))
sys.exit(exitCode)

