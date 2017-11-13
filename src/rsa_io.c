#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "rsa_io.h"

void rsa_fill_random_bytes(uint8_t *buffer, size_t size) {
    FILE *urandom = fopen("/dev/urandom", "rb");
    fread(buffer, sizeof(uint8_t), size, urandom);
    fclose(urandom);
}

static const int SIGNIFICANT_FIRST = 1;
static const int BIGENDIAN = 1;
static const int NO_SKIP_BITS = 0;

void rsa_read_key(struct rsa_key *key, FILE *stream) {
    assert(key != NULL);
    assert(stream != NULL);
    assert(key->modulus_size  == 0);
    assert(key->exponent_size == 0);

    uint16_t exponent_size_netorder = 0;
    size_t exponent_size_read = fread(&exponent_size_netorder, sizeof(uint16_t), 1, stream);
    if (exponent_size_read == 0) {
        fprintf(stderr, "cannot read RSA exponent size from file");
        return;
    }

    size_t exponent_size = ntohs(exponent_size_netorder);
    if (exponent_size == 0) {
        fprintf(stderr, "RSA exponent has no size\n");
        return;
    }

    uint8_t *exponent = malloc(exponent_size);
    size_t exponent_read = fread(exponent, sizeof(uint8_t), exponent_size, stream);
    if (exponent_read < exponent_size) {
        free(exponent);
        fprintf(stderr, "cannot read RSA exponent from file");
        return;
    }

    uint16_t modulus_size_netorder = 0;
    size_t modulus_size_read = fread(&modulus_size_netorder, sizeof(uint16_t), 1, stream);
    if (modulus_size_read == 0) {
        free(exponent);
        fprintf(stderr, "cannot read RSA modulus length from file");
        return;
    }

    size_t modulus_size = ntohs(modulus_size_netorder);
    if (modulus_size == 0) {
        free(exponent);
        fprintf(stderr, "RSA modulus has no size\n");
        return;
    }

    uint8_t *modulus = malloc(modulus_size);
    size_t modulus_read = fread(modulus, sizeof(uint8_t), modulus_size, stream);
    if (modulus_read < modulus_size) {
        free(exponent);
        free(modulus);
        fprintf(stderr, "cannot read RSA modulus from file");
        return;
    }

    key->modulus_size = modulus_size;
    key->exponent_size = exponent_size;
    mpz_import(key->modulus, modulus_size,
                SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS,
                modulus);
    mpz_import(key->exponent, exponent_size,
                SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS,
                exponent);

    free(modulus);
    free(exponent);
}

void rsa_write_key(const struct rsa_key *key, FILE *stream) {
    assert(   key != NULL);
    assert(stream != NULL);

    uint8_t *modulus_buffer = malloc(key->modulus_size);
    uint8_t *exponent_buffer = malloc(key->exponent_size);
    mpz_export(exponent_buffer, NULL,
                SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS,
                key->exponent);
    mpz_export(modulus_buffer, NULL,
                SIGNIFICANT_FIRST, sizeof(uint8_t), BIGENDIAN, NO_SKIP_BITS,
                key->modulus);

    uint16_t exponent_size_netorder = htons(key->exponent_size);
    fwrite(&exponent_size_netorder, sizeof(uint16_t), 1, stream);
    fwrite(exponent_buffer, sizeof(uint8_t), key->exponent_size, stream);
    uint16_t modulus_size_netorder = htons(key->modulus_size);
    fwrite(&modulus_size_netorder, sizeof(uint16_t), 1, stream);
    fwrite(modulus_buffer, sizeof(uint8_t), key->modulus_size, stream);

    free(modulus_buffer);
    free(exponent_buffer);
}

size_t rsa_read_message(uint8_t *buffer, size_t size, FILE *stream) {
    return fread(buffer, sizeof(uint8_t), size, stream);
}

size_t rsa_write_message(const uint8_t *buffer, size_t size, FILE *stream) {
    return fwrite(buffer, sizeof(uint8_t), size, stream);
}

size_t rsa_read_cipher(uint8_t *buffer, size_t block_size, FILE *stream) {
    uint16_t message_size_netorder = 0;
    size_t message_result = fread(&message_size_netorder, sizeof(uint16_t), 1, stream);
    if (message_result == 0) {
        return 0;
    }

    size_t message_size = ntohs(message_size_netorder);
    size_t read_result = fread(buffer, sizeof(uint8_t), block_size, stream);
    if (read_result < block_size) {
        fprintf(stderr, "cannot read cipher block\n");
        return 0;
    }
    return message_size;
}

size_t rsa_write_cipher(const uint8_t *buffer, size_t block_size, size_t message_size, FILE *stream) {
    uint16_t message_size_netorder = htons(message_size);
    size_t message_result = fwrite(&message_size_netorder, sizeof(uint16_t), 1, stream);
    if (message_result == 0) {
        fprintf(stderr, "cannot write message size to cipher block\n");
        return 0;
    }

    size_t write_result = fwrite(buffer, sizeof(uint8_t), block_size, stream);
    if (write_result < block_size) {
        fprintf(stderr, "cannot write cipher block\n");
        return 0;
    }
    return message_size;
}
