#!/usr/bin/env python2
# -*- coding: utf-8 -*-

# autovelox.py
# This file is part of epol/snmp
#
# Copyright (C) 2017 - Enrico Polesel
#
#
# This software is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software.  If not, see
# <https://www.gnu.org/licenses/>.

import easysnmp
import argparse
import sys
import datetime
import json
import time
import pprint
import terminaltables
import termcolor

parser = argparse.ArgumentParser()
parser.add_argument("-H", "--hostname", help="hostname",required=True)
parser.add_argument("-C", "--community", help="SNMP community",required=True)
parser.add_argument("-l", "--limit", help="Speed limit",type=int,required=False)

args = parser.parse_args()

hostname = args.hostname
community = args.community
limit = args.limit


try:
    s = easysnmp.Session(hostname=hostname,community=community, version=2)
except:
    print ("UNKNOWN - unable to establish SNMP connection to the host")
    sys.exit(3)

ifNames = s.walk('ifName')
ifIndexes = [ entry.oid_index  for entry in ifNames ]

data = {}

ifaces = []
for entry in ifNames:
    ifIndex = entry.oid_index
    ifName = entry.value
    ifOperStatus = s.get(('ifOperStatus',ifIndex)).value
    if ifOperStatus !='1':
       continue
    ifaces.append((ifName,ifIndex))
    data[ifName] = {}
    data[ifName]['points'] = []
    ifHighSpeed = int(s.get(('ifHighSpeed',ifIndex)).value)
    data[ifName]['speed'] = ifHighSpeed

nsample = 6
for i in range(nsample):
    for (ifName,ifIndex) in ifaces:
        ifOperStatus = s.get(('ifOperStatus',ifIndex)).value
        if ifOperStatus !='1':
            continue
#       print (ifName + " is UP")
        ifHCInOctets = int(s.get(('ifHCInOctets',ifIndex)).value)
        ifHCOutOctets = int(s.get(('ifHCOutOctets',ifIndex)).value)
        datenow = datetime.datetime.now()
        if len(data[ifName]['points']) >0:
            lastpoint = data[ifName]['points'][-1]
            deltat = datenow - lastpoint['date']
            deltain = ifHCInOctets - lastpoint['inoctets']
            deltaout = ifHCOutOctets - lastpoint['outoctets']
            speedin = deltain/deltat.total_seconds()/1000
            speedout = deltaout/deltat.total_seconds()/1000
        else:
            speedin = 0
            speedout = 0
        data[ifName]['points'].append({
            'date': datenow,
            'inoctets': ifHCInOctets,
            'outoctets': ifHCOutOctets,
            'speedin': speedin,
            'speedout': speedout
        })
#   pprint.pprint(data)
    time.sleep(4)

pprint.pprint(data)

velox = {}
rank = []
bad = set()
for iface in data:
    maxin = max([ point['speedin'] for point in data[iface]['points'] ])
    maxout = max([ point['speedout'] for point in data[iface]['points'] ])
    maxspeed = max(maxin,maxout)
    velox[iface] = maxspeed
    if limit is not None:
        if maxspeed > limit:
            bad.add(iface)
    rank.append(iface)

rank.sort(key=lambda iface: velox[iface])
rank.reverse()

color_data = lambda iface,data: termcolor.colored(data,'red') if iface in bad else data

maxtable = terminaltables.AsciiTable([ [ 'Name', 'Max measured speed (Kbit/s)' ] ] +
                                     [
                                         [ color_data(iface,iface), color_data(iface,str(velox[iface])) ]
                                         for iface in rank ])
maxtable.title = 'maxspeed'
print (maxtable.table)

if limit is None:
    color_large = lambda value: str(value)
else:
    color_large = lambda value: termcolor.colored(str(value),'yellow') if value > limit else str(value)

pointstable = terminaltables.AsciiTable ( [ [ 'Name', 'Speed (Mbit/s)' ]+ (nsample-1)*[ '[IN, OUT] (Kbit/s)' ] ] +
                                          [
                                            [ iface , data[iface]['speed'] ] +
                                            [ '['+color_large(round(point['speedin'],3)) +', ' + color_large(round(point['speedout'],3)) + ']' for point in data[iface]['points'][1:] ]
                                            for iface in rank ])
pointstable.title = 'points'
print(pointstable.table)
