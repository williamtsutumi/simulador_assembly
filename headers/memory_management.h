#ifndef MEMORY_MANAGEMENT
#define MEMORY_MANAGEMENT

#include "types.h"

void free_memory(FILE *input, FILE *output, Bus *bus, Bus *bus_buffer, ScoreBoard *score_board, FunctionalUnit **functional_units){
  free(functional_units);

  free(bus);
  free(bus_buffer);

  free(score_board);

  fclose(input);
  fclose(output);
}

void malloc_cpu(FunctionalUnit **functional_units, CPU_Configurations cpu_configs){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  (*functional_units) = (FunctionalUnit*)malloc(total_ufs * sizeof(FunctionalUnit));

  for (int i = 0; i < total_ufs; i++){
    if (i < cpu_configs.size_add_ufs){
      (*functional_units)[i].type = ADD_UF;
      (*functional_units)[i].type_index = i;
    }
      
    else if (i < cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs){
      (*functional_units)[i].type = MUL_UF;
      (*functional_units)[i].type_index = i - cpu_configs.size_add_ufs;
    }
    else{
      (*functional_units)[i].type = INTEGER_UF;
      (*functional_units)[i].type_index = i - (cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs);

    }
  }
}

void malloc_bus(Bus *bus, Bus *bus_buffer, CPU_Configurations cpu_configs, int memory_size){
  int num_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;

  (*bus).ufs_state = (FunctionalUnitStatus*)malloc(sizeof(FunctionalUnitStatus) * num_ufs);
  (*bus).ufs_data[0] = (UF_DataSignal*)malloc(sizeof(UF_DataSignal) * num_ufs);
  (*bus).ufs_data[1] = (UF_DataSignal*)malloc(sizeof(UF_DataSignal) * num_ufs);
  (*bus).memory = (DataSignal*)malloc(sizeof(DataSignal) * memory_size);

  (*bus_buffer).ufs_state = (FunctionalUnitStatus*)malloc(sizeof(FunctionalUnitStatus) * num_ufs);
  (*bus_buffer).ufs_data[0] = (UF_DataSignal*)malloc(sizeof(UF_DataSignal) * num_ufs);
  (*bus_buffer).ufs_data[1] = (UF_DataSignal*)malloc(sizeof(UF_DataSignal) * num_ufs);
  (*bus_buffer).memory = (DataSignal*)malloc(sizeof(DataSignal) * memory_size);
}

void malloc_scoreboard(ScoreBoard *score_board, CPU_Configurations cpu_configs, FunctionalUnit *functional_units, int num_instructions){

  // instructions states
  (*score_board).instructions_states = (InstructionState*)malloc(num_instructions * sizeof(InstructionState));

  for(int i = 0; i < num_instructions; i++){
      (*score_board).instructions_states[i].current_state = 
      (*score_board).instructions_states[i].fetch = 
      (*score_board).instructions_states[i].issue = 
      (*score_board).instructions_states[i].read_operands = 
      (*score_board).instructions_states[i].start_execute = 
      (*score_board).instructions_states[i].finish_execute = 
      (*score_board).instructions_states[i].write_result = -1; 
  }

  // ufs states
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  (*score_board).ufs_states = (FunctionalUnitState*)malloc(total_ufs * sizeof(FunctionalUnitState));
  for(int i = 0; i < total_ufs; i++){

    (*score_board).ufs_states[i].type = functional_units[i].type;
    (*score_board).ufs_states[i].type_index = functional_units[i].type_index;
    (*score_board).ufs_states[i].fi = -1;
    (*score_board).ufs_states[i].fj = -1;
    (*score_board).ufs_states[i].fk = -1;
    (*score_board).ufs_states[i].qj = -1;
    (*score_board).ufs_states[i].qk = -1;
    (*score_board).ufs_states[i].op = -1;
    (*score_board).ufs_states[i].rj = 0;
    (*score_board).ufs_states[i].rk = 0;

    (*score_board).ufs_states[i].busy = false;
  }
}

void malloc_memory(FunctionalUnit **functional_units, ScoreBoard *score_board, Bus *bus, Bus *bus_buffer, CPU_Configurations cpu_configs, int num_instructions, int memory_size){
  malloc_cpu(functional_units, cpu_configs);
  malloc_bus(bus, bus_buffer, cpu_configs, memory_size);
  malloc_scoreboard(score_board, cpu_configs, *functional_units, num_instructions);
}

void init_scoreboard(ScoreBoard *score_board){
  (*score_board).can_fetch = true;
  // (*score_board).can_issue = true;
  // (*score_board).can_read_operands = true;
  // (*score_board).can_execute = true;
  // (*score_board).can_write_result = true;
}

void init_functional_units(FunctionalUnit *functional_units, CPU_Configurations cpu_configs){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    functional_units[uf_index].current_cycle = 0;
  }
}

#endif