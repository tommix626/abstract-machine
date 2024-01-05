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


//FIXME: should not contain AM code.
extern void __am_asm_trap(void); //NOTE:get the mtvec addr
extern Context* __am_irq_handle(Context *c);

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''. (HOW??? ->callbacks?)
   * Then return the address of the interrupt/exception vector.
   */
  __am_irq_handle() //fillout the context
  return &__am_asm_trap; //cpu.pc will be set outside.
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
