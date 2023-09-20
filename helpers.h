#ifndef HELPERS
#define HELPERS

#include "types.h"

/* DEBUG helpers */

void print_ufs_current_cycle(FILE *output, CPU_Configurations cpu_configs, FunctionalUnit functional_units[]){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int i=0; i<total_ufs; i++){
    if (i == 0) printf("ADD UFS:\n");
    else if (i == cpu_configs.size_add_ufs) printf("MUL UFS:\n");
    else if (i == cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs) printf("INTEGER UFS:\n");
    printf("functional unit [%d].cycle: %d\n", i, functional_units[i].current_cycle);
  }
}



/* fetch_next_instruction helpers */

/* issue_instruction helpers */

bool has_idle_uf(){
  return true;
}

FunctionalUnitState *find_uf_with_type(UF_TYPE type){
  
}

FunctionalUnitState get_instruction_information(int instruction_binary){

}

/* read_operands helpers */

/* execute helpers */

/* write_result helpers */

/* Outros */

bool uf_is_ready(FunctionalUnit uf, CPU_Configurations cpu_configs){
  if (uf.type == ADD_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_add;
  else if (uf.type == MUL_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_mul;
  else if (uf.type == INTEGER_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_integer;
  return false;
}

void increment_all_uf_current_cycle(FILE *output, CPU_Configurations cpu_configs, FunctionalUnit functional_units[]){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int i=0; i<total_ufs; i++){
    if (uf_is_ready(functional_units[i], cpu_configs)){
      functional_units[i].current_cycle = 1;
      // write_result(functional_units[i]);
      continue;
    }
    // if is busy
    functional_units[i].current_cycle++;
  }
  print_ufs_current_cycle(output, cpu_configs, functional_units);
}

char* table_format_register(int number) {

    static char result[20];
    result[0] = '\0';
    
    if(number == -1) return result;

    snprintf(result, sizeof(result), "R%d", number);
    return result;
}
char* table_format_number(int number) {
  static char empty[1];
  empty[0] = '\0';
  if(number == -1){
    return empty;
  }

  static char result[20];
  // memset(result, '\0', sizeof(result));
  snprintf(result, sizeof(result), "%d", number);
  return result;
}

void print_instruction_status(InstructionState** instruction_states, int num_instructions){
  printf("Status das Instruções:\n");
  char* labels[] = {"Instruction", "Fetch", "Issue", "Read opearands", "Exec Complete", "Write result"};
  printf("|%-20s|%-15s|%-15s|%-15s|%-15s|%-15s|\n", labels[0], labels[1], labels[2], labels[3], labels[4], labels[5]);

  for(int i = 0; i < num_instructions; i++){
    printf("|%-20d|%-15s|%-15s|%-15s|%-15s|%-15s|\n",
    i,
    table_format_number((*instruction_states)[i].fetch),
    table_format_number((*instruction_states)[i].issue),
    table_format_number((*instruction_states)[i].read_operands),
    table_format_number((*instruction_states)[i].execute),
    table_format_number((*instruction_states)[i].write_result));
  }
}

void print_functional_unit_status(FunctionalUnitState* funcional_unit_states, int num_ufs){

  printf("Status das Unidades Funcionais:\n");
  
  char* labels[] = {"Name", "Busy", "Op", "Fi", "Fj", "Fk", "Qj", "Qk", "Rj", "Rk"};
  char* yesno[] = {"No", "Yes"};

  printf("|%-15s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|\n",
   labels[0], labels[1], labels[2], labels[3], labels[4], labels[5], labels[6], labels[7], labels[8], labels[9]);

  char* functional_unit_name[]= {"Integer", "Mul", "Add"};
  for(int i = 0; i < num_ufs; i++){
    printf("|%-15s|%-10s|%-10d|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|\n",
      functional_unit_name[funcional_unit_states[i].type],
      yesno[funcional_unit_states[i].busy],
      funcional_unit_states[i].op,
      table_format_register(funcional_unit_states[i].fi),
      table_format_register(funcional_unit_states[i].fj),
      table_format_register(funcional_unit_states[i].fk),
      table_format_number(funcional_unit_states[i].qj),
      table_format_number(funcional_unit_states[i].qk),
      yesno[funcional_unit_states[i].rj],
      yesno[funcional_unit_states[i].rk]);

  }
}


void print_result_register_status(FunctionalUnit* result_register_state[]){
  printf("Status dos Resultados dos Registradores:\n");
  char* functional_unit_name[]= {"Integer", "Mul", "Add"};
  
  printf("|");
  for(int i = 0; i < 16; i++){
    printf("%-5s|", table_format_register(i));
  }

  printf("\n");

  for(int i = 0; i < 16; i++){
    if(result_register_state[i] != NULL){
      printf("%-5s|", functional_unit_name[result_register_state[i]->type]);
    }
  }


  printf("\n|");
  for(int i = 16; i < 32; i++){
    printf("%-5s|", table_format_register(i));   
  }
  printf("\n");

  for(int i = 16; i < 32; i++){
    if(result_register_state[i] != NULL){
      printf("%-5s|", functional_unit_name[result_register_state[i]->type]);
    }
  }
}

void print_table(ScoreBoard* scoreboarding, int curr_cycle, int num_instructions, int num_ufs){
  printf("*******************************************************************************************\n");
  printf("Ciclo atual: %d\n", curr_cycle);
  print_instruction_status(&scoreboarding->instructions_states, num_instructions);
  print_functional_unit_status(scoreboarding->ufs_states, num_ufs);
  print_result_register_status(scoreboarding->result_register_state);
}

int dec_to_bin(int num){

  for(int i = 0; i < 32; i++){
    printf("%d", (num&(1 << (31-i)))!=0);
  }
  putchar('\n');

}


#endif