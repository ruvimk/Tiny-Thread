#include "thread.h"

TT_MUTEX mtx_leds; 

TT_THREAD thread1; 
uint8_t thread1_stack [TT_MIN_STACK_SIZE + 32]; 
void thread1_code (void) {
	while (1) {
		tt_mutex_lock (&mtx_leds); 
		PORTB = (PORTB & ~3) | (1<<1);
		tt_mutex_unlock (&mtx_leds); 
		tt_sleep_ms (300);
	}
}

int main (void) {
	DDRB = 0xFF;
	tt_init ();
	thread1 = {
		tt_prepare_stack (thread1_stack, 
						  sizeof (thread1_stack), 
						  thread1_code), 
		TT_PRIORITY_NORMAL, 
		0, 0, 0 
	};
	tt_add_thread (&thread1);
	sei ();
	while (1) {
		tt_mutex_lock (&mtx_leds); 
		PORTB &= ~3;
		tt_mutex_unlock (&mtx_leds); 
		tt_sleep_ms (500); 
	}
	return 0;
}



