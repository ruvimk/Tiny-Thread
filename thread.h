

#ifdef __DMC__ 
// For some reason Digital Mars C compiler does not have this predefined, so: 
typedef unsigned int uint32_t; 
typedef unsigned char uint8_t; 
#else 
// For some reason this works on GCC, but makes the Digital Mars C compiler break, so we only do this while NOT on DMC: 
typedef unsigned short size_t; 
#endif 

struct TT_THREAD_STRUCT; 
struct TT_MUTEX_STRUCT; 

// Structure that defines information about a thread's context:
struct TT_THREAD_STRUCT {
	void ** t_sp; // Stack Pointer.
	size_t priority; // 0 is the highest priority.
	uint32_t ready_at; // Tick count at which this thread is ready.
	struct TT_MUTEX_STRUCT * waiting_for; 
	struct TT_THREAD_STRUCT * next_thread; // Pointer to the next thread, or nullptr.
};
typedef struct TT_THREAD_STRUCT TT_THREAD;

// Shared resource access synchronization: 
struct TT_MUTEX_STRUCT { 
	TT_THREAD * taken_by; 
}; 
typedef struct TT_MUTEX_STRUCT TT_MUTEX; 

// Some declarations:
void tt_init (void); // Initialize this scheduler - call at the beginning of main (). 
void tt_add_thread (TT_THREAD * thread_info); // Add a thread to the queue. To start a thread, simply fill out a TT_THREAD structure, and call this function. 
void tt_remove_thread (TT_THREAD * thread_info); // Remove a thread from the queue. 
void * tt_prepare_stack (uint8_t * stack_begin_address, // Convenience function to help fill out the t_sp member of a TT_THREAD structure. 
						size_t stack_size_bytes, 
						void * code_start_address); 
void tt_yield (void); // Yield: let the next thread in the queue take over the CPU without suspending this thread. 
void tt_sleep_ticks (uint32_t ticks); // Suspend this thread for 'ticks' TCNT0 clocks. 
TT_THREAD * tt_get_current_thread (void); // Returns this thread's TT_THREAD *. 
void tt_suspend_thread (TT_THREAD * thread_info); // Suspends some thread indefinitely. 
void tt_wake_thread (TT_THREAD * thread_info); // Wakes up another thread. 
void tt_suspend_me (void); // Suspends this thread indefinitely. 
void tt_suspend_until_threads_change (TT_THREAD * thread_info); // Suspends a thread until one of the other threads exit. 
void tt_suspend_me_until_threads_change (void); // Suspends this thread until one of the other threads exit. 
void tt_wait_for_all_finish (void); // Keeps suspending this thread until all other threads exit. 
void tt_exit_thread (void); // Exits this thread. Note: another way to exit a thread is simply to 'return' from its thread function. 

#define TT_THREAD_MAIN &tt_obj_main_thread 
#define TT_THREAD_IDLE &tt_obj_idle_thread 

// The following three macros may be defined (BEFORE including thread.h) by the C program in 
// order to control what happens just before a thread switch is done. For example, check IO? 

// On timer up: executed from the ISR(TIMER0_OVF_vect), just before task switch. 
#ifndef TT_ONTIMERUP 
#define TT_ONTIMERUP() ; 
#endif 

// On thread yield: executed from yield (), just before task switch. 
#ifndef TT_ONTHREADYIELD 
#define TT_ONTHREADYIELD() ; 
#endif 

// On task switch: executed after ONTIMERUP or after ONTHREADYIELD, but before task switch in either case. 
#ifndef TT_ONTASKSWITCH 
#define TT_ONTASKSWITCH() ; 
#endif 


#ifndef BIT
#define BIT(n) (1<<n)
#endif

#ifndef TT_HW_CLOCK_SETUP 
// Use prescaler of 8, which results in a clock period of 128 us: 
#define TT_HW_CLOCK_SETUP (BIT (CS01)) 
#define TT_HW_CLOCK_PERIOD 128 
#endif 

// Brief descriptions of sizes:

// TT_STACK_PROGRAM_OVERHEAD: extra space on the stack for tt_exit_thread,
//    for the thread entry point address, and for an extra return address
// TT_STACK_STATE_SAVE_SIZE: space on the stack for state other than entry/return addresses
// TT_STACK_TOTAL_OVERHEAD: total number of bytes minimum needed on the stack per thread;
//    this is the minimum and does not factor in function calls that the thread does

