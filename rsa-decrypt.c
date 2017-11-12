#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "rsa_base.h"
#include "rsa_io.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s private_key encrypt_input message_output\n", argv[0]);
        return 1;
    }

    const char *key_path = argv[1];
    const char *encrypt_path = argv[2];
    const char *message_path = argv[3];

    /* open all the files */
    FILE *key_file = fopen(key_path, "rb");
    FILE *encrypt_file = fopen(encrypt_path, "rb");
    FILE *message_file = fopen(message_path, "wb");

    /* read the key */
    struct rsa_key private_key;
    rsa_key_init(&private_key);
    rsa_read_key(&private_key, key_file);

    /* prepare byteblocks for decryption */
    size_t key_size = private_key.modulus_size;
    struct rsa_byteblock input, output;
    rsa_byteblock_init(&input, key_size);
    rsa_byteblock_init(&output, key_size - 1);

    /* main cycle */
    size_t last_message_size = rsa_read_cipher(input.data, input.size, encrypt_file);
    while (last_message_size > 0) {
        rsa_decrypt(&output, &input, &private_key);
        rsa_write_message(output.data, last_message_size, message_file);
        last_message_size = rsa_read_cipher(input.data, input.size, encrypt_file);
    }

    /* clean */
    rsa_byteblock_destroy(&output);
    rsa_byteblock_destroy(&input);
    rsa_key_destroy(&private_key);
    fclose(encrypt_file);
    fclose(message_file);
    fclose(key_file);
    return 0;
}
