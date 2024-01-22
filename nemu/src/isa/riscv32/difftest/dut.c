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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

/// @brief 
/// @param ref_r the CPU_state of REF.
/// @param pc the dnpc of dut.
/// @return 
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  extern CPU_state cpu;
  bool status = true;
  for(int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32);i++) {
    if(ref_r->gpr[i] != cpu.gpr[i]) {
      status = false;
      WLog("Difference in register x%d(%s), DUT Value = %d [%#x] REF Value = %d[%#x]",i,reg_name(i),cpu.gpr[i],cpu.gpr[i],ref_r->gpr[i],ref_r->gpr[i]);
    }
  }
  
  if(!status) {
    WLog("Diff Test Failed at pc = %#x",pc);
  }
  return status;

}

void isa_difftest_attach() {
}
