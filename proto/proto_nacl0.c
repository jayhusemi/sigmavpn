//
//  proto_nacl0.c
//  Sigma nacl0 protocol code
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#define crypto_box_SECRETKEYBYTES 64
#define crypto_box_NONCEBYTES 0

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../hexdump.c"
#include "../types.h"
#include "../include/crypto_box_curve25519xsalsa20poly1305.h"

typedef struct sigma_proto_nacl
{
	sigma_proto baseproto;
	
	char encbuffer[1514 + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES];
	char decbuffer[1514 + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES];
	unsigned char privatekey[crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES];
	unsigned char publickey[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];
}
sigma_proto_nacl;

static int proto_set(sigma_proto* instance, char* param, char* value)
{	
	if (strcmp(param, "publickey") == 0)
		hex2bin(((sigma_proto_nacl*) instance)->publickey, value, crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES);
	
	if (strcmp(param, "privatekey") == 0)
		hex2bin(((sigma_proto_nacl*) instance)->privatekey, value, crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES);
	
	return 0;
}

static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
	unsigned char n[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	unsigned char tempbuffer[len];
	
	memset(n, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	memset(input, 0, crypto_box_curve25519xsalsa20poly1305_ZEROBYTES);

	int result = crypto_box_curve25519xsalsa20poly1305(
		tempbuffer,
		input,
		len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES,
		n,
		((sigma_proto_nacl*) instance)->publickey,
		((sigma_proto_nacl*) instance)->privatekey
	);
	
	memcpy(output, tempbuffer + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES, len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);

	return len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES;
}

static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
	if (len < crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES)
	{
		fprintf(stderr, "Short packet received: %d\n", len);
		return 0;
	}
	
	unsigned char n[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	unsigned char tempbuffer[len + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES];
	
	memset(n, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	memset(tempbuffer, 0, crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);
	memcpy(tempbuffer + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES, input, len + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);

	int result = crypto_box_curve25519xsalsa20poly1305_open(
		output,
		tempbuffer,
		len + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES,
		n,
		((sigma_proto_nacl*) instance)->publickey,
		((sigma_proto_nacl*) instance)->privatekey
	);
	
	if (result)
	{
		fprintf(stderr, "Decryption failed (length %i, given result %i)\n", len, result);
	}
	
	return len - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES;
}

static int proto_init(sigma_proto *instance)
{
	return 0;
}

extern sigma_proto* proto_descriptor()
{
	sigma_proto_nacl* proto_nacl0 = malloc(sizeof(sigma_proto_nacl));
	
	proto_nacl0->baseproto.encrypted = 1;
	proto_nacl0->baseproto.stateful = 0;
	proto_nacl0->baseproto.init = proto_init;
	proto_nacl0->baseproto.encode = proto_encode;
	proto_nacl0->baseproto.decode = proto_decode;
	proto_nacl0->baseproto.set = proto_set;
	proto_nacl0->baseproto.state = 0;
	
	return (sigma_proto*) proto_nacl0;
}
