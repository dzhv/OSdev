/*
*  kernel.c
*/
#include "screen/monitor.h"
#include "descriptors/descriptor_tables.h"
#include "timer/timer.h"
#include "paging/paging.h"
#include "heap/kheap.h"

int kmain(struct multiboot *mboot_ptr)
{		   
int i;
	// Initialise all the ISRs and segmentation
    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();     
    initialise_paging();

 monitor_write("MAIN\n");
for (i=0;i<500000000;i++) {}

    monitor_write("Hello, paging world!\n");

   // u32int *ptr = (u32int*)0xA0000000;
   // u32int do_page_fault = *ptr;

for (i=0;i<100000000;i++) {}
    u32int a = kmalloc(8);
    initialise_paging();
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


    while(1){

    }
	return 0;
}
