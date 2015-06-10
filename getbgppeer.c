/*
 * getbgppeer.c
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
  char* peer_ip=NULL;
  int exit_code = 0;
  char* agentaddress=NULL;
  char* community=NULL;

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
	  case 'p':
	    peer_ip = argv[i+1];
	    break;
	  }
      }
    if ( !agentaddress || !community || !peer_ip )
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

  {
    /* Create the request */

    /* Create a string with bgpPeerState.peer_ip */
    char* requested_oid = malloc(100*sizeof(char));
    sprintf(requested_oid, "1.3.6.1.2.1.15.3.1.2.%s", peer_ip);
    
    /* query for bgpPeerState.peer_ip */
    read_objid(requested_oid, anOID, &anOID_len);
    free(requested_oid);
  }

  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );

  switch (status)
    {
    case 0:
      /* OK with the query */
      /* TODO: check that it really is a integer */
      switch (*(response->variables->val.integer))
	{
	case 1:
	  printf("CRITICAL - peer connection %s is idle\n",peer_ip);
	  exit_code = 2;
	  break;
	case 2:
	  printf("CRITICAL - peer connection %s is connect\n",peer_ip);
	  exit_code = 2;
	  break;
	case 3:
	  printf("CRITICAL - peer connection %s is active\n",peer_ip);
	  exit_code = 2;
	  break;
	case 4:
	  printf("CRITICAL - peer connection %s is opensent\n",peer_ip);
	  exit_code = 2;
	  break;
	case 5:
	  printf("CRITICAL - peer connection %s is openconfirm\n",peer_ip);
	  exit_code = 2;
	  break;
	case 6:
	  printf("OK - peer connection %s is established\n",peer_ip);
	  exit_code = 0;
	  break;
	default:
	  printf("UNKNOWN - unknow bgpPeerState integer %ld for peer %s\n", *(response->variables->val.integer), peer_ip);
	  exit_code = 3;
	  break;
	}
      break;
    case 2:
      /* Variable not found, so the peer isn't in the table */
      printf("UNKNOWN - peer %s not found in table\n",peer_ip);
      exit_code = 3;
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
