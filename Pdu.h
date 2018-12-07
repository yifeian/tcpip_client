/*
 * Pdu.h
 *
 *  Created on: Nov 26, 2018
 *      Author: yifeifan
 */

#ifndef PDU_H_
#define PDU_H_
#include "network.h"

typedef struct {
	unsigned int version:2;
	unsigned int type:2;
	unsigned int token_length:4;
	unsigned int code:8;
	unsigned short id;
	unsigned char token[];
}coap_hdr_t;


typedef struct coap_address_t{
	uint16_t port;
	in_addr_t addr;
}coap_address_t;


#endif /* PDU_H_ */
