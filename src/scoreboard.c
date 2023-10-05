#include "../headers/scoreboard.h"
#include "../headers/helpers.h"


static void update_rj_and_rk(int opcode, FunctionalUnitState *state, FunctionalUnit *register_status[]){
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

// Retorna true se o estado da instrução fornecido terminou o estágio de execução
static bool scoreboard_finished_executing(InstructionState state, InstructionBinary binary, CPU_Configurations cpu_configs){
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


static void clear_uf_state(FunctionalUnitState *uf_state){
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

static void update_finished_instructions(ScoreBoard *score_board, Byte *memory, int inst_count){
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
static void update_write_result(Bus *bus, Byte *memory, ScoreBoard *score_board, FunctionalUnit *functional_units, CPU_Configurations cpu_configs, int curr_cycle, int inst_count){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;

  int count_instructions_sent_to_write = 0;
  for (int inst_idx = 0; inst_idx < inst_count; inst_idx++){
    if ((*score_board).instructions_states[inst_idx].current_state != EXECUTE) continue;

    int uf_idx = (*score_board).instructions_states[inst_idx].uf_index;
    InstructionBinary binary = get_instruction_from_memory(inst_idx, memory);
    int opcode = get_opcode_from_binary(binary);

    bool can_write = true;
    for (int i = 0; i < total_ufs; i++){
      if (i == uf_idx) continue; // Ignorar se é ele mesmo alterando o registrador
      
      if ((*score_board).ufs_states[i].fj == (*score_board).ufs_states[uf_idx].fi
        && (*score_board).ufs_states[i].rj) can_write = false;

      if (opcode == ADDI_OPCODE || opcode == SUBI_OPCODE) continue;

      if ((*score_board).ufs_states[i].fk == (*score_board).ufs_states[uf_idx].fi
        && (*score_board).ufs_states[i].rk) can_write = false;
    }

    
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

        for (int i = 0; i < total_ufs; i++){
          if ((*score_board).ufs_states[i].qj == uf_idx)
            (*score_board).ufs_states[i].rj = true;

          if ((*score_board).ufs_states[i].qk == uf_idx)
            (*score_board).ufs_states[i].rk = true;
        }
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
static void update_execute(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, int curr_cycle, int inst_count){
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
static void update_read_operands(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, int curr_cycle, int count_instructions){
  int count_inst_sent_to_read_operands = 0;

  for (int inst_idx = 0; inst_idx < count_instructions; inst_idx++){
    if ((*score_board).instructions_states[inst_idx].current_state != ISSUE
        && (*score_board).instructions_states[inst_idx].current_state != STAY_ISSUE) continue;

    int uf_idx = (*score_board).instructions_states[inst_idx].uf_index;
    
    // Condição para controle de dependencias
    if ((*score_board).ufs_states[uf_idx].rj == false
        || (*score_board).ufs_states[uf_idx].rk == false){

      (*score_board).instructions_states[inst_idx].current_state = STAY_ISSUE;

      add_pulse(bus,
      new_data_pulse(STALL_ISSUE, &(functional_units[uf_idx].status), sizeof(FunctionalUnitStatus)));

      continue;
    }

    if (count_inst_sent_to_read_operands < READ_OPERANDS_CAPACITY){
      count_inst_sent_to_read_operands++;

      (*score_board).instructions_states[inst_idx].current_state = READ_OPERANDS;
      (*score_board).instructions_states[inst_idx].read_operands = curr_cycle;

      add_pulse(bus, 
      new_data_pulse(CONTINUE_READ_OPERAND, &(functional_units[uf_idx].status), sizeof(FunctionalUnitStatus)));
    }
  }
}

// Checa se pode enviar alguma instrução para issue
static void update_issue(Bus *bus, FunctionalUnit *functional_units, ScoreBoard *score_board, InstructionRegister ir, int curr_cycle, int total_ufs, int inst_count){
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

        break;
      }
    }
    if (idle_uf_index != -1) break;
  }
  if(idle_uf_index == -1) return;

  add_pulse(bus,
  new_data_pulse(CONTINUE_ISSUE, &(functional_units[idle_uf_index].status), sizeof(FunctionalUnitStatus)));

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
  

  int destination = get_destination_register_from_instruction(ir.binary);
  (*score_board).result_register_state[destination] = &functional_units[idle_uf_index];
}

// Checa se pode enviar alguma instrução para fetch
static void update_fetch(ScoreBoard *score_board, int pc, int curr_cycle, int total_ufs){
  if ((*score_board).can_fetch){

    int instruction_index = (pc - PROGRAM_FIRST_ADDRESS) / 4;
    if ((*score_board).instructions_states[instruction_index].current_state == READ_OPERANDS
      || (*score_board).instructions_states[instruction_index].current_state == ISSUE
      || (*score_board).instructions_states[instruction_index].current_state == EXECUTE
      || (*score_board).instructions_states[instruction_index].current_state == WRITE_RESULT
    )
      return;

    (*score_board).can_fetch = false;


    (*score_board).instructions_states[instruction_index].current_state = FETCH;
    (*score_board).instructions_states[instruction_index].fetch = curr_cycle;

  }
}

// Controle do scoreboard

void update_scoreboard(ScoreBoard* score_board, Bus* bus, Byte* memory, FunctionalUnit functional_units[], CPU_Configurations cpu_configs, int curr_cycle, int inst_count, InstructionRegister ir, int pc){

  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;


  update_finished_instructions(score_board,
      memory,
      inst_count);
  
      
  update_write_result(bus,
      memory,
      score_board,
      functional_units,
      cpu_configs,
      curr_cycle,
      inst_count);
  
      
  update_execute(bus,
      functional_units,
      score_board,
      curr_cycle,
      inst_count);
  
      
  update_read_operands(bus,
      functional_units,
      score_board,
      curr_cycle,
      inst_count);
  
      
  update_issue(bus,
      functional_units,
      score_board,
      ir,
      curr_cycle,
      total_ufs,
      inst_count);
  
      
  update_fetch(score_board,
      pc,
      curr_cycle,
      total_ufs);
}