#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

/*
TODO -> decidir como setar o barramento e quando de fato enviar as informações do barramento
*/

#include "headers/assembly_parser.h"
#include "headers/types.h"
#include "headers/helpers.h"
#include "headers/memory_management.h"

CPU_Configurations g_cpu_configs;
FunctionalUnit *g_functional_units;
ScoreBoard g_score_board;
Bus g_bus;

Byte *g_memory;
int g_memory_size = 32;
int *g_registers;
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = PROGRAM_FIRST_ADDRESS; // PC
InstructionRegister g_instruction_register; // IR

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

void fetch_next_instruction(){
  int instruction_index = (g_program_counter - PROGRAM_FIRST_ADDRESS) / 4;
  if(g_instruction_count <= instruction_index){
    // assert(false && "acabou as intruções pra serem fetchadas");
  }
  g_score_board.instructions_states[instruction_index].current_state = FETCH;
  g_score_board.instructions_states[instruction_index].fetch = g_current_cycle;
  
  g_instruction_register.binary = get_instruction_from_memory(instruction_index, g_memory);
  g_instruction_register.program_counter = g_program_counter;
  g_program_counter += 4;

  // Controle do pipeline
  g_score_board.can_fetch = false;
  //*********************
}

void issue_instruction(){
  UF_TYPE type = get_uf_type_from_instruction(g_instruction_register.binary);

  int idle_uf_index=-1;
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_score_board.ufs_states[i].type == type && !g_score_board.ufs_states[i].busy){
      idle_uf_index = i;
      break;
    }
  }
  if(idle_uf_index == -1){
    printf("não tem unidades funcionais livres!\n");
    return;
  }

  // Controle do pipeline
  g_score_board.can_fetch = true;
  //*********************
  g_score_board.instructions_states[(g_instruction_register.program_counter - PROGRAM_FIRST_ADDRESS) / 4].current_state = ISSUE;
  g_score_board.instructions_states[(g_instruction_register.program_counter - PROGRAM_FIRST_ADDRESS) / 4].issue = g_current_cycle;


  g_score_board.ufs_states[idle_uf_index].busy = true;
  g_score_board.ufs_states[idle_uf_index].inst_program_counter = g_instruction_register.binary;

  g_score_board.ufs_states[idle_uf_index].fi = get_destination_register_from_instruction(g_instruction_register.binary);
  int op1, op2;
  get_operands_register_from_instruction(g_instruction_register.binary, &op1, &op2);
  g_score_board.ufs_states[idle_uf_index].fj = op1;
  g_score_board.ufs_states[idle_uf_index].fk = op2;
  g_score_board.ufs_states[idle_uf_index].op = get_binary_subnumber(g_instruction_register.binary, 26, 31);

  g_score_board.result_register_state[rand()%32] = &g_functional_units[rand()%total_ufs];
  //g_score_board.ufs_states[idle_uf_index].qj = g_score_board.result_register_state[g_score_board.ufs_states[idle_uf_index].fj]->type;
  //g_score_board.ufs_states[idle_uf_index].qk = g_score_board.result_register_state[g_score_board.ufs_states[idle_uf_index].fk]->type;

  

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
  // todo -> enviar para execução se possível ao inves de sempre enviar
  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state == READ_OPERANDS){
      g_score_board.instructions_states[i].current_state = EXECUTE;
      g_score_board.instructions_states[i].start_execute = g_current_cycle;
      g_score_board.instructions_states[i].finish_execute = g_current_cycle;
    }
  }

  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state == EXECUTE){
      int cycles_to_complete;
      int inst = get_instruction_from_memory(i, g_memory);
      // printf("inst: %d", inst);
      UF_TYPE inst_uf_type = get_uf_type_from_instruction(inst);
      if (inst_uf_type == ADD_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_add;
      else if (inst_uf_type == MUL_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_mul;
      else if (inst_uf_type == INTEGER_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_integer;
      
      if (g_score_board.instructions_states[i].start_execute < cycles_to_complete)
        g_score_board.instructions_states[i].start_execute++;
    }
  }
}

void write_result(){
  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state == EXECUTE){
      int cycles_to_complete;
      int inst = get_instruction_from_memory(i, g_memory);
      // printf("inst: %d", inst);
      UF_TYPE inst_uf_type = get_uf_type_from_instruction(inst);
      if (inst_uf_type == ADD_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_add;
      else if (inst_uf_type == MUL_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_mul;
      else if (inst_uf_type == INTEGER_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_integer;
      
      int start = g_score_board.instructions_states[i].start_execute;
      int finish = g_score_board.instructions_states[i].finish_execute;
      if (finish - start + 1 == cycles_to_complete){
        g_score_board.instructions_states[i].current_state = WRITE_RESULT;
        g_score_board.instructions_states[i].write_result = g_current_cycle;

        g_score_board.ufs_states[i].fi = -1;
        g_score_board.ufs_states[i].fj = -1;
        g_score_board.ufs_states[i].fk = -1;
        g_score_board.ufs_states[i].qj = -1;
        g_score_board.ufs_states[i].qk = -1;
        g_score_board.ufs_states[i].inst_program_counter = -1;
        g_score_board.ufs_states[i].busy = false;
        g_score_board.ufs_states[i].rj = false;
        g_score_board.ufs_states[i].rk = false;
      }
      else{
        g_score_board.instructions_states[i].finish_execute++;
      }

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
  if (g_score_board.can_fetch) fetch_next_instruction();
}

void run_simulation(FILE *output){
  bool has_active_instruction = true;
  while (has_active_instruction){
    getchar();
    run_one_cycle(output);
  }
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

      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  free_memory(input_file, output_stream, &g_bus, &g_score_board, &g_functional_units);

  return 0;

}