// TT_REGISTER_COUNT: the number of registers we have to save/restore for each thread

#ifdef __DMC__
	// Macro definitions for the Digital Mars C compiler
	#ifdef WIN32
		// 32-bit Windows
		#include <windows.h> 
		uint32_t tt_get_tick_count (void) { 
			return GetTickCount (); // Just call the Win32 API function. 
		} 
		
		#define TT_STACK_PROGRAM_OVERHEAD (3 * sizeof (void *))
		#define TT_REGISTER_COUNT 8
		#define TT_STACK_STATE_SAVE_SIZE (4 * TT_REGISTER_COUNT + 4)
		#define TT_STACK_TOTAL_OVERHEAD (TT_STACK_STATE_SAVE_SIZE + TT_STACK_PROGRAM_OVERHEAD)
		
		#define TT_SAVE_ALL() asm pushfd asm pushad
		#define TT_RESTORE_ALL() asm popad asm popfd
		
		#define TT_GET_SP() asm mov eax, esp
		#define TT_SET_SP() asm mov esp, eax
		
		#define __TT_CALL_FIND_NEXT_THREAD() asm call __tt_find_next_thread
		#define __TT_SET_CURRENT_THREAD() asm mov dword ptr [tt_current_thread], eax
		#define __TT_SAVE_CURRENT_THREAD_SP() \
			asm mov ebx, dword ptr [tt_current_thread] \
			asm mov [ebx], eax
		#define __TT_RETRIEVE_NEXT_THREAD_SP() \
			asm mov ebx, eax \
			asm mov eax, [ebx]
	#else
		// 16-bit DOS or OS/2
		// TODO: Perhaps implement these. If we want to run threads on DOS, anyway. 
	#endif
	#define TT_RESET_CLOCK()
	#define TT_RET() asm ret
	#define TT_IRET() TT_RET ()
	#define TT_CLI() ;
	#define TT_STI() ;
#else
#ifdef __AVR__
	#include <avr/io.h>
	#include <avr/interrupt.h>
	
	volatile uint32_t tt_tick_count;
	uint32_t tt_get_tick_count (void) {
		return tt_tick_count + TCNT0;
	}
	#define TT_RESET_CLOCK() tt_tick_count += TCNT0; TCNT0 = 0
	#define TT_SLEEP() asm volatile ("sleep" ::: "memory")
	#define TT_RET() asm volatile ("ret")
	#define TT_IRET() asm volatile ("reti")
	#define TT_CLI() asm volatile ("cli" ::: "memory")
	#define TT_STI() asm volatile ("sei" ::: "memory")
	
	#define TT_STACK_PROGRAM_OVERHEAD (3 * sizeof (void *))
	#define TT_REGISTER_COUNT 32
	#define TT_STACK_STATE_SAVE_SIZE (1 * TT_REGISTER_COUNT + 2)
	#define TT_STACK_TOTAL_OVERHEAD (TT_STACK_STATE_SAVE_SIZE + TT_STACK_PROGRAM_OVERHEAD)
	
	#define TT_SAVE_ALL() __asm__ __volatile__ ("push r1\npush r0\nin r0, %[p]\npush r0\nin r0, %[s]\npush r0\n	push r2\npush r3\npush r4\npush r5\npush r6\npush r7\npush r8\npush r9\npush r10\npush r11\npush r12\npush r13\npush r14\npush r15\npush r16\npush r17\npush r18\npush r19\npush r20\npush r21\npush r22\npush r23\npush r24\npush r25\npush r26\npush r27\npush r28\npush r29\npush r30\npush r31" :: [s] "I" (_SFR_IO_ADDR(SREG)), [p] "I" (_SFR_IO_ADDR(RAMPZ)) :"memory")
	#define TT_RESTORE_ALL() __asm__ __volatile__ ("pop r31\npop r30\npop r29\npop r28\npop r27\npop r26\npop r25\npop r24\npop r23\npop r22\npop r21\npop r20\npop r19\npop r18\npop r17\npop r16\npop r15\npop r14\npop r13\npop r12\npop r11\npop r10\npop r9\npop r8\npop r7\npop r6\npop r5\npop r4\npop r3\npop r2\n	pop r0\nout %[s], r0\npop r0\nout %[p], r0\n	pop r0\npop r1" :: [s] "I" (_SFR_IO_ADDR(SREG)), [p] "I" (_SFR_IO_ADDR(RAMPZ)) :"memory")

	#define TT_GET_SP() __asm__ __volatile__ ("in r22, %[l]\nin r23, %[h]\n" :: [l] "I" (_SFR_IO_ADDR(SPL)), [h] "I" (_SFR_IO_ADDR(SPH)) : "r22", "r23", "memory")
	#define TT_SET_SP() __asm__ __volatile__ ("out %[h], r23\nout %[l], r22\n" :: [l] "I" (_SFR_IO_ADDR(SPL)), [h] "I" (_SFR_IO_ADDR(SPH)) : "r22", "r23", "memory")
	
	#define __TT_CALL_FIND_NEXT_THREAD() __tt_find_next_thread ()
	#define __TT_SET_CURRENT_THREAD() __asm__ __volatile__ ("st %a[curr]+, r24\nst %a[curr], r25\n" :: [curr] "e" (&tt_current_thread) : "r24", "r25", "memory")
	#define __TT_SAVE_CURRENT_THREAD_SP() __asm__ __volatile__ ("st %a[tt]+, r22\nst %a[tt], r23\n" :: [tt] "e" (tt_current_thread) : "r22", "r23", "memory")
	#define __TT_RETRIEVE_NEXT_THREAD_SP() __asm__ __volatile__ ("ld r22, %a[tt]+\nld r23, %a[tt]\n" :: [tt] "e" (tt_current_thread) : "r22", "r23")
