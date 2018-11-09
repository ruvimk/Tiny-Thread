#include <stdio.h> 

#include "thread.h" 

TT_THREAD * pThreadMain; 
TT_THREAD * pThread1; 
TT_THREAD * pThread2; 
TT_THREAD * pThread3; 
TT_THREAD * pThread4; 
TT_THREAD * pThread5; 
TT_THREAD * pThread6; 

TT_MUTEX mutex_a; 

void test1 () { 
	size_t i; 
	for (i = 0; i < 10; i++) { 
		if (i == 7) tt_wake_thread (pThreadMain); 
		printf ("Thread 1\n"); 
		tt_yield (); 
	} 
} 
void test2 () { 
	printf ("\t\tThread 2\n"); 
	tt_suspend_me (); 
	printf ("\t\tThread 2 again!\n"); 
	size_t i; 
	for (i = 0; i < 12; i++) { 
		printf ("\t\tThread 2\n"); 
		if (i == 2) { 
			printf ("\t\tLocking mutex A\n"); 
			tt_mutex_lock (&mutex_a); 
		} 
		if (i == 7) { 
			printf ("\t\tUnlocking mutex A\n"); 
			tt_mutex_unlock (&mutex_a); 
		} 
		tt_yield (); 
	} 
} 
void test3 () { 
	size_t i = 0; 
	for (i = 0; i < 8; i++) { 
		if (i == 4) { 
			tt_wake_thread (pThread2); 
			tt_wake_thread (pThread4); 
		} 
		printf ("\tThread 3\n"); 
		tt_yield (); 
	} 
} 
void test4 () { 
	size_t i = 0; 
	for (i = 0; i < 8; i++) { 
		printf ("\tThread 4\n"); 
		tt_yield (); 
	} 
} 
void test5 () { 
	printf ("Thread 5 start\n"); 
	tt_sleep_ticks (2000); 
	printf ("Thread 5 stop\n"); 
} 

void test6 () { 
	size_t i = 0; 
	for (i = 0; i < 12; i++) { 
		printf ("\tThread 6\n"); 
		if (i == 2) { 
			printf ("\tLocking mutex A\n"); 
			tt_mutex_lock (&mutex_a); 
		} 
		if (i == 8) { 
			printf ("\tUnlocking mutex A\n"); 
			tt_mutex_unlock (&mutex_a); 
		} 
		tt_yield (); 
	} 
} 

int main (int argc, char * argv []) { 
	size_t i; 
	tt_init (); 
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
	thread2.priority = TT_PRIORITY_NORMAL - 4; 
	thread2.ready_at = 0; 
	thread2.waiting_for = 0; 
	thread2.next_thread = 0; 
	tt_add_thread (pThread2 = &thread2); 
	TT_THREAD thread3; 
	uint8_t stack3 [8192]; 
	thread3.t_sp = tt_prepare_stack (stack3, sizeof (stack3), test3); 
	thread3.priority = TT_PRIORITY_NORMAL - 2; 
	thread3.ready_at = 0; 
	thread3.waiting_for = 0; 
	thread3.next_thread = 0; 
	tt_add_thread (pThread3 = &thread3); 
	TT_THREAD thread4; 
	uint8_t stack4 [8192]; 
	thread4.t_sp = tt_prepare_stack (stack4, sizeof (stack4), test4); 
	thread4.priority = thread3.priority; 
	thread4.ready_at = -1; 
	thread4.waiting_for = 0; 
	thread4.next_thread = 0; 
	tt_add_thread (pThread4 = &thread4); 
	TT_THREAD thread5; 
	uint8_t stack5 [8192]; 
	thread5.t_sp = tt_prepare_stack (stack5, sizeof (stack5), test5); 
	thread5.priority = TT_PRIORITY_NORMAL; 
	thread5.ready_at = 0; 
	thread5.waiting_for = 0; 
	thread5.next_thread = 0; 
	tt_add_thread (pThread5 = &thread5); 
	TT_THREAD thread6; 
	uint8_t stack6 [8192]; 
	thread6.t_sp = tt_prepare_stack (stack6, sizeof (stack6), test6); 
	thread6.priority = thread3.priority; 
	thread6.ready_at = 0; 
	thread6.waiting_for = 0; 
	thread6.next_thread = 0; 
	tt_add_thread (pThread6 = &thread6); 
	// tt_debug (); 
	tt_suspend_me (); 
	for (i = 0; i < 12; i++) { 
		printf ("Hello World!\n"); 
		tt_yield (); 
	} 
	tt_wait_for_all_finish (); 
	return 0; 
} 

