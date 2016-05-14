#include "../paging/paging.h"
#include "../screen/monitor.h"
#include "task.h"
#include "../heap/kheap.h"

#define PAGE_SIZE 0x1000
#define PTR_SIZE 4

#define KERNEL_STACK 0xE0000000
#define KERNEL_STACK_SIZE 0x2000

#define TASK_SWITCHED_COOKIE 0xDEADBEEF
// imported from main.c
extern u32int initial_esp;

/// imported from process.s
extern u32int read_eip();
extern void do_switch_task(u32int eip, u32int esp, u32int ebp, u32int cr3);
// current running task
volatile task_t *current_task;

// head of the task list
volatile task_t *ready_queue;

// imported from paging.c
extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;

pid_t next_pid = 1;
int task_switch_enabled = 1;
//buffer_t messages_buffer;

void initialize_tasking() 
{ 
  // disable interrupts
  asm volatile("cli");

  // move the kernel stack so we know where it is
  move_stack((void*)KERNEL_STACK, KERNEL_STACK_SIZE);

  // initialize the first task (kernel)
  current_task = ready_queue = (task_t*)kmalloc(sizeof(task_t));
  memset((void*)current_task,0,sizeof(current_task));
  current_task->id = next_pid++;
  current_task->esp = current_task->ebp = 0;
  current_task->eip = 0;
  current_task->page_directory = current_directory;
  current_task->next = 0;

  // we've moved the stack, re-enable interrupts
  asm volatile("sti");
}

pid_t fork()
{
  asm volatile("cli");

  task_t *parent_task = (task_t*)current_task;
  
  page_directory_t *directory = clone_directory(current_directory);
  
  task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
  memset(new_task,0,sizeof(new_task));
  
  new_task->id = next_pid++;
  new_task->esp = new_task->ebp = 0;
  new_task->eip = 0;
  new_task->page_directory = directory;
  new_task->next = 0;
  new_task->task_state = RUNNING;


  // walk the task list to find the end and add the task
  task_t *ptr = (task_t*)ready_queue;
  while(ptr->next) {
    ptr = ptr->next;
  }
  ptr->next = new_task;

  // where the new task should start
  pid_t eip = read_eip();
  
  // if we're in the parent
  if(current_task == parent_task) {
    u32int esp = 0; 
    asm volatile("mov %%esp, %0" : "=r"(esp));
    u32int ebp = 0; 
    asm volatile("mov %%ebp, %0" : "=r"(ebp));
    new_task->esp = esp;
    new_task->ebp = ebp;
    new_task->eip = eip;
    // we're all done, re-enable interrupts
    asm volatile("sti");
    return new_task->id;
  } else {
    // it's afterwards, and we're in the child now
    // we're the child
    return 0;
  }
}

void switch_task()
{
  if (!task_switch_enabled) {
    return;
  }
  // just return if tasking isn't on yet
  if(!current_task) {
    return;
  }
  // read stack and base pointers for saving later
  u32int esp, ebp, eip = 0;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  asm volatile("mov %%ebp, %0" : "=r"(ebp)); 

  eip = read_eip();
  
  // if it's after the task switch and we're in a different task now
  if(eip == TASK_SWITCHED_COOKIE) {
    return;
  }
  // if we got here, we're still before the task switch
  current_task->eip = eip;
  current_task->esp = esp;
  current_task->ebp = ebp;

//--------------------------------------------------------------------------------------

  while (1) {
    current_task = current_task->next;
    if (!current_task) {
      current_task = ready_queue;
    }
    if (current_task->task_state == RUNNING) {
      break;
    } else {
      continue;
    }
  }
//--------------------------------------------------------------------------------------
  // get the next task to run
  //current_task = current_task->next; old
  if(!current_task) {
    current_task = ready_queue;
  }
  eip = current_task->eip;
  esp = current_task->esp;
  ebp = current_task->ebp;

  current_directory = current_task->page_directory;
  // okay, now do the actual task switch
  do_switch_task(eip, esp, ebp, current_directory->physicalAddr);
}

pid_t getpid() {  
  return current_task->id;
}

task_t* getProcess(pid_t id) {
    task_t *current = ready_queue;
    while (current) {
      if (current->id == id) {
        return current;
      }
      current = current->next;
    }

    return 0;
}

