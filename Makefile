CFLAGS = -Wall
LDLIBS = -lX11

all: xss xsswin

xss: LDLIBS += -lXss
