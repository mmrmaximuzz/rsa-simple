#ifndef RSA_BASE_H
#define RSA_BASE_H

#include <stdint.h>
#include <stddef.h>
#include <gmp.h>

struct rsa_byteblock {
    size_t   size;
    uint8_t *data;
};

void rsa_byteblock_init(struct rsa_byteblock *block, size_t size);
void rsa_byteblock_destroy(struct rsa_byteblock *block);


struct rsa_key {
    mpz_t modulus;
    mpz_t exponent;
    size_t modulus_size;
    size_t exponent_size;
};

void rsa_key_init(struct rsa_key *);
void rsa_key_destroy(struct rsa_key *);
void rsa_generate_key(size_t size, struct rsa_key *public_key, struct rsa_key *private_key);

void rsa_encrypt(struct rsa_byteblock *output, const struct rsa_byteblock *input, const struct rsa_key *key);
void rsa_decrypt(struct rsa_byteblock *output, const struct rsa_byteblock *input, const struct rsa_key *key);

#endif /* RSA_BASE_H */
