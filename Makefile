# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=boot.o main.o common/common.o screen/monitor.o descriptors/descriptor_tables.o descriptors/gdt.o interrupts/interrupt.o interrupts/isr.o timer/timer.o heap/kheap.o paging/paging.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS=-Tlink.ld
ASFLAGS=-felf


all: $(SOURCES) link

clean:
	-rm $(wildcard **/*.o) kernel
	-rm *.o

link:
	ld -m elf_i386 $(LDFLAGS) -o kernel $(SOURCES)

.c.o:
	gcc -m32 -fno-stack-protector -c $< -o $@
 
.s.o:
	nasm $(ASFLAGS) $<
