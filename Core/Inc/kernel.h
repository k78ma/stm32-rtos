
#ifndef INC_KERNEL_H_
#define INC_KERNEL_H_
#endif /* INC_KERNEL_H_ */

// setting registers for PendSV priority
#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV


#include <stdint.h>
#include <stdbool.h>

// KERNEL FUNCTION PROTOTYPES
void SVC_Handler_Main(unsigned int *svc_args);
bool osCreateThread(void(*thread_function)(void*));
void osKernelInitialize();
void osKernelStart();
void osYield(void);


// STRUCT DEFINITIONS
typedef struct k_thread{
	uint32_t* sp; //stack pointer
	void (*thread_function)(void*); //function pointer
} thread;