#else
	// TODO: Implement a port for ARM, perhaps? 
#endif
#endif

#define TT_SAVE() TT_SAVE_ALL ()
#define TT_RESTORE() TT_RESTORE_ALL ()

#define TT_PRIORITY_TOP 0
#define TT_PRIORITY_NORMAL 0x80
#define TT_PRIORITY_BOTTOM 0xFF

#define TT_READY_NOW 0
#define TT_READY_SUSPENDED -1
#define TT_READY_ONTHREADEXIT -2
#define TT_READY_MAXTIME -2

// For minimum stack size, we give 16 extra bytes of leeway in case
// the thread needs to call a function, such as tt_exit_thread (), etc.
#define TT_MIN_STACK_SIZE (TT_STACK_TOTAL_OVERHEAD + 16)

// Some internal variables for this thread scheduler: 
TT_THREAD * volatile tt_first_thread;
TT_THREAD * volatile tt_current_thread;

TT_THREAD tt_obj_main_thread;
TT_THREAD tt_obj_idle_thread;
#ifdef WIN32
uint8_t tt_idle_thread_stack [TT_MIN_STACK_SIZE + 4096];
#else
uint8_t tt_idle_thread_stack [TT_MIN_STACK_SIZE];
#endif

#ifdef WIN32
	__declspec (naked)
	void __tt_just_hang (void) {
		while (1) {
			Sleep (50);
			tt_yield ();
		}
		printf ("Idle F\n");
	}
#else
	__attribute__ ((naked))
	void __tt_just_hang (void) {
		while (1) {
			TT_SLEEP ();
		}
	}
#endif

//**********************************************************************
// tt_init () - initialize thread scheduler
// Not sure yet what we're going to use for interrupts or whatever
//
void tt_init (void) {
	// Set up thread:
	tt_obj_main_thread.priority = TT_PRIORITY_NORMAL;
	tt_first_thread = tt_current_thread = &tt_obj_main_thread;
	tt_obj_idle_thread.t_sp = tt_prepare_stack (tt_idle_thread_stack,
			sizeof (tt_idle_thread_stack), __tt_just_hang);
	tt_obj_idle_thread.priority = TT_PRIORITY_BOTTOM;
	tt_obj_idle_thread.ready_at = 0;
	tt_obj_idle_thread.next_thread = 0;
	tt_add_thread (&tt_obj_idle_thread);
	// On hardware systems, also do: set up clock, interrupt, and sleep mode.
#ifdef __AVR__
	// Set up clock:
	TCCR0 = TT_HW_CLOCK_SETUP; 
	TIMSK |= BIT (TOIE0); // Enable interrupts on TCNT0 overflow.
	// Enable sleeping (mode 000, Idle) and interrupts:
	MCUCR = (MCUCR & ~(BIT (SM0) | BIT (SM1) | BIT (SM2))) | BIT (SE);
#endif
}

#ifdef __DMC__
	__declspec (naked)
#else
	__attribute__ ((naked))
#endif
void __tt_restore_and_return (void) {
	TT_RESTORE (); 
	TT_STI (); 
	TT_RET ();
}

