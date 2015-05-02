#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

int elib_init(
	      struct snmp_session** ssp,
	      char* agentaddress,
	      char* community
	      );

int elib_get_one_response(
			   struct snmp_session *ss, /*ss: session to be used */
			   struct snmp_pdu **response,
			   oid anOID[],  /* OID to be asked */
			   size_t anOID_len,
			   int msg_type
			  );

int elib_close(
	       struct snmp_session** ssp
	       );
