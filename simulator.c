#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

/*
TODO -> decidir como setar o barramento e quando de fato enviar as informações do barramento
*/

#include "assembly_parser.h"
#include "types.h"
#include "helpers.h"
#include "configuration.h"


CPU_Configurations g_cpu_configs;
FunctionalUnit *g_functional_units;
ScoreBoard g_score_board;
Bus g_bus;

int *g_memory;
int *g_registers;
const char *g_code_file_name;
int *g_instructions;
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = 0; // PC
InstructionRegister g_instruction_register; // IR

bool read_args(int argc, char *argv[], int *memory_size, char **input_file_name, FILE **input_file, FILE **output_stream){
  for(int i = 1; i < argc; i+=2){

    if(strcmp(argv[i], "-p") == 0){
      *input_file_name = argv[i+1];
    }
    else if(strcmp(argv[i], "-m") == 0){
      // TODO
      // validar que argv[i+1] é uma inteiro
      *memory_size = atoi(argv[i+1]);
      g_memory = (int*)(malloc(sizeof(int)*(*memory_size)));
    }
    else if(strcmp(argv[i], "-o") == 0){
      *output_stream = fopen(argv[i+1], "w");
    }
  }

  *input_file = fopen(*input_file_name, "r");
  return *input_file != NULL;
}

void fetch_next_instruction(){
  g_score_board.instructions_states[g_program_counter].current_state = FETCH;
  g_score_board.instructions_states[g_program_counter].fetch = g_current_cycle;
  
  g_instruction_register.binary = g_instructions[g_program_counter];
  g_instruction_register.program_counter = g_program_counter;
  g_program_counter++;
}

void issue_instruction(){
  g_score_board.instructions_states[g_instruction_register.program_counter].current_state = ISSUE;
  g_score_board.instructions_states[g_instruction_register.program_counter].issue = g_current_cycle;

  UF_TYPE type = get_uf_type_from_instruction(g_instruction_register.binary);

  int idle_uf_index;
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_score_board.ufs_states[i].type == type && !g_score_board.ufs_states[i].busy){
      idle_uf_index = i;
    }
  }

  g_score_board.ufs_states[idle_uf_index].busy = true;
  g_score_board.ufs_states[idle_uf_index].inst_program_counter = g_instruction_register.binary;
}

void read_operands(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_score_board.instructions_states[i].current_state == ISSUE){
      g_score_board.instructions_states[i].current_state = READ_OPERANDS;
      g_score_board.instructions_states[i].read_operands = g_current_cycle;
    }
  }
}

void execute(){
  int inst;
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_score_board.instructions_states[i].current_state == READ_OPERANDS){
      g_score_board.instructions_states[i].current_state = EXECUTE;
      g_score_board.instructions_states[i].execute = g_current_cycle;
    }
  }
}

void write_result(){
  int inst;
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_score_board.instructions_states[i].current_state == EXECUTE){
      g_score_board.instructions_states[i].current_state = WRITE_RESULT;
      g_score_board.instructions_states[i].write_result = g_current_cycle;
    }
  }
}

void run_one_cycle(FILE *output){
  g_current_cycle++;

  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  print_table(&g_score_board, g_current_cycle, g_instruction_count, total_ufs);

  write_result();
  execute();
  if (g_score_board.can_read_operands) read_operands();
  if (g_score_board.can_issue) issue_instruction();
  if (g_bus.instruction_register == CONTINUE) fetch_next_instruction();
}

void run_simulation(FILE *output){
  bool has_active_instruction = true;
  while (has_active_instruction){
    getchar();
    run_one_cycle(output);
  }
}

/* Gerenciamento de memória */

void free_memory(FILE *input, FILE*output){
  free(g_functional_units);
  free(g_instructions);

  free(g_bus.ufs_data);
  free(g_bus.ufs_state);

  free(g_score_board.instructions_states);
  free(g_score_board.ufs_states);

  fclose(input);
  fclose(output);
}

void malloc_cpu(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  g_functional_units = (FunctionalUnit*)malloc(total_ufs * sizeof(FunctionalUnit));
  for (int i=0; i<total_ufs; i++){
    if (i < g_cpu_configs.size_add_ufs)
      (g_functional_units)[i].type = ADD_UF;
    else if (i < g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs)
      (g_functional_units)[i].type = MUL_UF;
    else
      (g_functional_units)[i].type = INTEGER_UF;
  }
}

void malloc_ufs_states(){
    int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
    g_score_board.ufs_states = (FunctionalUnitState*)malloc(total_ufs * sizeof(FunctionalUnitState));
    for(int i = 0; i < total_ufs; i++){
      g_score_board.ufs_states[i].type = g_functional_units[i].type;
      g_score_board.ufs_states[i].fi =
      g_score_board.ufs_states[i].fj =
      g_score_board.ufs_states[i].fk =
      g_score_board.ufs_states[i].qj =
      g_score_board.ufs_states[i].qk =
      g_score_board.ufs_states[i].op = -1;
    }

}

void malloc_instruction_states(){
    int num_instructions = g_instruction_count;

    g_score_board.instructions_states = (InstructionState*)malloc(num_instructions * sizeof(InstructionState));
    int num_allocated_inst = sizeof(g_score_board.instructions_states)/sizeof(InstructionState);

    for(int i = 0; i < num_instructions; i++){
      g_score_board.instructions_states[i].current_state = 
      g_score_board.instructions_states[i].fetch = 
      g_score_board.instructions_states[i].issue = 
      g_score_board.instructions_states[i].read_operands = 
      g_score_board.instructions_states[i].execute = 
      g_score_board.instructions_states[i].write_result = -1; 

    }
}

void malloc_bus(){
  int num_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  g_bus.ufs_state = (ControlSignal*)malloc(sizeof(ControlSignal) * num_ufs);
  g_bus.ufs_data = (DataSignal*)malloc(sizeof(DataSignal) * num_ufs);
}

void malloc_scoreboard(){
  malloc_instruction_states();
  malloc_ufs_states();
}

void malloc_memory(){
  malloc_cpu();
  malloc_bus();
  malloc_scoreboard();

}

void init_scoreboard(){
  g_score_board.can_fetch = true;
  g_score_board.can_issue = true;
  g_score_board.can_read_operands = true;
  g_score_board.can_execute = true;
  g_score_board.can_write_result = true;
}

int main(int argc, char *argv[])
{
  g_code_file_name = argv[0];


  int memory_size = 32;
  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &memory_size, &input_file_name, &input_file, &output_stream)){
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);
    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instructions, &g_instruction_count)){
      malloc_memory();
      init_scoreboard();
      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  free_memory(input_file, output_stream);

  return 0;

}
