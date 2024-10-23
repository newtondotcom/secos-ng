/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>

void bp_handler() {
	debug("bp_handler");
}

void bp_trigger() {
	__asm__("int $3"); 
}

void tp() {
	idt_reg_t idtr;
	get_idtr(idtr);
	debug("%x\n",(unsigned int)idtr.addr);

   	bp_trigger();
}
