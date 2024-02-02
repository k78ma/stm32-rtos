
#include "main.h"
#include "kernel.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define THREAD_STACK_SIZE 0x400
#define YIELD 10

// global variables
uint32_t* MSP;
uint32_t* stackptr;
uint32_t numThreadsRunning;
uint32_t MAX_THREADS;
//thread KERNEL_THREAD;

// lab 4
thread threadArray[9];
uint32_t currentThread;

extern void runFirstThread(void);

// system calls
void runThread(void) {
	__asm("SVC #1");
}

void osYield(void) {
	__asm("SVC 10");
}

// function for initializing kernel globals
void osKernelInitialize() {

	MSP = *(uint32_t**)0x0;
	numThreadsRunning = 0;
	MAX_THREADS = 10;

	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
}

// function for starting kernel
void osKernelStart() {
	runThread();
}

// function for managing stack pointers
uint32_t* get_next_thread_stack() {
	if (numThreadsRunning == MAX_THREADS)
		return NULL;

	return (uint32_t*)((uint32_t)MSP - (numThreadsRunning+1)*THREAD_STACK_SIZE);
}

bool osCreateThread(void(*thread_function)(void*)) {

	// allocate a new stack, return false if not possible
	stackptr = get_next_thread_stack();
	if (stackptr == NULL)
		return 0;


	// initialize xPSR and PC
	*(--stackptr) = 1<<24; // xPSR

	// PC needs to be set to the thread's stack pointer
	*(--stackptr) = (uint32_t)thread_function;

	// setting the stack to the correct value
	for (int a = 0; a < 14; a++) {
		*(--stackptr) = 0xA;
	}

	// update the thread metadata array
	threadArray[numThreadsRunning].sp = stackptr;
	threadArray[numThreadsRunning].thread_function = thread_function;


	// increment global thread count
	numThreadsRunning++;

	// return true upon completion
	return 1;

}

// Round-robin scheduler
void osSched() {
	threadArray[currentThread].sp = (uint32_t*)(__get_PSP() - 8*4);
	currentThread = (currentThread+1)%numThreadsRunning;
	__set_PSP((uint32_t)threadArray[currentThread].sp);
	return;
}


// SVC Handler
void SVC_Handler_Main(unsigned int *svc_args) {

	unsigned int svc_number;

	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/

	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number ) {
		case 1:
			__set_PSP((uint32_t)threadArray[currentThread].sp);
			runFirstThread();
			break;

		case YIELD:

			// Pend an interrupt to do the context switch
			_ICSR |= 1<<28;
			__asm("isb");
			break;


		case 17: //17 is sort of arbitrarily chosen
			printf("Success!\r\n");
			break;
		case 18:
			printf("Test\r\n");
			break;
		default: /* unknown SVC */
			break;
	}
}
