#include "../headers/types.h"
#include "../headers/bus.h"
#include "../headers/table.h"
#include "../headers/colors.h"

char* empty = "";

/* Utilidades */

int get_opcode_from_binary(InstructionBinary instruction){
  return instruction >> 26;
}

InstructionFormat get_inst_format_from_opcode(int opcode){
  if (opcode == ADD_OPCODE)  return FORMAT_R;
  if (opcode == ADDI_OPCODE) return FORMAT_I;
  if (opcode == SUB_OPCODE)  return FORMAT_R;
  if (opcode == SUBI_OPCODE) return FORMAT_I;
  if (opcode == MUL_OPCODE)  return FORMAT_R;
  if (opcode == DIV_OPCODE)  return FORMAT_R;

  if (opcode == AND_OPCODE) return FORMAT_R;
  if (opcode == OR_OPCODE)  return FORMAT_R;
  if (opcode == NOT_OPCODE) return FORMAT_R;

  if (opcode == BLT_OPCODE) return FORMAT_I;
  if (opcode == BGT_OPCODE) return FORMAT_I;
  if (opcode == BEQ_OPCODE) return FORMAT_I;
  if (opcode == BNE_OPCODE) return FORMAT_I;
  if (opcode == J_OPCODE)   return FORMAT_J;

  if (opcode == LW_OPCODE)   return FORMAT_I;
  if (opcode == SW_OPCODE)   return FORMAT_I;
  if (opcode == EXIT_OPCODE) return FORMAT_J;

  return FORMAT_INVALID;
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

static UF_TYPE get_uf_type_from_opcode(int op_code){
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
    || op_code == BLT_OPCODE
    || op_code == BGT_OPCODE
    || op_code == BEQ_OPCODE
    || op_code == BNE_OPCODE
    || op_code == J_OPCODE
  ) return INTEGER_UF;

  // exit
  return NOT_APPLIED_UF;
}

// [start_bit, end_bit]
static int get_binary_subnumber(InstructionBinary instruction, int start_bit, int end_bit){
  instruction >>= start_bit;
  int bitmask = (1ll << (end_bit - start_bit + 1))-1;

  return instruction & bitmask;
}

static UF_TYPE get_uf_type_from_instruction(InstructionBinary instruction){
  int op_code = get_binary_subnumber(instruction, 26, 31);
  return get_uf_type_from_opcode(op_code);
}

int get_rt_from_instruction_binary(InstructionBinary binary){
  return get_binary_subnumber(binary, 16, 20);
}

int get_rs_from_instruction_binary(InstructionBinary binary){
  return get_binary_subnumber(binary, 21, 25);
}

int get_rd_from_instruction_binary(InstructionBinary binary){
  return get_binary_subnumber(binary, 11, 15);
}

int get_imm_from_instruction_binary(InstructionBinary binary){
  int negative_mask = 0b00000000000000000100000000000000;
  if (negative_mask & binary) return -get_binary_subnumber(binary, 0, 13);
  else return get_binary_subnumber(binary, 0, 13);
}

// Retorna o índice (no array de registradores) do registrador de destino
int get_destination_register_from_instruction(InstructionBinary instruction){
  int op_code = get_binary_subnumber(instruction, 26, 31);

  if (
    op_code == ADD_OPCODE
  || op_code == SUB_OPCODE
  || op_code == MUL_OPCODE
  || op_code == DIV_OPCODE
  || op_code == AND_OPCODE
  || op_code == OR_OPCODE
  || op_code == NOT_OPCODE
  ) return get_rd_from_instruction_binary(instruction);

  else if (
      op_code == SUBI_OPCODE
    || op_code == ADDI_OPCODE
    || op_code == LW_OPCODE
    || op_code == SW_OPCODE
  ) return get_rt_from_instruction_binary(instruction);

  else if (
      op_code == BLT_OPCODE
    || op_code == BGT_OPCODE
    || op_code == BEQ_OPCODE
    || op_code == BNE_OPCODE
    || op_code == J_OPCODE
  ) return 0; // Usando 0 para indicar que o destino é o PC, já que o r0 não deve ser alterado


  return -1;
}

