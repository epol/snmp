/*
 * gettempn.c
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

#include <stdlib.h>
#include <stdio.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "elib.h"

int main(int argc, char** argv)
{
  struct snmp_session *ss; /*ss: returned by the library */
  
  oid anOID[MAX_OID_LEN]; 
  size_t anOID_len = MAX_OID_LEN;

  struct snmp_pdu *response=NULL;
  int status = -1;
  int member=-1;
  int exit_code = 0;
  char* agentaddress=NULL;
  char* community=NULL;
  int temperature=-1;
  int warning=0x10000000;
  int critical=0x10000000;

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
	  case 'n':
	    member = atoi(argv[i+1]);
	    break;
	  case 'w':
	    warning = atoi(argv[i+1]);
	    break;
	  case 'c':
	    critical = atoi(argv[i+1]);
	    break;
	  }
      }
    if ( !agentaddress || !community || (member==-1) )
      {
	printf("UNKNOWN - wrong parameters\n");
	exit(3);
      }
  }   

  status = elib_init(&ss, agentaddress, community );
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
  
  /* Create the request */

  /* query for jnxOperatingTemp.7 */
  read_objid("1.3.6.1.4.1.2636.3.1.13.1.7.7", anOID, &anOID_len);
  /* jnxOperatingTemp.7.member.0.0 */
  anOID[anOID_len++] = member+1;  /* In this MIB they count from 0 */
  anOID[anOID_len++] = 0;
  anOID[anOID_len++] = 0;

  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );

  switch (status)
    {
    case 0:
      /* OK with the query */
      /* TODO: check that it really is a integer */
      temperature = *(response->variables->val.integer);
      if (temperature == 0)
	{
	  printf("UNKNOWN - unable to get temperature for member %d\n",member);
	  exit_code = 3;
	  break;
	}
      if (temperature > critical)
	{
	  printf("CRITICAL - member %d is at %d degree |temperature=%dC;%d;%d;%d\n",member,temperature,temperature,warning,critical,0);
	  exit_code = 2;
	  break;
	}
      if (temperature > warning)
	{
	  printf("WARNING - member %d is at %d degree |temperature=%dC;%d;%d;%d\n",member,temperature,temperature,warning,critical,0);
	  exit_code = 1;
	  break;
	}
      printf("OK - member %d is at %d degree |temperature=%dC;%d;%d;%d\n",member,temperature,temperature,warning,critical,0);
      exit_code = 0;
      break;
    case 2:
      /* variable not found, so the member is not present */
      printf("UNKNOWN - member %d not found\n",member);
      exit_code = 2;
      break;
    case 3:
      exit_code = 3;
      break;
    default:
      /* This shouldn't happen, is a bug! */
      printf("UNKNOWN - error in parsing the status of elib_get_one_response\n");
      exit_code=3;
      break;
    }
  if (response)
    {
      snmp_free_pdu(response);
    }

  status = elib_close(&ss);
  return exit_code;
}
