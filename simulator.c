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
int *g_registrers;
const char *g_code_file_name;
int *g_instructions;
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = 0; // PC
int g_instruction_register; // IR

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
  if (g_bus.instruction_register == CONTINUE){
    g_instruction_register = g_instructions[g_program_counter];
    g_program_counter++;

    g_bus.instruction_register = STALL;
  }
}

void issue_instruction(){
  FunctionalUnitState instruction_info = get_instruction_information(g_instruction_register);

  if (has_idle_uf(instruction_info)){
    FunctionalUnitState *uf = find_uf_with_type(instruction_info.type);
    *uf = instruction_info;

    g_bus.instruction_register = CONTINUE;
  }
}

void read_operands(){

}

void execute(){
  
}

void write_result(){
  
}

void run_one_cycle(FILE *output){
  g_current_cycle++;

  write_result();
  execute();
  read_operands();
  issue_instruction();
  fetch_next_instruction();
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

void malloc_bus(){
  int num_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  g_bus.ufs_state = (ControlSignal*)malloc(sizeof(ControlSignal) * num_ufs);
  g_bus.ufs_data = (DataSignal*)malloc(sizeof(DataSignal) * num_ufs);
}

void malloc_memory(){
  malloc_cpu();
  malloc_bus();
}

int main(int argc, char *argv[])
{
  g_code_file_name = argv[0];

  __table_tests();

  int memory_size = 32;
  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &memory_size, &input_file_name, &input_file, &output_stream)){
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);
    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instructions)){
      malloc_memory();
      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  free_memory(input_file, output_stream);

  return 0;

}
