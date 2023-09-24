#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#include "headers/assembly_parser.h"
#include "headers/types.h"
#include "headers/helpers.h"
#include "headers/memory_management.h"

CPU_Configurations g_cpu_configs;
FunctionalUnit *g_functional_units;
ScoreBoard g_score_board;
Bus g_bus;
Bus g_bus_buffer;

Byte *g_memory;
int g_memory_size = 32;

int *g_registers;
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = PROGRAM_FIRST_ADDRESS; // PC
InstructionRegister g_instruction_register; // IR

/* Estágios do pipeline */

void fetch_next_instruction(){
}
void issue_instruction(){
  
}
void read_operands(){
  
}
void execute(){
  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state == EXECUTE){
      
    }
  }
}
void write_result(){
  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state == WRITE_RESULT){
      

    }
  }
}

/************************/

void update_scoreboard(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  update_write_result(g_memory, &g_score_board, g_cpu_configs, g_current_cycle, g_instruction_count);
  update_execute(&g_score_board, g_current_cycle, g_instruction_count);
  update_read_operands(&g_score_board, g_current_cycle, total_ufs);
  update_issue(&g_score_board, g_instruction_register, g_current_cycle, total_ufs);
  update_fetch(g_memory, &g_score_board, &g_instruction_register, &g_program_counter, g_current_cycle, total_ufs);
}

void send_data_to_bus(){

}

void receive_data_from_bus(){

}

void run_one_cycle(FILE *output){
  g_current_cycle++;

  update_scoreboard();
  send_data_to_bus();
  receive_data_from_bus();

  write_result();
  execute();
  // Possívelmente não vai ter essas três
  // read_operands();
  // issue_instruction();
  // fetch_next_instruction();

  // Imprimindo tabelas, não fiel à simulação
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  Byte inst_opcodes[g_instruction_count];
  for (int i = 0; i < g_instruction_count; i++){
    int inst = get_instruction_from_memory(i, g_memory);
    int opcode = get_opcode_from_binary(inst);
    inst_opcodes[i] = opcode;
    printf("opcode: %d\n", inst_opcodes[i]);
  }
  print_table(&g_score_board, g_current_cycle, inst_opcodes, g_instruction_count, total_ufs);
  // ****************************************

}

void run_simulation(FILE *output){
  bool has_active_instruction = true;
  while (has_active_instruction){
    getchar();
    run_one_cycle(output);
  }
}

bool read_args(int argc, char *argv[], char **input_file_name, FILE **input_file, FILE **output_stream){
  for(int i = 1; i < argc; i+=2){

    if(strcmp(argv[i], "-p") == 0){
      *input_file_name = argv[i+1];
    }
    else if(strcmp(argv[i], "-m") == 0){
      // todo -> validar que argv[i+1] é uma inteiro
      g_memory_size = atoi(argv[i+1]);
    }
    else if(strcmp(argv[i], "-o") == 0){
      *output_stream = fopen(argv[i+1], "w");
    }
  }

  g_memory = (Byte*)(malloc(sizeof(Byte)* (PROGRAM_FIRST_ADDRESS + g_memory_size*4)));
  *input_file = fopen(*input_file_name, "r");
  return *input_file != NULL;
}

int main(int argc, char *argv[])
{
  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &input_file_name, &input_file, &output_stream)){
    
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);

    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instruction_count, &g_memory, g_memory_size)){
      malloc_memory(&g_functional_units, &g_score_board, &g_bus, g_cpu_configs, g_instruction_count);
      init_scoreboard(&g_score_board);

      // printf("AAAAAAA: %d\n", g_memory[400]);
      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  free_memory(input_file, output_stream, &g_bus, &g_score_board, &g_functional_units);

  return 0;

}
