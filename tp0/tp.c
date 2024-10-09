/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t   *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

void tp() {
   debug("kernel mem [0x%p - 0x%p]\n", &__kernel_start__, &__kernel_end__);
   debug("MBI flags 0x%x\n", info->mbi->flags);

   multiboot_memory_map_t* entry = (multiboot_memory_map_t*)info->mbi->mmap_addr;
   while((uint32_t)entry < (info->mbi->mmap_addr + info->mbi->mmap_length)) {
      // TODO print "[start - end] type" for each entry
      string type;
      switch ( entry->type ) {
         case 1:
         type = "MULTIBOOT_MEMORY_AVAILABLE";
            break;
         case 2:
         type = "MULTIBOOT_MEMORY_RESERVED";
            break;
         case 3:
         type = "MULTIBOOT_MEMORY_ACPI_RECLAIMABLE";
            break;
         case 4:
         type = "MULTIBOOT_MEMORY_NVS";
         default:
            printf("MEMORY CARTOGRAPHY FAILED");
      }
    printf(" [0x%llx - 0x%llx] %s\n", entry->addr, entry->addr + entry->len, type);
      entry++;
   }

}
