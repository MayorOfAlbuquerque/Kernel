#include "hilevel.h"

/* Since we *know* there will be 2 processes, stemming from the 2 user
 * programs, we can
 *
 * - allocate a fixed-size process table (of PCBs), and use a pointer
 *   to keep track of which entry is currently executing, and
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to complete.
 */

#define maxNumOfProcs 100

//'current' is address of current process
pcb_t pcb[maxNumOfProcs];
pcb_t *current = NULL;
int finalElem = 0, timeforProcRemaining = 0, newest, hasBeenReset = 0;
file fdTable[maxNumOfProcs];
kPipe pipeList[10];
int lastPipe =0;

int findFDElem() {
    for(int i = 3; i < maxNumOfProcs; i++) {
        if(fdTable[i].inUse == 0) {
            return i;
        }
    }
    return -1;
}

int assignPipe(kPipe *p) {
    int x = findFDElem();
    memset(&fdTable[x], 0, sizeof(file));
    fdTable[x].inUse = 1;
    fdTable[x].pipe = p;
    return x;
}

void setNextProc(ctx_t* ctx, int currentX, int next) {
    if(currentX == next) { // if it's the same, doesn't need a context switch
        timeforProcRemaining = current->priority;
        return;
    }
    memcpy(&pcb[currentX].ctx, ctx, sizeof(ctx_t));
    memcpy(ctx, &pcb[next].ctx, sizeof(ctx_t));
    current = &pcb[next];
    timeforProcRemaining = current->priority;
    return;
}

int getCurrentProcPosition() {
    for(int i = 0; i <= maxNumOfProcs; i++) {
        if (current == &pcb[i]) {
            return i;
        }
    }
    return -1;
}

int getNextSlot() {
    for(int i = 0; i < maxNumOfProcs; i++) {
        if(pcb[i].priority == 0) {
            return i;
        }
    }
    return -1;
}

void scheduler(ctx_t* ctx) {
    if(timeforProcRemaining > 1) {
        timeforProcRemaining = timeforProcRemaining-1;
        return;
    }
    int pos = getCurrentProcPosition();
    int x = 1;
    while(pcb[pos+x].priority == 0 && pos+x < maxNumOfProcs) {

        x = x+1;
    }
    if((pos+x) == maxNumOfProcs) {
        setNextProc(ctx, pos, 0);
        return;
    }
    setNextProc(ctx, pos, pos+x);

    return;
}

extern void     receiverMain();
extern void     senderMain();
extern void     main_P3();
extern uint32_t tos_P3;
extern void     main_P4();
extern uint32_t tos_P4;
extern void     main_P5();
extern uint32_t tos_P5;
extern void     main_console();
extern uint32_t tos_console;
extern uint32_t tos_processSpace;

void hilevel_handler_rst(ctx_t* ctx) {
  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack.
   */

   TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
   TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit timer
   TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
   TIMER0->Timer1Ctrl |= 0x00000020; // enable timer interrupt
   TIMER0->Timer1Ctrl |= 0x00000080; // enable timer

   GICC0->PMR          = 0x000000F0; // unmask all interrupts
   GICD0->ISENABLER1  |= 0x00000010; // enable timer interrupt
   GICC0->CTLR         = 0x00000001; // enable GIC interface
   GICD0->CTLR         = 0x00000001; // enable GIC distributor

    memset(&pcb[0], 0, sizeof(pcb_t));
    pcb[0].pid = 1;
    pcb[0].ctx.cpsr = 0x50;
    pcb[0].priority = 1;
    pcb[0].ctx.pc = (uint32_t)(&main_console);
    pcb[0].ctx.sp = (uint32_t)(&tos_processSpace);
    pcb[0].tos = (uint32_t)(&tos_processSpace);
    timeforProcRemaining = 1;
    /* Once the PCBs are initialised, we (arbitrarily) select one to be
    * restored (i.e., executed) when the function then returns.
    */
    current = &pcb[0];
    PL011_putc(UART0, 'X', true);
    finalElem = 0;
    memcpy(ctx, &current->ctx, sizeof(ctx_t));
    int_enable_irq();
    hasBeenReset = 1;
    return;
}


