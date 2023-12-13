/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <elf.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
 //Log("Exercise: Please remove me in the source code and compile NEMU again.");
 // assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}



#ifdef CONFIG_FTRACE
typedef struct function_entity
{
  char name[128];
  uint64_t start;
  uint64_t size;
} FuncInfo;

extern FuncInfo ftable[100];


// init ftrace
static bool load_elf() {
  if (elf_file == NULL) {
    WLog("No ELF file is given.");
    return EXIT_FAILURE; // built-in image size
  }


  FILE *fp = fopen(elf_file, "rb");

  Elf64_Ehdr header;
  if (fread(&header, 1, sizeof(header), fp) != sizeof(header)) {
    WLog("Error reading ELF header");
    fclose(fp);
    return EXIT_FAILURE;
  }

  if (memcmp(header.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "Not an ELF file\n");
    fclose(fp);
    return EXIT_FAILURE;
  }

  // Read section headers
  Elf64_Shdr *shdrs = (Elf64_Shdr *)malloc(header.e_shnum * sizeof(Elf64_Shdr));
  fseek(fp, header.e_shoff, SEEK_SET);
  if (fread(shdrs, sizeof(Elf64_Shdr), header.e_shnum, fp) != header.e_shnum) {
    WLog("Error reading section headers");
    free(shdrs);
    fclose(fp);
    return EXIT_FAILURE;
  }

  // string table and symbol table
  Elf64_Shdr *strtab_shdr = &shdrs[header.e_shstrndx];

  Elf64_Shdr *symtab_shdr = NULL;
  for (int i = 0; i < header.e_shnum; i++) {
    // printf("%d\n",shdrs[i].sh_type);
    if (shdrs[i].sh_type == SHT_SYMTAB) {
      symtab_shdr = &shdrs[i];
      printf("Symbol Table (.symtab) found at offset: 0x%lx\n", symtab_shdr->sh_offset);
    }
  }
  if (symtab_shdr) {
    strtab_shdr = &shdrs[symtab_shdr->sh_link]; // sh_link points to the .strtab section
    printf("String Table (.strtab) found at offset: 0x%lx\n", strtab_shdr->sh_offset);
  }
  char *strtab = (char *)malloc(strtab_shdr->sh_size);
  fseek(fp, strtab_shdr->sh_offset, SEEK_SET);
  if (fread(strtab, 1, strtab_shdr->sh_size, fp) != strtab_shdr->sh_size) {
    WLog("Error reading string table");
    free(strtab);
    free(shdrs);
    fclose(fp);
    return EXIT_FAILURE;
  }

  if (!symtab_shdr || !strtab_shdr) {
    fprintf(stderr, "Symbol or string table not found\n");
    free(shdrs);
    free(strtab);
    fclose(fp);
    return EXIT_FAILURE;
  }

  Elf64_Sym *symbols = (Elf64_Sym *)malloc(symtab_shdr->sh_size);
  fseek(fp, symtab_shdr->sh_offset, SEEK_SET);
  if (fread(symbols, 1, symtab_shdr->sh_size, fp) != symtab_shdr->sh_size) {
    WLog("Error reading symbol table");
    free(symbols);
    free(shdrs);
    free(strtab);
    fclose(fp);
    return EXIT_FAILURE;
  }

  extern FuncInfo ftable[100];
  extern int fnum;
  extern char padding[100]; // seq of spaces
  fnum = 0; padding[0] = '\0';

  // Save symbols to ftable
  size_t num_symbols = symtab_shdr->sh_size / sizeof(Elf64_Sym);
  for (size_t i = 0; i < num_symbols; i++) {
    if (ELF64_ST_TYPE(symbols[i].st_info) == STT_FUNC) {  // Check if it's a function
      FuncInfo f;
      f.start = symbols[i].st_value;
      strcpy(f.name,&strtab[symbols[i].st_name]);
      f.size = symbols[i].st_size;
      ftable[fnum++] = f;
      printf("size:%ld, Function name: %s, Address: 0x%lx\n", symbols[i].st_size, &strtab[symbols[i].st_name], symbols[i].st_value);
    }
  }
  
  // Clean up
  free(strtab);
  free(symbols);
  free(shdrs);
  fclose(fp);
  return EXIT_SUCCESS;
}
#endif

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {"elf"      , required_argument, NULL, 'e'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'e': elf_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-e,--elf=ELF_file         use ftrace on target ELF.\n");
        printf("\n");
        exit(0);
    }
  }
  
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();
  
  /* ftrace initialization */
  IFDEF(CONFIG_FTRACE, load_elf());
  

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
