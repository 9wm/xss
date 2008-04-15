CFLAGS = -Wall
LDLIBS = -lX11

all: xss xsswin xcursorpos magic

xss: LDLIBS += -lXss

.PHONY: all