void get_operands_register_from_instruction(InstructionBinary instruction, int* op1, int* op2){
  int op_code = get_binary_subnumber(instruction, 26, 31);

  if(
     op_code == ADD_OPCODE
  || op_code == ADDI_OPCODE
  || op_code == SUB_OPCODE
  || op_code == SUBI_OPCODE
  || op_code == MUL_OPCODE
  || op_code == DIV_OPCODE
  || op_code == AND_OPCODE
  || op_code == OR_OPCODE
  || op_code == NOT_OPCODE
  ) *op1 = get_rs_from_instruction_binary(instruction), *op2 = -1;

  if(
     op_code == ADD_OPCODE
  || op_code == SUB_OPCODE
  || op_code == MUL_OPCODE
  || op_code == DIV_OPCODE
  || op_code == AND_OPCODE
  || op_code == OR_OPCODE
  ) *op2 = get_rt_from_instruction_binary(instruction);

  if(
     op_code == NOT_OPCODE
  ) *op2 = -1;

  if(
     op_code == BLT_OPCODE
  || op_code == BGT_OPCODE
  || op_code == BEQ_OPCODE
  || op_code == BNE_OPCODE
  ) *op1 = get_rt_from_instruction_binary(instruction), *op2 = -1;

  if (
    op_code == SW_OPCODE
  || op_code == LW_OPCODE
  ) *op1 = get_rt_from_instruction_binary(instruction), *op2 = get_rs_from_instruction_binary(instruction);

  if(op_code == J_OPCODE){ 
    *op1 = -1, *op2 = -1;
  }
}

bool can_uf_execute_instruction(UF_TYPE type, int opcode){
  if(type == INTEGER_UF){
    return 
      opcode == AND_OPCODE || 
      opcode == OR_OPCODE  ||
      opcode == NOT_OPCODE  ||
      opcode == BLT_OPCODE  ||
      opcode == BGT_OPCODE  ||
      opcode == BEQ_OPCODE  ||
      opcode == BNE_OPCODE  ||
      opcode == J_OPCODE  ||
      opcode == LW_OPCODE  ||
      opcode == SW_OPCODE;
  }
  else if(type == MUL_UF){
    return opcode == MUL_OPCODE ||
           opcode == DIV_OPCODE;
  }
  else if(type == ADD_UF){
    return opcode == ADD_OPCODE ||
           opcode == ADDI_OPCODE ||
           opcode == SUB_OPCODE ||
           opcode == SUBI_OPCODE;
  }
  else{
    return false;
  }
}

// instruction_index se refere à i-ésima instrução - 1
// primeira instrução => instruction_index 0
InstructionBinary get_instruction_from_memory(int instruction_index, Byte *mem){
  int mem_address = instruction_index*4 + PROGRAM_FIRST_ADDRESS;

  return ((unsigned)mem[mem_address] << 24) | (mem[mem_address + 1] << 16) | (mem[mem_address + 2] << 8) | (mem[mem_address + 3]);
}

unsigned int get_data_from_memory(int data_index, Byte *mem){
  int mem_address = data_index*4;

  return ((unsigned)mem[mem_address] << 24) | (mem[mem_address + 1] << 16) | (mem[mem_address + 2] << 8) | (mem[mem_address + 3]);
}



bool is_branch(int opcode){
  return opcode == BLT_OPCODE
      || opcode == BGT_OPCODE
      || opcode == BNE_OPCODE
      || opcode == BEQ_OPCODE
      || opcode == J_OPCODE;
}

bool is_conditional_branch(int opcode){
  return opcode == BLT_OPCODE
      || opcode == BGT_OPCODE
      || opcode == BNE_OPCODE
      || opcode == BEQ_OPCODE;
}

bool is_memory(int opcode){
  return opcode == LW_OPCODE
      || opcode == SW_OPCODE;
}




