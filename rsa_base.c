#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gmp.h>

#include "rsa_base.h"

void rsa_byteblock_init(struct rsa_byteblock *block, size_t size) {
    block->size = size;
    block->data = malloc(size);
}

void rsa_byteblock_destroy(struct rsa_byteblock *block) {
    free(block->data);
    block->size = 0;
    block->data = NULL;
}

/* mpz_t conversions */
static const int SIGNIFICANT_FIRST = 1;
static const int BIGENDIAN = 1;
static const int NO_SKIP_BITS = 0;
static void rsa_byteblock_to_mpz(mpz_t result, const struct rsa_byteblock *block) {
    assert(block->size != 0);
    assert(block->data != NULL);

    /* find the first significant byte in block */
    size_t zero_prefix = 0;
    while (zero_prefix < block->size && block->data[zero_prefix] == 0)
        ++zero_prefix;

    size_t bytes_to_import = block->size - zero_prefix;
    if (bytes_to_import == 0)
        mpz_set_ui(result, 0);
    else
        mpz_import(result, bytes_to_import,
                SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS,
                block->data + zero_prefix);
}

static const int BITS_IN_BYTE = 8;
static void rsa_mpz_to_byteblock(struct rsa_byteblock *result, const mpz_t mpz) {
    assert(result->size != 0);
    assert(result->data != NULL);

    if (mpz_cmp_ui(mpz, 0) == 0)
        memset(result->data, 0, result->size);
    else {
        size_t mpz_size = (mpz_sizeinbase(mpz, 2) + BITS_IN_BYTE - 1) / BITS_IN_BYTE;
        assert(result->size >= mpz_size);

        size_t zero_prefix = result->size - mpz_size;
        memset(result->data, 0, zero_prefix);
        mpz_export(result->data + zero_prefix, NULL,
                    SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS,
                    mpz);
    }
}


void rsa_key_init(struct rsa_key *key) {
    key->modulus_size = 0;
    key->exponent_size = 0;
    mpz_init(key->modulus);
    mpz_init(key->exponent);
}

void rsa_key_destroy(struct rsa_key *key) {
    key->modulus_size = 0;
    key->exponent_size = 0;
    mpz_clear(key->modulus);
    mpz_clear(key->exponent);
}


void rsa_generate_key(size_t key_size, struct rsa_key *public_key, struct rsa_key *private_key) {
    assert(key_size >= 256 && key_size <= 1024);
    assert(public_key->modulus_size == 0);
    assert(private_key->modulus_size == 0);

    size_t p_size = key_size / 2;
    size_t q_size = key_size - p_size;

    FILE *urandom = fopen("/dev/urandom", "rb");
    if (urandom == NULL) {
        fprintf(stderr, "error with random device /dev/urandom: %s\n", strerror(errno));
        return;
    }

    uint8_t *p_buffer = malloc(p_size);
    uint8_t *q_buffer = malloc(q_size);

    size_t p_read = fread(p_buffer, sizeof(uint8_t), p_size, urandom);
    size_t q_read = fread(q_buffer, sizeof(uint8_t), q_size, urandom);
    if (p_read < p_size || q_read < q_size) {
        fprintf(stderr, "error in getting bytes from urandom\n");
        free(q_buffer);
        free(p_buffer);
        fclose(urandom);
        return;
    }

    /* check first bytes of p and q - must be nonzero for correct keysize */
    while (p_buffer[0] == 0) {
        fread(&p_buffer[0], sizeof(uint8_t), 1, urandom);
    }
    while (q_buffer[0] == 0) {
        fread(&q_buffer[0], sizeof(uint8_t), 1, urandom);
    }

    mpz_t p, q, n, p_minusone, q_minusone, phi, d, e;
    mpz_inits(p, q, n, p_minusone, q_minusone, phi, d, e, NULL);
    mpz_import(p, p_size, SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS, p_buffer);
    mpz_import(q, q_size, SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS, q_buffer);

    mpz_nextprime(p, p);    /* p is prime */
    mpz_nextprime(q, q);    /* q is prime */
    mpz_mul(n, p, q);       /* n = p * q */

    mpz_sub_ui(p_minusone, p, 1);
    mpz_sub_ui(q_minusone, q, 1);
    mpz_mul(phi, p_minusone, q_minusone);   /* phi = (p - 1) * (q - 1) */

    mpz_set_str(e, "65537", 10);            /* e = 2 ** 16 + 1 standard public exponent */
    mpz_invert(d, e, phi);                  /* e * d == 1 (mod phi) */
    size_t public_exponent_size = (mpz_sizeinbase(e, 2) + 7) / 8;
    size_t private_exponent_size = (mpz_sizeinbase(d, 2) + 7) / 8;

    mpz_set(public_key->modulus, n);
    mpz_set(public_key->exponent, e);
    mpz_set(private_key->modulus, n);
    mpz_set(private_key->exponent, d);
    public_key->modulus_size = key_size;
    public_key->exponent_size = public_exponent_size;
    private_key->modulus_size = key_size;
    private_key->exponent_size = private_exponent_size;

    mpz_clears(p, q, n, p_minusone, q_minusone, phi, d, e, NULL);
    free(q_buffer);
    free(p_buffer);
    fclose(urandom);
}



/* RSA encryption functions */
static void rsa_power_mpz(mpz_t output, const mpz_t input, const struct rsa_key *key) {
    mpz_powm(output, input, key->exponent, key->modulus);
}

static void rsa_power_byteblock(
              struct rsa_byteblock *output,
        const struct rsa_byteblock *input,
        const struct rsa_key       *key)
{
    mpz_t data, power;
    mpz_inits(data, power, NULL);
    rsa_byteblock_to_mpz(data, input);
    rsa_power_mpz(power, data, key);
    rsa_mpz_to_byteblock(output, power);
    mpz_clears(data, power, NULL);
}

void rsa_encrypt(
              struct rsa_byteblock *output,
        const struct rsa_byteblock *input,
        const struct rsa_key       *key)
{
    assert(output->size == key->modulus_size);
    assert( input->size == key->modulus_size - 1);
    rsa_power_byteblock(output, input, key);
}

void rsa_decrypt(
              struct rsa_byteblock *output,
        const struct rsa_byteblock *input,
        const struct rsa_key       *key)
{
    assert( input->size == key->modulus_size);
    assert(output->size == key->modulus_size - 1);
    rsa_power_byteblock(output, input, key);
}
