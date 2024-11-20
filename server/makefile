
CFLAGS := -Wall -Werror -ggdb3


ifeq (${CROSS_COMPILE},)
	CROSS_COMPILE :=
endif

CC := ${CROSS_COMPILE}gcc


all: aesdsocket


writer: aesdsocket.c

	$(CC) $(CFLAGS) -c aesdsocket.c aesdsocket.o -o aesdsocket

clean:
	rm -f *.o aesdsocket
