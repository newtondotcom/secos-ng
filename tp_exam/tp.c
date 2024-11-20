/* GPLv2 (c) Airbus */
#include <debug.h>

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

void tache1() {
   asm volatile ("mov %eax, %cr0");
   debug("after iret\n");
}

void noyauIdentityMapped(){
	// Allocation PGD et maj CR3
	pde32_t *pgd = (pde32_t*)0x600000; // 00 0000 0001   -- 10 0000 0000   -- 0000 0000 0000
	set_cr3((uint32_t)pgd);

	// Allocation PTB
	pte32_t *ptb = (pte32_t*)0x601000;

	// Remplissage
	for(int i=0;i<1024;i++) {
	 	pg_set_entry(&ptb[i], PG_KRN|PG_RW, i);
	}
	memset((void*)pgd, 0, PAGE_SIZE);
	pg_set_entry(&pgd[0], PG_KRN|PG_RW, page_nr(ptb));

	// Activation pagination
	uint32_t cr0 = get_cr0();
	set_cr0(cr0|CR0_PG);


	// Préparer la pile pour l'exécution en ring 3 de la tache 1
   asm volatile (
   "push %0    \n" // ss
   "push %%ebp \n" // esp
   "pushf      \n" // eflags
   "push %1    \n" // cs
   "push %2    \n" // eip
   "iret"
   ::
    "i"(d0_sel),
    "i"(c0_sel),
    "r"(&test_ring0)
   );
}

void tacheIdentityMapped(){
	// Allocation PGD et maj CR3
	pde32_t *pgd = (pde32_t*)0x602000; // 00 0000 0010   -- 10 0000 0000   -- 0000 0000 0000
	set_cr3((uint32_t)pgd);

	// Allocation PTB
	pte32_t *ptb = (pte32_t*)0x603000;

	// Remplissage
	for(int i=0;i<1024;i++) {
	 	pg_set_entry(&ptb[i], PG_USR|PG_RW, i+1024);
	}
	pg_set_entry(&pgd[1], PG_KRN|PG_RW, page_nr(ptb));
}

void test_ring0()
{
    debug("Trying a kernel instruction...\n");
    asm volatile("mov %eax, %cr0");
    debug("We are in ring 0!\n");
    asm volatile("leave");
}

void tp() {

	// Initialisation de la GDT
	init_gdt();

	// Identity Mapping du noyau
	noyauIdentityMapped();
}
