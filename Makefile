CFLAGS = -Wall
LDLIBS = -lX11

all: xss xsswin xcursorpos

xss: LDLIBS += -lXss

.PHONY: all
