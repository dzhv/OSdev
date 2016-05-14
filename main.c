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
    message_t msg;

    while (1) {
        for (i = 0; i < 100000000; i++) {
            if (i % 10000000 == 0){
                monitor_write_number(getpid(), 10);
            }   
        }

        msg = create_message("DU GAIDELIAI", 2, 3);
        async_send(msg);
        msg = async_recv();
        monitor_write("First process got message: ");
        monitor_write(msg.body);
        monitor_write("\n");
    }
}

void process1(){
    int i = 0;

    while(1){
        message_t message = async_recv();
        monitor_write("Second process got message: ");
        monitor_write(message.body);
        monitor_write("\n");

        for (i = 0; i < 100000000; i++) {
            if (i % 10000000 == 0){
                monitor_write_number(getpid(), 10);
            }
        }

        message_t msg = create_message("TRYS GAIDELIAI", 3, 4);
        async_send(msg);    
    }
}

void process2(){
    int i = 0;
        while(1){
        message_t message = async_recv();
        monitor_write("Third process got message: ");
        monitor_write(message.body);
        monitor_write("\n");

        for (i = 0; i < 100000000; i++) {
            if (i % 10000000 == 0){
                monitor_write_number(getpid(), 10);
            }
        }

        message_t msg = create_message("KETURI GAIDELIAI", 3, 2);
        async_send(msg);    
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
    runFunctionAsync(process2);
    //runFunctionAsync(process3);
  
    while(1){

    }
	return 0;
}


