CFLAGS = -Wall
LDLIBS = -lX11

all: xss xsswin xcursorpos xkeygrab magic

xss: LDLIBS += -lXss

.PHONY: all
