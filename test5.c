#include <stdio.h> 

//#define tt_us_to_ticks(us) ((TICK_COUNT) us * 256 / TT_HW_CLOCK_PERIOD) 
//#define tt_ms_to_ticks(ms) (tt_us_to_ticks ((TICK_COUNT) ms * 1000)) 

#define TT_CLOCK_RANGE ((TICK_COUNT) 0x3d00c) 

//#define TT_DEBUG_USE_PRINTF 1 

#include "thread.h" 

// Test 5: so busy, clock wraps before a thread gets to resume from a sleep - its ready_at expires? 

TT_THREAD * pThreadMain; 
TT_THREAD * pThread1; 
TT_THREAD * pThread2; 

// Thread 1: Low-priority thread that sleeps for one second. 
void test1 () { 
	TICK_COUNT until = TT_CLOCK_RANGE - 8; 
	printf ("\tThread 1: attempting to sleep until %d ... \n", until); 
	DWORD ms0 = GetTickCount (); 
	tt_sleep_until (until); 
	DWORD ms1 = GetTickCount (); 
	DWORD diff = ms1 - ms0; 
	printf ("\tThread 1: now tick count: %d, elapsed clock wrap-arounds: %d \n", tt_get_tick_count (), tt_clock_wrap_count); 
} 

// Thread 2: Has higher priority, and works very busy starting just before thread 1 can execute, and ending just after clock wrap-around. 
void test2 () { 
	TICK_COUNT until = TT_CLOCK_RANGE / 2; 
	printf ("\t\tThread 2: waiting until %d tick count, so we can jump thread 1 just in time ... \n", until); 
	DWORD ms0 = GetTickCount (); 
	tt_sleep_until (until); 
	DWORD ms1 = GetTickCount (); 
	DWORD diff = ms1 - ms0; 
	printf ("\t\tThread 2: now doing extensive work ... \n", diff / 1000, diff / 100 % 10); 
	TICK_COUNT started = tt_tick_count = TT_CLOCK_RANGE - 4; // Just start from just before wrap-around. 
	while (tt_get_tick_count () >= started) { 
		// Before wrap-around of the tick count clock, let's do lots of work ... 
		DWORD was = GetTickCount (); 
		DWORD now; 
		do { 
			now = GetTickCount (); 
		} while (now - was < 50); 
		tt_dummy_isr (); // Interrupts happen, ticking the clock, but actually this thread has higher priority, so ISR keeps returning here. 
	} 
	DWORD ms2 = GetTickCount (); 
	diff = ms2 - ms0; 
	printf ("\t\tThread 2: tick count: %d, clock range: %d, clock wraps: %d; our job is done, so we're exiting the thread! \n", tt_get_tick_count (), TT_CLOCK_RANGE, tt_clock_wrap_count); 
} 

int main (int argc, char * argv []) { 
	DWORD ms0 = GetTickCount (); 
	tt_init (); 
	pThreadMain = tt_get_current_thread (); 
	TT_THREAD thread1; 
	uint8_t stack1 [8192]; 
	thread1.t_sp = tt_prepare_stack (stack1, sizeof (stack1), test1); 
	thread1.priority = TT_PRIORITY_NORMAL; 
	thread1.ready_at = 0; // Ready immediately. 
	thread1.waiting_for = 0; // Not waiting for any shared resource. 
	thread1.next_thread = 0; 
	tt_add_thread (pThread1 = &thread1); 
	TT_THREAD thread2; 
	uint8_t stack2 [8192]; 
	thread2.t_sp = tt_prepare_stack (stack2, sizeof (stack2), test2); 
	thread2.priority = TT_PRIORITY_NORMAL - 1; 
	thread2.ready_at = 0; // Ready immediately. 
	thread2.waiting_for = 0; 
	thread2.next_thread = 0; 
	tt_add_thread (pThread2 = &thread2); 
	printf ("Time: %g\n", 0.0f); 
	tt_wait_for_all_finish (); 
	// float now = (float) ((tt_get_tick_count () - count0) / tt_ms_to_ticks (10)) / 100.0f; 
	DWORD msNow = (GetTickCount () - ms0) / 1000; 
	// printf ("Final Clock Value: %g\n", now); 
	printf ("Time: ~%d s\n", msNow); 
	return 0; 
} 

