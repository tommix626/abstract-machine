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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_R, TYPE_B,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT(BITS(i, 19, 12) | BITS(i, 31, 31) << 8, 9) << 12 | BITS(i, 20, 20) << 11 | BITS(i, 30, 21) <<1 ; } while(0) 
#define immB() do { *imm = SEXT(BITS(i, 31, 31),1) << 12 | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1); } while(0)


static inline sword_t msbExtendedShift(sword_t src1, sword_t src2) {
    if (src2 <= 0 || src2 >= 32) {
        return src1;  // No shift or undefined shift
    }

    sword_t mask = src1 < 0 ? ~((1 << (32 - src2)) - 1) : 0;
    return (src1 >> src2) | mask;
}

#ifdef CONFIG_FTRACE
typedef struct function_entity
{
  char name[128];
  uint64_t start;
  uint64_t size;
} FuncInfo; //repetition in isa.h FIXME
//ftrace
FuncInfo ftable[1000];
int fnum;
char padding[1000]; // seq of spaces

static char* fetch_func_name(vaddr_t addr) {
  for (int i = 0; i < fnum; i++)
  {
    FuncInfo f = ftable[i];
    if(f.start <= addr && addr < (f.start+f.size)) {return ftable[i].name;}
  }
  panic("should ot reach here");
}

#define PADDING_SYMBOL "| "
#define PADDING_SYMBOL_LEN strlen(PADDING_SYMBOL)
//print ftrace command
static void ftrace(char *name, Decode *s, int rd, word_t src1, word_t imm) {
  if(0==strcmp(name,"jal") && rd == 1) {
    vaddr_t target_addr = s->pc+imm;
    char* fname = fetch_func_name(target_addr);
    DLog("%spc:>%#x call %s@%#x",padding,s->pc,fname,target_addr);
    strcat(padding,PADDING_SYMBOL);
  }
  else if(0==strcmp(name,"jalr")) {
    if(rd == 0 && imm == 0 && src1 == R(1)) {
      char* fname = fetch_func_name(s->pc);
      padding[strlen(padding)-strlen(PADDING_SYMBOL)] = '\0';
      DLog("%spc:>%#x return %s",padding,s->pc,fname);
    }
    else if(rd == 1) {
      vaddr_t target_addr = src1+imm;
      char* fname = fetch_func_name(target_addr);
      DLog("%spc:>%#x call %s@%#x",padding,s->pc,fname,target_addr);
      strcat(padding,PADDING_SYMBOL);
     }
  }
}
#endif

/// break bit string into segments to read the instruction
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  // DLog("Decode operand result: rd rs1 rs2 = %#x %#x %#x",*rd,rs1,rs2);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_B: src1R(); src2R(); immB(); break;
  }
}

/// @brief decoding!
static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  IFDEF(CONFIG_FTRACE,ftrace(name,s,rd,src1,imm)); \
  __VA_ARGS__ ;\
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", "auipc" , U, R(rd) = s->pc + imm /*WLog("Skip")*/ );
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", "lui"  , U, R(rd) = imm);

  /*Integer Arithmetic*/
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", "add"   , R, /*DLog("pc=%#x imm=%#x",(int)s->pc,(int)imm);*/  R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", "sub"   , R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", "xor"   , R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", "or"    , R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", "and"   , R, R(rd) = src1 & src2);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", "sll"   , R, R(rd) = src1 << src2);
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", "srl"   , R, R(rd) = src1 >> src2);
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", "sra"   , R, R(rd) =  msbExtendedShift(src1,src2)); //msb-extends 
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", "slt"   , R, R(rd) = ((sword_t)src1 < (sword_t)src2)?1:0);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", "sltu"  , R, R(rd) = (src1 < src2)?1:0); //zero-extended?

  INSTPAT("??????? ????? ????? 000 ????? 00100 11", "addi"   , I, R(rd) = src1 + imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", "andi"   , I, R(rd) = src1 & imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", "ori"    , I, R(rd) =  src1 | imm);
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", "xori"   , I, R(rd) = src1 ^ imm);

  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", "slli"   , I, R(rd) = src1 << (imm & 0x1f));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", "srli"   , I, R(rd) = src1 >> (imm & 0x1f));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", "srai"    , I, R(rd) =  msbExtendedShift(src1,(imm & 0x1f))); //msb-extends
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", "slti"   , I, R(rd) = ((sword_t)src1 < (sword_t)imm)?1:0);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", "sltiu"   , I, R(rd) = (src1 < imm)?1:0);

  /*Storing and Loading*/
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", "sb"     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", "sh"     , S , Mw(src1 + imm, 2, src2) );
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", "sw"     , S , Mw(src1 + imm, 4, src2) );
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", "lb"     , I, R(rd) = SEXT(Mr(src1 + imm, 1),8));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", "lh"     , I , R(rd) = SEXT(Mr(src1 + imm, 2),16) );
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", "lw"     , I , R(rd) = (sword_t)Mr(src1 + imm, 4) );
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", "lbu"    , I, R(rd) = Mr(src1 + imm, 1)); //example word_t is unsigned
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", "lhu"    , I, R(rd) = Mr(src1 + imm, 2)); //example word_t is unsigned

  /*branching*/
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", "beq"    , B ,  /*DLog("pc=%#x imm=%#x",(int)s->pc,(int)imm);*/ if((sword_t)src1==(sword_t)src2) s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", "bne"    , B ,  if((sword_t)src1!=(sword_t)src2) s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", "blt"    , B ,  if((sword_t)src1 < (sword_t)src2) s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", "bge"    , B ,  if((sword_t)src1>=(sword_t)src2) s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", "bltu"   , B ,  if(src1<src2) s->dnpc=s->pc+imm ); //zero extended unsign
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", "bgeu"   , B ,  if(src1>=src2) s->dnpc=s->pc+imm ); //zero extended

  /*Jumping*/
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", "jal"    , J ,  R(rd) = s->pc + 4; s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", "jalr"   , I ,  R(rd) = s->pc + 4; s->dnpc=src1+imm); /*this is I decoding*/

  /*Multiply Extension*/
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", "mul"  , R, R(rd) = ((sword_t)src1 * (sword_t)src2) & 0xffffffff);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", "mulh"  , R, R(rd) = ((int64_t)SEXT(src1,32) * (int64_t)SEXT(src2,32)) >> 32);
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", "mulsu"  , R, R(rd) = ((int64_t)SEXT(src1,32) * (uint64_t)src2) >> 32); //is this ok? or does unsigned is used for both
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", "mulu"  , R, R(rd) = ((uint64_t)src1 * (uint64_t)src2) >> 32);
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", "div"  , R, {if(src2==0) WLog("div by 0"); R(rd) = (sword_t)src1 / (sword_t)src2;});
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", "divu"  , R, {if(src2==0) WLog("div by 0"); R(rd) = src1 / src2;});
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", "rem"   , R, R(rd) = (sword_t)src1 % (sword_t)src2); //sign issue! same with dividend
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", "remu"  , R, R(rd) = src1 % src2);

  INSTPAT("0000000 00001 00000 000 00000 11100 11", "ebreak" , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", "inv"    , N, INV(s->pc)); //catch invalid instruction
  INSTPAT_END();
  // DLog("Inst end");
  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
