HOSTTYPE = $(shell uname -m)
BINDIR = $(HOME)/bin/$(HOSTTYPE)
CFLAGS = -Wall
LDLIBS = -L/usr/X11R6/lib -lX11

BINARIES = xss xsswin xcursorpos xkeygrab xbell magic

all: $(BINARIES)

install: $(BINARIES)
	cp $(BINARIES) $(BINDIR)

clean:
	rm -f $(BINARIES)

xss: LDLIBS += -lXss

.PHONY: all install clean
