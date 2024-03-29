# --- build options ---
PREFIX = /usr/local
BINDIR = bin
SCOREDIR = /var/games/termtris
# ---------------------

src = $(wildcard src/unix/*.c) $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = termtris

CFLAGS = -pedantic -Wall -O2 -g -Isrc -DSCOREDIR=\"$(SCOREDIR)\" -MMD

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/$(BINDIR)
	cp $(bin) $(DESTDIR)$(PREFIX)/$(BINDIR)/$(bin)
	mkdir -p $(DESTDIR)$(SCOREDIR)
	chmod 0777 $(DESTDIR)$(SCOREDIR)
	touch $(DESTDIR)$(SCOREDIR)/scores
	chmod 0666 $(DESTDIR)$(SCOREDIR)/scores

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(BINDIR)/$(bin)
