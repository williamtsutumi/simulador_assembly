#ifndef HELPERS
#define HELPERS

#include "types.h"
char* empty = "";

/* Utilidades */

int get_opcode_from_binary(int instruction){
  return instruction >> 26;
}

char *get_inst_name_from_opcode(Byte op_code){
  if (op_code == ADD_OPCODE)  return ADD;
  else if (op_code == ADDI_OPCODE) return ADDI;
  else if (op_code == SUB_OPCODE)  return SUB;
  else if (op_code == SUBI_OPCODE) return SUBI;
  else if (op_code == MUL_OPCODE)  return MUL;
  else if (op_code == DIV_OPCODE)  return DIV;
  else if (op_code == AND_OPCODE)  return AND;
  else if (op_code == OR_OPCODE)   return OR;
  else if (op_code == NOT_OPCODE)  return NOT;
  else if (op_code == BLT_OPCODE)  return BLT;
  else if (op_code == BGT_OPCODE)  return  BGT;
  else if (op_code == BEQ_OPCODE)  return  BEQ;
  else if (op_code == BNE_OPCODE)  return  BNE;
  else if (op_code == J_OPCODE)    return  J;
  else if (op_code == LW_OPCODE)   return  LW;
  else if (op_code == SW_OPCODE)   return  SW;
  else if (op_code == EXIT_OPCODE) return  EXIT;

  return "";
}

UF_TYPE get_uf_type_from_opcode(int op_code){
  if (
      op_code == ADD_OPCODE
    || op_code == ADDI_OPCODE
    || op_code == SUB_OPCODE
    || op_code == SUBI_OPCODE
    ) return ADD_UF;

  else if (
      op_code == MUL_OPCODE
    || op_code == DIV_OPCODE
  ) return MUL_UF;

  else if (
      op_code == AND_OPCODE
    || op_code == OR_OPCODE
    || op_code == NOT_OPCODE
    || op_code == LW_OPCODE
    || op_code == SW_OPCODE
  ) return INTEGER_UF;

  // blt, bgt, beq, bne, j, exit
  return NOT_APPLIED_UF;
}

int get_binary_subnumber(int instruction, int start_bit, int end_bit){
  instruction >>= start_bit;
  int bitmask = (1ll << (end_bit - start_bit + 1))-1;

  return instruction & bitmask;
}

UF_TYPE get_uf_type_from_instruction(int instruction){
  int op_code = get_binary_subnumber(instruction, 26, 31);
  return get_uf_type_from_opcode(op_code);
}

// Retorna o índice (no array de registradores) do registrador de destino
int get_destination_register_from_instruction(int instruction){
  int op_code = get_binary_subnumber(instruction, 26, 31);

  if (
    op_code == ADD_OPCODE
  || op_code == SUB_OPCODE
  || op_code == MUL_OPCODE
  || op_code == DIV_OPCODE
  || op_code == AND_OPCODE
  || op_code == OR_OPCODE
  || op_code == NOT_OPCODE
  ) return get_binary_subnumber(instruction, 11, 15);

  else if (
      op_code == SUBI_OPCODE
    || op_code == ADDI_OPCODE
  ) return get_binary_subnumber(instruction, 16, 20);

  return -1;
}

void get_operands_register_from_instruction(int instruction, int* op1, int* op2){
  int op_code = get_binary_subnumber(instruction, 26, 31);

  if (
    op_code == ADD_OPCODE
  || op_code == SUB_OPCODE
  || op_code == MUL_OPCODE
  || op_code == DIV_OPCODE
  || op_code == AND_OPCODE
  || op_code == OR_OPCODE
  || op_code == NOT_OPCODE
  ) *op1 = get_binary_subnumber(instruction, 21, 25), *op2 = get_binary_subnumber(instruction, 16, 20);

  else if (
      op_code == SUBI_OPCODE
    || op_code == ADDI_OPCODE
    || op_code == NOT_OPCODE
  ) *op1 = get_binary_subnumber(instruction, 21, 25), *op2 = -1;

  else{
    *op1 = -1, *op2 = -1;
  }
}

// instruction_index se refere à i-ésima instrução - 1
// primeira instrução => instruction_index 0
int get_instruction_from_memory(int instruction_index, Byte *mem){
  int mem_address = instruction_index*4 + PROGRAM_FIRST_ADDRESS;

  return (mem[mem_address] << 24) | (mem[mem_address + 1] << 16) | (mem[mem_address + 2] << 8) | (mem[mem_address + 3]);
}




