/*
*  kernel.c
*/
#include "screen/monitor.h"
#include "descriptors/descriptor_tables.h"
#include "timer/timer.h"
#include "paging/paging.h"
#include "heap/kheap.h"
#include "multitasking/task.h"

u32int initial_esp;

int kmain(struct multiboot *mboot_ptr, u32int initial_stack)
{
    initial_esp = initial_stack;
	// Initialise all the ISRs and segmentation

    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();     
   // initialize_paging();


   // u32int *ptr = (u32int*)0xA0000000;
   // u32int do_page_fault = *ptr;

    u32int a = kmalloc(8);
    initialize_paging();
    u32int b = kmalloc(8);
    u32int c = kmalloc(8);
    monitor_write("a: ");
    monitor_write_number(a, 16);
    monitor_write(", b: ");
    monitor_write_number(b, 16);
    monitor_write("\nc: ");
    monitor_write_number(c, 16);
    kfree(c);
    kfree(b);
    u32int d = kmalloc(12);
    monitor_write(", d: ");
    monitor_write_number(d, 16);

//multitasking
    initialize_tasking();
    asm volatile("sti");
    init_timer(50);
    pid_t ret = fork();
    pid_t my_pid = getpid();
    monitor_write("\nfork() returned: ");
    monitor_write_number(ret,16);
    monitor_write(", and getpid() returned ");
    monitor_write_number(my_pid,16);
    monitor_write("\n============================================================================\n");
  
    while(1){

    }
	return 0;
}
