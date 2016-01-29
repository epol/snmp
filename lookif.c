/*
 * lookif.c
 * This file is part of epol/snmp
 *
 * Copyright (C) 2015 - Enrico Polesel
 *
 * epol/snmp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * epol/snmp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with epol/snmp. If not, see <http://www.gnu.org/licenses/>
 * or write to the Free Software Foundation, Inc., 51 Franklin Street, 
 * Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "elib.h"


int main(int argc, char** argv)
{
  struct snmp_session *ss=NULL; /*ss: returned by the library */
  struct snmp_pdu *response=NULL; /* information from the host */

  oid anOID[MAX_OID_LEN]; 
  size_t anOID_len = MAX_OID_LEN;

  int status=-1;
  int exit_code=-1;
  int found = 0;
  char* agentaddress=NULL;
  char* community=NULL;
  char* ifName=NULL;
  int ifName_len = -1;
  int warn=-1, crit=-1;
  int ifIndex=1;
  long int ifStatus = 0;
  long int ifSpeed = 0;
  unsigned long int ifInOctets = 0;
  unsigned long int ifOutOctets = 0;
  unsigned long int ifInUcastPkts = 0;
  unsigned long int ifOutUcastPkts = 0;
  unsigned long int ifInMulticastPkts = 0;
  unsigned long int ifOutMulticastPkts = 0;
  unsigned long int ifInBroadcastPkts = 0;
  unsigned long int ifOutBroadcastPkts = 0;
  
  {
    int i = 1;
    for ( i = 1 ; i<argc ; i+=2)
      {
	if (argv[i][0] != '-')
	  {
	    printf("UNKNOWN - unknown parameter %s",argv[i]);
	    exit(3);
	    }
	switch (argv[i][1])
	  {
	  case 'H':
	    agentaddress = argv[i+1];
	    break;
	  case 'C':
	    community = argv[i+1];
	    break;
	  case 'i':
	    ifName = argv[i+1];
	    ifName_len = strlen(ifName);
	    break;
	  case 'w':
	    warn = atoi(argv[i+1]);
	    break;
	  case 'c':
	    crit = atoi(argv[i+1]);
	    break;
	  }
      }
    if ( !agentaddress || !community || !ifName )
      {
	printf("UNKNOWN - wrong parameters\n");
	exit(3);
      }
  }   

  status = elib_init(&ss,agentaddress, community);
  switch (status)
    {
    case 0:
      /* Connection opened */
      break;
    case 3:
      /* Unable to open connection: UNKNOWN */
      return 3;
      break;
    default:
      /* A library bug */
      printf ("Error in elib_init\n");
      return 3;
    }

  /* 
   * Look for the interface name
   */
  found = 0;
  while(!found)
    {
      get_node("IF-MIB::ifName", anOID, &anOID_len);
      anOID[anOID_len++] = ifIndex;
      
      status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GETNEXT  );
      
      switch(status)
	{
	case 0:
	  /* OK with the query */
	  if (response->variables->name[anOID_len-2]>1)
	    /* We are looking on the next table  */
	    {
	      printf("UNKNOWN - unable to find interface %s\n",ifName);
	      exit_code=3;
	      found=1;
	    }
	  else
	    {
	      /*      print_variable(vars->name, vars->name_length, vars);*/
	      ifIndex = response->variables->name[anOID_len-1];
	      if ( ( ifName_len == response->variables->val_len ) && (strncmp(response->variables->val.string, ifName, ifName_len) == 0 ))
		{
		  exit_code=0;
		  found = 1;
		}
	    }
	  break;	
	case 2:
	case 3:
	  printf("UNKNOWN - error while looking for the interface\n");
	  exit_code=3;
	  break;
	default:
	  printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	  exit_code=3;
	  break;
	}
    }
  if (exit_code==0)
    {
      get_node("IF-MIB::ifOperStatus", anOID, &anOID_len);
      anOID[anOID_len++] = ifIndex;
      
      status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
      
      switch(status)
	{
	case 0:
	  /* OK with the query */
	  ifStatus = 2 - *(response->variables->val.integer);
	  exit_code=0;
	  break;	
	case 2:
	case 3:
	  printf("UNKNOWN - error while checking link status\n");
	  exit_code=3;
	  break;
	default:
	  printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	  exit_code=3;
	  break;
	}
      
      if ( (exit_code==0) && ifStatus )
	{
	  get_node("IF-MIB::ifHighSpeed",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifSpeed = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link speed\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCInOctets",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifInOctets = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link input octets\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCOutOctets",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifOutOctets = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link output octets\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCInUcastPkts",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifInUcastPkts = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link input unicast packets\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCOutUcastPkts",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifOutUcastPkts = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link output unicast packtes\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCInMulticastPkts",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifInMulticastPkts = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link input multicast packets\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCOutMulticastPkts",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifOutMulticastPkts = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link output multicast packtes\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }
	  
	  get_node("IF-MIB::ifHCInBroadcastPkts",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifInBroadcastPkts = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link input broadcast packets\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }

	  get_node("IF-MIB::ifHCOutBroadcastPkts",anOID, &anOID_len);
	  anOID[anOID_len++] = ifIndex;
	  
	  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );
	  
	  switch(status)
	    {
	    case 0:
	      /* OK with the query */
	      ifOutBroadcastPkts = *(response->variables->val.integer);
	      exit_code=0;
	      break;	
	    case 2:
	    case 3:
	      printf("UNKNOWN - error while checking link output broadcast packtes\n");
	      exit_code=3;
	      break;
	    default:
	      printf("UNKNOWN - error in parsing status of elib_get_one_response\n");
	      exit_code=3;
	      break;
	    }
	}
      
      if ( exit_code==0 )
	{
	  if ( !ifStatus )
	    {
	      printf("CRITICAL - link %s is down\n",ifName);
	      exit_code=2;
	    }
	  else
	    {
	      if ( ifSpeed < crit)
		{
		  printf ("CRITICAL - link %s is up at %ld Mbit/s| inOctets=%lu outOctets=%lu inUcastPkts=%lu outUcastPkts=%lu inMulticastPkts=%lu outMulticastPkts=%lu inBroadcastPkts=%lu outBroadcastPkts=%lu\n",ifName,ifSpeed,ifInOctets,ifOutOctets,ifInUcastPkts,ifOutUcastPkts,ifInMulticastPkts,ifOutMulticastPkts,ifInBroadcastPkts,ifOutBroadcastPkts);
		  exit_code=2;
		}
	      else
		{
		  if ( ifSpeed < warn)
		    {
		      printf ("WARNING - link %s is up at %ld Mbit/s| inOctets=%lu outOctets=%lu inUcastPkts=%lu outUcastPkts=%lu inMulticastPkts=%lu outMulticastPkts=%lu inBroadcastPkts=%lu outBroadcastPkts=%lu\n",ifName,ifSpeed,ifInOctets,ifOutOctets,ifInUcastPkts,ifOutUcastPkts,ifInMulticastPkts,ifOutMulticastPkts,ifInBroadcastPkts,ifOutBroadcastPkts);		  
		      exit_code=1;
		    }
		  else
		    {
		      printf ("OK - link %s is up at %ld Mbit/s| inOctets=%lu outOctets=%lu inUcastPkts=%lu outUcastPkts=%lu inMulticastPkts=%lu outMulticastPkts=%lu inBroadcastPkts=%lu outBroadcastPkts=%lu\n",ifName,ifSpeed,ifInOctets,ifOutOctets,ifInUcastPkts,ifOutUcastPkts,ifInMulticastPkts,ifOutMulticastPkts,ifInBroadcastPkts,ifOutBroadcastPkts);		  
		      exit_code=0;
		    }
		}
	    }
	}
    }
  
  if (response)
    {
      snmp_free_pdu(response);
    }
  elib_close(&ss);
  return exit_code;
}
