/* GPLv2 (c) Airbus */
#include <debug.h>
#include <segmem.h>
#include <string.h>

void userland() {
   asm volatile ("mov %eax, %cr0");
}

void print_gdt_content(gdt_reg_t gdtr_ptr) {
    seg_desc_t* gdt_ptr;
    gdt_ptr = (seg_desc_t*)(gdtr_ptr.addr);
    int i=0;
    while ((uint32_t)gdt_ptr < ((gdtr_ptr.addr) + gdtr_ptr.limit)) {
        uint32_t start = gdt_ptr->base_3<<24 | gdt_ptr->base_2<<16 | gdt_ptr->base_1;
        uint32_t end;
        if (gdt_ptr->g) {
            end = start + ( (gdt_ptr->limit_2<<16 | gdt_ptr->limit_1) <<12) + 4095;
        } else {
            end = start + (gdt_ptr->limit_2<<16 | gdt_ptr->limit_1);
        }
        debug("%d ", i);
        debug("[0x%x ", start);
        debug("- 0x%x] ", end);
        debug("seg_t: 0x%x ", gdt_ptr->type);
        debug("desc_t: %d ", gdt_ptr->s);
        debug("priv: %d ", gdt_ptr->dpl);
        debug("present: %d ", gdt_ptr->p);
        debug("avl: %d ", gdt_ptr->avl);
        debug("longmode: %d ", gdt_ptr->l);
        debug("default: %d ", gdt_ptr->d);
        debug("gran: %d ", gdt_ptr->g);
        debug("\n");
        gdt_ptr++;
        i++;
    }
}

void print_segment_selector(){
    uint16_t ss_value = get_ss();
    printf("SS: %d\n", ss_value >> 3);
    
    uint16_t ds_value = get_ds();
    printf("DS: %d\n", ds_value >> 3);
    
    uint16_t es_value = get_es();
    printf("ES: %d\n", es_value >> 3);
    
    uint16_t fs_value = get_fs();
    printf("FS: %d\n", fs_value >> 3);
    
    uint16_t gs_value = get_gs();
    printf("GS: %d\n", gs_value >> 3);

    uint16_t cs_value = get_cs();
    printf("CS: %d\n", cs_value >> 3);
}

void tp() {
    gdt_reg_t gdtr_ptr;
    get_gdtr(gdtr_ptr);
    print_gdt_content(gdtr_ptr);

    print_segment_selector();

    // L'adresse de base de la GDT doit être alignée sur 8 octets
    seg_desc_t new_gdt[7];

    // Le premier descripteur (indice 0) doit être NULL.
    new_gdt[0].raw = 0ULL;

    // Code, 32 bits RX, flat, indice 1
    new_gdt[1].limit_1 = 0xffff;   //:16;     /* bits 00-15 of the segment limit */
    new_gdt[1].base_1 = 0x0000;    //:16;     /* bits 00-15 of the base address */
    new_gdt[1].base_2 = 0x00;      //:8;      /* bits 16-23 of the base address */
    new_gdt[1].type = 11;//Code,RX //:4;      /* segment type */
    new_gdt[1].s = 1;              //:1;      /* descriptor type */
    new_gdt[1].dpl = 0; //ring0    //:2;      /* descriptor privilege level */
    new_gdt[1].p = 1;              //:1;      /* segment present flag */
    new_gdt[1].limit_2 = 0xf;      //:4;      /* bits 16-19 of the segment limit */
    new_gdt[1].avl = 1;            //:1;      /* available for fun and profit */
    new_gdt[1].l = 0; //32bits     //:1;      /* longmode */
    new_gdt[1].d = 1;              //:1;      /* default length, depend on seg type */
    new_gdt[1].g = 1;              //:1;      /* granularity */
    new_gdt[1].base_3 = 0x00;      //:8;      /* bits 24-31 of the base address */

    // Données, 32 bits RW, flat, indice 2.
    new_gdt[2].limit_1 = 0xffff;   //:16;     /* bits 00-15 of the segment limit */
    new_gdt[2].base_1 = 0x0000;    //:16;     /* bits 00-15 of the base address */
    new_gdt[2].base_2 = 0x00;      //:8;      /* bits 16-23 of the base address */
    new_gdt[2].type = 3; //data,RW //:4;      /* segment type */
    new_gdt[2].s = 1;              //:1;      /* descriptor type */
    new_gdt[2].dpl = 0; //ring0    //:2;      /* descriptor privilege level */
    new_gdt[2].p = 1;              //:1;      /* segment present flag */
    new_gdt[2].limit_2 = 0xf;      //:4;      /* bits 16-19 of the segment limit */
    new_gdt[2].avl = 1;            //:1;      /* available for fun and profit */
    new_gdt[2].l = 0; // 32 bits   //:1;      /* longmode */
    new_gdt[2].d = 1;              //:1;      /* default length, depend on seg type */
    new_gdt[2].g = 1;              //:1;      /* granularity */
    new_gdt[2].base_3 = 0x00;      //:8;      /* bits 24-31 of the base address */

    printf("Nouvelle GDT : %lu\n", (long unsigned int)new_gdt);


    // Mettre à jour les sélecteurs de segment (cs/ss/ds/...) afin qu'ils pointent vers les descripteurs précédemment définis.
    set_gdtr(new_gdt);
    gdt_reg_t new_gdtr_ptr;
    // Charger l'adresse de base de la nouvelle GDT
    new_gdtr_ptr.addr = (long unsigned int)new_gdt;
    new_gdtr_ptr.limit = sizeof(new_gdt) -1;
    // get_gdtr(new_gdtr_ptr); -- MARCHE PAS

    print_gdt_content(new_gdtr_ptr);

    char  src[64];
    char *dst = 0;
    memset(src, 0xff, 64);
    _memcpy8(dst, src, 32);
}