/* Utilidades Scoreboard */

void clear_uf_state(FunctionalUnitState *uf_state){
  (*uf_state).fi = -1;
  (*uf_state).fj = -1;
  (*uf_state).fk = -1;
  (*uf_state).qj = -1;
  (*uf_state).qk = -1;
  (*uf_state).op = -1;
  (*uf_state).busy = false;
  (*uf_state).rj = false;
  (*uf_state).rk = false;
  (*uf_state).inst_program_counter = -1;
}

int actually_execute(int opcode, int operand1, int operand2){
  if (opcode == ADD_OPCODE)  return operand1 + operand2;
  if (opcode == ADDI_OPCODE) return operand1 + operand2; // Sim, é igual ao anterior
  if (opcode == SUB_OPCODE)  return operand1 - operand2;
  if (opcode == SUBI_OPCODE) return operand1 - operand2; // Sim, é igual ao anterior
  if (opcode == MUL_OPCODE)  return operand1 * operand2;
  if (opcode == DIV_OPCODE)  return operand1 / operand2;
  if (opcode == AND_OPCODE)  return operand1 & operand2;
  if (opcode == OR_OPCODE)   return operand1 | operand2;
  if (opcode == NOT_OPCODE)  return ~operand1;
  
  // todo -> não tenho certeza o que fazer com essas
  // if (opcode == BLT_OPCODE)  
  // if (opcode == BGT_OPCODE) 
  // if (opcode == BEQ_OPCODE) 
  // if (opcode == BNE_OPCODE) 
  // if (opcode == J_OPCODE) 
  // if (opcode == LW_OPCODE) 
  // if (opcode == SW_OPCODE) 
  // if (opcode == EXIT_OPCODE) 

}

// Checa se pode enviar alguma instrução para write result
// Senão, continua a executar
void update_write_result(Bus *bus_buffer, Byte *memory, ScoreBoard *score_board, CPU_Configurations cpu_configs, int curr_cycle, int inst_count){
  // todo -> enviar para write result se possível ao invés de sempre enviar
  int count_instructions_sent_to_write = 0;
  for (int i = 0; i < inst_count; i++){
    if ((*score_board).instructions_states[i].current_state == EXECUTE){
      int cycles_to_complete;
      int inst = get_instruction_from_memory(i, memory);
      // printf("inst: %d", inst);
      UF_TYPE inst_uf_type = get_uf_type_from_instruction(inst);
      if (inst_uf_type == ADD_UF) cycles_to_complete = cpu_configs.cycles_to_complete_add;
      else if (inst_uf_type == MUL_UF) cycles_to_complete = cpu_configs.cycles_to_complete_mul;
      else if (inst_uf_type == INTEGER_UF) cycles_to_complete = cpu_configs.cycles_to_complete_integer;
      
      int start = (*score_board).instructions_states[i].start_execute;
      int finish = (*score_board).instructions_states[i].finish_execute;
      printf("PORRA\n");
      if (finish - start + 1 == cycles_to_complete){
        (*bus_buffer).ufs_state[i] = STALL;
        
        if (count_instructions_sent_to_write < WRITE_RESULT_CAPACITY){
          count_instructions_sent_to_write++;

          (*bus_buffer).ufs_state[i] = CONTINUE_WRITE_RESULT;

          (*score_board).instructions_states[i].current_state = WRITE_RESULT;
          (*score_board).instructions_states[i].write_result = curr_cycle;

          int uf_idx = (*score_board).instructions_states[i].uf_index;
          // todo -> tem algum problema com o clear pq a instrução continuar sendo printada
          clear_uf_state(&((*score_board).ufs_states[uf_idx]));
        }
      }
      else{
        (*bus_buffer).ufs_state[i] = CONTINUE_EXECUTE;
        (*score_board).instructions_states[i].finish_execute++;
      }
    }
  }
}

// Checa se pode enviar alguma instrução para execute
void update_execute(ScoreBoard *score_board, int curr_cycle, int inst_count){
  // todo -> enviar para execução se possível ao invés de sempre enviar
  for (int i = 0; i < inst_count; i++){
    if ((*score_board).instructions_states[i].current_state == READ_OPERANDS){
      (*score_board).instructions_states[i].current_state = EXECUTE;
      (*score_board).instructions_states[i].start_execute = curr_cycle;
      (*score_board).instructions_states[i].finish_execute = curr_cycle;
    }
  }
}

