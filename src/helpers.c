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

static void get_operands_register_from_instruction(InstructionBinary instruction, int* op1, int* op2){
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

// Retorna true se o estado da instrução fornecido terminou o estágio de execução
bool scoreboard_finished_executing(InstructionState state, InstructionBinary binary, CPU_Configurations cpu_configs){
  UF_TYPE inst_uf_type = get_uf_type_from_instruction(binary);

  int cycles_to_complete;
  if (inst_uf_type == ADD_UF) cycles_to_complete = cpu_configs.cycles_to_complete_add;
  else if (inst_uf_type == MUL_UF) cycles_to_complete = cpu_configs.cycles_to_complete_mul;
  else if (inst_uf_type == INTEGER_UF) cycles_to_complete = cpu_configs.cycles_to_complete_integer;

  int start = state.start_execute;
  int finish = state.finish_execute;
  int uf_idx = state.uf_index;
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
  (*uf_state).qj = -1;
  (*uf_state).qk = -1;
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
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;

  int count_instructions_sent_to_write = 0;
  for (int inst_idx = 0; inst_idx < inst_count; inst_idx++){
    if ((*score_board).instructions_states[inst_idx].current_state != EXECUTE) continue;

    int uf_idx = (*score_board).instructions_states[inst_idx].uf_index;
    InstructionBinary binary = get_instruction_from_memory(inst_idx, memory);
    int opcode = get_opcode_from_binary(binary);

    bool can_write = true;
    for (int i = 0; i < total_ufs; i++){
      if ((*score_board).ufs_states[i].fj == (*score_board).ufs_states[uf_idx].fi
        && (*score_board).ufs_states[i].rj) can_write = false;

      if (opcode == ADDI_OPCODE || opcode == SUBI_OPCODE) continue;

      if ((*score_board).ufs_states[i].fk == (*score_board).ufs_states[uf_idx].fi
        && (*score_board).ufs_states[i].rk) can_write = false;
    }


    int cycles_to_complete;
    // todo -> tirar esse get from memory
    
    
    
    if (scoreboard_finished_executing((*score_board).instructions_states[inst_idx], binary, cpu_configs)){

      // add_pulse(bus, 
      // new_data_pulse(STALL, &(functional_units[uf_idx].status), sizeof(FunctionalUnitStatus)));
      int destination = get_destination_register_from_instruction(binary);
      if (count_instructions_sent_to_write < WRITE_RESULT_CAPACITY && can_write){
        count_instructions_sent_to_write++;

        add_pulse(bus, 
        new_data_pulse(CONTINUE_WRITE_RESULT, &(functional_units[uf_idx].status), sizeof(FunctionalUnitStatus)));

        (*score_board).instructions_states[inst_idx].current_state = WRITE_RESULT;
        (*score_board).instructions_states[inst_idx].write_result = curr_cycle;
        
        (*score_board).result_register_state[destination] = NULL;

        red();
        printf("Destination: %d\n", destination);
        for (int i = 0; i < total_ufs; i++){
          printf("qj: %d\n", (*score_board).ufs_states[i].qj);
          printf("qk: %d\n", (*score_board).ufs_states[i].qk);
          if ((*score_board).ufs_states[i].qj == destination)
            (*score_board).ufs_states[i].rj = true;

          if ((*score_board).ufs_states[i].qk == destination)
            (*score_board).ufs_states[i].rk = true;
        }
        reset();
      }
    }
    else{
      add_pulse(bus, 
      new_data_pulse(CONTINUE_EXECUTE, &(functional_units[uf_idx].status), sizeof(FunctionalUnitStatus)));
      
      (*score_board).instructions_states[inst_idx].finish_execute++;
    }
  }
}

// Checa se pode enviar alguma instrução para execute
void update_execute(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, int curr_cycle, int inst_count){
  for (int i = 0; i < inst_count; i++){
    if ((*score_board).instructions_states[i].current_state == READ_OPERANDS){

      (*score_board).instructions_states[i].current_state = EXECUTE;
      (*score_board).instructions_states[i].start_execute = curr_cycle;
      (*score_board).instructions_states[i].finish_execute = curr_cycle;

      int uf_index = (*score_board).instructions_states[i].uf_index;
      add_pulse(bus, 
      new_data_pulse(CONTINUE_EXECUTE, &(functional_units[uf_index].status), sizeof(FunctionalUnitStatus)));
      //(*bus_buffer).ufs_state[uf_index] = CONTINUE_EXECUTE;
    }
  }
}

// Checa se pode enviar alguma instrução para read operands
void update_read_operands(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, int curr_cycle, int count_instructions){
  for (int inst_idx = 0; inst_idx < count_instructions; inst_idx++){
    if ((*score_board).instructions_states[inst_idx].current_state != ISSUE) continue;

    int uf_idx = (*score_board).instructions_states[inst_idx].uf_index;
    // Condição para controle de dependencias
    if ((*score_board).ufs_states[uf_idx].rj == false
        || (*score_board).ufs_states[uf_idx].rk == false) continue;

    (*score_board).instructions_states[inst_idx].current_state = READ_OPERANDS;
    (*score_board).instructions_states[inst_idx].read_operands = curr_cycle;

    add_pulse(bus, 
    new_data_pulse(CONTINUE_READ_OPERAND, &(functional_units[uf_idx].status), sizeof(FunctionalUnitStatus)));
  }
}

// Checa se pode enviar alguma instrução para issue
void update_issue(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, InstructionRegister ir, CPU_Configurations cpu_configs, int curr_cycle, int total_ufs, int inst_count){
  int opcode = get_opcode_from_binary(ir.binary);
  if (opcode == EXIT_OPCODE) return;

  UF_TYPE type = get_uf_type_from_instruction(ir.binary);
  int idle_uf_index = -1;
  for (int i = 0; i < inst_count; i++){
    if ((*score_board).instructions_states[i].current_state != FETCH) continue;
  
    // Condição para controle de dependencias
    int destination = get_destination_register_from_instruction(ir.binary);
    if ((*score_board).result_register_state[destination] != NULL) continue;

    for (int uf_index = 0; uf_index < total_ufs; uf_index++){

      FunctionalUnitState uf_state = (*score_board).ufs_states[uf_index];
      if (uf_state.type == type && !uf_state.busy){
        idle_uf_index = uf_index;
        // printf("type: %d\n", type);
        // printf("uf type: %d\n", uf_state.type);
        // yellow();
        // printf("Dando issue na uf de idx: %d\n", idle_uf_index);
        // reset();
        add_pulse(bus, 
        new_data_pulse(CONTINUE_ISSUE, &(functional_units[idle_uf_index].status), sizeof(FunctionalUnitStatus)));
        break;
      }
    }
  }
  if(idle_uf_index == -1){
    // printf("não tem unidades funcionais livres!\n");
    return;
  }

  if (!is_branch(opcode)) (*score_board).can_fetch = true;
  
  
  (*score_board).instructions_states[(ir.program_counter - PROGRAM_FIRST_ADDRESS) / 4].current_state = ISSUE;
  (*score_board).instructions_states[(ir.program_counter - PROGRAM_FIRST_ADDRESS) / 4].issue = curr_cycle;
  (*score_board).instructions_states[(ir.program_counter - PROGRAM_FIRST_ADDRESS) / 4].uf_index = idle_uf_index;


  (*score_board).ufs_states[idle_uf_index].busy = true;
  (*score_board).ufs_states[idle_uf_index].fi = get_destination_register_from_instruction(ir.binary);
  int op1, op2;
  get_operands_register_from_instruction(ir.binary, &op1, &op2);
  (*score_board).ufs_states[idle_uf_index].fj = op1;
  (*score_board).ufs_states[idle_uf_index].fk = op2;
  (*score_board).ufs_states[idle_uf_index].op = get_binary_subnumber(ir.binary, 26, 31);

  update_rj_and_rk(opcode, &(*score_board).ufs_states[idle_uf_index], (*score_board).result_register_state);

  if ((*score_board).ufs_states[idle_uf_index].rj){
    (*score_board).ufs_states[idle_uf_index].qj = -1;
  }
  else{
    int uf_idx = get_uf_idx_from_name((*score_board).result_register_state[op1], functional_units, total_ufs);
    (*score_board).ufs_states[idle_uf_index].qj = uf_idx;
  }

  if ((*score_board).ufs_states[idle_uf_index].rk){
    (*score_board).ufs_states[idle_uf_index].qk = -1;
  }
  else{
    int uf_idx = get_uf_idx_from_name((*score_board).result_register_state[op2], functional_units, total_ufs);
    (*score_board).ufs_states[idle_uf_index].qk = uf_idx;
  }
  
  red();
  printf("Tá no update issue\n");
  printf("op1: %d\n", op1);
  printf("op2: %d\n", op2);
  printf("qj: %d\n", (*score_board).ufs_states[idle_uf_index].qj);
  printf("qk: %d\n", (*score_board).ufs_states[idle_uf_index].qk);
  reset();

  int destination = get_destination_register_from_instruction(ir.binary);
  (*score_board).result_register_state[destination] = &functional_units[idle_uf_index];
}

// Checa se pode enviar alguma instrução para fetch
void update_fetch(Bus *bus, Byte *memory, ScoreBoard *score_board, InstructionRegister *ir, int pc, int curr_cycle, int total_ufs, int instruction_count){
  if ((*score_board).can_fetch){

    int instruction_index = (pc - PROGRAM_FIRST_ADDRESS) / 4;

    (*score_board).can_fetch = false;
    // printf("inst index: %d\n", instruction_index);
    // if(instruction_count <= instruction_index){
    //   assert(false && "acabou as intruções pra serem fetchadas");
    // }

    (*score_board).instructions_states[instruction_index].current_state = FETCH;
    (*score_board).instructions_states[instruction_index].fetch = curr_cycle;

    // printf("instruction index no update fetch: %d\n", instruction_index);
    // printf("ir binary no update fetch: %d\n", (*bus_buffer).ir_binary.data);
  }
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
    int qj = functional_unit_states[i].qj;
    int qj_type = qj != -1 ? functional_unit_states[qj].type : -1;
    int qj_type_index = qj != -1 ? functional_unit_states[qj].type_index : -1;

    int qk = functional_unit_states[i].qk;
    int qk_type = qk != -1 ? functional_unit_states[qk].type : -1;
    int qk_type_index = qk != -1 ? functional_unit_states[qk].type_index : -1;

    add_row(
      &t,
      functional_unit_states[i].type, functional_unit_states[i].type_index,
      functional_unit_states[i].busy,
      functional_unit_states[i].op,
      functional_unit_states[i].fi,
      functional_unit_states[i].fj,
      functional_unit_states[i].fk,
      qj_type, qj_type_index,
      qk_type, qk_type_index,
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

void print_table(ScoreBoard* scoreboarding, int curr_cycle, Byte inst_opcodes[], int num_instructions, int num_ufs){
  printf("*******************************************************************************************\n");
  printf("Ciclo atual: %d\n", curr_cycle);
  print_instruction_status(&scoreboarding->instructions_states, inst_opcodes, num_instructions);
  print_functional_unit_status(scoreboarding->ufs_states, num_ufs);
  print_result_register_status(scoreboarding->result_register_state);
}
