# --- build options ---
PREFIX = /usr/local
BINDIR = bin
SCOREDIR = /var/games/termtris
# ---------------------

src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = termtris

CFLAGS = -pedantic -Wall -O2 -g -DSCOREDIR=\"$(SCOREDIR)\"

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@echo depfile $@
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

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

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(BINDIR)/$(bin)
