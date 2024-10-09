/* GPLv2 (c) Airbus */
#include <debug.h>
#include <segmem.h>

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
    printf("SS: %d\n", ss_value);
    
    uint16_t ds_value = get_ds();
    printf("DS: %d\n", ds_value);
    
    uint16_t es_value = get_es();
    printf("ES: %d\n", es_value);
    
    uint16_t fs_value = get_fs();
    printf("FS: %d\n", fs_value);
    
    uint16_t gs_value = get_gs();
    printf("GS: %d\n", gs_value);

    uint16_t cs_value = get_cs();
    printf("CS: %d\n", cs_value);
}

void tp() {
    gdt_reg_t gdtr_ptr;
    get_gdtr(gdtr_ptr);
    print_gdt_content(gdtr_ptr);

    print_segment_selector();
}
