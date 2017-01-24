#!/usr/bin/env python2
# -*- coding: utf-8 -*-

# merudiscovery.py
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
import requests

def APHW_tostring(hwint):
    if hwint == 0:
        return "apUnknownModel"
    elif hwint == 12:
        return "ap300"
    elif hwint == 13:
        return "ap310"
    elif hwint == 14:
        return "ap311"
    elif hwint == 15:
        return "ap320"
    elif hwint == 18:
        return "ap301"
    elif hwint == 19:
        return "ap302"
    elif hwint == 20:
        return "ap301i"
    elif hwint == 21:
        return "ap310i"
    elif hwint == 22:
        return "ap302i"
    elif hwint == 23:
        return "ap320i"
    elif hwint == 24:
        return "ap1010"
    elif hwint == 25:
        return "ap1020"
    elif hwint == 26:
        return "psm3x"
    elif hwint == 27:
        return "ap400"
    elif hwint == 28:
        return "ap433e"
    elif hwint == 29:
        return "ap433i"
    elif hwint == 30:
        return "ap432e"
    elif hwint == 31:
        return "ap432i"
    elif hwint == 32:
        return "ap1010e"
    elif hwint == 33:
        return "ap1020e"
    elif hwint == 34:
        return "ap433is"
    elif hwint == 35:
        return "ap433es"
    elif hwint == 36:
        return "oap432e"
    elif hwint == 37:
        return "oap433e"
    elif hwint == 38:
        return "oap433es"
    elif hwint == 39:
        return "ap110"
    elif hwint == 40:
        return "ap120"
    elif hwint == 41:
        return "ap1014i"
    elif hwint == 42:
        return "ap332e"
    elif hwint == 43:
        return "ap332i"
    elif hwint == 44:
        return "ap832e"
    elif hwint == 45:
        return "ap832i"
    elif hwint == 46:
        return "ap822e"
    elif hwint == 47:
        return "ap822i"
    elif hwint == 48:
        return "ap122"
    elif hwint == 49:
        return "oap832e"
    else:
        return "Code not found in meru MIB"


parser = argparse.ArgumentParser()
parser.add_argument("-m", "--meruhost", help="Hostname of the meru controller",required=True)
parser.add_argument("-C","-c", "--community", help="SNMP community of the meru controller",required=True)
parser.add_argument("-i","--icingahost", help="Hostname of the icinga host", required=True)
parser.add_argument("-u","--icingausername", help="Username for the icinga API", required=True)
parser.add_argument("-p","--icingapassword", help="Password for the icinga API", required=True)

args = parser.parse_args()

try:
    snmp_session = easysnmp.Session(hostname=args.meruhost,community=args.community, version=2)
except:
    print ("Unable to open SNMP connection, is the hostname of the merucontroller correct?")
    sys.exit(1)
icinga_session = requests.Session()
icinga_session.verify = False
icinga_session.auth = (args.icingausername,args.icingapassword)

icinga_url = 'https://'+args.icingahost+':5665/v1/'

try:
    APs_desc = snmp_session.walk('.1.3.6.1.4.1.15983.1.1.4.2.1.1.2')
except:
    print("Unable to enumerate APs, is the host reachable and the community correct?")
    sys.exit(1)

r = icinga_session.post(icinga_url+'objects/hosts',
           headers={'X-HTTP-Method-Override':'GET'},
           data=json.dumps({'attrs': ['vars'],
                            'filter': '"meru-AP" in host.templates'}))
r.raise_for_status()

old_APS = r.json()['results']
# old_ids: the IDs of the APs already present in icinga
old_ids = set([ int(h['attrs']['vars']['id']) for h in old_APS ])
# conv: conver ap id in icinga host object name
conv = { int(h['attrs']['vars']['id']) : str(h['name'])  for h in old_APS }
# new_ids: the IDs of the AP found in the search
new_ids = set()

updated = 0
added = 0
deleted = 0

for AP in APs_desc:
    index = int(AP.oid.split('.')[-1])
    description = AP.value
    id = int(snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.4',index)).value)
    contact = snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.6',index)).value
    location = snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.8',index)).value
    floor = snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.3',index)).value
    building = snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.9',index)).value
    serial = snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.13',index)).value
    hardware_int = int(snmp_session.get(('.1.3.6.1.4.1.15983.1.1.4.2.1.1.19',index)).value)
    hardware = APHW_tostring(hardware_int)
    new_ids.add(id)
    
    icinga_host = {'templates': ['meru-AP'],
                   'attrs': {'display_name': description,
                             'vars.id': str(id),
                             'vars.index': str(index),
                             'vars.contact': contact,
                             'vars.location': location,
                             'vars.floor': floor,
                             'vars.building': building,
                             'vars.serial': serial,
                             'vars.hwtype': hardware
                   }}
    if id in old_ids:
        # object already present, updating info...
        r = icinga_session.post(icinga_url+'objects/hosts/'+conv[id],
                               data=json.dumps(icinga_host),
                               headers={'Accept':'application/json'})
        r.raise_for_status()
        updated += 1
    else:
        # new object
        conv[id]='AP-'+str(id)
        r = icinga_session.put(icinga_url+'objects/hosts/'+conv[id],
                               data=json.dumps(icinga_host),
                               headers={'Accept':'application/json'})
        r.raise_for_status()
        added += 1

# deleted_ids: the IDs of the APs that no longer are in the controller
deleted_ids = old_ids - new_ids
for id in deleted_ids:
    r = icinga_session.delete(icinga_url+'objects/hosts/'+conv[id],
                              params = {'cascade':1},
                              headers={'Accept': 'application/json'})
    r.raise_for_status()
    deleted += 1

print ("Finish. Added: {added}, updated: {updated}, deleted: {deleted}".format(added=str(added),updated=str(updated),deleted=str(deleted)))

            

    



    




                                                                                                                                                                                                                                                                                                                                                                                      
