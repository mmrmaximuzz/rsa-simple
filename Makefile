CC := gcc
FLAGS := -Wall -O2 -std=c11 -fdiagnostics-color=always

all:
	$(CC) $(FLAGS) *.c -c
	$(CC) $(FLAGS) rsa_io.o rsa_base.o rsa-generate-key.o -o rsa-generate-key -lgmp
	$(CC) $(FLAGS) rsa_io.o rsa_base.o rsa-encrypt.o -o rsa-encrypt -lgmp
	$(CC) $(FLAGS) rsa_io.o rsa_base.o rsa-decrypt.o -o rsa-decrypt -lgmp

