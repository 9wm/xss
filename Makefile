BINDIR = $(HOME)/bin
CFLAGS = -Wall
LDLIBS = -lX11

BINARIES = xss xsswin xcursorpos xkeygrab xbell magic

all: $(BINARIES)

install: $(BINARIES)
	cp $(BINARIES) $(BINDIR)

xss: LDLIBS += -lXss

.PHONY: all install
