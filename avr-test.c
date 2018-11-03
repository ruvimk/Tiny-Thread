#include <avr/interrupt.h>
#include "thread.h"

void test1 (void) {
	while (1) {
		TT_CLI ();
		PORTB = (PORTB & ~3) | BIT (1);
		TT_STI ();
		tt_sleep_ticks (5 << 8);
	}
}

int main (void) {
	DDRE |= BIT (5) | BIT (6) | BIT (7);
	DDRB = 0xFF;
	tt_init ();
	void * stack1 [TT_MIN_STACK_SIZE / sizeof (void *)];
	TT_THREAD thread1 = {
		tt_prepare_stack (stack1, sizeof (stack1), test1),
		TT_PRIORITY_NORMAL,
		0, // Immediately.
		0
	};
	tt_add_thread (&thread1);
	sei ();
	while (1) {
		TT_CLI ();
		PORTB &= ~(BIT (0) | BIT (1));
		PORTE ^= BIT (6);
		PORTE ^= BIT (6);
		TT_STI ();
		tt_sleep_ticks (5 << 8);
	}
	return 0;
}



