#include <stdio.h> 

// Set the overflow to a very small amount of time, so we only have to wait <1s for this to happen, so we can test it. Do this #define *before* including thread.h, of course! 
#define TT_CLOCK_RANGE (((TICK_COUNT) 1) << 19) 

#include "thread.h" 

// Test 3: test the timing and clock overflow of threads 

TT_THREAD * pThreadMain; 
TT_THREAD * pThread1; 

TT_MUTEX mutex_a; 

void test1 () { 
	size_t i; 
	for (i = 0; i < 10; i++) { 
		printf ("Thread 1: Blink ON\n"); 
		tt_sleep_ms (500); 
		printf ("Thread 1: Blink OFF\n"); 
		tt_sleep_ms (500); 
	} 
} 

int main (int argc, char * argv []) { 
	size_t i; 
	tt_init (); 
	// TICK_COUNT count0 = tt_get_tick_count (); 
	DWORD ms0 = GetTickCount (); 
	pThreadMain = tt_get_current_thread (); 
	TT_THREAD thread1; 
	mutex_a.taken_by = 0; 
	uint8_t stack1 [8192]; 
	thread1.t_sp = tt_prepare_stack (stack1, sizeof (stack1), test1); 
	thread1.priority = TT_PRIORITY_NORMAL; 
	thread1.ready_at = 0; // Ready immediately. 
	thread1.waiting_for = 0; // Not waiting for any shared resource. 
	thread1.next_thread = 0; 
	tt_add_thread (pThread1 = &thread1); 
	printf ("Time: %g\n", 0.0f); 
	tt_wait_for_all_finish (); 
	// float now = (float) ((tt_get_tick_count () - count0) / tt_ms_to_ticks (10)) / 100.0f; 
	DWORD msNow = (GetTickCount () - ms0) / 1000; 
	// printf ("Final Clock Value: %g\n", now); 
	printf ("Time: ~%d s\n", msNow); 
	return 0; 
} 

