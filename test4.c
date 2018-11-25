#include <stdio.h> 

#include "thread.h" 

// Test 4: mutex locks and threads sleeping 

TT_THREAD * pThreadMain; 
TT_THREAD * pThread1; 
TT_THREAD * pThread2; 

TT_MUTEX mutex_a; 

// Thread 1: Locks a mutex that thread 2 needs, and then sleeps for 2 seconds before unlocking it. 
void test1 () { 
	printf ("\tThread 1: attempting to lock mutex ... \n"); 
	tt_mutex_lock (&mutex_a); 
	printf ("\tThread 1: mutex locked; sleeping 2 seconds ... \n"); 
	DWORD prev = GetTickCount (); 
	tt_sleep_ms (2000); 
	printf ("\tThread 1: Done sleeping. %d seconds passed. \n", (GetTickCount () - prev) / 1000); 
	tt_mutex_unlock (&mutex_a); 
	printf ("\tThread 1: done! \n"); 
} 

// Thread 2: Has higher priority, and needs access to the mutex. However, sleeps a little to let thread 1 take it first, just for this test case. 
void test2 () { 
	printf ("Thread 2: waiting 100 ms ... \n"); 
	tt_sleep_ms (100); 
	printf ("Thread 2: attempting to lock mutex ... \n"); 
	tt_mutex_lock (&mutex_a); 
	printf ("Thread 2: mutex locked. \n"); 
	tt_mutex_unlock (&mutex_a); 
	printf ("Thread 2: done! \n"); 
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