/* Debug */

void print_uf(FunctionalUnit uf){
  printf("uf.instruction_binary: %u, opcode: %d\n", uf.instruction_binary, get_opcode_from_binary(uf.instruction_binary));
  printf("uf.operand1: %d\n", uf.operand1);
  printf("uf.operand2: %d\n", uf.operand2);
  printf("uf.operation_result: %d\n", uf.operation_result);
  printf("uf.status: %d\n", uf.status);
}





/* Utilidades Scoreboard */

int get_uf_idx_from_name(FunctionalUnit *uf, FunctionalUnit *all_ufs, int total_ufs){
  for (int i = 0; i < total_ufs; i++){
    if ((*uf).type == all_ufs[i].type && (*uf).type_index == all_ufs[i].type_index) return i;
  }

  return -1;
}

// Retorna true se a unidade funcional fornecida terminou o estágio de execução
bool uf_finished_executing(FunctionalUnit *uf, CPU_Configurations cpu_configs){
  UF_TYPE type = (*uf).type;
  int cycles_to_complete;
  if (type == ADD_UF) cycles_to_complete = cpu_configs.cycles_to_complete_add;
  else if (type == MUL_UF) cycles_to_complete = cpu_configs.cycles_to_complete_mul;
  else if (type == INTEGER_UF) cycles_to_complete = cpu_configs.cycles_to_complete_integer;
  else{
    cycles_to_complete = 0;
    printf("%d\n", type);
    assert(false && "Unkown UF Type");
  }

  return uf->current_cycle == cycles_to_complete;
}

// Retorna true se o estado da instrução fornecido terminou o estágio de execução
bool scoreboard_finished_executing(InstructionState state, InstructionBinary binary, CPU_Configurations cpu_configs){
  UF_TYPE inst_uf_type = get_uf_type_from_instruction(binary);

  int cycles_to_complete;
  if (inst_uf_type == ADD_UF) cycles_to_complete = cpu_configs.cycles_to_complete_add;
  else if (inst_uf_type == MUL_UF) cycles_to_complete = cpu_configs.cycles_to_complete_mul;
  else if (inst_uf_type == INTEGER_UF) cycles_to_complete = cpu_configs.cycles_to_complete_integer;
  else{
    printf("%d\n", inst_uf_type);
    cycles_to_complete = 0;
    assert(false && "Unkown UF Type");
  }
  

  int start = state.start_execute;
  int finish = state.finish_execute;
  return finish - start + 1 == cycles_to_complete;
}

void update_rj_and_rk(int opcode, FunctionalUnitState *state, FunctionalUnit *register_status[]){
  InstructionFormat format = get_inst_format_from_opcode(opcode);
  int op1 = (*state).fj;
  int op2 = (*state).fk;
  if (opcode == J_OPCODE) (*state).rj = true; // J tem apenas um operando e é um número fixo
  else (*state).rj = register_status[op1] == NULL;

  if (format == FORMAT_R || is_conditional_branch(opcode)){
    (*state).rk = register_status[op2] == NULL;
  }
  // Operando2 é um número fixo
  else (*state).rk = true;
}

void clear_uf_state(FunctionalUnitState *uf_state){
  (*uf_state).fi = -1;
  (*uf_state).fj = -1;
  
  (*uf_state).fk = -1;
  
  (*uf_state).qj = NULL;
  (*uf_state).qk = NULL;
  
  (*uf_state).op = -1;
  
  (*uf_state).busy = false;
  (*uf_state).rj = false;
  (*uf_state).rk = false;
}


// Retorna o resultado das operações. Em casos de branch, apenas retorna true ou false
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
  
  if (opcode == BLT_OPCODE) return operand1 < operand2;
  if (opcode == BGT_OPCODE) return operand1 > operand2;
  if (opcode == BEQ_OPCODE) return operand1 == operand2;
  if (opcode == BNE_OPCODE) return operand1 != operand2;
  if (opcode == J_OPCODE)   return true;
  if (opcode == LW_OPCODE)  return operand1;
  if (opcode == SW_OPCODE)  return operand1;
  
  
  // Não é um retorno inválido, mas, com a execução correta, nunca deve chegar nesta linha
  return -1;
}

