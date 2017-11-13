#ifndef RSA_IO_H
#define RSA_IO_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "rsa_base.h"

void rsa_fill_random_bytes(uint8_t *buffer, size_t size);

void rsa_read_key(struct rsa_key *key, FILE *stream);
void rsa_write_key(const struct rsa_key *key, FILE *stream);

size_t rsa_read_message(uint8_t *buffer, size_t size, FILE *stream);
size_t rsa_write_message(const uint8_t *buffer, size_t size, FILE *stream);

size_t rsa_read_cipher(uint8_t *buffer, size_t block_size, FILE *stream);
size_t rsa_write_cipher(const uint8_t *buffer, size_t block_size, size_t message_size, FILE *stream);

#endif /* RSA_IO_H */
