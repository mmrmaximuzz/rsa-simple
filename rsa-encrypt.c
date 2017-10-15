#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "rsa_base.h"
#include "rsa_io.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s public_key message_file encrypt_output\n", argv[0]);
        return 1;
    }

    const char *key_path = argv[1];
    const char *message_path = argv[2];
    const char *encrypt_path = argv[3];

    /* open all the files */
    FILE *key_file = fopen(key_path, "rb");
    FILE *message_file = fopen(message_path, "rb");
    FILE *encrypt_file = fopen(encrypt_path, "wb");

    /* read the key */
    struct rsa_key public_key;
    rsa_key_init(&public_key);
    rsa_read_key(&public_key, key_file);

    /* prepare byteblocks for encryption */
    size_t key_size = public_key.modulus_size;
    struct rsa_byteblock input, output;
    rsa_byteblock_init(&input, key_size - 1);
    rsa_byteblock_init(&output, key_size);

    /* main cycle (block sized messages) */
    size_t last_message_size = rsa_read_message(input.data, input.size, message_file);
    while (last_message_size == input.size) {
        rsa_encrypt(&output, &input, &public_key);
        rsa_write_cipher(output.data, output.size, input.size, encrypt_file);
        last_message_size = rsa_read_message(input.data, input.size, message_file);
    }

    /* last block (not full message) */
    if (last_message_size > 0) {
        /* fill tail by random bytes */
        rsa_fill_random_bytes(input.data + last_message_size, input.size - last_message_size);
        rsa_encrypt(&output, &input, &public_key);
        rsa_write_cipher(output.data, output.size, last_message_size, encrypt_file);
    }

    /* clean */
    rsa_byteblock_destroy(&output);
    rsa_byteblock_destroy(&input);
    rsa_key_destroy(&public_key);
    fclose(encrypt_file);
    fclose(message_file);
    fclose(key_file);
}
