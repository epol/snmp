#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# getupsbatstat.py
# This file is part of epol/snmp
#
# Copyright (C) 2019 - Enrico Polesel
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

parser = argparse.ArgumentParser()
parser.add_argument("-H", "--hostname", help="hostname",required=True)
parser.add_argument("-C", "--community", help="SNMP community",required=True)

args = parser.parse_args()

hostname = args.hostname
community = args.community

try:
    s = easysnmp.Session(hostname=hostname,community=community, version=2)
except:
    print ("UNKNOWN - unable to establish SNMP connection to the host")
    sys.exit(3)

try:
    batvalue = int(s.get('1.3.6.1.2.1.33.1.2.1.0').value) # upsBatteryStatus.0
except:
    print ("UNKNOWN - unable to get upsBatteryStatus variable")
    sys.exit(3)

if batvalue == 1: # unkown
    print ("UNKNOWN - battery is in unknown state")
    sys.exit(3)
elif batvalue == 2: # batteryNormal
    print ("OK - battery is in normal state")
    sys.exit(0)
elif batvalue == 3: # batteryLow
    print ("WARNING - battery is in low state")
    sys.exit(1)
elif batvalue == 4: # batteryDeplated
    print ("CRITICAL - battery is in depleted state")
    sys.exit(2)
