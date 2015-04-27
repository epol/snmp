#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

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
  int member=0;

  
  /*
   * Initialize the SNMP library
   */
  init_snmp("snmpapp");

  /*
   * Initialize a "session" that defines who we're going to talk to
   */
  snmp_sess_init( &session );                   /* set up defaults */
  session.peername = argv[1];

  /* set the SNMP version number */
  session.version = SNMP_VERSION_1;

  /* set the SNMPv1 community name used for authentication */
  session.community = argv[2];
  session.community_len = strlen(session.community);


  /*
   * Open the session
   */
  ss = snmp_open(&session);                     /* establish the session */

  if (!ss) {
    snmp_perror("ack");
    snmp_log(LOG_ERR, "Unable to connect to %s!!!\n",session.peername);
    exit(2);
  }

  while(1)
  {
    /*
     * Create the PDU for the data for our request.
     *   1) We're going to GET the system.sysDescr.0 node.
     */
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    
    read_objid("1.3.6.1.4.1.2636.3.40.1.4.1.1.1.3", anOID, &anOID_len);
    anOID[anOID_len++] = member;
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
      if (vars)
	{
	  vars = response->variables;
	  if (vars->name[14]>3)
	    {
	      break;
	    }
	  else
	    {
	      print_variable(vars->name, vars->name_length, vars);
	      printf("%d %ld\n", (uint8_t)(vars->name)[15], *vars->val.integer);
	      member = vars->name[15];
	    }
	}
      else
	{
	  printf("Error in getting variables\n");
	}
    } else {
      /*
       * FAILURE: print what went wrong!
       */
      
      if (status == STAT_SUCCESS)
	fprintf(stderr, "Error in packet\nReason: %s\n",
		snmp_errstring(response->errstat));
      else
	snmp_sess_perror("snmpget", ss);
    }
  }
  /*
   * Clean up:
   *  1) free the response.
   *  2) close the session.
   */
  if (response)
    snmp_free_pdu(response);
  snmp_close(ss);
  
}
