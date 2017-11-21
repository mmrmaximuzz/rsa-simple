INCDIR := ./include
SRCDIR := ./src
OBJDIR := ./build
TESTDIR := ./tests
EXEDIR := ./bin
SUPPRESS := >/dev/null 2>/dev/null

OUTPUTDIRS := $(OBJDIR) $(EXEDIR)
RSABASE := $(OBJDIR)/rsa_base.o $(OBJDIR)/rsa_io.o

CC := gcc
CFLAGS := -Wall -O2 -std=c11 -fdiagnostics-color=always -I $(INCDIR)
LINKS := -lgmp



all: rsagenerate rsaencrypt rsadecrypt



$(OBJDIR):
	mkdir $(OBJDIR)

$(EXEDIR):
	mkdir $(EXEDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)



rsagenerate: $(OUTPUTDIRS) $(RSABASE) $(OBJDIR)/rsa_generate_key.o
	$(CC) -o $(EXEDIR)/rsakeygen $(RSABASE) $(OBJDIR)/rsa_generate_key.o $(CFLAGS) $(LINKS)

rsaencrypt: $(OUTPUTDIRS) $(RSABASE) $(OBJDIR)/rsa_encrypt.o
	$(CC) -o $(EXEDIR)/rsaencrypt $(RSABASE) $(OBJDIR)/rsa_encrypt.o $(CFLAGS) $(LINKS)

rsadecrypt: $(OUTPUTDIRS) $(RSABASE) $(OBJDIR)/rsa_decrypt.o
	$(CC) -o $(EXEDIR)/rsadecrypt $(RSABASE) $(OBJDIR)/rsa_decrypt.o $(CFLAGS) $(LINKS)



install: $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen
	cp $(EXEDIR)/rsaencrypt /usr/bin
	cp $(EXEDIR)/rsadecrypt /usr/bin
	cp $(EXEDIR)/rsakeygen  /usr/bin

uninstall:
	rm /usr/bin/rsaencrypt $(SUPPRESS)
	rm /usr/bin/rsadecrypt $(SUPPRESS)
	rm /usr/bin/rsakeygen $(SUPPRESS)



test: $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen
	@cp $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen $(TESTDIR) $(SUPPRESS)
	bash $(TESTDIR)/stress_test.sh $(TESTDIR)
	@rm $(TESTDIR)/rsaencrypt $(TESTDIR)/rsadecrypt $(TESTDIR)/rsakeygen $(SUPPRESS)



.PHONY = clean
clean:
	@rm -f $(OBJDIR)/*.o ${SUPPRESS}
	@rm -f $(EXEDIR)/rsaencrypt $(EXEDIR)/rsadecrypt $(EXEDIR)/rsakeygen ${SUPPRESS}
	@echo "Cleaned"
