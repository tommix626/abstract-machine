#include <proc.h>
#include <elf.h>


#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

// This function assumes 'elf_start' is a pointer to the beginning of the ELF file in memory
static uint32_t load_elf(Elf_Ehdr elf_start, uint32_t elf_start_addr) {
    // assert(elf_start != NULL); //FIXME: this is not a pointer, so must have some value, no need to check.

   
    
    Elf_Ehdr *header = &elf_start;
// printf("##\n");
    // Verify ELF Magic Number
    assert(
    header->e_ident[0] == 0x7F &&
    header->e_ident[1] == 'E' &&
    header->e_ident[2] == 'L' &&
    header->e_ident[3] == 'F'
    );

    // printf("header->e_phnum=%hu\n",header->e_phnum);
    // Iterate over program headers
    Elf_Phdr *phdr = (Elf_Phdr *)(elf_start_addr + header->e_phoff);
    // printf("phoff=%u\n",header->e_phoff);
    // printf("init self addr=%#x\n", elf_start_addr);
    // printf("init phdr addr=%p\n", phdr);
    for (int i = 0; i < header->e_phnum; i++) {
        // printf("phdr addr=%p, size=%d\t", phdr,sizeof(Elf_Phdr));
        // Print program header information (You can replace this with your own logic)
        // For example:
        // printf("Program Header %d: Type: %d, Offset: 0x%p, VirtAddr: 0x%p, FileSize:%#x, MemSize:%#x\n", 
        //         i, phdr->p_type, phdr->p_offset, phdr->p_vaddr,phdr->p_filesz,phdr->p_memsz);
        
        // printf("  Type: %d\n", phdr.p_type);
        // printf("  Offset: 0x%x\n", phdr.p_offset);
        // printf("  VirtAddr: 0x%x\n", phdr.p_vaddr);
        // printf("  PhysAddr: 0x%x\n", phdr.p_paddr);
        // printf("  FileSize: %u\n", phdr.p_filesz);
        // printf("  MemSize: %u\n", phdr.p_memsz);
        if(phdr->p_type == 1){ //1 stands for loading.
          Log("copying (%#x,%#x) to %p",phdr->p_offset,phdr->p_offset+phdr->p_filesz,(void *)phdr->p_vaddr);
          ramdisk_read((void *)phdr->p_vaddr,phdr->p_offset,phdr->p_filesz);
          Log("cleaning margin amount:%u-%u\n",phdr->p_memsz,phdr->p_filesz);
          memset((void *)phdr->p_vaddr+phdr->p_filesz,0,phdr->p_memsz-phdr->p_filesz);
        }
        phdr++;
    }

    assert(header->e_entry!=0);
    return header->e_entry;
}

extern Elf_Ehdr ramdisk_start;
static uintptr_t loader(PCB *pcb, const char *filename) {
  return load_elf(ramdisk_start,(uint32_t)&ramdisk_start);
}

void naive_uload(PCB *pcb, const char *filename) {
  Log("naive_uload start!");
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

