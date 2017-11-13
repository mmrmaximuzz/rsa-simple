CC := gcc
INCDIR := ./include
SRCDIR := ./src
OBJDIR := ./build
CFLAGS := -Wall -O2 -std=c11 -fdiagnostics-color=always -I $(INCDIR)
EXEDIR := ./bin
LINKS := -lgmp
RSABASE := $(OBJDIR)/rsa_base.o $(OBJDIR)/rsa_io.o


all: rsagenerate rsaencrypt rsadecrypt

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

rsaencrypt: $(RSABASE) $(OBJDIR)/rsa_encrypt.o
	$(CC) -o $(EXEDIR)/rsaencrypt $(RSABASE) $(OBJDIR)/rsa_encrypt.o $(CFLAGS) $(LINKS)

rsadecrypt: $(RSABASE) $(OBJDIR)/rsa_decrypt.o
	$(CC) -o $(EXEDIR)/rsadecrypt $(RSABASE) $(OBJDIR)/rsa_decrypt.o $(CFLAGS) $(LINKS)

rsagenerate: $(RSABASE) $(OBJDIR)/rsa_generate_key.o
	$(CC) -o $(EXEDIR)/rsakeygen $(RSABASE) $(OBJDIR)/rsa_generate_key.o $(CFLAGS) $(LINKS)

install:
	cp $(EXEDIR)/rsaencrypt /usr/bin
	cp $(EXEDIR)/rsadecrypt /usr/bin
	cp $(EXEDIR)/rsakeygen  /usr/bin

uninstall:
	rm /usr/bin/rsaencrypt
	rm /usr/bin/rsadecrypt
	rm /usr/bin/rsakeygen

clean:
	rm $(OBJDIR)/*.o
	rm $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen
