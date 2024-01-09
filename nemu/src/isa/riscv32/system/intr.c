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

#include <isa.h> //CPU externed in here



//called when nemu running ecall intstruction
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  IFDEF(CONFIG_ETRACE,DLog("etrace: raising interruption and set mcause=%d mepc=%#x",NO,epc));
  cpu.CSR[CSR_MCAUSE] = NO;
  // if(NO==11) { //if yield.
  //   epc+=4;
  // }
  cpu.CSR[CSR_MEPC] = epc; //FIXME: what is epc, should we use epc instead of cpu.pc (because cpu.pc is updated after the instruction?)
  IFDEF(CONFIG_ETRACE,DLog("raise_intr return mtvec addr=%#x",cpu.CSR[CSR_MTVEC]));
  return cpu.CSR[CSR_MTVEC];
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
