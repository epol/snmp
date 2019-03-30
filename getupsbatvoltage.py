#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# getupsbatvoltage.py
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
parser.add_argument("-w", "--warning", help="Warning threshold pair (dV) e.g. 700,900", required=False, type=str)
parser.add_argument("-c", "--critical", help="Critical threshold pair (dV)", required=False, type=str)

args = parser.parse_args()

hostname = args.hostname
community = args.community

if args.critical:
    try:
        ( criticalmin, criticalmax ) = ( int(v) for v in args.critical.split(',') )
    except:
        print ("Wrong argument syntax, please provide threshold in the form int,int")
        sys.exit(-1)
if args.warning:
    try:
        ( warningmin, warningmax ) = ( int(v) for v in args.warning.split(',') )
    except:
        print ("Wrong argument syntax, please provide threshold in the form int,int")
        sys.exit(-1)

try:
    s = easysnmp.Session(hostname=hostname,community=community, version=2)
except:
    print ("UNKNOWN - unable to establish SNMP connection to the host")
    sys.exit(3)

try:
    voltvalue = int(s.get('1.3.6.1.2.1.33.1.2.5.0').value) # upsBatteryVoltage
except:
    print ("UNKNOWN - unable to get upsBatteryVoltage variable")
    sys.exit(3)

exitstatus = "OK"
exitcode = 0

if args.critical:
    if voltvalue < criticalmin or voltvalue > criticalmax:
        exitstatus = "CRITICAL"
        exitcode = 2
if args.warning:
    if voltvalue < warningmin or voltvalue > warningmax:
        exitstatus = "WARNING"
        exitcode = 1

print ("{exitstatus} - Battery voltage is {voltvalue2}V| batteryVoltage={voltvalue}".format(exitstatus=exitstatus, voltvalue=voltvalue, voltvalue2 = voltvalue/10))
sys.exit(exitcode)
