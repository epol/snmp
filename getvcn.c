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
	  }
      }
    if ( !agentaddress || !community || (member==-1) )
      {
	printf("UNKNOWN - wrong parameters");
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

  /* query for jnxVirtualChassisMemberRole */
  read_objid("1.3.6.1.4.1.2636.3.40.1.4.1.1.1.3", anOID, &anOID_len);
  /* jnxVirtualChassisMemberRole.member */
  anOID[anOID_len++] = member;

  status = elib_get_one_response(ss,&response, anOID, anOID_len, SNMP_MSG_GET  );

  switch (status)
    {
    case 0:
      /* OK with the query */
      /* TODO: check that it really is a integer */
      switch (*(response->variables->val.integer))
	{
	case 1:
	  printf("OK - master\n");
	  break;
	case 2:
	  printf("OK - backup\n");
	  break;
	case 3:
	  printf("OK - linecard\n");
	  break;
	default:
	  //	  printf("UNKNOWN - unknow jnxVirtualChassisMemberRole integer %ld for member %d\n", *var.val.integer, member);
	  exit_code = 3;
	  break;
	}
      break;
    case 2:
      /* variable not found, so the member is not present */
      printf("CRITICAL - member %d not found\n",member);
      exit_code = 2;
      break;
    case 3:
      exit_code = 3;
    }
  if (response)
    {
      snmp_free_pdu(response);
    }

  status = elib_close(&ss);
  return exit_code;
}
