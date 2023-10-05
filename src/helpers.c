#include "../headers/types.h"
#include "../headers/bus.h"
#include "../headers/table.h"
#include "../headers/colors.h"
#include "../headers/scoreboard.h"

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
int get_binary_subnumber(InstructionBinary instruction, int start_bit, int end_bit){
  instruction >>= start_bit;
  int bitmask = (1ll << (end_bit - start_bit + 1))-1;

  return instruction & bitmask;
}

UF_TYPE get_uf_type_from_instruction(InstructionBinary instruction){
  int op_code = get_binary_subnumber(instruction, 26, 31);
  return get_uf_type_from_opcode(op_code);
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
  ) return get_binary_subnumber(instruction, 11, 15);

  else if (
      op_code == SUBI_OPCODE
    || op_code == ADDI_OPCODE
    || op_code == LW_OPCODE
    || op_code == SW_OPCODE
  ) return get_binary_subnumber(instruction, 16, 20);

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
    op_code == SW_OPCODE
  || op_code == LW_OPCODE
  ) *op1 = get_binary_subnumber(instruction, 21, 25), *op2 = get_binary_subnumber(instruction, 0, 15);

  else if (
      op_code == SUBI_OPCODE
    || op_code == ADDI_OPCODE
    || op_code == NOT_OPCODE
  ) *op1 = get_binary_subnumber(instruction, 21, 25), *op2 = get_binary_subnumber(instruction, 0, 15);

  else if (
      op_code == BLT_OPCODE
    || op_code == BGT_OPCODE
    || op_code == BEQ_OPCODE
    || op_code == BNE_OPCODE
  ) *op1 = get_binary_subnumber(instruction, 21, 25), *op2 = get_binary_subnumber(instruction, 16, 20);

  else{ // J
    *op1 = -1, *op2 = -1;
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
  if (type == MUL_UF) cycles_to_complete = cpu_configs.cycles_to_complete_mul;
  if (type == INTEGER_UF) cycles_to_complete = cpu_configs.cycles_to_complete_integer;

  return (*uf).current_cycle == cycles_to_complete;
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
  // if (opcode == LW_OPCODE)  return operand1; LW não usa essa função
  if (opcode == SW_OPCODE)  return operand1;
  
  
  // Não é um retorno inválido, mas, com a execução correta, nunca deve chegar nesta linha
  return -1;
}


