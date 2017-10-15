#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "rsa_base.h"
#include "rsa_io.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("usage: %s key_length_in_bytes output_public output_private\n", argv[0]);
        return 1;
    }

    int key_size = atoi(argv[1]);
    const char *public_path = argv[2];
    const char *private_path = argv[3];
    
    struct rsa_key public, private;
    rsa_key_init(&public);
    rsa_key_init(&private);

    rsa_generate_key(key_size, &public, &private);

    FILE *public_file = fopen(public_path, "wb");
    FILE *private_file = fopen(private_path, "wb");
    rsa_write_key(&public, public_file);
    rsa_write_key(&private, private_file);
    fclose(public_file);
    fclose(private_file);

    rsa_key_destroy(&public);
    rsa_key_destroy(&private);
    return 0;
}
