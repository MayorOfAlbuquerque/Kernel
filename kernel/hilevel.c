#include "hilevel.h"

/* Since we *know* there will be 2 processes, stemming from the 2 user
 * programs, we can
 *
 * - allocate a fixed-size process table (of PCBs), and use a pointer
 *   to keep track of which entry is currently executing, and
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to complete.
 */


//'current' is address of current process
pcb_t pcb[100];
pcb_t *current = NULL;
int finalElem = 0;

void setNextProc(ctx_t* ctx, int currentX, int next) {
    if(currentX == next) { // if it's the same, doesn't need a context switch
        return;
    }
    memcpy(&pcb[currentX].ctx, ctx, sizeof(ctx_t));
    memcpy(ctx, &pcb[next].ctx, sizeof(ctx_t));
    current = &pcb[next];
}

int getCurrentProcPosition() {
    for(int i = 0; i < sizeof(pcb)/sizeof(pcb_t); i++) {
        if (current == &pcb[i]) {
            return i;
        }
    }
}

void swapProcessesInPCBTable(int proc1, int proc2) {
    pcb_t temp = pcb[proc1];
    pcb[proc2] = pcb[proc1];
    pcb[proc1] = temp;
    return;
}

void sortPCBTable() {
    for(int x = 1; x < finalElem; x++) {
        int insertElem = x;
        while(insertElem > 0 && pcb[insertElem].priority > pcb[insertElem-1].priority) {
            swapProcessesInPCBTable(insertElem, insertElem-1);
            insertElem = insertElem -1;
        }
    }
}

void scheduler(ctx_t* ctx) {
    int pos = (current->pid)-1;
    if(pos == finalElem) {
        setNextProc(ctx, pos, 0);
    }
    else {
        setNextProc(ctx, pos, pos+1);
    }

/*
    int pos = (current->pid)-1;
    int nextPos = pos+1;

    while(nextPos <= finalElem) {
        if(!pcb[nextPos].deleted) { // if delete = false
            setNextProc(ctx, pos, nextPos);
            return;
        }
        nextPos = nextPos +1;
    }
    setNextProc(ctx, pos, 0);*/
}

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

    memset( &pcb[0], 0, sizeof( pcb_t ) );
    pcb[0].pid = 1;
    pcb[0].ctx.cpsr = 0x50;
    pcb[0].deleted = false;
    pcb[0].priority = 100;
    pcb[0].ctx.pc = (uint32_t)(&main_console);
    pcb[0].ctx.sp = (uint32_t)(&tos_console);

    /* Once the PCBs are initialised, we (arbitrarily) select one to be
    * restored (i.e., executed) when the function then returns.
    */

    current = &pcb[0]; memcpy(ctx, &current->ctx, sizeof(ctx_t));
    int_enable_irq();

    return;
}


void hilevel_handler_irq(ctx_t *ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if(id == GIC_SOURCE_TIMER0) {
      scheduler(ctx);
      PL011_putc( UART0, ' ', true );
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
        case 0x00 : { // 0x00 => yield()
            scheduler(ctx);
            break;
        }
        case 0x01 : { // 0x01 => write( fd, x, n )
            int fd = (int)(ctx->gpr[0]);
            char* x = (char*)(ctx->gpr[1]);
            int n = (int)(ctx->gpr[2]);

            for(int i=0; i < n; i++) {
                PL011_putc(UART0, *x++, true);
            }

            ctx->gpr[0] = n;
            break;
        }
        case 0x03 : {                   //fork
            //create child with unique Pid
            //copy of parent address space and resources
            int childSlot = finalElem+1;
            finalElem = childSlot;
            memset(&pcb[childSlot], 0, sizeof(pcb_t));
            memcpy(&pcb[childSlot], &current->ctx, sizeof(ctx_t));

            pcb[childSlot].pid = childSlot+1;
            pcb[childSlot].ctx.cpsr = 0x50;
            pcb[childSlot].deleted = 0;
            pcb[childSlot].ctx.pc = (uint32_t)(&main_console);
            pcb[childSlot].ctx.sp = (uint32_t)(&tos_processSpace) + (1000*(childSlot+1));
            pcb[childSlot].priority = (uint32_t) ctx->gpr[0];
            //return value of 0 for child
            ctx->gpr[0] = 0;
            pcb[childSlot].ctx.gpr[0] = childSlot+1;

            sortPCBTable();
            break;
        }
        case 0x04 : {                   //exit
            current->deleted = true;
            current->priority = 0;
            finalElem = finalElem -1;
            sortPCBTable();
            break;
        }
        case 0x05 : {                   //exec
            int procToExec = (uint32_t) ctx->gpr[0];
            ctx->pc = (uint32_t) procToExec;
            break;
        }
        default : { // 0x?? => unknown/unsupported
            break;
        }
    }
    return;
}
