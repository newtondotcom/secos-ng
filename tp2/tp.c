/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>

void bp_handler() {
   // TODO
}

void bp_trigger() {
	// TODO
}

void tp() {
	// TODO print idtr
	idt_reg_t idtr;
	get_idtr(idtr);
	debug("%d\n",(unsigned int)idtr.addr);

	// TODO call bp_trigger
   //bp_trigger();
}
