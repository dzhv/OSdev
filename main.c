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

void process0() {
    int i=0;

    while (1) {
        for (i = 0; i < 10000000; i++) {
            if (i % 1000000 == 0){
                monitor_write_number(getpid(), 10);
            }   
        }

        message_t msg = create_message("asd", 2, 3);
        message_t response = sync_send(msg);
    }
}

void process1(){
    int i = 0;

    while(1){
        sync_recv();
        PANIC("AJAJAI ENTERING 3 PROCESS");
        for (i = 0; i < 100000000; i++) {
            if (i % 1000000 == 0){
                monitor_write_number(getpid(), 10);
            }
        }

        message_t msg = create_message("dsa", 0, 2);
        async_send(msg);    
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
    runFunctionAsync(process0);

    runFunctionAsync(process1);
    //runFunctionAsync(process1);
    //runFunctionAsync(process2);
    //runFunctionAsync(process3);
  
    while(1){

    }
	return 0;
}


