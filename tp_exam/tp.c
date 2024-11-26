/* GPLv2 (c) Airbus */
#include <debug.h>
#include <intr.h>
#include <cr.h>
#include <pagemem.h>
#include <io.h>
#include <segmem.h>

#define c0_dsc(_d) gdt_flat_dsc(_d,0,SEG_DESC_CODE_XR)
#define d0_dsc(_d) gdt_flat_dsc(_d,0,SEG_DESC_DATA_RW)
#define c3_dsc(_d) gdt_flat_dsc(_d,3,SEG_DESC_CODE_XR)
#define d3_dsc(_d) gdt_flat_dsc(_d,3,SEG_DESC_DATA_RW)

#define gdt_flat_dsc(_dSc_,_pVl_,_tYp_)                                 \
   ({                                                                   \
      (_dSc_)->raw     = 0;                                             \
      (_dSc_)->limit_1 = 0xffff;                                        \
      (_dSc_)->limit_2 = 0xf;                                           \
      (_dSc_)->type    = _tYp_;                                         \
      (_dSc_)->dpl     = _pVl_;                                         \
      (_dSc_)->d       = 1;                                             \
      (_dSc_)->g       = 1;                                             \
      (_dSc_)->s       = 1;                                             \
      (_dSc_)->p       = 1;                                             \
   })

#define c0_sel  gdt_krn_seg_sel(c0_idx)
#define d0_sel  gdt_krn_seg_sel(d0_idx)

#define c0_idx  1
#define d0_idx  2
#define c3_idx  3
#define d3_idx  4

seg_desc_t GDT[6];

pde32_t *pgd_kernel = (pde32_t*)0x600000;
pde32_t *pgd_user1 = (pde32_t*)0x700000;
pde32_t *pgd_user2 = (pde32_t*)0x800000;
pte32_t *ptb_shared = (pte32_t*)0x900000;

__attribute__((section(".user1"))) void user1() {
    while (1) {
        debug("User 1\n");
    }
}

__attribute__((section(".user2"))) void user2() {
    while (1) {
        debug("User 2\n");
    }
}

void init_gdt() {
    debug("--------------------------------\n");
    debug("Initialisation de la GDT...\n");
    gdt_reg_t gdtr;

    GDT[0].raw = 0ULL;

    c0_dsc( &GDT[c0_idx] );
    d0_dsc( &GDT[d0_idx] );
    c3_dsc( &GDT[c3_idx] );
    d3_dsc( &GDT[d3_idx] );

    gdtr.desc  = GDT;
    gdtr.limit = sizeof(GDT) - 1;
    set_gdtr(gdtr);

    set_cs(c0_sel);

    set_ss(d0_sel);
    set_ds(d0_sel);
    set_es(d0_sel);
    set_fs(d0_sel);
    set_gs(d0_sel);
    debug("Adresse de la GDT : 0x%x\n", (unsigned int) gdtr.addr);
    debug("GDT initialisée !\n");
    debug("--------------------------------\n");
}

void clock_handler() {
    // Sauvegarde du contexte
    asm volatile ("pusha");
    // TODO : Changement de contexte
    debug("CLOCK INTERRUPT\n");
    // Indiquer au hardware qu'on a traité l'interruption
    outb(0x20, 0x20);
    // Restauration du contexte
    asm volatile ("popa");
    asm volatile ("leave; iret");
}

void init_idt() {
    debug("--------------------------------\n");
    debug("Initialisation de l'IDT...\n");
    idt_reg_t idtr;
    get_idtr(idtr);
    debug("Adresse de l'IDT : 0x%x\n", (unsigned int) idtr.addr); 
    // Référencement du handler pour l'interruption 32
    int_desc_t *clock_dsc = &idtr.desc[32];
    clock_dsc->offset_1 = (uint16_t)((uint32_t)clock_handler);
    clock_dsc->offset_2 = (uint16_t)(((uint32_t)clock_handler) >> 16);
    // Activation des interruptions matérielles
    asm volatile ("sti");
    debug("IDT initialisée !\n");
    debug("--------------------------------\n");
}

