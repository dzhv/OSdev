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

void process1(){
    int i = 0;
        while(1){
            i++;
            if (i % 1000000 == 0){
                monitor_write("1");    
            }     
        }
}

void process2(){
    int i = 0;
        while(1){
            i++;
            if (i % 1000000 == 0){
                monitor_write("2");    
            }     
        }
}

void process3(){
    int i = 0;
        while(1){
            i++;
            if (i % 1000000 == 0){
                monitor_write("3");    
            }     
        }
}


int kmain(struct multiboot *mboot_ptr, u32int initial_stack)
{
    initial_esp = initial_stack;
	
    init_descriptor_tables();
    
    monitor_clear();
   
    initialize_paging();
    
    initialize_tasking();

    //monitor_clear();
    monitor_write("Operacine sistema pasileido\n");

    asm volatile("sti");        // enable interrupts
    init_timer(50);
    
    u32int temp = kmalloc(10000);
    kfree(temp);

   // runFunctionAsync(process1);
   // runFunctionAsync(process2);
   // runFunctionAsync(process3);


  
    while(1){

    }
	return 0;
}


