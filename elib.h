#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

int elib_init(
	      struct snmp_session** ssp,
	      char* agentaddress,
	      char* community
	      );
