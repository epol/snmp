#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# getupscharge.py
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
parser.add_argument("-H", "--hostname", help="hostname", required=True)
parser.add_argument("-C", "--community", help="SNMP community", required=True)
parser.add_argument("-w", "--warning", help="Warning threshold (percent)", required=False, type=int)
parser.add_argument("-c", "--critical", help="Critical threshold (percent)", required=False, type=int)

args = parser.parse_args()

hostname = args.hostname
community = args.community

try:
    s = easysnmp.Session(hostname=hostname,community=community, version=2)
except:
    print ("UNKNOWN - unable to establish SNMP connection to the host")
    sys.exit(3)

try:
    chargevalue = int(s.get('1.3.6.1.2.1.33.1.2.4.0').value) # upsEstimatedChargeRemaining
except:
    print ("UNKNOWN - unable to get upsEstimatedChargeRemaining variable")
    sys.exit(3)

exitstatus = "OK"
exitcode = 0

if args.critical and chargevalue < args.critical:
    exitstatus = "CRITICAL"
    exitcode = 2
elif args.warning and chargevalue < args.warning:
    exitstatus = "WARNING"
    exitcode = 1

print ("{exitstatus} - Estimated battery charge is {chargevalue}%| estimatedChargeRemaining={chargevalue}".format(exitstatus=exitstatus,chargevalue=chargevalue))
sys.exit(exitcode)
