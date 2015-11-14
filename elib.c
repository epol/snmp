/*
 * elib.c
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

#include "elib.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


int elib_init(
	      struct snmp_session** ssp,
	      char* agentaddress,
	      char* community
	      )
{
  struct snmp_session session; /* session: our target */ 

  /*
   * Initialize the SNMP library
   */
  init_snmp("snmpapp");

  /*
   * Initialize a "session" that defines who we're going to talk to
   */
  snmp_sess_init( &session );                   /* set up defaults */
  session.peername = agentaddress;

  /* set the SNMP version number */
  session.version = SNMP_VERSION_2c;

  /* set the SNMPv1 community name used for authentication */
  session.community = community;
  session.community_len = strlen(session.community);

  /*
   * Open the session
   */
  *ssp = snmp_open(&session);                     /* establish the session */

  if (!ssp)
    {
      printf("UNKNOWN - unable to connect to %s\n",session.peername);
      return 3;
    }
  else
    {
      return 0;
    }
}


int elib_get_one_response(
			   struct snmp_session *ss, /*ss: session to be used */
			   struct snmp_pdu **responsep,
			   oid anOID[],  /* OID to be asked */
			   size_t anOID_len,
			   int msg_type
			  )
{
  struct snmp_pdu *pdu; /* information to send to the host */
  int status;
  int exit_code = -1;
  /*
   * Create the PDU for the data for our request.
   */
  pdu = snmp_pdu_create(msg_type);
  snmp_add_null_var(pdu, anOID, anOID_len);
  
  /*
   * Send the Request out.
   */
  status = snmp_synch_response(ss, pdu, responsep);
  
  
  /*
   * Process the response.
   */
  if (status == STAT_SUCCESS && (*responsep)->errstat == SNMP_ERR_NOERROR)
    {
      /*
       * SUCCESS: Print the result variables
       */
      if (*responsep)
	{
	  if ((*responsep)->variables )
	    {
	      if ((*responsep)->variables->val_len)
		{
		  exit_code = 0;
		}
	      else
		{
		  printf("UNKNOWN - variable length is zero\n");
		  exit_code = 3;
		}
	    }
	  else
	    {
	      printf("UNKNOWN - error in getting variables\n");
	      exit_code = 3;
	    }
	}
      else
	{
 	  printf("UNKNOWN - error in getting variables\n");
	  exit_code = 3;
	}
    }
  else
    {
      /*
       * FAILURE: print what went wrong!
       */
      
      if (status == STAT_SUCCESS)
	{
	  if ((*responsep)->errstat == SNMP_ERR_NOSUCHNAME)
	    {
	      /* printf("CRITICAL - variable not found\n"); */
	      exit_code = 2;
	    }
	  else
	    {
	      printf("UNKNOWN - error in packet. Reason: %s\n",	snmp_errstring((*responsep)->errstat));
	      exit_code = 3;
	    }
	}
      else
	{
	  printf ("UNKNOWN - error in getting values\n");
	  exit_code = 3;
	}
    }
  /*  if (pdu)
    {
      snmp_free_pdu(pdu);   
      } */
  return exit_code;
}

int elib_close(
	       struct snmp_session** ssp
	       )
{
  snmp_close(*ssp);
  return 0;
}
