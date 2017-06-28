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
parser.add_argument("-n", "--index", type=int, help="SNMP index of the interface to query",required=False)
parser.add_argument("-iH","--icingahost", help="Hostname of the icinga host", required=False)
parser.add_argument("-iu","--icingausername", help="Username for the icinga API", required=False)
parser.add_argument("-ip","--icingapassword", help="Password for the icinga API", required=False)

args = parser.parse_args()

ifName = args.interface
hostname = args.hostname
community = args.community
warningSpeed = args.warning
criticalSpeed = args.critical
ifIndex = args.index


try:
    s = easysnmp.Session(hostname=hostname,community=community, version=2)
except:
    print ("UNKNOWN - unable to establish SNMP connection to the host")
    sys.exit(3)


if (ifIndex is None) or (s.get(('ifName',ifIndex)).value != ifName):
    try:
        ifNames = s.walk('ifName')
    except:
        print ("UNKNOWN - unable to enumerate interface names")
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
    if (args.icingahost != None) and (args.icingausername != None) and (args.icingapassword != None):
        import requests
        import json
        try:
            request_result = requests.post('https://'+args.icingahost+':5665/v1/objects/services/',
                                           verify=False,
                                           auth=(args.icingausername,args.icingapassword),
                                           data=json.dumps(
                                               {'attrs': {'vars.interface_index':str(ifIndex)},
                                                'filter':'service.vars.interface=="{ifName}" && host.address=="{hostname}"'.format(ifName=ifName,hostname=hostname)}),
                                           headers={'accept':'application/json'}
            )
            request_result.raise_for_status()
        except:
            print ("UNKNOWN - error in updating icinga infos")
            sys.exit(3)
        
        
try:
    ifOperStatus = s.get(('ifOperStatus',ifIndex)).value
except:
    print ("UNKNOWN - unable to get interface {ifName} status".format(ifName=ifName))
    sys.exit(3)

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
    ifHighSpeed = int(s.get(('ifHighSpeed',ifIndex)).value)
except:
    print ("UNKNOWN - unable to get interface {ifName} speed".format(ifName=ifName))
    sys.exit(3)
    
statisticalData = {}
first_low = lambda s: s[:1].lower() + s[1:] if s else ''
for name in ["ifHCInOctets", "ifHCOutOctets", "ifHCInUcastPkts", "ifHCOutUcastPkts","ifHCInMulticastPkts","ifHCOutMulticastPkts","ifHCInBroadcastPkts","ifHCOutBroadcastPkts"]:
    try:
        statisticalData[first_low(name.lstrip('ifHC'))] = s.get((name,ifIndex)).value
    except:
        print ("UNKNOWN - unable to get interface {ifName} {variable}".format(ifName=ifName,variable=name))
        sys.exit(3)
statisticalData['speed'] = ifHighSpeed*1000*1000

if ifHighSpeed < criticalSpeed:
    returnState = "CRITICAL"
    exitCode = 2
elif ifHighSpeed < warningSpeed:
    returnState = "WARNING"
    exitCode = 1
else:
    returnState = "OK"
    exitCode = 0

statisticalDataString = ' '.join([ "{name}={value}".format(name=name,value=statisticalData[name]) for name in statisticalData.keys()])
    
print ("{returnState} - link {ifName} is up at {ifHighSpeed} Mbit/s| {statisticalData}".format(returnState=returnState, ifName=ifName,ifHighSpeed=ifHighSpeed,statisticalData=statisticalDataString))
sys.exit(exitCode)

