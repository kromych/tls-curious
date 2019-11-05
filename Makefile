CFLAGS=-nodefaultlibs -nostdinc \
	-ffreestanding -fno-asynchronous-unwind-tables \
	-fno-stack-clash-protection \
	-fno-pie -fno-PIC \
	-ftls-model=local-exec \
	-Wall -O2 -ggdb3

LDFLAGS=

TARGET=x64-tls-ldt-msr

${TARGET}: ${TARGET}.o
	ld -o ${LDFLAGS} ${TARGET} ${TARGET}.o 

${TARGET}.o: ${TARGET}.c lib*.c
	gcc -c ${CFLAGS} ${TARGET}.c

clean:
	rm -f ${TARGET} ${TARGET}.o ${TARGET}.s strace.log.*

run: ${TARGET}
	./${TARGET}

trace: ${TARGET}
	strace -o strace.log -ff ./${TARGET}