// Checa se pode enviar alguma instrução para read operands
void update_read_operands(ScoreBoard *score_board, int curr_cycle, int total_ufs){
  for (int i = 0; i < total_ufs; i++){
    if ((*score_board).instructions_states[i].current_state == ISSUE){
      (*score_board).instructions_states[i].current_state = READ_OPERANDS;
      (*score_board).instructions_states[i].read_operands = curr_cycle;
    }
  }
}

// Checa se pode enviar alguma instrução para issue
void update_issue(ScoreBoard *score_board, InstructionRegister ir, int curr_cycle, int total_ufs){
  UF_TYPE type = get_uf_type_from_instruction(ir.binary);
  int idle_uf_index = -1;
  for (int i = 0; i < total_ufs; i++){
    // todo -> branchs não condicionais tá entrando em qualquer lugar
    if (type == NOT_APPLIED_UF && !(*score_board).ufs_states[i].busy){
      idle_uf_index = i;
      break;
    }
    if ((*score_board).ufs_states[i].type == type && !(*score_board).ufs_states[i].busy){
      idle_uf_index = i;
      break;
    }
  }
  if(idle_uf_index == -1){
    printf("não tem unidades funcionais livres!\n");
    return;
  }

  // Controle do pipeline
  (*score_board).can_fetch = true;
  (*score_board).instructions_states[idle_uf_index].uf_index = idle_uf_index;
  //*********************
  (*score_board).instructions_states[(ir.program_counter - PROGRAM_FIRST_ADDRESS) / 4].current_state = ISSUE;
  (*score_board).instructions_states[(ir.program_counter - PROGRAM_FIRST_ADDRESS) / 4].issue = curr_cycle;


  (*score_board).ufs_states[idle_uf_index].busy = true;
  (*score_board).ufs_states[idle_uf_index].inst_program_counter = ir.binary;

  (*score_board).ufs_states[idle_uf_index].fi = get_destination_register_from_instruction(ir.binary);
  int op1, op2;
  get_operands_register_from_instruction(ir.binary, &op1, &op2);
  (*score_board).ufs_states[idle_uf_index].fj = op1;
  (*score_board).ufs_states[idle_uf_index].fk = op2;
  (*score_board).ufs_states[idle_uf_index].op = get_binary_subnumber(ir.binary, 26, 31);

  // (*score_board).result_register_state[rand()%32] = &g_functional_units[rand()%total_ufs];
  //(*score_board).ufs_states[idle_uf_index].qj = (*score_board).result_register_state[(*score_board).ufs_states[idle_uf_index].fj]->type;
  //(*score_board).ufs_states[idle_uf_index].qk = (*score_board).result_register_state[(*score_board).ufs_states[idle_uf_index].fk]->type;
}

// Checa se pode enviar alguma instrução para fetch
void update_fetch(Byte *memory, ScoreBoard *score_board, InstructionRegister *ir, int *pc, int curr_cycle, int total_ufs){
  if ((*score_board).can_fetch){
    int instruction_index = ((*pc) - PROGRAM_FIRST_ADDRESS) / 4;
    // if(g_instruction_count <= instruction_index){
    //   assert(false && "acabou as intruções pra serem fetchadas");
    // }
    (*score_board).instructions_states[instruction_index].current_state = FETCH;
    (*score_board).instructions_states[instruction_index].fetch = curr_cycle;

    (*ir).binary = get_instruction_from_memory(instruction_index, memory);
    (*ir).program_counter = (*pc);
    (*pc) += 4;


    (*score_board).can_fetch = false;
  }
}




/* Print table helpers */

char* table_format_text(char* pfx, int number) {

    char* result = (char*)malloc(sizeof(char) * 30);
    result[0] = '\0';
    
    if(number == -1) return result;

    snprintf(result, sizeof(result), "%s%d", pfx, number);
    return result;
}

char* table_format_number(int number) {
  char* result = (char*)malloc(sizeof(char) * 30);
  result[0] = '\0';

  if(number == -1){
    return result;
  }
  // memset(result, '\0', sizeof(result));
  snprintf(result, sizeof(result), "%d", number);
  return result;
}

