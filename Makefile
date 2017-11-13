CC := gcc
INCDIR := ./include
SRCDIR := ./src
OBJDIR := ./build
CFLAGS := -Wall -O2 -std=c11 -fdiagnostics-color=always -I $(INCDIR)
EXEDIR := ./bin
LINKS := -lgmp
RSABASE := $(OBJDIR)/rsa_base.o $(OBJDIR)/rsa_io.o
TESTDIR := ./tests
SUPPRESS := >/dev/null 2>/dev/null


all: rsagenerate rsaencrypt rsadecrypt

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

rsaencrypt: $(RSABASE) $(OBJDIR)/rsa_encrypt.o
	$(CC) -o $(EXEDIR)/rsaencrypt $(RSABASE) $(OBJDIR)/rsa_encrypt.o $(CFLAGS) $(LINKS)

rsadecrypt: $(RSABASE) $(OBJDIR)/rsa_decrypt.o
	$(CC) -o $(EXEDIR)/rsadecrypt $(RSABASE) $(OBJDIR)/rsa_decrypt.o $(CFLAGS) $(LINKS)

rsagenerate: $(RSABASE) $(OBJDIR)/rsa_generate_key.o
	$(CC) -o $(EXEDIR)/rsakeygen $(RSABASE) $(OBJDIR)/rsa_generate_key.o $(CFLAGS) $(LINKS)

test: all
	@cp $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen $(TESTDIR) $(SUPPRESS)
	bash $(TESTDIR)/stress_test.sh $(TESTDIR)
	@rm $(TESTDIR)/rsaencrypt $(TESTDIR)/rsadecrypt $(TESTDIR)/rsakeygen $(SUPPRESS)

install:
	cp $(EXEDIR)/rsaencrypt /usr/bin
	cp $(EXEDIR)/rsadecrypt /usr/bin
	cp $(EXEDIR)/rsakeygen  /usr/bin

uninstall:
	rm /usr/bin/rsaencrypt $(SUPPRESS)
	rm /usr/bin/rsadecrypt $(SUPPRESS)
	rm /usr/bin/rsakeygen $(SUPPRESS)

.PHONY = clean
clean:
	@rm -f $(OBJDIR)/*.o ${SUPPRESS}
	@rm -f $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen ${SUPPRESS}
	@echo "Cleaned"
