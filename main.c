/*
*  kernel.c
*/
#include "screen/monitor.h"
#include "descriptors/descriptor_tables.h"
#include "timer/timer.h"
#include "paging/paging.h"

int kmain(struct multiboot *mboot_ptr)
{		
	// Initialise all the ISRs and segmentation
    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();     
    initialise_paging();

    monitor_write("Hello, paging world!\n");

    u32int *ptr = (u32int*)0xA0000000;
    u32int do_page_fault = *ptr;




    while(1){

    }
	return 0;
}
