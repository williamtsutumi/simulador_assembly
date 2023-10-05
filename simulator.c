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
#include "headers/bus.h"
#include "headers/scoreboard.h"
#include "headers/table.h"

CPU_Configurations g_cpu_configs;
FunctionalUnit *g_functional_units;
ScoreBoard g_score_board;
Bus g_bus;

Byte *g_memory;
int g_memory_size = 200;

int g_registers[NUM_REGISTERS];
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = PROGRAM_FIRST_ADDRESS; // PC
InstructionRegister g_instruction_register; // IR

/* Execução nas unidades funcionais */

void fetch_next_instruction(){
  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state != FETCH) continue;

    int curr_inst_index = (g_instruction_register.program_counter - PROGRAM_FIRST_ADDRESS) / 4;

    if (i == curr_inst_index) continue; // A instrução já foi fetchada
    
    int inst_index = i;
    g_instruction_register.binary = get_instruction_from_memory(inst_index, g_memory);
    g_instruction_register.program_counter = g_program_counter;


    g_program_counter += 4;

    break;
  }
}
void issue_instruction(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_ISSUE) continue;

    add_pulse(&g_bus,
    new_data_pulse(g_instruction_register.binary, &(g_functional_units[uf_index].instruction_binary), sizeof(InstructionBinary)));

  }
}
void read_operands(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_READ_OPERAND) continue;

    InstructionBinary binary = g_functional_units[uf_index].instruction_binary;
    int opcode = get_opcode_from_binary(binary);

    InstructionFormat format = get_inst_format_from_opcode(opcode);

    int operand1_index, operand2_index, operand2, imm;
    if (is_branch(opcode)){
      if (format == FORMAT_I){
        operand1_index = get_rs_from_instruction_binary(binary);
        operand2_index = get_rt_from_instruction_binary(binary);

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand2_index]), &(g_functional_units[uf_index].operand2), sizeof(int)));
      }
      else if (format == FORMAT_J){
        imm = get_imm_from_instruction_binary(binary);

        add_pulse(&g_bus, 
        new_data_pulse(imm, &(g_functional_units[uf_index].operand1), sizeof(int)));
      }
    }
    else if (opcode == SW_OPCODE){
      operand1_index = get_rt_from_instruction_binary(binary);
      operand2_index = get_rs_from_instruction_binary(binary);
      
      add_pulse(&g_bus, 
      new_pulse(&g_registers[operand1_index], &(g_functional_units[uf_index].operand1), sizeof(int)));

      add_pulse(&g_bus, 
      new_pulse(&g_registers[operand2_index], &(g_functional_units[uf_index].operand2), sizeof(int)));
    }
    else if (opcode == LW_OPCODE){
      operand1_index = get_rs_from_instruction_binary(binary);
      operand2 = get_imm_from_instruction_binary(binary);

      add_pulse(&g_bus, 
      new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));
      
      add_pulse(&g_bus, 
      new_data_pulse(operand2, &(g_functional_units[uf_index].operand2), sizeof(int)));
    }
    else{
      if (format == FORMAT_R){
        operand1_index = get_rs_from_instruction_binary(binary);
        operand2_index = get_rt_from_instruction_binary(binary);

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));
        
        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand2_index]), &(g_functional_units[uf_index].operand2), sizeof(int)));
      }
      else if (format == FORMAT_I){
        operand1_index = get_rs_from_instruction_binary(binary);
        operand2 = get_imm_from_instruction_binary(binary);

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));
        
        add_pulse(&g_bus, 
        new_data_pulse(operand2, &(g_functional_units[uf_index].operand2), sizeof(int)));
      }
    }
  }
}
void execute(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int i = 0; i < total_ufs; i++){
    if (g_functional_units[i].status != CONTINUE_EXECUTE) continue;

    g_functional_units[i].current_cycle++;
  
    if (uf_finished_executing(&g_functional_units[i], g_cpu_configs)){
      g_functional_units[i].current_cycle = 0;

      InstructionBinary binary = g_functional_units[i].instruction_binary;
      int opcode = get_opcode_from_binary(binary);
      int operand1 = g_functional_units[i].operand1;
      int operand2 = g_functional_units[i].operand2;
      int result = actually_execute(opcode, operand1, operand2);

      if (opcode == LW_OPCODE){
        g_functional_units[i].operation_result = get_data_from_memory(operand1 + operand2, g_memory);
        /*
        add_pulse(&g_bus, 
        new_data_pulse(&g_memory[operand1+operand2], &(g_functional_units[i].operation_result)+sizeof(Byte)*0, sizeof(Byte)));

        add_pulse(&g_bus, 
        new_data_pulse(&g_memory[operand1+operand2+1], &(g_functional_units[i].operation_result)+sizeof(Byte)*1, sizeof(Byte)));

        add_pulse(&g_bus, 
        new_data_pulse(&g_memory[operand1+operand2+2], &(g_functional_units[i].operation_result)+sizeof(Byte)*2, sizeof(Byte)));

        add_pulse(&g_bus, 
        new_data_pulse(&g_memory[operand1+operand2+3], &(g_functional_units[i].operation_result)+sizeof(Byte)*3, sizeof(Byte)));
        */
        
      }
      else if (is_branch(opcode)){
        if (result){ // Se o desvio é tomado
          // Desvio com PC fixo
          if (opcode == J_OPCODE) g_functional_units[i].operation_result = operand1;
          // Desvio com PC calculado
          else g_functional_units[i].operation_result = g_program_counter + 4*get_imm_from_instruction_binary(binary);
        }
        else{
          // Atribuindo 0 pois, para essa simulação, 0 não é um valor válido para o PC
          // Então, representa não atualizar o PC
          g_functional_units[i].operation_result = 0;
        }
      }
      // Caso geral: add, addi, sub, subi, mul, div, and, or, not
      else{
        g_functional_units[i].operation_result = result;
      }

    }
  }
}
void write_result(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_WRITE_RESULT) continue;

    g_functional_units[uf_index].status = STALL; // Aqui stall indica que não está em uso

    InstructionBinary binary = g_functional_units[uf_index].instruction_binary;
    int opcode = get_opcode_from_binary(binary);
    if (opcode == SW_OPCODE){
      int mem_address = 4*(g_functional_units[uf_index].operand2 + get_imm_from_instruction_binary(binary));

      int op_result = g_functional_units[uf_index].operation_result;
      int byte0 = (op_result >> 24) & 0b11111111;
      int byte1 = (op_result >> 16) & 0b11111111;
      int byte2 = (op_result >> 8) & 0b11111111;
      int byte3 = (op_result >> 0) & 0b11111111;

      add_pulse(&g_bus, 
      new_data_pulse(byte0, &g_memory[mem_address + 0], sizeof(Byte)));

      add_pulse(&g_bus, 
      new_data_pulse(byte1, &g_memory[mem_address + 1], sizeof(Byte)));

      add_pulse(&g_bus, 
      new_data_pulse(byte2, &g_memory[mem_address + 2], sizeof(Byte)));

      add_pulse(&g_bus,
      new_data_pulse(byte3, &g_memory[mem_address + 3], sizeof(Byte)));
    }
    else if (is_branch(opcode)){
      if (g_functional_units[uf_index].operation_result != 0){
        g_program_counter = g_functional_units[uf_index].operation_result;
        // add_pulse(&g_bus, 
        // new_pulse(&g_functional_units[uf_index].operation_result, &g_program_counter, sizeof(int)));
      }
    }
    else{
      int result = get_destination_register_from_instruction(binary);

      add_pulse(&g_bus, 
      new_pulse(&g_functional_units[uf_index].operation_result, &g_registers[result], sizeof(int)));
    }
  }
}


