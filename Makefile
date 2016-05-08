
SOURCES=boot.o main.o multitasking/process.o common/common.o screen/monitor.o descriptors/descriptor_tables.o descriptors/gdt.o interrupts/interrupt.o interrupts/isr.o timer/timer.o common/ordered_map.o paging/paging.o heap/kheap.o multitasking/task.o

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
