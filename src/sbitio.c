/*
 * sbitio.c
 *
 * Bit I/O on memory buffers.
 *
 * Copyright (c) 2015 F. Aragon. All rights reserved.
 *
 */

#include "sbitio.h"

void sbio_write_init(sbio_t *bio, unsigned char *b)
{
	bio->acc = bio->off = 0;
	bio->bw = b;
}

void sbio_write(sbio_t *bio, size_t c, size_t cbits)
{
	unsigned char *b = bio->bw;
	if (bio->acc) {
		size_t xbits = 8 - bio->acc;
		b[bio->off++] |= (c << bio->acc);
		c >>= xbits;
		cbits -= xbits;
	}
	size_t copy_size = cbits / 8;
	for (; copy_size > 0;) {
		b[bio->off++] = (unsigned char)c;
		c >>= 8;
		copy_size --;
	}
	bio->acc = cbits % 8;
	if (bio->acc)
		b[bio->off] = (unsigned char)c;
}

size_t sbio_write_close(sbio_t *bio)
{
	if (bio->acc) {
		bio->off++;
		bio->acc = 0;
	}
	return bio->off;
}

void sbio_read_init(sbio_t *bio, const unsigned char *b)
{
	bio->acc = bio->off = bio->accbuf = 0;
	bio->br = b;
}

size_t sbio_read(sbio_t *bio, size_t code_bits)
{
	size_t code = 0;
	const unsigned char *b = bio->br;
	if (bio->acc) {
		code |= bio->accbuf;
		code_bits -= bio->acc;
	}
	for (; code_bits >= 8;) {
		code |= ((size_t)b[bio->off++] << bio->acc);
		code_bits -= 8;
		bio->acc += 8;
	}
	if (code_bits > 0) {
		bio->accbuf = b[bio->off++];
		code |= ((bio->accbuf & S_NBITMASK(code_bits)) << bio->acc);
		bio->accbuf >>= code_bits;
		bio->acc = 8 - code_bits;
	} else {
		bio->acc = 0;
	}
	return code;
}

