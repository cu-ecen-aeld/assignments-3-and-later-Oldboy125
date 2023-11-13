all: writer

writer: writer.c
ifeq (${CROSS_COMPILE},aarch64-none-linux-gnu-)
	aarch64-none-linux-gnu-gcc -c writer.c
	aarch64-none-linux-gnu-gcc writer.o -o writer
else
	gcc -c writer.c
	gcc writer.o -o writer
endif

clean:
	rm *.o writer