#define TT_LO8(x) (((uint16_t) x) & 0xFF) 
#define TT_HI8(x) (((uint16_t) x >> 8) & 0xFF) 
#define TT_SWAP_ENDIAN(x) ((TT_LO8 (x) << 8) | TT_HI8 (x)) 

void * tt_prepare_stack (uint8_t * stack_begin_address,
						size_t stack_size_bytes,
						void * code_start_address) {
	#ifdef WIN32
	void ** p = (void **) stack_begin_address;
	size_t s = stack_size_bytes / sizeof (void *); 
#define M_PUSH(x) (p[--s] = x) 
	M_PUSH (tt_exit_thread); 
	M_PUSH (code_start_address); 
		uint32_t t_eflags;
		asm pushfd asm pop dword ptr [t_eflags]
		M_PUSH ((void *) t_eflags); 
		size_t i;
		for (i = 0; i < TT_REGISTER_COUNT; i++) 
			M_PUSH (0); 
		M_PUSH (__tt_restore_and_return); 
		return &p[s]; 
#undef M_PUSH 
	#else
	uint8_t * m_sp = stack_begin_address + stack_size_bytes - 1; 
#define M_PUSH(x) (*m_sp-- = x) 
		M_PUSH (TT_LO8 (tt_exit_thread)); 
		M_PUSH (TT_HI8 (tt_exit_thread)); 
		M_PUSH (TT_LO8 (code_start_address)); 
		M_PUSH (TT_HI8 (code_start_address)); 
		M_PUSH (0); 
		M_PUSH (0); 
		M_PUSH (RAMPZ); 
		M_PUSH (SREG); 
		size_t i; 
		for (i = 0; i < TT_REGISTER_COUNT - 2; i++) 
			M_PUSH (0); 
		M_PUSH (TT_LO8 (__tt_restore_and_return)); 
		M_PUSH (TT_HI8 (__tt_restore_and_return)); 
		return m_sp; 
#undef M_PUSH 
	#endif
}

#ifdef WIN32
// tt_debug () 
// Prints out the current threadpool state. 
void tt_debug (void) {
	TT_THREAD * p = tt_first_thread;
	printf ("Debug (now = %d): \n", tt_get_tick_count ());
	while (p) {
		printf ("Thread; priority: %d; ready: %d;\n", p->priority, p->ready_at);
		p = p->next_thread;
	}
	printf ("\n");
}
#endif

void tt_add_thread (TT_THREAD * thread_info) {
	TT_THREAD * p = tt_first_thread;
	size_t need = thread_info->priority;
	if (need <= p->priority) {
		// This thread should be first; add it at the beginning.
		tt_first_thread = thread_info;
		thread_info->next_thread = p;
		return;
	}
	while (p->next_thread && need > p->next_thread->priority)
		p = p->next_thread;
	if (!p->next_thread) {
		// It stopped because we reached the list end.
		thread_info->next_thread = 0;
		p->next_thread = thread_info;
	} else {
		// It stopped because the next thread is of the same or lower priority as this.
		thread_info->next_thread = p->next_thread;
		p->next_thread = thread_info;
	}
	// tt_debug ();
}

void tt_remove_thread (TT_THREAD * thread_info) {
	TT_THREAD * p = tt_first_thread;
	if (p == thread_info) {
		// This is the first thread; remove it.
		tt_first_thread = p->next_thread;
		p = tt_first_thread;
	}
	while (p) {
		if (p->next_thread == thread_info) {
			// Remove the next thread if its pointer is equal to the thread to remove:
			p->next_thread = thread_info->next_thread;
		}
		if (p->ready_at == TT_READY_ONTHREADEXIT) {
			p->ready_at = 0;
		}
		p = p->next_thread;
	}
	// tt_debug ();
}

// __tt_find_next_thread () - internal function 
// Searches the threadpool for the highest-priority 
// thread to execute next. Only returns the next lower 
// priority if all the highest-priority threads are 
// inactive - in other words, ensures the high-priority 
// threads actually finish their work before letting 
// lower-priority threads do anything. 
TT_THREAD * __tt_find_next_thread (void) {
	// tt_debug ();
	TT_THREAD * p = tt_first_thread;
	uint32_t now = tt_get_tick_count (); 
	TT_THREAD * n = 0; // Next thread. 
	while (p) { 
		if (p->ready_at <= now) { 
			// This is the next thread to execute. 
			if (p->waiting_for) { 
				// Oh, but it's waiting for a shared resource. 
				// Go find the thread that is using that resource! 
				n = p; 
			} else return p; // Not waiting? Go for it! 
		} 
		p = p->next_thread;
	} 
	if ((!p || p->priority > tt_current_thread->priority)
		&& tt_current_thread->ready_at <= now) {
		// No other threads are ready, so keep running this thread.
		return tt_current_thread;
	}
	return p; // Otherwise, sleep or something if p is null (no threads ready).
}

