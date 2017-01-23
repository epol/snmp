#!/usr/bin/env python2
# -*- coding: utf-8 -*-

# meru_AvailabilityStatus.py
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
import json

parser = argparse.ArgumentParser()
parser.add_argument("-m", "--meruhost", help="Hostname of the meru controller",required=True)
parser.add_argument("-C","-c", "--community", help="SNMP community of the meru controller",required=True)
parser.add_argument("-i","--index",help="Index of the AP in the SNMP table",required=True,type=int)

args = parser.parse_args()

snmp_session = easysnmp.Session(hostname=args.meruhost,community=args.community, version=2)

try:
    desc = snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.2',args.index)).value
    avstatus = int(snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.27',args.index)).value)
except:
    print("UNKNOWN - Unable to get SNMP infos. Is the host reachable and the community correct?")
    sys.exit(3)

if avstatus == 1:
    state = 2
    message = "availabilityStatus is Power off"
elif avstatus == 2:
    state = 2
    message = "availabilityStatus is Offline"
elif avstatus == 3:
    state = 0
    message = "availabilityStatus is Online"
elif avstatus == 4:
    state = 2
    message = "availabilityStatus is Failed"
elif avstatus == 5:
    state = 1
    message = "availabilityStatus is In Test"
elif avstatus == 6:
    state = 2
    message = "availabilityStatus is Not Installed"                                                        

    
if state == 0:
    statestr = "OK"
elif state == 1:
    statestr = "WARNING"
elif state == 2:
    statestr = "CRITICAL"
elif state == 3:
    statestr = "UNKNOWN"

print ("{statestr} - {desc} {message}".format(statestr=statestr,desc=desc,message=message))
sys.exit(state)