/************************************/



void run_one_cycle(FILE *output){

  g_current_cycle++;
  

  write_result();
  execute();
  read_operands();
  issue_instruction();
  fetch_next_instruction();

  
  update_scoreboard(&g_score_board,
                    &g_bus,
                    g_memory, 
                    g_functional_units, 
                    g_cpu_configs, 
                    g_current_cycle, 
                    g_instruction_count, 
                    g_instruction_register, 
                    g_program_counter);

  dispatch_pulses(&g_bus);


  // Imprimindo tabelas, não fiel à simulação
  Byte inst_opcodes[g_instruction_count];
  for (int i = 0; i < g_instruction_count; i++){
    int inst = get_instruction_from_memory(i, g_memory);
    int opcode = get_opcode_from_binary(inst);
    inst_opcodes[i] = opcode;
  }
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  print_table(&g_score_board, g_current_cycle, inst_opcodes, g_instruction_count, total_ufs, g_registers, output);
  // ****************************************

}

bool program_has_exited(){
  int output = true;

  int count_active_instructions=0;
  for (int i = 0; i < g_instruction_count; i++){ 
    printf("estado da instrução %d = %d\n", i, g_score_board.instructions_states[i].current_state);
    if (g_score_board.instructions_states[i].current_state != FINISHED && g_score_board.instructions_states[i].current_state != AWAIT){
      count_active_instructions++;
      //break;
    }
  }
  int g_total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_integer_ufs + g_cpu_configs.size_mul_ufs;
  for(int i = 0; i < g_total_ufs; i++){
    if(get_opcode_from_binary(g_instruction_register.binary) != EXIT_OPCODE){
      output = false;
    }
  }
  if(count_active_instructions != 1){
    output = false;
  }


  return output;
}

void run_simulation(FILE *output){
  while (!program_has_exited()){
    getchar();
    run_one_cycle(output);

    for (int i=0; i<6; i++) printf("M[%d]: %d\n", i, get_data_from_memory(i, g_memory));
  }
  printf("Program Exited.\n");

}



// Main e leitura da entrada

bool read_args(int argc, char *argv[], char **input_file_name, FILE **input_file, FILE **output_stream){
  for(int i = 1; i < argc; i+=2){

    if(strcmp(argv[i], "-p") == 0){
      *input_file_name = argv[i+1];
    }
    else if(strcmp(argv[i], "-m") == 0){
      // Validando se é um número
      if (strspn(argv[i+1], "0123456789") != strlen(argv[i+1])) return false;

      g_memory_size = atoi(argv[i+1]);
    }
    else if(strcmp(argv[i], "-o") == 0){
      *output_stream = fopen(argv[i+1], "w");
    }
  }

  *input_file = fopen(*input_file_name, "r");
  return *input_file != NULL;
}

int main(int argc, char *argv[])
{
  memset(g_registers, 0, sizeof(g_registers));

  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &input_file_name, &input_file, &output_stream)){
    g_memory = (Byte*)(malloc(sizeof(Byte)* (PROGRAM_FIRST_ADDRESS + g_memory_size*4)));

    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);

    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instruction_count, g_memory, g_memory_size)){
      malloc_memory(&g_functional_units, &g_score_board, g_cpu_configs, g_instruction_count, g_memory_size);
      init_scoreboard(&g_score_board);
      init_functional_units(g_functional_units, g_cpu_configs);

      printf("printando instructions binaries\n");
      for (int i=0; i<g_instruction_count; i++){
        printf("instruction[%u]: %d, opcode: %d\n", i, get_instruction_from_memory(i, g_memory), get_opcode_from_binary(get_instruction_from_memory(i, g_memory)));
      }

      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura da linha de comando.\n");
  }

  // free_memory(input_file, output_stream, &g_score_board, &g_functional_units);

  return 0;

}