void update_finished_instructions(ScoreBoard *score_board, Byte *memory, int inst_count){
  for (int i = 0; i < inst_count; i++){
    if ((*score_board).instructions_states[i].current_state == WRITE_RESULT){
      // todo -> tirar esse get from memory
      int inst = get_instruction_from_memory(i, memory);
      int opcode = get_opcode_from_binary(inst);
      if (is_branch(opcode)) (*score_board).can_fetch = true;

      (*score_board).instructions_states[i].current_state = FINISHED;
      
      int uf_idx = (*score_board).instructions_states[i].uf_index;
      clear_uf_state(&((*score_board).ufs_states[uf_idx]));
    }
  }
}

// Checa se pode enviar alguma instrução para write result
// Senão, continua a executar
void update_write_result(Bus *bus, Byte *memory, ScoreBoard *score_board, FunctionalUnit *functional_units, CPU_Configurations cpu_configs, int curr_cycle, int inst_count){

}

// Checa se pode enviar alguma instrução para execute
void update_execute(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, int curr_cycle, int inst_count){

}

// Checa se pode enviar alguma instrução para read operands
void update_read_operands(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, int curr_cycle, int count_instructions){
  
  
}

// Checa se pode enviar alguma instrução para issue
void update_issue(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, InstructionRegister ir, CPU_Configurations cpu_configs, int curr_cycle, int total_ufs, int inst_count){
  
}

// Checa se pode enviar alguma instrução para fetch
void update_fetch(Bus *bus, Byte *memory, ScoreBoard *score_board, InstructionRegister *ir, int pc, int curr_cycle, int total_ufs, int instruction_count){
  
}

UF_TYPE get_uf_type_from_index(int uf_index, CPU_Configurations cpu_configs){
  if(uf_index < cpu_configs.size_add_ufs) return ADD_UF;
  else if(uf_index < cpu_configs.size_add_ufs+cpu_configs.size_mul_ufs) return MUL_UF;
  else return INTEGER_UF;
}



void print_instruction_status(InstructionState** instruction_states, Byte inst_opcodes[], int num_instructions){
  Table t;

  table_init(&t, INSTRUCTION_STATUS);

  for(int i = 0; i < num_instructions; i++){
    add_row(
      &t,
      inst_opcodes[i],
      (*(instruction_states))[i].fetch,
      (*(instruction_states))[i].issue,
      (*(instruction_states))[i].read_operands,
      (*(instruction_states))[i].start_execute, (*(instruction_states))[i].finish_execute,
      (*(instruction_states))[i].write_result
    );
  }

  table_print(&t);

  free_table(&t);
}

void print_functional_unit_status(FunctionalUnitState* functional_unit_states, int num_ufs){

  Table t;

  table_init(&t, FUNCTIONAL_UNIT_STATUS);

  

  for(int i = 0; i < num_ufs; i++){

    add_row(
      &t,
      functional_unit_states[i].type, functional_unit_states[i].type_index,
      functional_unit_states[i].busy,
      functional_unit_states[i].op,
      functional_unit_states[i].fi,
      functional_unit_states[i].fj,
      functional_unit_states[i].fk,
      functional_unit_states[i].qj,
      functional_unit_states[i].qk,
      functional_unit_states[i].rj,
      functional_unit_states[i].rk
    );
  }

    table_print(&t);

    free_table(&t);

}

void print_result_register_status(FunctionalUnit* result_register_state[]){

  Table t;
  table_init(&t, RESULT_REGISTER_STATUS);

  add_row(&t, result_register_state);

  table_print(&t);

  free_table(&t);

}