// __tt_task_switch () - internal function 
// Does the actual task switching, the 
// bare-metal work involved with that. 
#ifdef __DMC__
	__declspec(naked)
#else
	__attribute__ ((naked))
#endif
void __tt_task_switch (void) {
	TT_GET_SP ();
	__TT_SAVE_CURRENT_THREAD_SP ();
#ifdef __AVR__
	// This seems to compile into fewer instructions if we just use C:
	tt_current_thread = __tt_find_next_thread ();
#else
	__TT_CALL_FIND_NEXT_THREAD ();
	__TT_SET_CURRENT_THREAD ();
#endif
	__TT_RETRIEVE_NEXT_THREAD_SP ();
	TT_SET_SP ();
	TT_RET ();
}

void tt_yield (void) {
	TT_CLI ();
	TT_RESET_CLOCK ();
	TT_SAVE ();
	TT_ONTHREADYIELD (); 
	TT_ONTASKSWITCH (); 
	__tt_task_switch ();
	TT_RESTORE ();
	TT_STI ();
}

#define tt_us_to_ticks(us) ((uint32_t) us * 256 / TT_HW_CLOCK_PERIOD) 
#define tt_ms_to_ticks(ms) (tt_us_to_ticks ((uint32_t) ms * 1000)) 
void tt_sleep_ticks (uint32_t ticks) {
	tt_current_thread->ready_at = tt_get_tick_count () + ticks;
	tt_yield ();
}
void tt_sleep_us (uint32_t microseconds) { 
	// There are 256 ticks per every CLOCK_PERIOD microseconds: 
	tt_sleep_ticks (tt_us_to_ticks (microseconds)); 
} 
void tt_sleep_ms (uint32_t milliseconds) { 
	tt_sleep_ticks (tt_ms_to_ticks (milliseconds)); 
} 
void tt_sleep_until (uint32_t tick_count) { 
	tt_current_thread->ready_at = tick_count; 
	tt_yield (); 
} 

TT_THREAD * tt_get_current_thread (void) {
	return tt_current_thread;
}

void tt_suspend_thread (TT_THREAD * thread_info) {
	thread_info->ready_at = TT_READY_SUSPENDED;
}
void tt_wake_thread (TT_THREAD * thread_info) {
	thread_info->ready_at = 0; // Ready immediately.
	// If the thread we're waking up is of higher priority than us,
	// then we schedule it to run right away:
	if (thread_info->priority < tt_current_thread->priority)
		tt_yield ();
}

void tt_suspend_me (void) {
	tt_suspend_thread (tt_current_thread);
	tt_yield ();
}

void tt_suspend_until_threads_change (TT_THREAD * thread_info) {
	thread_info->ready_at = TT_READY_ONTHREADEXIT;
}

void tt_suspend_me_until_threads_change (void) {
	tt_suspend_until_threads_change (tt_current_thread);
	tt_yield ();
}

void tt_wait_for_all_finish (void) {
	TT_THREAD * me = tt_get_current_thread ();
	TT_THREAD * idler = &tt_obj_idle_thread;
	size_t other_threads = 0;
	do {
		tt_suspend_me_until_threads_change ();
		TT_THREAD * p = tt_first_thread;
		other_threads = 0;
		while (p) {
			if (p != me && p != idler)
				other_threads++;
			p = p->next_thread;
		}
	} while (other_threads);
}

void tt_exit_thread (void) {
	tt_remove_thread (tt_current_thread);
	tt_suspend_me ();
}

#ifdef __AVR__
ISR(TIMER0_OVF_vect, ISR_NAKED) {
	TT_SAVE ();
	PORTE |= BIT (5);
	tt_tick_count += BIT (8);
	TT_ONTIMERUP (); 
	TT_ONTASKSWITCH (); 
	__tt_task_switch ();
	PORTE &= ~BIT (5);
	TT_RESTORE ();
	TT_IRET ();
}
#endif




