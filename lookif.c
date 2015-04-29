#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

void get_response(
		  struct snmp_session *ss, /*ss: returned by the library */
		  struct snmp_pdu **response, /* information from the host */
		  oid anOID[],
		  size_t anOID_len
		  )
{
  struct snmp_pdu *pdu; /* information to send to the host */
  int status;
  /*
   * Create the PDU for the data for our request.
   *   1) We're going to GET the system.sysDescr.0 node.
   */
  pdu = snmp_pdu_create(SNMP_MSG_GET);
  snmp_add_null_var(pdu, anOID, anOID_len);
  
  /*
   * Send the Request out.
   */
  status = snmp_synch_response(ss, pdu, response);
  
  
  /*
   * Process the response.
   */
  if (status == STAT_SUCCESS && (*response)->errstat == SNMP_ERR_NOERROR) {
    /*
     * SUCCESS: Print the result variables
     */
    if (*response)
      {
	return;
      }
    else
      {
	printf("UNKNOWN - error in getting variables\n");
	exit(3);
      }
    } else {
    /*
     * FAILURE: print what went wrong!
     */
    
      if (status == STAT_SUCCESS)
	{
	  if ((*response)->errstat == SNMP_ERR_NOSUCHNAME)
	    {
	      printf("CRITICAL - variable not found\n");
	      exit(2);
	    }
	  else
	    {
	      printf("UNKNOWN - error in packet. Reason: %s\n",	snmp_errstring((*response)->errstat));
	      exit(3);
	    }
	}
      else
	{
	  printf ("UNKNOWN - error in getting values\n");
	  exit(3);
	}
  }
}


int main(int argc, char** argv)
{
  struct snmp_session session; /* session: our target */ 
  struct snmp_session *ss; /*ss: returned by the library */
  struct snmp_pdu *pdu; /* information to send to the host */
  struct snmp_pdu *response; /* information from the host */

  oid anOID[MAX_OID_LEN]; 
  size_t anOID_len = MAX_OID_LEN;

  struct variable_list *vars; /* variables in the response */
  int status;
  char* agentaddress=NULL;
  char* community=NULL;
  char* ifName=NULL;
  int ifName_len = -1;
  int warn=-1, crit=-1;
  int ifIndex=1;
  long int ifStatus = 0;
  long int ifSpeed = 0;
  
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
  ss = snmp_open(&session);                     /* establish the session */

  if (!ss) {
    printf("UNKNOWN - unable to connect to %s\n",session.peername);
    exit(3);
  }

  /* 
   * Look for the interface name
   */
  while(1)
  {
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);

    get_node("IF-MIB::ifName", anOID, &anOID_len);
      //    read_objid("1.3.6.1.4.1.2636.3.40.1.4.1.1.1.3", anOID, &anOID_len);
    anOID[anOID_len++] = ifIndex;
    snmp_add_null_var(pdu, anOID, anOID_len);
    
    /*
     * Send the Request out.
     */
    status = snmp_synch_response(ss, pdu, &response);
    
    
    /*
     * Process the response.
     */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
      /*
       * SUCCESS: Print the result variables
       */
      if (response)
	{
	  vars = response->variables;
	  if (vars->name[anOID_len-2]>1)
	    {
	      printf("UNKNOWN - unable to find interface %s\n",ifName);
	      exit(3);
	    }
	  else
	    {
	      //	      print_variable(vars->name, vars->name_length, vars);
	      //	      printf("%d %s\n", (vars->name)[anOID_len-1], vars->val.string);
	      ifIndex = vars->name[anOID_len-1];
	      if ( ( ifName_len == vars->val_len ) && (strncmp(vars->val.string, ifName, ifName_len) == 0 ))
		{
		  break;
		}
	    }
	}
      else
	{
	  printf("UNKNOWN - Error in getting variables\n");
	  exit(3);
	}
    } else {
      /*
       * FAILURE: print what went wrong!
       */
      
      if (status == STAT_SUCCESS)
	{
	  printf("UNKNOWN - Error in packet. Reason: %s\n",
		snmp_errstring(response->errstat));
	  exit(3);
	}
      else
	{
	  printf("UNKNOWN - ");
	  snmp_sess_perror("snmpget", ss);
	  exit(3);
	}
    }
  }
  //  printf("Index found: %d\n",ifIndex);

  get_node("IF-MIB::ifOperStatus", anOID, &anOID_len);
  anOID[anOID_len++] = ifIndex;
  
  get_response(ss,&response,anOID,anOID_len);
  vars = response->variables;
  //  printf("%d\n",*(vars->val.integer));
  ifStatus = 2 - *(vars->val.integer);

  if ( ifStatus )
    {
      get_node("IF-MIB::ifHighSpeed",anOID, &anOID_len);
      anOID[anOID_len++] = ifIndex;

      get_response(ss,&response,anOID,anOID_len);
      vars = response->variables;
      //      printf("%d\n",*(vars->val.integer));
      ifSpeed = *(vars->val.integer);
    }

  if ( !ifStatus )
    {
      printf("CRITICAL - link %s is down\n",ifName);
      exit(2);
    }
  else
    {
      if ( ifSpeed < crit)
	{
	  printf("CRITICAL - link %s is up at %ld Mbit/s\n",ifName,ifSpeed);
	  exit(2);
	}
      if ( ifSpeed < warn)
	{
	  printf("WARNING - link %s is up at %ld Mbit/s\n",ifName,ifSpeed);
	  exit(1);
	}
      printf ("OK - link %s is up at %ld Mbit/s\n",ifName,ifSpeed);
    }
     
  
  /*
   * Clean up:
   *  1) free the response.
   *  2) close the session.
   */
  if (response)
    {
      snmp_free_pdu(response);
    }
  snmp_close(ss);
  return 0;
}