void init_kernel_pgd() {
    debug("--------------------------------\n");
    uint32_t cr3 = get_cr3();
	debug("Valeur courante du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("Initialisation du PGD du noyau à l'adresse physique 0x600000...\n");
	set_cr3((uint32_t)pgd_kernel);
    cr3 = get_cr3();
    memset((void*)pgd_kernel, 0, PAGE_SIZE);
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("--------------------------------\n");
}

void init_kernel_ptb() {
    debug("--------------------------------\n");
    pte32_t *ptb_kernel = (pte32_t*)0x601000;
    debug("Initialisation de la PTB du noyau à l'adresse physique %p...\n", ptb_kernel);
    for(int i = 0; i < 1024; i++) {
	 	pg_set_entry(&ptb_kernel[i], PG_KRN|PG_RW, i);
	}
    pg_set_entry(&pgd_kernel[0], PG_KRN|PG_RW, page_get_nr(ptb_kernel));
    debug("Initialisation réussie !\n");
    debug("--------------------------------\n");
}

void init_user1_pgd() {
    debug("--------------------------------\n");
    debug("Initialisation du PGD de la tâche user1 à l'adresse physique %p...\n", pgd_user1);
	set_cr3((uint32_t)pgd_user1);
    uint32_t cr3 = get_cr3();
    memset((void*)pgd_user1, 0, PAGE_SIZE);
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("Réaffectation du registre CR3 au PGD du noyau...\n");
    set_cr3((uint32_t)pgd_kernel);
    cr3 = get_cr3();
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("--------------------------------\n");
}

void init_user1_ptb() {
    debug("--------------------------------\n");
    pte32_t *ptb_user1 = (pte32_t*)0x701000;
    debug("Initialisation de la PTB de la tâche user1 à l'adresse physique %p...\n", ptb_user1);
    for(int i = 0; i < 1024; i++) {
	 	pg_set_entry(&ptb_user1[i], PG_USR|PG_RW, i);
	}
    pg_set_entry(&pgd_user1[0], PG_USR|PG_RW, page_get_nr(ptb_user1));
    debug("Initialisation réussie !\n");
    debug("--------------------------------\n");
    debug("Mapping de la zone mémoire partagée à l'adresse physique %p...\n", ptb_shared);
    for(int i = 0; i < 1024; i++) {
	 	pg_set_entry(&ptb_shared[i], PG_USR|PG_RW, i);
	}
    pg_set_entry(&pgd_user1[1], PG_USR|PG_RW, page_get_nr(ptb_shared));
    debug("Mapping réussi : %p -> 0x%x !\n", &pgd_user1[1], pgd_user1[1].raw);
    debug("--------------------------------\n");
}

void init_user2_pgd() {
    debug("--------------------------------\n");
    debug("Initialisation du PGD de la tâche user2 à l'adresse physique %p...\n", pgd_user2);
	set_cr3((uint32_t)pgd_user2);
    uint32_t cr3 = get_cr3();
    memset((void*)pgd_user2, 0, PAGE_SIZE);
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("Réaffectation du registre CR3 au PGD du noyau...\n");
    set_cr3((uint32_t)pgd_kernel);
    cr3 = get_cr3();
    debug("Nouvelle valeur du registre CR3 : 0x%x\n", (unsigned int) cr3);
    debug("--------------------------------\n");
}

void init_user2_ptb() {
    debug("--------------------------------\n");
    pte32_t *ptb_user2 = (pte32_t*)0x801000;
    debug("Initialisation de la PTB de la tâche user2 à l'adresse physique %p...\n", ptb_user2);
    for(int i = 0; i < 1024; i++) {
	 	pg_set_entry(&ptb_user2[i], PG_USR|PG_RW, i);
	}
    pg_set_entry(&pgd_user2[0], PG_USR|PG_RW, page_get_nr(ptb_user2));
    debug("Initialisation réussie !\n");
    debug("--------------------------------\n");
    debug("Mapping de la zone mémoire partagée à l'adresse physique %p...\n", ptb_shared);
    for(int i = 0; i < 1024; i++) {
	 	pg_set_entry(&ptb_shared[i], PG_USR|PG_RW, i);
	}
    pg_set_entry(&pgd_user2[1], PG_USR|PG_RW, page_get_nr(ptb_shared));
    debug("Mapping réussi !\n");
    debug("Mapping réussi : %p -> 0x%x !\n", &pgd_user2[1], pgd_user2[1].raw);
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
    init_gdt();
    init_idt();
    init_kernel_pgd();
    init_kernel_ptb();
    init_user1_pgd();
    init_user1_ptb();
    init_user2_pgd();
    init_user2_ptb();
    enable_paging();
}