void print_instruction_status(InstructionState** instruction_states, Byte inst_opcodes[], int num_instructions){
  printf("Status das Instruções:\n");
  char* labels[] = {"Instruction", "Fetch", "Issue", "Read operands", "Execution", "Write result"};

  yellow();
  printf("|%-20s|%-15s|%-15s|%-15s|%-15s|%-15s|\n", labels[0], labels[1], labels[2], labels[3], labels[4], labels[5]);
  reset();

  for(int i = 0; i < num_instructions; i++){
    /*
    char fetch[20];
    if ((*instruction_states)[i].fetch == -1) fetch[0] = '\0';
    else snprintf(fetch, sizeof(fetch), "%d", (*instruction_states)[i].fetch);

    char issue[20];
    if ((*instruction_states)[i].issue == -1) issue[0] = '\0';
    else snprintf(issue, sizeof(issue), "%d", (*instruction_states)[i].issue);

    char read_operands[20];
    if ((*instruction_states)[i].read_operands == -1) read_operands[0] = '\0';
    else snprintf(read_operands, sizeof(read_operands), "%d", (*instruction_states)[i].read_operands);

    char execute[20];
    if ((*instruction_states)[i].execute == -1) execute[0] = '\0';
    else snprintf(execute, sizeof(execute), "%d", (*instruction_states)[i].execute);

    char write_result[20];
    if ((*instruction_states)[i].write_result == -1) write_result[0] = '\0';
    else snprintf(write_result, sizeof(write_result), "%d", (*instruction_states)[i].write_result);
    */
    char *exec = table_format_number((*instruction_states)[i].start_execute);
    if (exec != NULL && exec[0] != '\0'){
      strcat(exec, " - ");
      strcat(exec, table_format_number((*instruction_states)[i].finish_execute));
    }
    printf("|%-20s|%-15s|%-15s|%-15s|%-15s|%-15s|\n",
      get_inst_name_from_opcode(inst_opcodes[i]),
      table_format_number((*instruction_states)[i].fetch),
      table_format_number((*instruction_states)[i].issue),
      table_format_number((*instruction_states)[i].read_operands),
      exec,
      table_format_number((*instruction_states)[i].write_result));


  }
}

void print_functional_unit_status(FunctionalUnitState* funcional_unit_states, int num_ufs){

  printf("Status das Unidades Funcionais:\n");
  
  char* labels[] = {"Name", "Busy", "Op", "Fi", "Fj", "Fk", "Qj", "Qk", "Rj", "Rk"};
  char* yesno[] = {"No", "Yes"};

  yellow();
  printf("|%-15s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|\n",
   labels[0], labels[1], labels[2], labels[3], labels[4], labels[5], labels[6], labels[7], labels[8], labels[9]);
  reset();

  char* functional_unit_name[]= {"Int", "Mul", "Add"};
  char *instruction_names[] = INSTRUCTION_NAMES;

  for(int i = 0; i < num_ufs; i++){
    printf("|%-15s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|%-10s|\n",
    
      table_format_text(functional_unit_name[funcional_unit_states[i].type], funcional_unit_states[i].type_index),
      yesno[funcional_unit_states[i].busy],
      funcional_unit_states[i].op == -1 ? empty : instruction_names[funcional_unit_states[i].op],
      table_format_text("R", funcional_unit_states[i].fi),
      table_format_text("R", funcional_unit_states[i].fj),
      table_format_text("R", funcional_unit_states[i].fk),
      table_format_number(funcional_unit_states[i].qj),
      table_format_number(funcional_unit_states[i].qk),
      yesno[funcional_unit_states[i].rj],
      yesno[funcional_unit_states[i].rk]);

  }
}

void print_result_register_status(FunctionalUnit* result_register_state[]){
  printf("Status dos Resultados dos Registradores:\n");
  char* functional_unit_name[]= {"Integer", "Mul", "Add"};
  
  for(int k = 0; k < 2; k++){
    
    yellow();
    printf("|");
    for(int i = k*16; i < (k+1)*16; i++){
      printf("%-7s|", table_format_text("R", i));
    }
    printf("\n");
    printf("|");
    reset();

    for(int i = k*16; i < (k+1)*16; i++){
      if(result_register_state[i] != NULL){
        printf("%-7s|", table_format_text(functional_unit_name[result_register_state[i]->type],
        result_register_state[i]->type_index));
      }
    }
    printf("\n");

  }
  



}

void print_table(ScoreBoard* scoreboarding, int curr_cycle, Byte inst_opcodes[], int num_instructions, int num_ufs){
  printf("*******************************************************************************************\n");
  printf("Ciclo atual: %d\n", curr_cycle);
  print_instruction_status(&scoreboarding->instructions_states, inst_opcodes, num_instructions);
  print_functional_unit_status(scoreboarding->ufs_states, num_ufs);
  print_result_register_status(scoreboarding->result_register_state);
}

#endif