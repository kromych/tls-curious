CFLAGS=-nodefaultlibs -nostdinc \
	-ffreestanding -fno-asynchronous-unwind-tables \
	-fno-stack-clash-protection \
	-fno-pie -fno-PIC \
	-ftls-model=local-exec \
	-Wall -O0 -ggdb3

LDFLAGS=

TARGET=x64-tls-ldt-msr

x64-lin-reg: ${TARGET}.o
	ld -o ${LDFLAGS} ${TARGET} ${TARGET}.o 

x64-lin-reg.o: ${TARGET}.c
	gcc -c ${CFLAGS} ${TARGET}.c

clean:
	rm -f ${TARGET} ${TARGET}.o ${TARGET}.s