void move_stack(void *new_stack_start, u32int size)
{
  u32int i = 0;
  // allocate some space for the new stack
  for(i = (u32int)new_stack_start;
      i >= ((u32int)new_stack_start-size);
      i -= PAGE_SIZE) {

    alloc_frame(get_page(i,1,current_directory),0,1);
  }

  // A page table has been changed, so we flush tlb
  u32int pd_addr = 0;
  asm volatile("mov %%cr3, %0" : "=r"(pd_addr));
  asm volatile("mov %0, %%cr3" : : "r"(pd_addr));
  
  // Old ESP and EBP, read from registers.
  u32int old_stack_pointer = 0; 
  asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
  u32int old_base_pointer = 0;  
  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer)); 

  u32int offset = (u32int)new_stack_start - initial_esp;

  u32int new_stack_pointer = old_stack_pointer + offset;
  u32int new_base_pointer = old_base_pointer + offset;

  // copy the stack
  memcpy((void*)new_stack_pointer, 
	 (void*)old_stack_pointer, initial_esp-old_stack_pointer);
  // update pointers on the stack into the stack
  for(i = (u32int)new_stack_start; 
      i > (u32int)new_stack_start-size; i -= PTR_SIZE) {
    
    u32int tmp = *(u32int*)i;
    // if the value is in stack range...
    if((old_stack_pointer < tmp) && (tmp < initial_esp)) {
      tmp += offset;
      u32int *tmp2 = (u32int*)i;
      *tmp2 = tmp;
    }
  }
  
  // change stacks
  asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
  asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}

void runFunctionAsync(void (*function)( void ) ){
    pid_t current_task_id = getpid();
    fork();
    if (getpid() == current_task_id){
        return;
    }
    (*function)();
}

void async_send(message_t msg)
{
  task_switch_enabled = 0;
  msg.src=current_task->id; //we must not rely on it's set
  task_t *dst = getProcess(msg.dst);
 
//  buffer_t tmpbuff = dst->messages_buffer;//map_buffer(msg->dst); //temporarily map destination's buffer into sender process' address space
  //if (tmpbuff->count==MAXITEMS) { //if receiver buffer is full, block
//    pushwaitqueue(msg.dst,current_process); //record this process in dst's sender queue
 //   block(current_process);
 // }

    push_message(msg);
  //push_message(tmpbuff,msg);
  if (dst->task_state == SUSPENDED) dst->task_state = RUNNING;

//  if(isblocked(msg.dst)) awake(msg.dst);  //if destination process is blocked for receiving, awake it
 // unmap_buffer();
  
  task_switch_enabled = 1;
}

message_t async_recv()
{
  message_t *tmp=0;

  task_switch_enabled = 0;
  monitor_write("\n count: ");
  monitor_write_number(current_task->messages_buffer.count, 10);
  monitor_write("\n");

  if (current_task->messages_buffer.count==0){
    current_task->task_state = SUSPENDED;//block(current_process); //if there's nothing to get, block
    task_switch_enabled = 1;
    switch_task();
  } 
  
  tmp = pop_message(current_task->id);
  //tmp=pop_message(buff);
  //while(topwaitqueue()!=NULL) awake(popwaitqueue()); //awake blocked processes waiting to send
  task_switch_enabled = 1;
  return (*tmp);
}

message_t sync_send(message_t msg)
{
  async_send(msg); //we send the message
  return(async_recv()); //and we block waiting for the response
}

message_t sync_recv()
{
  message_t tmp;
  tmp=async_recv();  //wait for a message to arrive

  //tmp=consume(tmp);  //process the message and return a response message
  async_send(tmp);   //send it back to the caller
  
  return tmp;
}

void push_message(message_t msg) {

  task_t *process = getProcess(msg.dst); 
  int head = process->messages_buffer.head;
  if (head + 1 >= MAX_MESSAGES) {
    process->messages_buffer.head = 0;
  } else {
    process->messages_buffer.head = head+1;
  }
  process->messages_buffer.buffer[head] = msg;
  process->messages_buffer.count++;
  /*monitor_write("\nPROCESS ID: ");
  monitor_write_number(process->id, 10);
  monitor_write("\n");
  monitor_write("\nHEAD: ");
  monitor_write_number(head, 10);
  monitor_write("\n");
  monitor_write("\nMESSAGE(PUSH): ");
  monitor_write(msg.body);
  monitor_write("\n");*/

}

message_t* pop_message(pid_t id) {
  task_t *process = getProcess(id);
  if (process->messages_buffer.count == 0) {
    return 0;
  }

  int tail = process->messages_buffer.tail;
  message_t return_msg = process->messages_buffer.buffer[tail];
  if (++tail >= MAX_MESSAGES) process->messages_buffer.tail = 0;
  process->messages_buffer.count--;
  /*monitor_write("\nTAIL: ");
  monitor_write_number(tail, 10);
  monitor_write("\n");
  monitor_write("\nMESSAGE(POP): ");
  monitor_write(return_msg.body);
  monitor_write("\n");*/
  return &return_msg;
}

message_t create_message(char* message, pid_t src, pid_t dst) {
  message_t msg;
  msg.body = message;
  msg.src = src;
  msg.dst = dst;
  return msg;
}