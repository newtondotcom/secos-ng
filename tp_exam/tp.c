/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>
#include <cr.h>
#include <pagemem.h>

__attribute__((section(".user"))) void user1() {
    while (1) {
        debug("User 1\n");
    }
}

__attribute__((section(".user"))) void user2() {
    while (1) {
        debug("User 2\n");
    }
}

void init_gdt() {
    
}

void clock_handler() {
    asm volatile ("pusha");
    debug("CLOCK INTERRUPT\n");
    asm volatile ("popa");
    asm volatile ("leave; iret");
}

void init_idt() {
    idt_reg_t idtr;
    get_idtr(idtr);
    debug("IDT @ 0x%x\n", (unsigned int) idtr.addr); 
    int_desc_t *clock_dsc = &idtr.desc[32];
    clock_dsc->offset_1 = (uint16_t)((uint32_t)clock_handler);
    clock_dsc->offset_2 = (uint16_t)(((uint32_t)clock_handler) >> 16);
    asm volatile ("sti");
}

void init_kernel_pgd() {
    debug("--------------------------------\n");
    uint32_t cr3 = get_cr3();
	debug("Valeur courante du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("Initialisation du PGD du noyau à l'adresse physique 0x600000\n");
    pde32_t *pgd = (pde32_t*)0x600000;
	set_cr3((uint32_t)pgd);
    cr3 = get_cr3();
    memset((void*)pgd, 0, PAGE_SIZE);
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("--------------------------------\n");
}

void init_user1_pgd() {
    debug("--------------------------------\n");
    debug("Initialisation du PGD de la tâche user1 à l'adresse physique 0x700000\n");
    pde32_t *pgd = (pde32_t*)0x700000;
	set_cr3((uint32_t)pgd);
    uint32_t cr3 = get_cr3();
    memset((void*)pgd, 0, PAGE_SIZE);
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("Réaffectation du registre CR3 au PGD du noyau...\n");
    set_cr3((uint32_t)(pde32_t*)0x600000);
    cr3 = get_cr3();
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("--------------------------------\n");
}

void init_user2_pgd() {
    debug("--------------------------------\n");
    debug("Initialisation du PGD de la tâche user2 à l'adresse physique 0x800000\n");
    pde32_t *pgd = (pde32_t*)0x800000;
	set_cr3((uint32_t)pgd);
    uint32_t cr3 = get_cr3();
    memset((void*)pgd, 0, PAGE_SIZE);
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("Réaffectation du registre CR3 au PGD du noyau...\n");
    set_cr3((uint32_t)(pde32_t*)0x600000);
    cr3 = get_cr3();
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("--------------------------------\n");
}

void enable_paging() {
    debug("--------------------------------\n");
    debug("Acivation du paging...\n");
    uint32_t cr0 = get_cr0();
	set_cr0(cr0|CR0_PG);
    debug("Acivation réussie !\n");
    debug("--------------------------------\n");
}

void tp() {
    int count = 0;
    init_gdt();
    init_idt();
    init_kernel_pgd();
    init_user1_pgd();
    init_user2_pgd();
    enable_paging();
    while (1) {
        count++;
    }
}
