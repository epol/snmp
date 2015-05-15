/*
 * elib.h
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

#ifndef _ELIB_H
#define _ELIB_H
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

#endif

