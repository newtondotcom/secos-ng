/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>
#include <pagemem.h>
#include <cr.h>
#include <gpr.h>

#define c0_idx  1
#define d0_idx  2
#define c3_idx  3
#define d3_idx  4
#define ts_idx  5

#define c0_sel  gdt_krn_seg_sel(c0_idx)
#define d0_sel  gdt_krn_seg_sel(d0_idx)
#define c3_sel  gdt_usr_seg_sel(c3_idx)
#define d3_sel  gdt_usr_seg_sel(d3_idx)
#define ts_sel  gdt_krn_seg_sel(ts_idx)

uint32_t compteur;

// void __attribute__((section(".user"))) user1(uint32_t *compteur) {
// 	while(true){
// 		*compteur += 1; 
// 	}
// }

// void __attribute__((section(".user"))) user2(uint32_t *counter) {
// 	debug("Counter value: %d \n", *counter);
//    //asm volatile ("mov %eax, %cr0");
//    //debug("after iret\n");
// 		// Appel système pour signaler la mise à jour du compteur
//         asm volatile(
//             "movl $2, %%eax\n"         // Numéro de l'appel système pour sys_update_counter
//             "movl %0, %%ebx\n"         // Pointeur vers le compteur
//             "int $0x80\n"              // Interruption logicielle
//             :
//             : "r"(shared_counter)
//             : "%eax", "%ebx"
//         );
// }

// void handler80(){ // affichage noyau appelé par la tache 2
//     uint32_t syscall_number;
//     uint32_t *user_pointer;

//     // Lire le numéro de l'appel système et les arguments dans les registres
//     asm volatile("movl %%eax, %0" : "=r"(syscall_number));
//     asm volatile("movl %%ebx, %0" : "=r"(user_pointer));

//     if (syscall_number == 1) { 
//         // Validation de l'adresse utilisateur (si nécessaire)
//         uint32_t *kernel_pointer = user_pointer;
        
//         // Afficher la valeur du compteur
//         debug(*kernel_pointer);
//     }

//     // Restaurer le contexte utilisateur et retourner
//     asm volatile("iret");
// }

void handler32(){ // horloge : irq0

	// Changer le contexte en fonction de la tache active
	debug("BONJOUR");
    // Restaurer le contexte utilisateur et retourner
    //asm volatile("iret");
}

void init_idt() {
    debug("\nInterrupt configuration... \n");
    debug("\tGetting idtr adress... ");
    idt_reg_t idtr; 
    get_idtr(idtr);
    debug(" Success !\n");
    debug("\t\tidtr addr : %lx \n", idtr.addr);

    //Interruption de l'horloge
    debug("\tConfiguring clock interrupt... ");
    idtr.desc[32].offset_1 = (int) &handler32;
    debug(" Success !\n");
}

// void set_handler(uint32_t num, uint32_t handler) {
// 	idt_reg_t idtr;
// 	get_idtr(idtr);
// 	int_desc_t *dsc = &idtr.desc[num];
// 	dsc->offset_1 = (uint16_t)handler;
// 	dsc->offset_2 = (uint16_t)(handler >> 16);
// }

void tp() {
	init_idt();
	// set_handler(0x80, (uint32_t)handler80);
	int count = 0;
	// set_handler(0x20, (uint32_t)handler32);
	asm volatile ("sti");
	asm volatile ("int $32");
	while(1) {
		count++;
	}
}
