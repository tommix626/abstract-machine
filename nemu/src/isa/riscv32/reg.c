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
#include "local-include/reg.h"

//use reg_name(reg_id) to get name of regs, total number of reg is MUXDEF(CONFIG_RVE, 16, 32);
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  for(int reg_id = 0; reg_id < MUXDEF(CONFIG_RVE, 16, 32); reg_id++){
    printf("%-15s %#-10x %-10u\n", reg_name(reg_id), cpu.gpr[reg_id],cpu.gpr[reg_id]);
  }
}

const int get_reg_id_by_name(const char * s){
  for(int reg_id = 0; reg_id < MUXDEF(CONFIG_RVE, 16, 32); reg_id++){
    if(strcmp(s,reg_name(reg_id)) == 0){
      return reg_id;
    }
  }
  return -1;
}

/// @brief return the value of the register named s. success shows if the operation is successful
/// @param s 
/// @param success 
/// @return 
word_t isa_reg_str2val(const char *s, bool *success) {
  int reg_id = get_reg_id_by_name(s+1);
  if(reg_id == -1){
    WLog("invalid reg name");
    *success = false;
    return 0;
  }
  return cpu.gpr[reg_id];
}