void hilevel_handler_irq(ctx_t *ctx) {
  // Step 2: read  the interrupt identifier so we know the source.
  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.
  if(id == GIC_SOURCE_TIMER0) {
      scheduler(ctx);
      PL011_putc(UART0, ' ', true);
      TIMER0->Timer1IntClr = 0x01;
  }

  // Step 5: write the interrupt identifier to signal we're done.
  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */
    switch(id) {
        case 0x00 : {                    // 0x00 => yield()
            scheduler(ctx);
            break;
        }
        case 0x01 : {                   //write( fd, x, n )
            int fd = (int)(ctx->gpr[0]);
            char* x = (char*)(ctx->gpr[1]);
            int n = (int)(ctx->gpr[2]);
            //writing to stdout
            if(fd == 1) {
                for(int i=0; i < n; i++) {
                    PL011_putc(UART0, *x++, true);
                }
            }
            //writing to stderr
            else if(fd == 2) {
                char *str = "STDERR On write of string: ";
                for(int i=0; i < 27; i++) {     
                    PL011_putc(UART0, *str++, true);
                }
                for(int i=0; i < n; i++) {
                    PL011_putc(UART0, *x++, true);
                }
            }
            else if(!fdTable[fd].writeEnd) {
                char *str = "STDERR write on wrong end of pipe\n";
                for(int i=0; i < 35; i++) {     
                    PL011_putc(UART0, *str++, true);
                }
            }

            //write to pipe
            else if(fdTable[fd].pipe->writable) {
                memset((char*)fdTable[fd].pipe->data, 0, sizeof(fdTable[fd].pipe->data));
                memcpy((char*)fdTable[fd].pipe->data, x, n);
                fdTable[fd].pipe->size = n;
                fdTable[fd].pipe->writable = 0;
            }
           
            ctx->gpr[0] = n;
            break;
        }
        case 0x02 : {                   //read
            //for loop data up to size of buffer
            int fd = (int)(ctx->gpr[0]);
            int n = fdTable[fd].pipe->size;
            if(fdTable[fd].pipe->writable) {
                ctx->gpr[0] = 1;
                break;
            }

            if(fdTable[fd].pipe->size > (int)(ctx->gpr[2])){
                char *str = "STDERR data greater than read buffer";
                for(int i=0; i < 36; i++) {
                    PL011_putc(UART0, *str++, true);
                }
            }
            //place in register
            
            memcpy((char*)ctx->gpr[1], fdTable[fd].pipe->data, (int)(ctx->gpr[2]));
            fdTable[fd].pipe->writable = 1;
            ctx->gpr[0] = 0;
            break;
        }
        case 0x03 : {                   //fork
            //create child with unique Pid
            //copy of parent address space and resources
            int childSlot = getNextSlot();
            if(childSlot > finalElem) {
                finalElem = childSlot;
            }
            memset(&pcb[childSlot], 0, sizeof(pcb_t));
            memcpy(&pcb[childSlot].ctx, ctx, sizeof(ctx_t));

            //give child tos
            uint32_t TOSS = (uint32_t)(&tos_processSpace) - (uint32_t)(0x00001000*(childSlot));
            pcb[childSlot].tos = TOSS;
            //find sp offset for child
            uint32_t spOffset = ((uint32_t)(current->tos)-(uint32_t)(ctx->sp));
            //copy stack into child process space
            memcpy(TOSS-0x00001000, current->tos-0x00001000, 0x00001000);

            pcb[childSlot].pid = childSlot+1;
            pcb[childSlot].ctx.cpsr = 0x50;
            pcb[childSlot].ctx.pc = ctx->pc;
            //set child sp offset
            pcb[childSlot].ctx.sp = (uint32_t)(&tos_processSpace) - ((0x00001000*(childSlot))+spOffset);
            pcb[childSlot].priority = (uint32_t) ctx->gpr[0];
            newest = childSlot;
            //return value of 0 for child
            ctx->gpr[0] = pcb[childSlot].pid;
            pcb[childSlot].ctx.gpr[0] = 0;
            break;
        }
        case 0x04 : {                   //exit
            current->priority = 0;
            timeforProcRemaining = 0;
            scheduler(ctx);
            break;
        }
        case 0x05 : {                   //exec
            memset(&pcb[newest].tos-0x00001000, 0, 0x00001000);
            pcb[newest].ctx.sp = pcb[newest].tos;
            ctx->pc = (uint32_t) ctx->gpr[0];
            break;
        }
        case 0x07 : {                   //Pipe
            PL011_putc(UART0, 'P', true);

            //create pipe structure

            memset(&pipeList[lastPipe], 0, sizeof(kPipe));
            pipeList[lastPipe].writable = 1;

            //find empty elem in fdTable
            //and assign pointer to pipe
            int fd1 = assignPipe(&pipeList[lastPipe]);
            fdTable[fd1].writeEnd = 0;
            int fd2 = assignPipe(&pipeList[lastPipe]);
            fdTable[fd2].writeEnd = 1;
            lastPipe = lastPipe+1;
            //return pipe locations, 1 is read, 2 is write
            ctx->gpr[0] = fd1;
            ctx->gpr[1] = fd2;
            break;
        }
        default : { // 0x?? => unknown/unsupported
            break;
        }
    }
    return;
}