void print_registers(int* registers){
  Table t;
  table_init(&t, REGISTER_RESULT);

  add_row(&t, registers);

  table_print(&t);

  free_table(&t);
}

void write_instruction_result(int destination, FunctionalUnit* uf, int* registers, Bus* bus, int* program_counter){
  // write to PC
  if(destination == 0){
    add_pulse(
      bus,
      new_pulse(&(uf->operation_result), program_counter, sizeof(int))
    );
  }
  else{
    add_pulse(
      bus,
      new_pulse(&(uf->operation_result), &registers[destination], sizeof(int))
    );
  }
  dispatch_pulses(bus);
}

void actually_read_operands(int fj, int fk, int* registers, FunctionalUnit* uf, Bus* bus){

  printf("PULSO!\n");
  printf("%d %d\n", fj, fk);
  printf("%d\n", registers[fj]);

  if(fj != -1)
    add_pulse(
      bus,
      new_pulse(&(registers[fj]), &(uf->operand1), sizeof(int))
    );

  printf("PULSO!\n");
  printf("%d %d\n", fj, fk);
  
  if(fk != -1)
    add_pulse(
      bus,
      new_pulse(&registers[fk], &(uf->operand2), sizeof(int))
    );

  printf("PULSO!\n");
  printf("%d %d\n", fj, fk);

  uf->imm = get_imm_from_instruction_binary(uf->instruction_binary);

  dispatch_pulses(bus);
}

void execute_instruction(FunctionalUnit* uf, int op, int program_counter, Byte* memory){
  if (op == ADD_OPCODE)  uf->operation_result = uf->operand1 + uf->operand2;
  else if (op == ADDI_OPCODE) uf->operation_result = uf->operand1 + uf->imm; 
  else if (op == SUB_OPCODE)  uf->operation_result = uf->operand1 - uf->operand2;
  else if (op == SUBI_OPCODE) uf->operation_result = uf->operand1 - uf->imm; 
  else if (op == MUL_OPCODE)  uf->operation_result = uf->operand1 * uf->operand2;
  else if (op == DIV_OPCODE)  uf->operation_result = uf->operand1 / uf->operand2;
  else if (op == AND_OPCODE)  uf->operation_result = uf->operand1 & uf->operand2;
  else if (op == OR_OPCODE)   uf->operation_result = uf->operand1 | uf->operand2;
  else if (op == NOT_OPCODE)  uf->operation_result = ~uf->operand1;
  else if (op == BLT_OPCODE){
    uf->operation_result = program_counter + 4;

    if(uf->operand1 < uf->operand2){
      uf->operation_result += uf->imm;
    }
  }  
  else if (op == BGT_OPCODE){
    uf->operation_result = program_counter + 4;

    if(uf->operand1 > uf->operand2){
      uf->operation_result += uf->imm;
    }
  } 
  else if (op == BEQ_OPCODE){
    uf->operation_result = program_counter + 4;

    if(uf->operand1 == uf->operand2){
      uf->operation_result += uf->imm;
    }
  } 
  else if (op == BNE_OPCODE){
    uf->operation_result = program_counter + 4;

    printf("BNE = %d %d\n", uf->operand1, uf->operand2);
    if(uf->operand1 != uf->operand2){
      uf->operation_result += uf->imm;
    }
  } 
  else if (op == LW_OPCODE){
    // usar barramento pra isso
    uf->operation_result = get_data_from_memory(uf->operand1 + uf->imm, memory);
  }
  else if (op == SW_OPCODE){
    uf->operation_result = uf->operand1;
  }   
  

}


void print_table(ScoreBoard* scoreboarding, int curr_cycle, Byte inst_opcodes[], int num_instructions, int num_ufs){
  printf("*******************************************************************************************\n");
  printf("Ciclo atual: %d\n", curr_cycle);
  print_instruction_status(&scoreboarding->instructions_states, inst_opcodes, num_instructions);
  print_functional_unit_status(scoreboarding->ufs_states, num_ufs);
  print_result_register_status(scoreboarding->result_register_state);
}
