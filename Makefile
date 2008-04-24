BINDIR = $(HOME)/bin
CFLAGS = -Wall
LDLIBS = -lX11

BINARIES = xss xsswin xcursorpos xkeygrab xbell magic

all: $(BINARIES)

install: $(BINARIES)
	cp $(BINARIES) $(BINDIR)

clean:
	rm -f $(BINARIES)

xss: LDLIBS += -lXss

.PHONY: all install clean
